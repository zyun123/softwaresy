#ifndef MAINCONTROL_H
#define MAINCONTROL_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QJsonObject>
#include <QThread>
#include <QTimer>
#include <QAtomicInteger>
#include <QApplication>
#include "global.h"
#include "DeviceControl/cameracontrol.h"
#include "DeviceControl/robotcontrol.h"
#include "DeviceControl/bedcontrol.h"
#include "MultiTask/cameragetdataworker.h"
#include "MultiTask/coreprocedureworker.h"
#include "Other/frameprovider.h"
#include <QProcess>

class MainControl : public QObject
{
    Q_OBJECT
public:
    explicit MainControl(QObject *parent = nullptr);
    ~MainControl();

    void setfp(QVector<QSharedPointer<FrameProvider>> fp, QVector<QSharedPointer<FrameProvider>> fp_s)
    {
        m_fp = fp;
        m_fp_shadow = fp_s;
    }

signals:
    void showWarningMsg(QString title, QString msg);
    void blockUI(QString s, int t);
    void setCamThreadStart();
    void identifyOK(int errCode);
    void cureOK(bool suc);
    void setTestStart();
    void switchPoseOK();//通知UI switchpos结束
    void pauseTreatmentOK();//自动中止调理后通知UI
    void sigUpdateProgress(double curpro);//更新启动进度
    void getOpenFile(QString, QString, QString, QSharedPointer<QString>);

    //机械臂阻塞信号
    void confirmMsgL(QString s1, QString s2);
    void confirmMsgR(QString s1, QString s2);
    //业务子线程阻塞信号
    void confirmMsgCore(QString s1, QString s2);

    //开启业务子线程
    void startIdentify(QString, QString);
    void startCure(QString);
    void startCali(QJsonObject);
    void startCaliSensor(int);
    void checkCali(int, QString);
    void goUPAndDown(int, bool, QString);
    void sig_genCaliBias();
    void sig_iniRedPoint();
    void sig_recordRedPoint(int, double, double, int, double);
    void sig_goToRedPoint(int);

    //预留给服务端
    void serverLoginOK(bool result, QString role);
    void updateAppList(QJsonArray);

    //更新UI显示设备状态信息
    void updateBedStat(int);
    void updateRbPos1(int, QString);//刷新控制面板机械臂位姿
    void updateRbPos2(int, QString);//刷新控制面板机械臂关节角
    void updateRbStatus(int, QJsonObject);//刷新tcpforce robotmode programstatus
    void switchDevSta(int);//刷新UI状态栏 0左机械臂 1右机械臂 2~5相机 6调理床

    void iniForCareVersion();

    void refreshCodeImg(int);//刷新UI图片

public slots:
    void onGUICall(QJsonObject callinfo);//gui统一调用接口
    void iniTimers();//初始化定时器
    void iniControls();//初始化设备控制对象及各子线程
    void deal_mvover();//处理床到位消息
    void resetCamStream();//重启相机进程
    void onPause();//响应中止信号
    void uploadCureRecord(QJsonObject obj);//上传调理记录

private:
    QSharedPointer<BedControl> m_bedCtrl;
    QVector<QSharedPointer<RobotControl>> m_robots;
    QVector<QSharedPointer<CameraControl>> m_cameras;
    QVector<QSharedPointer<FrameProvider>> m_fp;
    QVector<QSharedPointer<FrameProvider>> m_fp_shadow;

    //连接患者数据服务器
    BaseClient *m_dbclient;
    QThread m_dbClientThread;
    QByteArray m_rcvcache;
    QTimer *m_refreshTimer;

    QVector<QThread*> m_camGetThreads;//相机获取数据线程
    QThread m_coreProThread;//核心事务线程 包括识别、调理、标定

    QVector<CameraGetDataWorker*> m_camGetWorkers;//相机获取工作对象
    CoreProcedureWorker *m_coreProWorker;//核心事务对象

    bool m_isSwitchingPos;//正在执行换姿势
    QAtomicInteger<bool> m_pauseLock;//中止过程锁，防止多个信号同时触发中止

    //相机进程启动相关
    QTimer *m_startCamPTimer;
    QProcess *m_camprocess;
    QAtomicInteger<bool> m_camProcessRestartFlag;

    void iniDevices();//所有设备初始化
    void goToEulerAngle(int, QString);//移动至欧拉角姿态

    void onPanelRBStateRefresh(int index);//机械臂控制面板状态刷新
    void restartApp(){qApp->exit(999);}//重启软件

    void startTreatment(QString meridian, bool autonext);//开始调理
    void pauseTreatment(bool normal);//暂停调理
    void recognizeJingLuo(QString args);//识别经络
    void switchFakeIdentify();//切换图片识别
    void switchUsing3D();//切换3d识别
    void switchPose();//通知切换姿势
    void pauseCamStream();//触发屏保关闭相机流
    void camshot(QString zishi);//截取当前相机图像数据
    void camshotInDiffPos(QString zishi);//截取床在不同位置图像

    //设备管理-机器人运行功能
    void freeDrive(int index, bool open);//自由驱动
    void robotPlay(int index);//启动
    void robotStop(int index);//停止
    void safePos(int index);//安全位置
    void photoPos(int index);//拍照位置
    void robotsStop();//双机停止
    void robotsPause();//双机暂停
    void robotsStart();//双机启动
    void oneBtnStart();//一键启动
    void endForce(int index);//关力控
    void clearForce(int index);//清力控
    void reduce(int index);//开关缩减模式
    void unlockProtectiveStop(int index);//解除保护性停止
    void powerOn(int index);//上电
    void closePopUp(int index);//关弹窗
    void movej(int index, int type, QString args);
    void movel(int index, QString args);
    void speedl(int index, QString args);
    void speedj(int index, QString args);
    void stopl(int index, QString args);
    void stopj(int index, QString args);
    void contact(int index, QString args);
    void rawCommand(int index, QString args);//参数列表下发

    //床直接控制
    void mvbed(int pos);

    //设备管理-调理头设置功能
    void vibrateOff();//关震动
    void viboff(int index);
    void vibrateOn();//开震动
    void vibon(int index);
    void heatingOff();//关加热
    void heatingOn();//开加热

    //设备管理-标定管理功能
    void calibrateStart(QJsonObject args);
    void checkCaliStart(int rbindex, QString camindex);
    void goUpDown(int rbindex, bool up, QString step);
};

#endif // MAINCONTROL_H
