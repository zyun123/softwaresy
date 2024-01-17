#include <fstream>
#include <iostream>
#include <fstream>
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

#include "libobsensor/ObSensor.hpp"
#include "libobsensor/hpp/Error.hpp"

#include <libobsensor/h/StreamProfile.h>

#include <thread>
#include <mutex>
#include <iostream>
#include <sstream>
#include <string>

#define TIMEOUT_IN_MS 1000
#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

using namespace std;

std::shared_ptr<ob::FrameSet> currentFrameSet[4];
std::mutex                    frameSetMutex[4];

void thread_getimage(string ip, string filepath, int projid)
{
    cout << ip << " " << filepath << " " << projid << endl;
    try{
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

        ob::Context ctx;
        auto deviceList = ctx.queryDeviceList();
        auto device = deviceList->getDeviceBySN(ip.data());  // using serial number to create device
        auto pipe = std::make_shared<ob::Pipeline>(device);

        std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();
        auto colorProfileList = pipe->getStreamProfileList(OB_SENSOR_COLOR);
        auto colorProfile = colorProfileList->getVideoStreamProfile(1280, 720, OB_FORMAT_RGB888, 15);
        config->enableStream(colorProfile);

        OBAlignMode alignMode = ALIGN_DISABLE;
        auto depthProfileList = pipe->getD2CDepthProfileList(colorProfile, ALIGN_D2C_HW_MODE);
        if(depthProfileList->count() > 0) {
            alignMode = ALIGN_D2C_HW_MODE;
        }
        else {
            depthProfileList = pipe->getD2CDepthProfileList(colorProfile, ALIGN_D2C_SW_MODE);
            if(depthProfileList->count() > 0) {
                alignMode = ALIGN_D2C_SW_MODE;
            }
        }
        auto depthProfile = depthProfileList->getVideoStreamProfile(1024, 1024, OB_FORMAT_Y16, 15);
        // auto depthProfile = depthProfileList->getVideoStreamProfile(640, 576, OB_FORMAT_Y16, 15);
        config->enableStream(depthProfile);
        config->setAlignMode(alignMode);

        // pipe->start(config);
        pipe->start(config, [&](std::shared_ptr<ob::FrameSet> frameSet) {
            std::lock_guard<std::mutex> lock(frameSetMutex[projid - 1001]);
            currentFrameSet[projid - 1001] = frameSet;
        });

        auto paras = pipe->getCameraParam();
        float fx = paras.depthIntrinsic.fx;
        float fy = paras.depthIntrinsic.fy;
        float cx = paras.depthIntrinsic.cx;
        float cy = paras.depthIntrinsic.cy;

        memcpy(shmempt + 1, &fx, sizeof(float));
        memcpy(shmempt + 1 + sizeof(float), &fy, sizeof(float));
        memcpy(shmempt + 1 + sizeof(float) * 2, &cx, sizeof(float));
        memcpy(shmempt + 1 + sizeof(float) * 3, &cy, sizeof(float));
        *shmempt = 3;

        while (1)
        {
            // auto frameSet = pipe->waitForFrames(200);
            std::shared_ptr<ob::FrameSet> frameSet;
            {
                std::lock_guard<std::mutex> lock(frameSetMutex[projid - 1001]);
                frameSet = currentFrameSet[projid - 1001];
            }

            if(frameSet)
            {
                auto depth_frame = frameSet->depthFrame();
                auto color_frame = frameSet->colorFrame();
                if(color_frame && depth_frame)
                {
                    if(*shmempt == 1)
                    {
                        char *cbuf = (char*)(color_frame->data());
                        cv::Mat imgdata_RGB(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, cbuf);
                        cv::Mat imgdata_BGR;
                        cv::cvtColor(imgdata_RGB, imgdata_BGR, cv::COLOR_RGB2BGR);
                        memcpy(shmempt + 1, imgdata_BGR.data, IMAGE_WIDTH * IMAGE_HEIGHT * 3);

                        char *dbuf = (char*)(depth_frame->data());
                        memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3, dbuf, IMAGE_WIDTH * IMAGE_HEIGHT * 2);

                        *shmempt = 2;
                    }
                }
            }
        }
    }
    catch(ob::Error &e)
    {
        std::cerr << "function:" << e.getName() << "\nargs:" << e.getArgs() << "\nmessage:" << e.getMessage() << "\ntype:" << e.getExceptionType() << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    ifstream ifile;
    ifile.open("./system.ini", ios::in);
    if (!ifile)
    {
        cout << "no system.ini found\n";
        return -1;
    }
    bool find = false;
    vector<string> cam_ips;
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
                    cam_ips.push_back(serials.substr(0, subpos));
                    serials = serials.substr(subpos + 1, serials.length() - subpos - 1);
                    subpos = serials.find(".");
                }
                cam_ips.push_back(serials);
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
    cout << "cams num: " << cam_ips.size() << endl;
    for (int i = 0; i < cam_ips.size(); i++)
    {
        thread *t = new thread(thread_getimage, cam_ips[i], string("./camprocess/tmp") + (char) ('1' + i), 1001 + i);
        camthreads.push_back(t);
    }
    for (int i = 0; i < camthreads.size(); i++) 
        camthreads[i]->join();

    return 0;
}