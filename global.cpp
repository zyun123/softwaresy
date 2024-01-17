#include "global.h"
#include <QDebug>
#include <QFile>
#include <QTextCodec>

QString g_sysVer = "V1.0.0";
QString g_deviceID = "sy3";
QString g_cacheUserID = "";
QString g_cachePWD = "";
QMap<QString, QString> g_runArgs;
QString g_ServerIP = QString("127.0.0.1");
int g_ServerPort = 12345;
QString g_HostIP = QString("192.168.1.100");
quint16 g_Host1Port = 8081;
quint16 g_Host2Port = 8080;
QString g_RobotleftIP = QString("192.168.1.102");
QString g_RobotrightIP = QString("192.168.1.101");
quint16 g_RobotDashPort = 29999;
quint16 g_RobotCommPort = 30002;
quint16 g_RobotRTPort = 30003;
QString g_BedIP = "192.168.1.22";
quint16 g_BedPort = 1234;
int g_cameraNum = 0;
QStringList g_cameraSerialNums = {};
QString g_cameraOrder = "mhlr";
QString g_saveImgPath = "";

QSharedPointer<QJsonObject> g_JingLuoConfig = nullptr;
QSharedPointer<QJsonObject> g_JingLuoConfig_Body = nullptr;
QSharedPointer<QJsonObject> g_CureInsertsConfig = nullptr;
QSharedPointer<QJsonObject> g_CurePtsBiasConfig = nullptr;
QSharedPointer<QJsonObject> g_CurePoseConfig = nullptr;
QSharedPointer<QJsonObject> g_CureForceBiasConfig = nullptr;
QSharedPointer<QJsonObject> g_CurePtPauseConfig = nullptr;
QSharedPointer<QJsonObject> g_RepredictPtsConfig = nullptr;
QSharedPointer<QJsonObject> g_CureContactConfig = nullptr;
QSharedPointer<QJsonObject> g_CureContactConfig_Body = nullptr;
QSharedPointer<QJsonObject> g_CureForceOverrideConfig = nullptr;
QSharedPointer<QJsonObject> g_JingLuoCoverConfig = nullptr;
QSharedPointer<QJsonObject> g_CaliConfig = nullptr;
QSharedPointer<QJsonObject> g_CaliConfig_Manual = nullptr;
QSharedPointer<QJsonObject> g_CaliConfig_bed =nullptr;

QAtomicInteger<int> g_cureVersion = 0;
QAtomicInteger<bool> g_usingImg = false;
QAtomicInteger<int> g_cureForceVal = 3;
QAtomicInteger<bool> g_coreBlockFlag = false;

QAtomicInteger<bool> g_testEMC = false;
QAtomicInteger<int> g_testCali = 0;
double g_testSoftProtect = 25;
double g_testIniBedPos = -0.3;

QSqlDatabase g_db;

Global::Global()
{   
    //读取ini
    QSettings settingsread("system.ini",QSettings::IniFormat);
    settingsread.setIniCodec(QTextCodec::codecForName("utf-8"));

    g_sysVer = settingsread.value("DEVICE/SYS-VERSION").toString();
    g_deviceID = settingsread.value("DEVICE/DEVICE-ID").toString();
    g_cacheUserID = settingsread.value("DEVICE/CACHE-USER").toString();
    g_cachePWD = settingsread.value("DEVICE/CACHE-PWD").toString();
    g_HostIP = settingsread.value("IP-HOST/IP").toString();
    g_Host1Port = settingsread.value("IP-HOST/PORTLEFT").toUInt();
    g_Host2Port = settingsread.value("IP-HOST/PORTRIGHT").toUInt();
    g_RobotleftIP = settingsread.value("IP-ROBOTS/LEFTIP").toString();
    g_RobotrightIP = settingsread.value("IP-ROBOTS/RIGHTIP").toString();
    g_RobotDashPort = settingsread.value("IP-ROBOTS/DASHPORT").toUInt();
    g_RobotCommPort = settingsread.value("IP-ROBOTS/COMMPORT").toUInt();
    g_RobotRTPort = settingsread.value("IP-ROBOTS/REALTIMEPORT").toUInt();

    g_BedIP = settingsread.value("IP-BED/IP").toString();
    g_BedPort = settingsread.value("IP-BED/PORT").toUInt();

    g_ServerIP = settingsread.value("IP-SERVER/IP").toString();
    g_ServerPort = settingsread.value("IP-SERVER/PORT").toInt();

    g_cameraNum = settingsread.value("Camera-Arg/CameraNum").toInt();
    QString tmpstr = settingsread.value("Camera-Arg/SerialNums").toString();
    g_cameraSerialNums = tmpstr.split(".");
    g_cameraOrder = settingsread.value("Camera-Arg/CameraOrder").toString();
    g_saveImgPath = settingsread.value("Camera-Arg/SaveImgPath").toString();

    g_testSoftProtect = settingsread.value("TEST/SOFTPROTECT").toDouble();
    g_testIniBedPos = settingsread.value("TEST/INIBED").toDouble();

    LOG_INFO << "system settings loaded";
}

void Global::loadConfig()
{
    auto readfile = [](QSharedPointer<QJsonObject> &cfg, QString path){
        QFile file(path);
        if(file.open(QIODevice::ReadOnly))
        {
            QJsonDocument jsondoc = QJsonDocument::fromJson(file.readAll());
            cfg = QSharedPointer<QJsonObject>(new QJsonObject(jsondoc.object()));
            file.close();
        }
    };

    readfile(g_JingLuoConfig, "config/jingluo_config_full.json");
    readfile(g_JingLuoConfig_Body, "config/jingluo_config_body.json");
    readfile(g_CureInsertsConfig, "config/cure_insertpts.json");
    readfile(g_CurePtsBiasConfig, "config/cure_bias.json");
    readfile(g_CureForceBiasConfig, "config/cure_forcebias.json");
    readfile(g_CurePtPauseConfig, "config/cure_pause.json");
    readfile(g_RepredictPtsConfig, "config/cure_repredict.json");
    readfile(g_CureContactConfig, "config/cure_contact_full.json");
    readfile(g_CureContactConfig_Body, "config/cure_contact_body.json");
    readfile(g_CurePoseConfig, "config/cure_pose.json");
    readfile(g_CureForceOverrideConfig, "config/cure_force_override.json");
    readfile(g_JingLuoCoverConfig, "config/jingluo_coverup.json");
    readfile(g_CaliConfig, "config/cali_bias.json");
    readfile(g_CaliConfig_Manual, "config/cali_bias_manual.json");
    readfile(g_CaliConfig_bed, "config/cali_bed.json");
}

void Global::setSysValue(QString key, QString val)
{
    QSettings settingsread("system.ini",QSettings::IniFormat);
    settingsread.setValue(key, val);
}

bool Global::inidb()
{
    QFile dbfile("sql.db");
    auto created = dbfile.exists();
    g_db = QSqlDatabase::addDatabase("QSQLITE");
    g_db.setDatabaseName("sql.db");
    if(!g_db.open())
    {
        LOG_EXCEPTION_SYS << g_db.lastError();
        return false;
    }
    if(!created)
    {
        QSqlQuery q;
        // create user_info table
        q.prepare(QLatin1String(R"(
                                CREATE TABLE "user_info" (
                                    "userid"	TEXT NOT NULL UNIQUE,
                                    "password"	TEXT NOT NULL,
                                    "name"	TEXT NOT NULL,
                                    "role"	TEXT NOT NULL,
                                    "music"  REAL NOT NULL,
                                    "volume"  REAL NOT NULL,
                                    "screen_lock"  INTEGER NOT NULL,
                                    PRIMARY KEY("userid")
                                ))"));
        q.exec();
        q.prepare(QLatin1String(R"(insert into user_info(userid, password, name, role, music, volume, screen_lock) values(?, ?, ?, ?, ?, ?, ?))"));
        q.addBindValue("admin1");
        q.addBindValue("sy123456");
        q.addBindValue("admin");
        q.addBindValue("admin");
        q.addBindValue(1);
        q.addBindValue(1);
        q.addBindValue(1);
        q.exec();
        q.addBindValue("root1");
        q.addBindValue("sy123456");
        q.addBindValue("root");
        q.addBindValue("root");
        q.addBindValue(1);
        q.addBindValue(1);
        q.addBindValue(1);
        q.exec();
        // create patient_info table
        q.prepare(QLatin1String(R"(
                                CREATE TABLE "patient_info" (
                                    "name"	TEXT NOT NULL,
                                    "idcard"	TEXT NOT NULL UNIQUE,
                                    "gender"	TEXT,
                                    "age"	INTEGER,
                                    "phone"	TEXT NOT NULL UNIQUE,
                                    "creator"	TEXT NOT NULL,
                                    "create_time"	TEXT NOT NULL,
                                    "note"	TEXT,
                                    PRIMARY KEY("idcard")
                                ))"));
        q.exec();
        // create formula_tpl table
        q.prepare(QLatin1String(R"(
                                CREATE TABLE "formula_tpl" (
                                    "name"	TEXT NOT NULL UNIQUE,
                                    "meridians"	TEXT,
                                    "name_wtf"	TEXT,
                                    "detail"	TEXT,
                                    PRIMARY KEY("name")
                                ))"));
        q.exec();
        // create formula table
        q.prepare(QLatin1String(R"(
                                CREATE TABLE "formula" (
                                    "serial_num"	TEXT NOT NULL,
                                    "patient_id"	TEXT NOT NULL,
                                    "diagnosis"	TEXT,
                                    "meridians"	TEXT NOT NULL,
                                    "cure_mode"	TEXT NOT NULL,
                                    "cure_force"	TEXT NOT NULL,
                                    "cure_times"	TEXT NOT NULL,
                                    "note"	TEXT,
                                    "creator"	TEXT NOT NULL,
                                    "create_time"	TEXT NOT NULL,
                                    "formula_mod"	TEXT NOT NULL,
                                    PRIMARY KEY("serial_num")
                                ))"));
        q.exec();
        // create cure_record table
        q.prepare(QLatin1String(R"(
                                CREATE TABLE "cure_record" (
                                    "serial_num"	TEXT NOT NULL,
                                    "phone"	TEXT,
                                    "patient_name"	TEXT,
                                    "operator_id"	TEXT NOT NULL,
                                    "meridians"	TEXT NOT NULL,
                                    "start_time"	TEXT NOT NULL,
                                    "end_time"	TEXT NOT NULL,
                                    "dur_time"	TEXT NOT NULL,
                                    "times"	    INTEGER NOT NULL,
                                    "device_id"	TEXT NOT NULL,
                                    PRIMARY KEY("serial_num")
                                ))"));
        q.exec();
    }
    return true;
}
