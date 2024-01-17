#ifndef PUBDEF_H
#define PUBDEF_H

#include <QString>
#include <QMetaType>
#include <QDebug>

#define DEFAULT_CURE_DAMP 0.005 //调理默认阻尼大小
#define DEFAULT_CURE_GAIN 1 //调理默认增益大小
#define DEFAULT_CURE_SPEED 0.018 //调理默认机械臂移速
#define DEFAULT_CURE_ACC 0.05 //调理默认机械臂加减速

#define SOFT_FORCE_PROTECT 35//力控软保护阈值

#define BED_POS_ARG 65535.0 //床位置参数转换系数

//识别算法输出点信息结构体
struct IdentifyPointInfo
{
    QString acuname;//点位名称
    int camid;
    int rbindex;
    double pix_x;//像素坐标
    double pix_y;
    double cam_x;//相机坐标
    double cam_y;
    double cam_z;
    double robot_x;//机械臂坐标
    double robot_y;
    double robot_z;
    double force_x;
    double force_y;
    double force_z;
    IdentifyPointInfo()
    {
        acuname = "";
        camid = 0;
        rbindex = 0;
        pix_x = 0;
        pix_y = 0;
        cam_x = 0;
        cam_y = 0;
        cam_z = 0;
        robot_x = 0;
        robot_y = 0;
        robot_z = 0;
        force_x = 0;
        force_y = 0;
        force_z = 0;
    }
    IdentifyPointInfo(QString acu,int cid,int rid,double px,double py,double cx,
                      double cy,double cz,double rbx,double rby,double rbz,double fx,double fy,double fz)
    {
        acuname = acu;
        camid = cid;
        rbindex = rid;
        pix_x = px;
        pix_y = py;
        cam_x = cx;
        cam_y = cy;
        cam_z = cz;
        robot_x = rbx;
        robot_y = rby;
        robot_z = rbz;
        force_x = fx;
        force_y = fy;
        force_z = fz;
    }
    friend QDebug& operator<<(QDebug out, const IdentifyPointInfo &info)
    {
        out << "IdentifyPointInfo(" << info.acuname
                                    << "," << info.camid
                                    << "," << info.rbindex
                                    << "," << info.pix_x
                                    << "," << info.pix_y
                                    << "," << info.cam_x
                                    << "," << info.cam_y
                                    << "," << info.cam_z
                                    << "," << info.robot_x
                                    << "," << info.robot_y
                                    << "," << info.robot_z
                                    << "," << info.force_x
                                    << "," << info.force_y
                                    << "," << info.force_z << ")";
    }
};

Q_DECLARE_METATYPE(IdentifyPointInfo)

#endif // PUBDEF_H
