#ifndef ROBOTCONTROL_H
#define ROBOTCONTROL_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QJsonArray>
#include "Comm/devserver.h"
#include "global.h"
#include "Comm/baseclient.h"
#include "Comm/realtimemonclient.h"

#define RIGHT_SAFE_POS        {-1.571, -1.571, -1.571, -1.571, 1.571, 3.141}
#define LEFT_SAFE_POS         {1.571, -1.571, 1.571, -1.571, -1.571, 0}
#define RIGHT_PHOTO_POS       {-2.216, 0.069, -2.844, -1.919, 0.174, 3.141}
#define LEFT_PHOTO_POS        {2.216, -3.211, 2.844, -1.221, -0.174, 0}
#define LEFT_CAM_L_CALI_POS   {1.324, -0.243, -1.454, -2.510, 1.484, 1.267}
#define LEFT_CAM_R_CALI_POS   {1.221, -1.221, 1.745, -2.792, -1.570, 1.221}
#define LEFT_CAM_M_CALI_POS   {1.221, -1.047, 2.007, -4.188, -1.396, 1.571}
#define LEFT_CAM_H_CALI_POS   {0.640, -0.913, 1.253, -2.441, -1.108, 1.986}
#define RIGHT_CAM_L_CALI_POS  {-1.221, -2.094, -1.7453, -0.174, 1.221, 1.745}
#define RIGHT_CAM_R_CALI_POS  {-1.165, -2.976, 1.452, -0.523, -1.716, 1.903}
#define RIGHT_CAM_M_CALI_POS  {-1.221, -2.094, -2.007, 1.047, 1.396, 1.571}
#define RIGHT_CAM_H_CALI_POS  {-0.647, -2.184, -1.229, -0.723, 1.141, 0.984}

#define ARGS_LEN 28//发给机械臂的浮点数数量

struct msg_move
{
    double msgtype;
    double mvtype;//移动类型：1movel 2movej 3speedl 4speedj 11stopl 12stopj
    double posetype;//1位置 2角度
    double argvec[6];//位姿 角度 速度向量
    double speed;//移速
    double acc;//加速度
    double funcRT;//函数返回时间 move默认0 speed得无限大
    double radius;//交融半径m
    msg_move(){
        msgtype = 1;
        mvtype = 2;
        posetype = 1;
        memset(argvec, 0, sizeof(argvec));
        speed = 0.1;
        acc = 1.0;
        funcRT = 0;
        radius = 0;
    }
};//移动指令

struct msg_force_damp
{
    double msgtype;
    double forcetype;
    double fdamp;//阻尼 0~1
    msg_force_damp(){
        msgtype = 2;
        forcetype = 1;
        fdamp = 0;
    }
};//力控指令-设置阻尼

struct msg_force_gain
{
    double msgtype;
    double forcetype;
    double fgain;//增益 0~2
    msg_force_gain(){
        msgtype = 2;
        forcetype = 2;
        fgain = 1;
    }
};//力控指令-设置增益

struct msg_force_startforce
{
    double msgtype;
    double forcetype;
    double aixs_bias[6];//力控坐标系相对基座偏移 发6个1力按照工具坐标系走
    double direction[6];//力控方向
    double force_value[6];//力控值
    double ftype;//力控类型
    double speed[6];//寻力速度
    msg_force_startforce(){
        msgtype = 2;
        forcetype = 3;
        memset(aixs_bias, 0, sizeof(aixs_bias));
        memset(direction, 0, sizeof(direction));
        memset(force_value, 0, sizeof(force_value));
        direction[2] = 1;
        ftype = 2;
        speed[0] = 0.05;
        speed[1] = 0.05;
        speed[2] = 0.035;
        speed[3] = 0.05;
        speed[4] = 0.05;
        speed[5] = 0.05;
    }
};//力控指令

struct msg_force_endforce
{
    double msgtype;
    double forcetype;
    double stopacc;
    msg_force_endforce(){
        msgtype = 2;
        forcetype = 4;
        stopacc = 0.5;
    }
};//力控指令-关力控

struct msg_force_clearforce
{
    double msgtype;
    double forcetype;
    msg_force_clearforce(){
        msgtype = 2;
        forcetype = 5;
    }
};//力控指令-力控清零

struct msg_ctrl_io
{
    double msgtype;
    double ctrltype;
    double ioindex;//io序号 从0起 0 振动 1 缩减
    double value;//设置true 1, false 2
    msg_ctrl_io(){
        msgtype = 3;
        ctrltype = 1;
        ioindex = 0;
        value = 1;
    }
};//控制指令-IO

struct msg_ctrl_freedrive
{
    double msgtype;
    double ctrltype;
    double enable;//true 1, false 2
    msg_ctrl_freedrive(){
        msgtype = 3;
        ctrltype = 2;
        enable = 1;
    }
};//控制指令-自由驱动

struct msg_ctrl_tcp
{
    double msgtype;
    double ctrltype;
    double tcp[6];
    msg_ctrl_tcp(){
        msgtype = 3;
        ctrltype = 3;
        memset(tcp, 0, sizeof(tcp));
    }
};//控制指令-设置tcp

struct msg_ctrl_payload
{
    double msgtype;
    double ctrltype;
    double mass;//重量
    double cog[3];//重心
    msg_ctrl_payload(){
        msgtype = 3;
        ctrltype = 4;
        mass = 0;
        memset(cog, 0, sizeof(cog));
    }
};//控制指令-设置payload

struct msg_ctrl_get_fsensor_force
{
    double msgtype;
    double ctrltype;
    msg_ctrl_get_fsensor_force(){
        msgtype = 3;
        ctrltype = 5;
    }
};//控制指令-获取外置清零力控值 deprecated

struct msg_ctrl_enable_fsensor
{
    double msgtype;
    double ctrltype;
    double enable;
    double mass;
    double offset[3];
    double cog[3];
    //kunwei old
//    msg_ctrl_enable_fsensor(){
//        msgtype = 3;
//        ctrltype = 6;
//        enable = 1;
//        mass = 0.44;
//        offset[0] = 0;
//        offset[1] = 0;
//        offset[2] = 0.048;
//        cog[0] = 0;
//        cog[1] = 0;
//        cog[2] = 0.025;
//    }
    //kunwei new
//    msg_ctrl_enable_fsensor(){
//        msgtype = 3;
//        ctrltype = 6;
//        enable = 1;
//        mass = 0.32;
//        offset[0] = 0;
//        offset[1] = 0;
//        offset[2] = 0.038;
//        cog[0] = 0;
//        cog[1] = 0;
//        cog[2] = 0.019;
//    }
    //kunwei big
    msg_ctrl_enable_fsensor(){
        msgtype = 3;
        ctrltype = 6;
        enable = 1;
        mass = 0.58;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0.044;
        cog[0] = 0;
        cog[1] = 0;
        cog[2] = 0.023;
    }
};//控制指令-开关外置传感器

struct msg_ctrl_contact
{
    double msgtype;
    double ctrltype;
    double contactdir[6];
    double speed[6];
    double spdacc;
    double stopacc;
    msg_ctrl_contact(){
        msgtype = 3;
        ctrltype = 7;
        memset(contactdir, 0, sizeof(contactdir));
        memset(speed, 0, sizeof(speed));
        spdacc = 0.1;
        stopacc = 1;
    }
};//控制指令-探寻接触物体

struct msg_comm
{
    double args[28];
    msg_comm(){
        memset(args, 0, sizeof(args));
    }
};//通用

class RobotControl : public QObject
{
    Q_OBJECT
public:
    explicit RobotControl(int rbindex, QObject *parent = nullptr);
    ~RobotControl();

    void iniCon();
    QVector<double> getZeroForce(){return m_zero_force;}
    void startAutoForce(QVector<double> start_pos, QVector<double> target_pos, msg_force_startforce &normal_f,
                        msg_force_startforce &climb_f, QVector<double> lift_dir);//开启力控自适应
    void endAutoForce();//关闭力控自适应

    void setRTProgStatFlag(bool flag){emit sigRTProgSFlag(flag);}//设置机械臂脚本运行状态监测标志
    void setRTSoftProtectiveStopFlag(bool flag){emit sigRTSPSFlag(flag);}//设置力控软保护标志

    void onConfirm(){m_block_flag = false;}

    //端口控制
    void endforce();//关闭力控
    bool clearforce();//力控清零
    void reduce(bool tip = false);//切换缩减模式
    void setforce(msg_force_startforce msg);//开启力控
    void setDamp(double damp);//设置阻尼
    void setGain(double gain);//设置增益
    void freeDrive(bool open, bool tip = false);//自由驱动
    void blockFreeDrive(bool flag){m_blockFreeDrive = flag;}//禁用自由驱动
    void robotReset();//重置机械臂状态
    void robotPlay(bool tip = false);//启动
    void robotStop(bool tip = false);//停止
    void robotPause();//暂停
    void unlockProtectiveStop(bool tip = false);//解除保护性停止
    void closePopUp(bool tip = false);//关弹窗
    void powerOn(bool tip = false);//上电
    void vibrateOff();//关震动
    void vibrateOn();//开震动
    void setTCP(QVector<double> tcp);//设置标定中心板坐标
    void setPayLoad(double mass, QVector<double> cog);//设置负载
    void safePos(bool *result = nullptr);//安全位置
    void photoPos(bool *result = nullptr);//拍照位置
    void calibratePos(QChar type);//标定位置

    void moveToPos(char type, int posetype, QVector<double> pos, double acc, double vel);
    void speedOn(char type, QVector<double> spdv, double acc, double rt = 10000);//设置往某个方向移动 必须通过stop结束运动
    void stopMoving(double acc = 1, char type = 'l');
    void contact(QVector<double> spd, double acc, double stop_acc);//探寻触碰
    int waitReplyFromTCPClient(QStringList expected, int timeout = 15 * 1000);//等待8080/8081返回预期值 timeout超时ms 返回值：正常0 超时1 连接断开2
    void stopWaitMove(){m_stopWaitForReply_8080 = true;}
    void resetStopWaitMove(){m_stopWaitForReply_8080 = false;}
    void comPlot(){emit connectToPlot();}

    template<typename T>
    void commRobotCtrl(T &msg, QString log = "");//8080端口指令发送接口
    bool sndDashCmd(QString cmd, QStringList expected, int reSndTimes = 3);//29999端口指令发送接口

    bool isConnected(){return m_rtConnected;}//返回机械臂连接状态
    RealtimeRobotStatus getStatus();//获取30003端口机械臂状态
    bool isInPhotoPos();//判断是否已在识别位置

    static QVector<double> eulerAngleToRotationVec(QVector<double> eangle);//欧拉角转旋转向量
    static QVector<double> vecToEulerAngle(QVector<double> origin_v, QVector<double> target_v);//求两向量欧拉角
    static QVector<double> rotateVecAlongAxis(QVector<double> vec, QVector<double> axis, double theta);//返回一个向量绕另一向量旋转角度后的向量
    static QVector<double> xVec(QVector<double> a, QVector<double> b);//求向量叉乘
    static double dotVec(QVector<double> a, QVector<double> b);//求向量点乘
    static QVector<double> rotateVecByRotationVec(QVector<double> vec, QVector<double> r_vec);//返回一个向量经旋转向量旋转后向量

signals:
    void warn(QString title, QString msg);
    void warn_b(QString title, QString msg);
    void informPauseTreatment();
    void blockUI(QString s, int t);
    void switchDevSta(int);
    void connectToPlot();

    void sigRTSPSFlag(bool);
    void sigRTProgSFlag(bool);

    void sigSetAutoArgs(QVector<double>, QVector<double>);

public slots:
    void onReduceForce();
    void onRecoverForce(QString reason);

private:
    int m_robotIndex;//机械臂序号
    bool m_reduce;//是否缩减模式中
    QAtomicInteger<bool> m_isRunningScript;//是否在运行脚本
    QAtomicInteger<bool> m_free_drive_flag;//是否在自由驱动中，此时不可发送移动相关指令
    QAtomicInteger<bool> m_isMoving;//是否在执行移动相关指令，此时不可自由驱动
    QAtomicInteger<bool> m_isForcing;//是否在执行力控相关指令，此时不可自由驱动
    QAtomicInteger<bool> m_blockFreeDrive;//是否禁用自由驱动
    QAtomicInteger<bool> m_block_flag;//阻塞线程等待UI确认
    BaseClient *m_DashClient;
    RealtimeMonClient *m_RTClient;
    DevServer *m_robotServer;
    QVector<QThread*> m_threads;

    QAtomicInteger<bool> m_dashConnected;//29999连接状态
    QAtomicInteger<bool> m_rtConnected;//30003连接状态
    QAtomicInteger<bool> m_serverConnected;//8080/8081连接状态

    QAtomicInteger<bool> m_stopWaitForReply_8080;
    QVector<double> m_zero_force;

    //自适应相关参数
    QAtomicInteger<bool> m_auto_force_on;//自适应开启标志
    msg_force_startforce m_auto_force_normal;//自适应正常力
    msg_force_startforce m_auto_force_climb;//自适应爬坡力
    QVector<double> m_auto_lift_dir;//自适应爬坡方向
    QVector<double> m_auto_pathpt;//自适应目标点

    QMutex m_mutex_automode;//保证自适应模式下同时间只有但线程控制机械臂

    void addToThreadPool(QObject *obj);
    QVector<double> jsonarrayToVector(QJsonArray arr);

    bool queryDashBoard(QString query, QStringList expected);//29999查询等待
    bool waitReplyFromDashBoard(QStringList expected, int times = 5);//等待29999返回预期值 times校验几次接收

    void blockForConfirm(QString title, QString msg);
};

#endif // ROBOTCONTROL_H
