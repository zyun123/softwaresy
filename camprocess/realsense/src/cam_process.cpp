#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <memory.h>

#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

using namespace std;

void getCamsNumAndSerial(int &num, vector<string> &serial)
{
    num = 0;
    serial.clear();

    rs2_error* e = 0;
    rs2_context* ctx = rs2_create_context(RS2_API_VERSION, &e);
    rs2_device_list* device_list = rs2_query_devices(ctx, &e);
    num = rs2_get_device_count(device_list, &e);

    for(int i = 0; i < num; i++)
    {
        rs2_device* dev = rs2_create_device(device_list, i, &e);
        serial.push_back(rs2_get_device_info(dev, RS2_CAMERA_INFO_SERIAL_NUMBER, &e));
        rs2_delete_device(dev);
    }

    rs2_delete_device_list(device_list);
    rs2_delete_context(ctx);
}

bool checkCamerasConnected(int cfg_num, vector<string> &cfg_list)
{
    int camnum = 0;
    vector<string> reallist;
    getCamsNumAndSerial(camnum, reallist);
    if (camnum < cfg_num)
        return false;
    for (int i = 0; i < cfg_list.size(); i++)
    {
        bool find = false;
        for (int j = 0; j < reallist.size(); j++)
        {
            if (reallist[j] == cfg_list[i])
            {
                find = true;
                break;
            }
        }
        if (!find)
            return false;
    }
    return true;
}

void thread_getimage(string serial, string filepath, int projid)
{
      cout << serial << " " << filepath << " " << projid << endl;
      rs2::pipeline pipe;
      rs2::config rsconfig;
      rsconfig.enable_device(serial);
      rsconfig.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 15);
      rsconfig.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_BGR8, 15);
      auto profile = pipe.start(rsconfig);

      // roi of exposure set ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      rs2::roi_sensor color_sensor = profile.get_device().query_sensors().at(1).as<rs2::roi_sensor>();
      rs2::roi_sensor depth_sensor = profile.get_device().query_sensors().at(0).as<rs2::roi_sensor>();
      color_sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);
      depth_sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1); // default is enable ,but do it again before set roi of exposure,and set roi more times until is sucessed,#8004,
      // Getting ROI works fine.
      color_sensor.set_option(RS2_OPTION_POWER_LINE_FREQUENCY, 1.f); // { { 0.f, "Disabled"}  { 1.f, "50Hz" }, { 2.f, "60Hz" }, { 3.f, "Auto" }, }));
      // color_sensor.set_option(RS2_OPTION_GAIN, 32);//do not set it when auto exposure enabled
      color_sensor.set_option(RS2_OPTION_BACKLIGHT_COMPENSATION, 1);
      rs2::region_of_interest colorRoi{100, 200, 1200, 500}, depthRoi{100, 200, 1200, 500}; // min_x,min_y,max_x,max_y

      std::ostringstream ss;
      // depth sensor++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      for (int i = 0; i < 8; i++)
      {

            try
            {
                  // Step 2: send it to firmware
                  if (depth_sensor.is<rs2::roi_sensor>())
                  {
                        depth_sensor.as<rs2::roi_sensor>().set_region_of_interest(depthRoi);
                        break;
                  }
            }
            catch (const rs2::error &e)
            {
                  ss.str("");
                  ss << rs2_exception_type_to_string(e.get_type())
                     << " in " << e.get_failed_function() << "("
                     << e.get_failed_args() << "):\n"
                     << e.what();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      try
      {
            // Step 2: send it to firmware
            if (depth_sensor.is<rs2::roi_sensor>())
            {
                  depthRoi = depth_sensor.as<rs2::roi_sensor>().get_region_of_interest();
            }
      }
      catch (const rs2::error &e)
      {
            ss.str("");
            ss << rs2_exception_type_to_string(e.get_type())
               << " in " << e.get_failed_function() << "("
               << e.get_failed_args() << "):\n"
               << e.what();
      }

      // depth sensor------------------------------------------------------

      // color sensor +++++++++++++++++++++++++++++++++++++++++++++++++++++
      for (int i = 0; i < 8; i++)
      {

            try
            {
                  // Step 2: send it to firmware
                  if (color_sensor.is<rs2::roi_sensor>())
                  {
                        color_sensor.as<rs2::roi_sensor>().set_region_of_interest(colorRoi);
                        break;
                  }
            }
            catch (const rs2::error &e)
            {
                  ss.str("");
                  ss << rs2_exception_type_to_string(e.get_type())
                     << " in " << e.get_failed_function() << "("
                     << e.get_failed_args() << "):\n"
                     << e.what();
                  // qDebug()<<"loop"<<i<<" : "<<QString::fromStdString (ss.str()) ;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      try
      {
            // Step 2: send it to firmware
            if (color_sensor.is<rs2::roi_sensor>())
            {
                  colorRoi = color_sensor.as<rs2::roi_sensor>().get_region_of_interest();
            }
      }
      catch (const rs2::error &e)
      {
            ss.str("");
            ss << rs2_exception_type_to_string(e.get_type())
               << " in " << e.get_failed_function() << "("
               << e.get_failed_args() << "):\n"
               << e.what();
      }

      // color sensor--------------------------------------------------------------------------
      // roi of exposure set----------------------------------------------------------------

      auto intrinsics = profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>().get_intrinsics();
      cout << "intrinsics: fx " << intrinsics.fx << " fy " << intrinsics.fy << " ppx " << intrinsics.ppx << " ppy " << intrinsics.ppy << std::endl;

      auto shkey = ftok(filepath.data(), projid);
      int shmid = shmget(shkey, 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + 4 + 1280 * (2 * 2 + 3 * 4) + 200, 0666 | IPC_CREAT);
      if (shmid == -1)
      {
            fprintf(stderr, "shmget failed: %d\n", errno);
            exit(-1);
      }
      auto shmem = shmat(shmid, NULL, 0);
      if (shmem == (void *)-1)
      {
            fprintf(stderr, "shmat failed\n");
            exit(-1);
      }
      unsigned char *shmempt = (unsigned char *)shmem;

      memcpy(shmempt + 1, &(intrinsics.fx), sizeof(float));
      memcpy(shmempt + 1 + sizeof(float), &(intrinsics.fy), sizeof(float));
      memcpy(shmempt + 1 + sizeof(float) * 2, &(intrinsics.ppx), sizeof(float));
      memcpy(shmempt + 1 + sizeof(float) * 3, &(intrinsics.ppy), sizeof(float));
      *shmempt = 5;

      rs2::temporal_filter temp_filter;
      rs2::disparity_transform depth_to_disparity(true);
      rs2::disparity_transform disparity_to_depth(false);
      rs2::align align_to_color(RS2_STREAM_COLOR);

      uint8_t *last_frame_d = (uint8_t*)malloc(IMAGE_HEIGHT * IMAGE_WIDTH * 2);
      memset(last_frame_d, 0, IMAGE_HEIGHT * IMAGE_WIDTH * 2);
      while (1)
      {
            rs2::frameset frameset = pipe.wait_for_frames();
            if(*shmempt == 2 || *shmempt == 3)
            {
                  frameset = align_to_color.process(frameset);
                  auto color = frameset.get_color_frame();
                  memcpy(shmempt + 1, color.get_data(), color.get_data_size());
                  auto depth = frameset.get_depth_frame();
                  depth = depth_to_disparity.process(depth);
                  depth = temp_filter.process(depth);
                  depth = disparity_to_depth.process(depth);
                  if(memcmp(last_frame_d, (uint8_t*)(depth.get_data()), IMAGE_HEIGHT * IMAGE_WIDTH * 2) == 0)
                  {
                        cout << "depth repeat\n";
                        continue;
                  }
                  else
                        memcpy(last_frame_d, (uint8_t*)(depth.get_data()), IMAGE_HEIGHT * IMAGE_WIDTH * 2);

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
                              auto d = depth.get_distance(pix_x, pix_y);
                              if(d == 0)
                              {
                                    for(uint16_t x = pix_x - 2; x < pix_x + 3; x++)
                                    {
                                          for(uint16_t y = pix_y - 2; y < pix_y + 3; y++)
                                          {
                                                if(x < 0 || x > IMAGE_WIDTH || y < 0 || y > IMAGE_HEIGHT)
                                                      continue;
                                                d = depth.get_distance(x, y);
                                                if(d != 0)
                                                {
                                                      pix_x = x;
                                                      pix_y = y;
                                                      break;
                                                }
                                          }
                                          if(d != 0)
                                                break;
                                    }
                              }
                              float xyz[3];
                              float pixs[2];
                              pixs[0] = pix_x;
                              pixs[1] = pix_y;
                              rs2_deproject_pixel_to_point(xyz, &intrinsics, pixs, d);
                              xyz[0] *= 1000;
                              xyz[1] *= 1000;
                              xyz[2] *= 1000;
                              // cout << xyz[0] << " " << xyz[1] << " " << xyz[2] << endl;
                              memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + sizeof(uint32_t) + 1280 * 4 + 3 * i * sizeof(float), xyz, sizeof(float));
                              memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + sizeof(uint32_t) + 1280 * 4 + (3 * i + 1) * sizeof(float), xyz+1, sizeof(float));
                              memcpy(shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + sizeof(uint32_t) + 1280 * 4 + (3 * i + 2) * sizeof(float), xyz+2, sizeof(float));
                        }
                        //保存png深度图
                        char tmp[200];
                        memcpy(tmp, shmempt + 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + 4 + 1280 * (2 * 2 + 3 * 4), 200);
                        string dfile = tmp + string(".png");
                        auto dmat = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_16UC1, (uint8_t*)(depth.get_data()));
                        cv::imwrite(dfile, dmat);
                        *shmempt = 1;
                  }
                  else
                        *shmempt = 0;
            }
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
      vector<string> serialnums;
      vector<thread*> camthreads;
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
      if(!checkCamerasConnected(serialnums.size(), serialnums))
            return 0;
      for (int i = 0; i < serialnums.size(); i++)
      {
            thread *t = new thread(thread_getimage, serialnums[i], string("./camprocess/tmp") + (char)('1' + i), 1001 + i);
            camthreads.push_back(t);
      }
      for (int i = 0; i < camthreads.size(); i++)
            camthreads[i]->join();
      return 0;
}