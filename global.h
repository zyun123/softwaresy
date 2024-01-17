#ifndef GLOBAL_H
#define GLOBAL_H

#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include "Other/Logger.h"
#include <QtSql>

extern QMap<QString, QString> g_runArgs;//命令行参数
//system.ini
extern QString g_sysVer;//sy3系统版本号
extern QString g_deviceID;//设备ID
extern QString g_cacheUserID;//历史用户
extern QString g_cachePWD;//历史用户密码
extern QString g_ServerIP;//数据服务端ip
extern int g_ServerPort;//数据服务端端口
extern QString g_HostIP;//需要从配置文件读取
extern quint16 g_Host1Port;//需要从配置文件读取
extern quint16 g_Host2Port;//需要从配置文件读取
extern QString g_RobotleftIP;//需要从配置文件读取
extern QString g_RobotrightIP;//需要从配置文件读取
extern quint16 g_RobotDashPort;//dashboard端口29999
extern quint16 g_RobotCommPort;//通信端口30002
extern quint16 g_RobotRTPort;//实时消息端口30003
extern QString g_BedIP;//调理床IP
extern quint16 g_BedPort;//调理床连接端口
extern int g_cameraNum;//相机数量
extern QStringList g_cameraSerialNums;//相机序列号
extern QString g_cameraOrder;//相机顺序
extern QString g_saveImgPath;//识别、调理、截图图像保存目录
//config/*.json
extern QSharedPointer<QJsonObject> g_JingLuoConfig;//经络配置
extern QSharedPointer<QJsonObject> g_JingLuoConfig_Body;//经络配置(无头脚)
extern QSharedPointer<QJsonObject> g_CureInsertsConfig;//经络插点配置
extern QSharedPointer<QJsonObject> g_CurePtsBiasConfig;//经络点偏移量修正（非标定向）
extern QSharedPointer<QJsonObject> g_CurePoseConfig;//调理姿态配置
extern QSharedPointer<QJsonObject> g_CureForceBiasConfig;//调理力控修正值
extern QSharedPointer<QJsonObject> g_CureForceOverrideConfig;//力控方向覆盖
extern QSharedPointer<QJsonObject> g_CurePtPauseConfig;//调理经络点停留时间配置
extern QSharedPointer<QJsonObject> g_RepredictPtsConfig;//需二次识别的点配置
extern QSharedPointer<QJsonObject> g_CureContactConfig;//接触方向配置
extern QSharedPointer<QJsonObject> g_CureContactConfig_Body;//接触方向配置(无头脚)
extern QSharedPointer<QJsonObject> g_JingLuoCoverConfig;//经络点隐藏显示配置
extern QSharedPointer<QJsonObject> g_CaliConfig;//坐标标定校准配置
extern QSharedPointer<QJsonObject> g_CaliConfig_Manual;//坐标标定校准配置(人工)
extern QSharedPointer<QJsonObject> g_CaliConfig_bed;//床和机械臂校准配置

extern QAtomicInteger<int> g_cureVersion;//调理版本 0:全身 1:躯干不带头脚
extern QAtomicInteger<bool> g_usingImg;//是否使用固定图片识别调理
extern QAtomicInteger<int> g_cureForceVal;//当前调理力控值
extern QAtomicInteger<bool> g_coreBlockFlag;//业务线程阻塞标志

extern QAtomicInteger<bool> g_testEMC;
extern QAtomicInteger<int> g_testCali;//标定验证模式 0先动床后采点 1先采点后动床
extern double g_testSoftProtect;//力控软保护值
extern double g_testIniBedPos;//标定扎点验证床初始位置

extern QSqlDatabase g_db;

class Global
{
public:
    Global();

    template<typename T>
    static T fromBigEndian(char* src)//多字节大小端转换
    {
        T ret;
        memcpy(&ret, src, sizeof(T));
        ret = qFromBigEndian(ret);
        return ret;
    }

    template<typename T>
    static T getValue(char *src)//单字节类型转换
    {
        T ret;
        memcpy(&ret, src, sizeof(T));
        return ret;
    }

    static void printBytes(QByteArray data)
    {
        QString out = "";
        for(int i = 0; i < data.size(); i++)
        {
            out += QString("%1").arg((unsigned char)data.at(i), 2, 16, QLatin1Char('0')) + " ";
        }
        LOG_INFO << out;
    }

    static QVector<double> fromArray(double *arr, int size)
    {
        QVector<double> vec;
        for(int i = 0; i < size; i++)
            vec.append(arr[i]);
        return vec;
    }

    static QString arrToString(double *arr, int size)
    {
        QString ret = "[";
        for(int i = 0; i < size; i++)
        {
            ret += QString::number(arr[i], 'g', 3);
            if(i < size - 1)
                ret += ',';
            else
                ret += ']';
        }
        return ret;
    }

    template<typename T, typename T1, typename ...Args>
    static void parallel(T (T1::*func)(Args...), QSharedPointer<T1> obj1, QSharedPointer<T1> obj2, Args ...args)
    {
        std::thread t1(func, obj1, args...);
        std::thread t2(func, obj2, args...);
        t1.join();
        t2.join();
    }

    static void loadConfig();
    static bool inidb();
    static void setSysValue(QString key, QString val);
};

#endif // GLOBAL_H
