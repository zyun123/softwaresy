#ifndef CAMERACONTROL_H
#define CAMERACONTROL_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <NumCpp.hpp>
#include "pubdef.h"
#include "global.h"
#include <QVideoFrame>
#include <QProcess>
#include <opencv2/opencv.hpp>
#include "include/sycrypt.hpp"

#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

class CameraControl : public QObject
{
    Q_OBJECT
public:
    explicit CameraControl(QString serialnum, int index, QObject *parent = nullptr);
    ~CameraControl();
    void inishm();
    bool checkExtrinsics();//检测是否已标定该相机
    nc::NdArray<double> getExtrinsics(int rbindex);
    void getImageFromshm();//从子进程读取相机数据
    QSharedPointer<quint8> getBGRData();//获取BGR数据
    QSharedPointer<quint8> getDepthData();//获取深度数据
    void cam2dToCam3d(double pix_x, double pix_y, double d, double &x, double &y);//内参转换
    void cam3dToRobot3d(int rbIndex, double &x, double &y, double &z);//外参转换
    void caliFix(int rbIndex, double &x, double &y, double &z);//标定矫正
    void updateExtrinsics(nc::NdArray<double> ex, int rbindex);
    bool checkBedCollision();//检测床体碰撞
    QString getCurePosture()
    {
        m_curePosture_mutex.lock();
        auto tmp = m_curePosture;
        m_curePosture_mutex.unlock();
        return tmp;
    }
    void setCurePosture(QString posture)
    {
        m_curePosture_mutex.lock();
        m_curePosture = posture;
        m_curePosture_mutex.unlock();
    }

    static void restartCamProcess()
    {
        //重启相机进程
        QProcess clrPro;
        clrPro.start(QString("killall cam_process-") + g_runArgs["CamType"]);
        clrPro.waitForFinished(1000);
    }

    static double getDepthValue(int pix_x, int pix_y, QSharedPointer<quint8> pdat);//获取深度值
    static double getEstimateDepthValue(int pix_x, int pix_y, QSharedPointer<quint8> pdat);//获取估算深度值 较差估算结果返回负数
    static void savePic(QString path, QString filename, QSharedPointer<quint8> pdat, int type);//保存rgb或深度图
    static void saveEncodeMat(QString file, cv::Mat &mat);//cv::mat加密并保存

    float m_intrinsics_fx;
    float m_intrinsics_fy;
    float m_intrinsics_cx;
    float m_intrinsics_cy;

signals:
    void frameArrived(QSharedPointer<QImage>);
    void frameArrived_s(QSharedPointer<QImage>);
    void getOpenFile(QString, QString, QString, QSharedPointer<QString>);
    void switchDevSta(int);

private:
    unsigned char *m_shmempt;//相机进程共享内存
    QString m_serialnum;
    int m_index;
    QMutex m_mutex;
    quint8 *m_frameData;
    QMutex m_extrinsics_mutex;
    int m_discon_cnt;//相机断连计数

    nc::NdArray<double> m_extrinsics0;
    nc::NdArray<double> m_extrinsics1;

    //用于调理期间保存图片
    QString m_curePosture;
    QMutex m_curePosture_mutex;
    int m_framecnt;
    SYEncryptor *m_encryptor;
};

#endif // CAMERACONTROL_H
