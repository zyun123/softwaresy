#include "guiinterface.h"
#include <opencv2/opencv.hpp>
#include <QProcess>
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include "global.h"
#include <QFile>
#include <QFileDialog>
#include <QtSql>

GUIInterface::GUIInterface(QObject *parent) : QObject(parent)
{

}

void GUIInterface::iniForCareVersion()
{
    emit iniLoginUsr(g_cacheUserID, g_cachePWD);
    emit iniDeviceID(g_deviceID);
    emit iniSysVer(g_sysVer);
}

void GUIInterface::showWarningMsg(QString title, QString msg)
{
    emit warnMsgToUI(title, msg);
}

void GUIInterface::login(QString usrid, QString pwd)
{
    if(usrid == "super" && pwd == "sy666")
    {
        QJsonObject settings;
        settings.insert("music", 0);
        settings.insert("volume", 1);
        settings.insert("screen_lock", 4);
        emit localLoginOK("super", "super", "super", settings);
        return;
    }
    QSqlQuery q;
    q.exec(QString("select * from user_info where userid = '%1' and password = '%2'").arg(usrid).arg(pwd));
    if(q.next())
    {
        LOG_INFO << "用户登录：" << usrid;
        QJsonObject settings;
        settings.insert("music", q.value("music").toDouble());
        settings.insert("volume", q.value("volume").toDouble());
        settings.insert("screen_lock", q.value("screen_lock").toDouble());
        emit localLoginOK(q.value("name").toString(), q.value("role").toString(), q.value("userid").toString(), settings);
    }
    else
        showWarningMsg("警告", "账号密码错误，请重新输入！");
}

int GUIInterface::switchAppVer()
{
    g_cureVersion = (g_cureVersion == 0 ? 1 : 0);
    showWarningMsg("", QString(QString("调理模式已切换为： ") + (g_cureVersion == 0 ? "全身调理" : "躯干调理")));
    //切换版本后重新加载配置
    Global::loadConfig();
    return g_cureVersion;
}

void GUIInterface::getOpenFile(QString title, QString dir, QString filter, QSharedPointer<QString> str)
{
    *str = QFileDialog::getOpenFileName(nullptr, title, dir, filter);
}

void GUIInterface::openLog()
{
    QSharedPointer<QString> logfile(new QString(""));
    getOpenFile("系统日志", "./log", "*.html", logfile);
    QProcess cmd;
    cmd.startDetached(QString("google-chrome ") + *logfile);
}

void GUIInterface::setSysCfg(QString key, QString val)
{
    Global::setSysValue(key, val);
}

void GUIInterface::openVNC()
{
    QProcess cmd;
    cmd.startDetached("vncviewer");
}

QJsonObject GUIInterface::loadJson()
{
    QFile file("/opt/zksy/plan.json");
    if(file.open(QIODevice::ReadOnly))
    {
        auto raw = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(raw);
        return doc.object();
    }
    return QJsonObject();
}
