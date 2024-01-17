#ifndef BEDCONTROL_H
#define BEDCONTROL_H

#include <QObject>
#include <QMutex>
#include "Comm/baseclient.h"
#include "global.h"
#include "pubdef.h"
#include "cameracontrol.h"

#define R_MIN_PER_MM_SEC 600 //床移动速度与机械臂移动速度mm/s换算关系
#define DEFAULT_BED_SPEED 18 //床默认移动速度 mm/s

#define LIGHT_ALL_ON 0x0f
#define LIGHT_ALL_OFF 0x00
#define LIGHT_TYPE_TOP 0x01
#define LIGHT_TYPE_RED 0x02
#define LIGHT_TYPE_GREEN 0x04
#define LIGHT_TYPE_BLUE 0x08

#define USB_ALL_ON 0x3f
#define USB_DO1_ON 0x01
#define USB_DO2_ON 0x02
#define USB_DO3_ON 0x04
#define USB_DO4_ON 0x08
#define USB_DO5_ON 0x10
#define USB_DO6_ON 0x20

struct BedStatus
{
    double pos;//床位置
    bool isMoving;//床电机运动
    bool isElecOn1;//音圈电机1
    bool isElecOn2;//音圈电机2
    bool isHeatOn1;//音圈电机加热1
    bool isHeatOn2;//音圈电机加热2
    bool isNodeOn;//电机节点是否完成启动
    BedStatus()
    {
        pos = 0.0;
        isMoving = false;
        isElecOn1 = false;
        isElecOn2 = false;
        isHeatOn1 = false;
        isHeatOn2 = false;
        isNodeOn = false;
    }
};

class BedControl : public QObject
{
    Q_OBJECT
public:
    explicit BedControl(QObject *parent = nullptr);
    ~BedControl();

    void iniCon();
    void iniDev();
    void iniHeadCam(QSharedPointer<CameraControl> camobj){m_headcam = camobj;}

    double m_move_threhold_head;//床头方向床最大偏移量
    double m_move_threhold_tail;//床尾方向床最大偏移量

    //床通信控制，返回值为是否发送成功
    bool restart();//新床重启
    bool moveTo(double posInMeter, double speedlvl = 1.0, int timeout = 60 * 1000);//移动床
    bool stop();//立即停止
    bool elecOnOff(bool vibrateOn = false, bool heatOn = false, quint8 l_rate = 0x15, quint8 r_rate = 0x15); //音圈电机控制
    bool lightsOnOff(quint8 type = 0);//设置开关灯，默认全关
    bool usbOnOff(quint8 type = 0);//usb上电控制, 默认全关

    bool isConnected(){return m_isConnected;}
    std::tuple<BedStatus, bool> getBedStatus();

    void stopWaitMove(){m_stopWaitForMove = true;}
    void resetStopWaitMove(){m_stopWaitForMove = false;}

signals:
    void mvover();
    void warn(QString title, QString msg);
    void informPauseTreatment();
    void switchDevSta(int);

private:
    BaseClient *m_bedClient;
    QThread *m_socketTh;
    QAtomicInteger<bool> m_stopWaitForMove;
    QMutex m_getStaLock;
    QAtomicInteger<bool> m_isConnected;
    QSharedPointer<CameraControl> m_headcam;
    QAtomicInteger<bool> m_reconnect_flag;

    QByteArray transToBytes(QList<quint8> list);
    bool waitForReply(QByteArray sndMsg, int timeout = 1000);
    bool waitForMove(double posInMeter, int timeout = 60 * 1000);//等待床移动到位置pos timeout=60s
};

#endif // BEDCONTROL_H
