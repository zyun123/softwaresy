#ifndef GUIINTERFACE_H
#define GUIINTERFACE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QMessageBox>
#include "Other/Logger.h"
#include <QApplication>
#include "global.h"

class GUIInterface : public QObject
{
    Q_OBJECT
public:
    explicit GUIInterface(QObject *parent = nullptr);

    void iniTimers(){emit iniTimersSignal();}
    void startIniDev(){emit startIniDevSignal();}

    Q_INVOKABLE void login(QString usrname, QString pwd);
    Q_INVOKABLE void qmllog(QString log_msg){LOG_INFO << log_msg;}
    Q_INVOKABLE int switchAppVer();//切换配置文件版本
    Q_INVOKABLE void openLog();//打开系统日志
    Q_INVOKABLE void setSysCfg(QString key, QString val);//设置系统配置
    Q_INVOKABLE void callonUploadCureRecord(QJsonObject obj){emit uploadCureRecord(obj);}
    Q_INVOKABLE void setCureForceVal(int val){g_cureForceVal = val;}
    Q_INVOKABLE void openVNC();

    Q_INVOKABLE QJsonObject loadJson();

    Q_INVOKABLE void guiCall(QJsonObject args, bool confirm = false, QString tip = "")
    {
        if(confirm)
            QMessageBox::warning(nullptr, "", tip.toLocal8Bit());
        else if(tip != "")
            LOG_INFO << tip;
        emit guiCallSig(args);
    }

    Q_INVOKABLE bool switchEMC()
    {
        g_testEMC = g_testEMC ? false : true;
        return g_testEMC;
    }

    Q_INVOKABLE int switchTestCaliMod()
    {
        g_testCali = (g_testCali == 0 ? 1 : 0);
        return g_testCali;
    }

signals:
    void iniTimersSignal();
    void startIniDevSignal();

    void guiCallSig(QJsonObject);

    void switchPoseOK();
    void identifyOK(int errCode);
    void cureOK(bool suc);
    void pauseTreatmentOK();
    void sigUpdateProgress(double curpro);

    void updateBedStat(int pos);
    void updateRbPos1(int index, QString msg);
    void updateRbPos2(int index, QString msg);
    void updateRbStatus(int index, QJsonObject info);
    void blockUI(QString s, int t);
    void switchDevSta(int index);

    void warnMsgToUI(QString title, QString msg);
    void confirmMsgL(QString s1, QString s2);
    void confirmMsgR(QString s1, QString s2);
    void confirmMsgCore(QString s1, QString s2);
    void iniDeviceID(QString id);
    void iniSysVer(QString ver);
    void iniLoginUsr(QString id, QString pwd);

    void localLoginOK(QString name, QString role, QString id, QJsonObject settings);

    void serverLoginOK(bool result, QString role);
    void updateAppList(QJsonArray applist);
    void uploadCureRecord(QJsonObject);

public slots:
    void showWarningMsg(QString title, QString msg);
    void iniForCareVersion();//保健版一键启动用
    void getOpenFile(QString, QString, QString, QSharedPointer<QString>);
};

#endif // GUIINTERFACE_H
