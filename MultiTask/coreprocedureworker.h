#ifndef COREPROCEDUREWORKER_H
#define COREPROCEDUREWORKER_H

#include <QObject>
#include <QJsonObject>
#include "DeviceControl/robotcontrol.h"
#include "DeviceControl/cameracontrol.h"
#include "DeviceControl/bedcontrol.h"
#include "global.h"
#include "pubdef.h"
#include "opencv2/opencv.hpp"
#include "include/Predicts.h"

//按不同坐标采用不同姿态
struct pose_book
{
    double low_x;
    double high_x;
    double low_y;
    double high_y;
    QVector<double> pose_left;
    QVector<double> pose_right;
};

class CoreProcedureWorker : public QObject
{
    Q_OBJECT
public:
    explicit CoreProcedureWorker(QVector<QSharedPointer<CameraControl>> camobjs, QVector<QSharedPointer<RobotControl>> robots,
                                 QSharedPointer<BedControl> bedctrl, QObject *parent = nullptr);
    void setUsing3D(){m_using3D = m_using3D ? false : true;}
    bool isTreating(){return m_isTreating;}
    void stopTreat(){m_stopTreatment = true;}
    void resetStopTreat(){m_stopTreatment = false;}

signals:
    void warn(QString title, QString msg);
    void warn_b(QString title, QString msg);
    void informPauseTreatment();
    void identifyFinished(int);
    void cureFinished(bool);
    void infromresetCams();
    void blockUI(QString s, int t);
    void refreshCodeImg(int);

public slots:
    //标定
    void goCalibrate(QJsonObject args);
    void goToChessBoard(int rbindex, QString camindex);
    void upAndDown(int rbindex, bool up, QString step);
    //标定精度修正
    void genCaliBias();
    //标定精度验证
    void iniRedPoint();//初始化设备位置
    void recordRedPoint(int camid, double pix_x, double pix_y, int rbindex, double preserve);//录入选取像素点
    void goToRedPoint(int mod);//移动机械臂验证精度
    //校准机械臂外置传感器
    void goCalibrateSensor(int rbindex);
    //识别
    void identifyJingLuo(QString posture, QString jlargs);
    //调理
    void startCure(QString meridian);

private:
    QVector<QSharedPointer<RobotControl>> m_robots;
    QVector<QSharedPointer<CameraControl>> m_camobjs;
    QSharedPointer<BedControl> m_bedCtrl;
    KPPredictFactory *m_predictObj;
    QMap<QString, QVector<QStringList>> m_cureList;//调理穴位点列表
    QMap<QString, IdentifyPointInfo> m_acupoints;//经络点信息集合
    QMap<QString, double> m_iniBedPos;//初始床位置
    QAtomicInteger<bool> m_using3D;//是否使用3d识别
    QString m_curPosture;//当前调理人体姿势
    double m_bedPos;//床实时位置
    QAtomicInteger<bool> m_isTreating;//是否正在运行调理
    QAtomicInteger<bool> m_stopTreatment;//中止调理
    QVector<pose_book> m_poseBook;//机械臂姿态集合
    int m_cali_camid;//校验相机序号
    int m_cali_rbindex;//校验机械臂序号
    double m_contact_bias;//接触模式抬起量
    QVector<QVector<double>> m_cali_points;//待校验机械臂坐标点
    QAtomicInteger<bool> m_iniRedPtFlag;

    QVector<double> getPoseByXY(int rbindex, double x, double y);//根据左机械臂目标位置调整机械臂姿态
    void errquit(QString log, QString uimsg);//退出调理并报错
    void normalquit();//正常退出调理
    QVector<double> calForceDir(QVector<double> p1, QVector<double> p2, QVector<double> ref_v, double deg);//计算p1到p2的力控方向配置
    QVector<double> caliCordsByBed(int rbindex, double bed_dis, QVector<double> cords);//采集后移床校准机械臂坐标
    void cureNextPoint(int rbindex, IdentifyPointInfo &from_pt, QVector<double> &to_pt, QVector<double> &last_point,
                       msg_force_startforce &force, bool vibon = false);//调理下一个经络点

    void identifyIn2D(QString posture, QString jlargs);//2d识别
    bool repredictIn2D(QStringList acunames);//2d二次识别
    void identifyIn3D(QString posture, QString jlargs);//3d识别
    QMap<QString, QVector<QStringList>> genAcunameMap(QStringList jllist);//根据经络名生成左右穴位点名
    int predictIn2D(QStringList acunames, QVector<QSharedPointer<quint8>> rgbs, int mode, QMap<QString, IdentifyPointInfo> &acupoints, std::map<std::string, std::vector<double> > &boxes);//调用ai识别2d获取像素坐标
    int calRobotXYZ(QVector<QSharedPointer<quint8>> depths, QMap<QString, IdentifyPointInfo> &acupoints);//像素坐标转机械臂3d坐标
    void calculatepts(IdentifyPointInfo &pt, double bed_bias);//计算相机坐标系坐标和机械臂坐标系坐标
    void insertPixels(QMap<QString, QVector<QStringList>> &curelist, QMap<QString, IdentifyPointInfo> &acupoints);//根据配置文件插入像素坐标
    void calIniBedPos(QMap<QString, QVector<QStringList>> &curelist, QMap<QString, IdentifyPointInfo> &acupoints);//规划调理床初始位置
    bool isRbPtValid(int rbindex, double x, double y, double z);//校验调理机械臂坐标是否超限
    void saveIdentifyRawImgs(QVector<QSharedPointer<quint8>> rgbs, QVector<QSharedPointer<quint8>> depths, bool repredicting = false);//保存识别原图
    QMap<QChar, cv::Mat> drawIdentifyResult(QMap<QString, QVector<QStringList>> &curelist, QMap<QString, IdentifyPointInfo> &acupoints,
                                            std::map<std::string, std::vector<double> > &boxes, QVector<QSharedPointer<quint8>> rgbs, bool coverup);//绘制2d识别结果并返回
    void waitForConfirm(QString title, QString msg);//阻塞等待用户确认
    double getMidMean(vector<double> &vec);//排序数组并计算中位数平均值
};

#endif // COREPROCEDUREWORKER_H
