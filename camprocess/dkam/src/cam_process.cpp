#include "dkam_zhicamera_api.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <thread>
#include <unistd.h>

#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 1024

using namespace std;

int main(int argc, char *argv[])
{
    int k = -1;
    //打印SDK日志 error, debug,warnning, info 1是打开 0是关闭
    SetLogLevel(1, 0, 0, 1);
    int camer_num = DiscoverCamera();
    cout << "camer_num=" << camer_num << endl;
    //对局域网内的相机进行排序0：IP 1:series number
    int camer_sort = CameraSort(1);
    cout << "camer_sort=" << camer_sort << endl;
    //创建相机参数

    //显示局域网内相机的IP
    for (int i = 0; i < camer_num; i++)
    {
        cout << "IP为:" << CameraIP(i) << endl;
        if (strcmp(CameraIP(i), "192.168.1.103") == 0)
        {
            k = i;
        }
    }
    Camera_Object_C *camera_obj1 = CreateCamera(k);
    //连接相机
    int connect = CameraConnect(camera_obj1);
    cout << "connect=" << connect << endl;

    if (connect == 0)
    {
        //获取当前红外相机的宽和高
        int width = -1;
        int height = -1;
        int height_gray = GetCameraHeight(camera_obj1, &height, 0);
        int width_gray = GetCameraWidth(camera_obj1, &width, 0);
        cout << "camera  Grey width:" << width << "---Grey height:" << height << endl;

        //获取当前RGB相机的宽和高
        int width_RGB = -1;
        int height_RGB = -1;
        int width_rgb = GetCameraWidth(camera_obj1, &width_RGB, 1);
        int height_rgb = GetCameraHeight(camera_obj1, &height_RGB, 1);
        cout << "camera RGB width:" << width_RGB << "-----RGB height:" << height_RGB << endl;

        //定义点云数据大小
        PhotoInfo *point_data = new PhotoInfo;
        point_data->pixel = new char[width * height * 6];
        memset(point_data->pixel, 0, width * height * 6);

        //定义红外数据大小
        PhotoInfo *gray_data = new PhotoInfo;
        gray_data->pixel = new char[width * height];
        memset(gray_data->pixel, 0, width * height);

        //定义RGB数据大小
        PhotoInfo *RGB_data = new PhotoInfo;
        RGB_data->pixel = new char[width_RGB * height_RGB * 3];
        memset(RGB_data->pixel, 0, width_RGB * height_RGB * 3);
        //定义xyz数据大小
        PhotoInfo *xyz_data = new PhotoInfo;
        xyz_data->pixel = new char[width_RGB * height_RGB * 6];
        memset(xyz_data->pixel, 0, width_RGB * height_RGB * 6);
        //定义xyz数据大小
        float *dddd = (float *) malloc(width * height * sizeof(float) * 6);
        //定义点云RGB融合的数据大小
        float *rgb_cloud = new float[width * height * 6 * sizeof(float)];
        //定义点云红外融合的数据大小
        float *gray_cloud = new float[width * height * 6 * sizeof(float)];

        //设置相机RGB触发模式 0 连拍模式  1 触发模式
        int tirggerModergb = SetRGBTriggerMode(camera_obj1, 0);
        cout << "tirggerModeRGB:" << tirggerModergb << endl;
        //设置相机红外触发模式 0 连拍模式  1 触发模式
        int tirggerMode = SetTriggerMode(camera_obj1, 0);
        cout << "tirggerMode:" << tirggerMode << endl;
        //开启数据流通道(0:红外 1:点云 2:RGB)
        int stream_gray = StreamOn(camera_obj1, 0);
        cout << "stream_gray=" << stream_gray << endl;

        int stream_point = StreamOn(camera_obj1, 1);
        cout << "stream_point=" << stream_point << endl;

        int stream_RGB = StreamOn(camera_obj1, 2);
        cout << "stream_RGB=" << stream_RGB << endl;
        //开始接受数据
        int acquistion = AcquisitionStart(camera_obj1);
        cout << "acquistion=" << acquistion << endl;

        //使用zhisensor自带的补点，参数2是补点，参数0是默认
        SetPointCloudPostProcessMode(camera_obj1,2);
        int ss = 1;

        auto shkey = ftok("./camprocess/tmp1", 1001);
        int shmid = shmget(shkey, 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + 4 + 1280 * (2 * 2 + 3 * 4) + 200, 0666 | IPC_CREAT);
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

        while (1)
        {
            cout << "**** caputre " << ss++ << " number***" << endl;
            cout << "连接相机的IP为:" << CameraIP(k) << endl;
            cout << "camera 1 version" << CameraVerions(camera_obj1) << endl;
            //刷新缓冲区数据
            FlushBuffer(camera_obj1, 0);
            FlushBuffer(camera_obj1, 1);
            FlushBuffer(camera_obj1, 2);

            //采集点云
            int capturePoint = -1;
            capturePoint = TimeoutCapture(camera_obj1, 1, point_data, 3000000);
            cout << "capture_Pointimage: " << capturePoint << endl;
            //采集红外
            int captureGray = -1;
            captureGray = TimeoutCapture(camera_obj1, 0, gray_data, 3000000);
            cout << "capture_Grayimage: " << captureGray << endl;
            //采集RGB
            int captureRGB = -1;
            captureRGB = TimeoutCapture(camera_obj1, 2, RGB_data, 3000000);
            cout << "capture_RGBimage: " << captureRGB << endl;

            if(*shmempt == 2 || *shmempt == 3)
            {
                FusionImageTo3D(camera_obj1, RGB_data, point_data, dddd);

                for(int pix_y = 0; pix_y < IMAGE_HEIGHT; pix_y++)
                    for(int pix_x = 0; pix_x < IMAGE_WIDTH; pix_x++)
                    {
                        uint8_t b = dddd[(pix_y * IMAGE_WIDTH + pix_x) * 6 + 3];
                        uint8_t g = dddd[(pix_y * IMAGE_WIDTH + pix_x) * 6 + 4];
                        uint8_t r = dddd[(pix_y * IMAGE_WIDTH + pix_x) * 6 + 5];
                        memcpy(shmempt + 1 + (pix_y * IMAGE_WIDTH + pix_x) * 3, &r, sizeof(uint8_t));
                        memcpy(shmempt + 1 + (pix_y * IMAGE_WIDTH + pix_x) * 3 + 1, &g, sizeof(uint8_t));
                        memcpy(shmempt + 1 + (pix_y * IMAGE_WIDTH + pix_x) * 3 + 2, &b, sizeof(uint8_t));
                    }
              
                if(*shmempt == 3)
                {
                    uint32_t kpnum;
                    memcpy(&kpnum, shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3, sizeof(uint32_t));
                    for(int i = 0; i < kpnum; i++)
                    {
                        uint16_t pix_x;
                        uint16_t pix_y;
                        memcpy(&pix_x, shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + 4 + 2 * i * sizeof(uint16_t), sizeof(uint16_t));
                        memcpy(&pix_y, shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + 4 + (2 * i + 1) * sizeof(uint16_t), sizeof(uint16_t));
                        auto x = dddd[(pix_y * IMAGE_WIDTH + pix_x) * 6];
                        auto y = dddd[(pix_y * IMAGE_WIDTH + pix_x) * 6 + 1];
                        auto z = dddd[(pix_y * IMAGE_WIDTH + pix_x) * 6 + 2];
                        memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + sizeof(uint32_t) + 1280 * 4 + 3 * i * sizeof(float), &x, sizeof(float));
                        memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + sizeof(uint32_t) + 1280 * 4 + (3 * i + 1) * sizeof(float), &y, sizeof(float));
                        memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + sizeof(uint32_t) + 1280 * 4 + (3 * i + 2) * sizeof(float), &z, sizeof(float));
                    }
                    *shmempt = 1;
                }
                else
                    *shmempt = 0;
            }
            memset(point_data->pixel, 0, width * height * 6);
            memset(gray_data->pixel, 0, width * height);
            memset(RGB_data->pixel, 0, width_RGB * height_RGB * 3);
            memset(rgb_cloud, 0, width * height * 6);
            memset(gray_cloud, 0, width * height * 6);
        }
        //释放内存
        delete[] point_data->pixel;
        delete point_data;
        delete[] gray_data->pixel;
        delete gray_data;
        delete[] RGB_data->pixel;
        delete RGB_data;
        delete[] rgb_cloud;
        delete[] gray_cloud;
        //关闭数据流通道
        int streamoff_gray = StreamOff(camera_obj1, 0);
        cout << "streamoff_gray=" << streamoff_gray << endl;
        int streamoff_point = StreamOff(camera_obj1, 1);
        cout << "streamoff_point=" << streamoff_point << endl;
        int streamoff_rgb = StreamOff(camera_obj1, 2);
        cout << "streamoff_rgb=" << streamoff_rgb << endl;
        //断开相机连接
        int disconnect = CameraDisconnect(camera_obj1);
        cout << "disconnect=" << disconnect << endl;
        DestroyCamera(camera_obj1);
    }
    return 0;
}