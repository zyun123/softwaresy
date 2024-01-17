#include <fstream>
#include <iostream>
#include <fstream>
#include <k4a/k4a.h>
#include <map>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define TIMEOUT_IN_MS 1000
#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

using namespace std;

map<string, k4a_device_t> g_devices;

string get_serial(k4a_device_t device)
{
    size_t serial_number_length = 0;

    if (K4A_BUFFER_RESULT_TOO_SMALL != k4a_device_get_serialnum(device, NULL, &serial_number_length))
    {
        cout << "Failed to get serial number length" << endl;
        k4a_device_close(device);
        exit(-1);
    }

    char *serial_number = new (std::nothrow) char[serial_number_length];
    if (serial_number == NULL)
    {
        cout << "Failed to allocate memory for serial number (" << serial_number_length << " bytes)" << endl;
        k4a_device_close(device);
        exit(-1);
    }

    if (K4A_BUFFER_RESULT_SUCCEEDED != k4a_device_get_serialnum(device, serial_number, &serial_number_length))
    {
        cout << "Failed to get serial number" << endl;
        delete[] serial_number;
        serial_number = NULL;
        k4a_device_close(device);
        exit(-1);
    }

    string s(serial_number);
    delete[] serial_number;
    serial_number = NULL;
    return s;
}

void thread_getimage(string serial, string filepath, int projid)
{
    cout << serial << " " << filepath << " " << projid << endl;
    auto device = g_devices[serial];

    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    config.color_resolution = K4A_COLOR_RESOLUTION_720P;
    config.depth_mode = K4A_DEPTH_MODE_WFOV_UNBINNED;
    config.camera_fps = K4A_FRAMES_PER_SECOND_15;

    k4a_calibration_t calibration;
    if (K4A_RESULT_SUCCEEDED != k4a_device_get_calibration(device, config.depth_mode, config.color_resolution, &calibration))
    {
        printf("Failed to get calibration\n");
        return;
    }
    auto transformation = k4a_transformation_create(&calibration);
    if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
    {
        printf("Failed to start device\n");
        return;
    }

    auto shkey = ftok(filepath.data(), projid);
    int shmid = shmget(shkey, 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 5, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget failed: %d\n", errno);
        exit(-1);
    }
    auto shmem = shmat(shmid, NULL, 0);
    if (shmem == (void *) -1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(-1);
    }
    unsigned char *shmempt = (unsigned char *) shmem;

    float fx = calibration.color_camera_calibration.intrinsics.parameters.param.fx;
    float fy = calibration.color_camera_calibration.intrinsics.parameters.param.fy;
    float cx = calibration.color_camera_calibration.intrinsics.parameters.param.cx;
    float cy = calibration.color_camera_calibration.intrinsics.parameters.param.cy;

    memcpy(shmempt + 1, &fx, sizeof(float));
    memcpy(shmempt + 1 + sizeof(float), &fy, sizeof(float));
    memcpy(shmempt + 1 + sizeof(float) * 2, &cx, sizeof(float));
    memcpy(shmempt + 1 + sizeof(float) * 3, &cy, sizeof(float));
    *shmempt = 3;

    k4a_capture_t capture = NULL;
    while (1)
    {
        // Get a depth frame
        auto ret_cap = k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS);
        if (ret_cap == K4A_WAIT_RESULT_TIMEOUT)
        {
            printf("Timed out waiting for a capture\n");
            k4a_capture_release(capture);
            continue;
        }
        else if (ret_cap == K4A_WAIT_RESULT_FAILED)
        {
            printf("Failed to read a capture\n");
            break;
        }

        if(*shmempt == 1)
        {
            k4a_image_t colorimage;
            colorimage = k4a_capture_get_color_image(capture);
            if (colorimage == NULL)
            {
                printf("Color None\n");
                k4a_image_release(colorimage);
                k4a_capture_release(capture);
                continue;
            }
            k4a_image_t depthimage;
            depthimage = k4a_capture_get_depth_image(capture);
            if (depthimage == NULL)
            {
                printf("Depth16 None\n");
                k4a_image_release(depthimage);
                k4a_capture_release(capture);
                continue;
            }

            auto cbuf = k4a_image_get_buffer(colorimage);
            k4a_image_t transformed_depth_image = NULL;
            if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, IMAGE_WIDTH, IMAGE_HEIGHT,
                                                        IMAGE_WIDTH * (int) sizeof(uint16_t), &transformed_depth_image))
            {
                printf("Failed to create transformed depth image\n");
                break;
            }
            if (K4A_RESULT_SUCCEEDED != k4a_transformation_depth_image_to_color_camera(transformation, depthimage, transformed_depth_image))
            {
                printf("Failed to compute transformed depth image\n");
                break;
            }
            auto dbuf = k4a_image_get_buffer(transformed_depth_image);
            cv::Mat imgdata_BGRA(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC4, cbuf);
            cv::Mat imgdata_BGR;
            cv::cvtColor(imgdata_BGRA, imgdata_BGR, cv::COLOR_BGRA2BGR);
            memcpy(shmempt + 1, imgdata_BGR.data, IMAGE_WIDTH * IMAGE_HEIGHT * 3);
            memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3, dbuf, IMAGE_WIDTH * IMAGE_HEIGHT * 2);
            *shmempt = 2;
            k4a_image_release(colorimage);
            k4a_image_release(depthimage);
            k4a_image_release(transformed_depth_image);
        }
        k4a_capture_release(capture);
    }
    if (device != NULL)
    {
        k4a_device_close(device);
    }
}

int main(int argc, char *argv[])
{
    uint32_t device_count = k4a_device_get_installed_count();
    printf("detected: %d\n", device_count);
    if (device_count == 0)
    {
        printf("No K4A devices found\n");
        return 0;
    }
    char tmps[100];
    size_t serisize = 0;
    for (int i = 0; i < device_count; i++)
    {
        k4a_device_t device = NULL;
        if (K4A_RESULT_SUCCEEDED != k4a_device_open(i, &device))
        {
            printf("Failed to open device\n");
            return 0;
        }
        g_devices[get_serial(device)] = device;
    }

    ifstream ifile;
    ifile.open("./system.ini", ios::in);
    if (!ifile)
    {
        cout << "no system.ini found\n";
        return -1;
    }
    bool find = false;
    vector<string> serialnums;
    vector<thread *> camthreads;
    while (!ifile.eof())
    {
        string line;
        getline(ifile, line);
        if (!line.empty())
        {
            string key = "SerialNums=";
            auto pos = line.find(key);
            if (pos == 0)
            {
                auto serials = line.substr(key.length(), line.length() - key.length());
                auto subpos = serials.find(".");
                while (subpos != string::npos)
                {
                    serialnums.push_back(serials.substr(0, subpos));
                    serials = serials.substr(subpos + 1, serials.length() - subpos - 1);
                    subpos = serials.find(".");
                }
                serialnums.push_back(serials);
                find = true;
                break;
            }
        }
    }
    if (!find)
    {
        cout << "cams config error\n";
        return -1;
    }
    cout << "cams num: " << serialnums.size() << endl;
    for (int i = 0; i < serialnums.size(); i++)
    {
        if(g_devices.find(serialnums[i]) == g_devices.end())
            return -1;
    }
    for (int i = 0; i < serialnums.size(); i++)
    {
        thread *t = new thread(thread_getimage, serialnums[i], string("./camprocess/tmp") + (char) ('1' + i), 1001 + i);
        camthreads.push_back(t);
    }
    for (int i = 0; i < camthreads.size(); i++) 
        camthreads[i]->join();

    return 0;
}