#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextCodec>
#include <qqml.h>
#include <QTimer>
#include <QtSql>
#include "Other/ShowImage.h"
#include "maincontrol.h"
#include "guiinterface.h"
#include "Other/frameprovider.h"
#include "global.h"
#include "Other/Logger.h"
#include "Other/mysignalspy.h"
#include "DBModels/dbmodels.h"
#include <signal.h>

int main(int argc, char *argv[])
{
    //解析命令行参数
    for(int i = 1; i < argc; i++)
    {
        auto argi = QString::fromStdString(argv[i]);
        if(argi.startsWith("--SaveCureImg="))
            g_runArgs.insert("SaveCureImg", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--BedVer="))
            g_runArgs.insert("BedVer", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--CamType="))
            g_runArgs.insert("CamType", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--EnableLeftSensor="))
            g_runArgs.insert("EnableLeftSensor", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--EnableRightSensor="))
            g_runArgs.insert("EnableRightSensor", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--LinkRealBot="))
            g_runArgs.insert("LinkRealBot", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--LinkToServer="))
            g_runArgs.insert("LinkToServer", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--EnableLog="))
            g_runArgs.insert("EnableLog", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--AutoForceThrehold="))
            g_runArgs.insert("AutoForceThrehold", argi.mid(argi.indexOf('=') + 1));
        else if(argi.startsWith("--EnableEncryption="))
            g_runArgs.insert("EnableEncryption", argi.mid(argi.indexOf('=') + 1));
        //默认值
        if(!g_runArgs.contains("SaveCureImg"))
            g_runArgs.insert("SaveCureImg", "true");
        if(!g_runArgs.contains("BedVer"))
            g_runArgs.insert("BedVer", "3");
        if(!g_runArgs.contains("CamType"))
            g_runArgs.insert("CamType", "orbbec");
        if(!g_runArgs.contains("EnableLeftSensor"))
            g_runArgs.insert("EnableLeftSensor", "false");
        if(!g_runArgs.contains("EnableRightSensor"))
            g_runArgs.insert("EnableRightSensor", "false");
        if(!g_runArgs.contains("LinkRealBot"))
            g_runArgs.insert("LinkRealBot", "true");
        if(!g_runArgs.contains("LinkToServer"))
            g_runArgs.insert("LinkToServer", "false");
        if(!g_runArgs.contains("EnableLog"))
            g_runArgs.insert("EnableLog", "true");
        if(!g_runArgs.contains("AutoForceThrehold"))
            g_runArgs.insert("AutoForceThrehold", "6,20");
        if(!g_runArgs.contains("EnableEncryption"))
            g_runArgs.insert("EnableEncryption", "true");
    }

    qRegisterMetaType<QSharedPointer<QImage>>("QSharedPointer<QImage>");
    if(g_runArgs["EnableLog"] == "true")
        Logger::initLog("log");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("zksy");
    QCoreApplication::setOrganizationDomain("zksy");

//    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    QApplication app(argc, argv);

    //机械臂时间同步
    QProcess cmd;
    cmd.start("bash -c ./sync_robot_time.sh");
    cmd.waitForFinished();
    //开启外置传感器喂数脚本
//    if(g_runArgs["EnableLeftSensor"] == "true" && g_runArgs["EnableRightSensor"] == "true")
//        cmd.start("bash -c ./start_force.sh");
    //加载配置文件
    Global global;
    Global::loadConfig();
    //建立本地数据库
    if(!Global::inidb())
        return -1;

    QQmlApplicationEngine engine;

    qmlRegisterType<UserManageModel>("UserManageModel", 1, 0, "UserManageModel");
    qmlRegisterType<PatientManageModel>("PatientManageModel", 1, 0, "PatientManageModel");
    qmlRegisterType<FormulaTplManageModel>("FormulaTplManageModel", 1, 0, "FormulaTplManageModel");
    qmlRegisterType<FormulaManageModel>("FormulaManageModel", 1, 0, "FormulaManageModel");
    qmlRegisterType<CureRecordModel>("CureRecordModel", 1, 0, "CureRecordModel");

    qmlRegisterType<AppointmentListModel>("AppointmentListModel", 1, 0, "AppointmentListModel");

    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QApplication::quit);

    engine.addImportPath(TaoQuickImportPath);
    engine.rootContext()->setContextProperty("taoQuickImagePath", TaoQuickImagePath);
    engine.rootContext()->setContextProperty("mainImageResoucePath", MainImageResourcePath);

    GUIInterface guiInterface;
//    MySignalSpy spy;
//    spy.setupDynamicConnections(&guiInterface);
    engine.rootContext()->setContextProperty("guiIntf", &guiInterface);

    ShowImage *CodeImage = new ShowImage();
    engine.rootContext()->setContextProperty("CodeImage",CodeImage);
    engine.addImageProvider(QLatin1String("CodeImg"), CodeImage->m_pImgProvider);

    //相机实时输出流
    QVector<QSharedPointer<FrameProvider>> fps;
    for(int i = 0; i < 4; i++)
    {
        QSharedPointer<FrameProvider> np(new FrameProvider);
        fps.append(np);
    }
    if(g_cameraOrder.contains('m')) engine.rootContext()->setContextProperty("fp_m", fps[g_cameraOrder.indexOf('m')].data());
    if(g_cameraOrder.contains('h')) engine.rootContext()->setContextProperty("fp_h", fps[g_cameraOrder.indexOf('h')].data());
    if(g_cameraOrder.contains('l')) engine.rootContext()->setContextProperty("fp_l", fps[g_cameraOrder.indexOf('l')].data());
    if(g_cameraOrder.contains('r')) engine.rootContext()->setContextProperty("fp_r", fps[g_cameraOrder.indexOf('r')].data());

    QVector<QSharedPointer<FrameProvider>> fps_shadow;
    for(int i = 0; i < 4; i++)
    {
        QSharedPointer<FrameProvider> np(new FrameProvider);
        fps_shadow.append(np);
    }
    if(g_cameraOrder.contains('m')) engine.rootContext()->setContextProperty("fp_m_shadow", fps_shadow[g_cameraOrder.indexOf('m')].data());
    if(g_cameraOrder.contains('h')) engine.rootContext()->setContextProperty("fp_h_shadow", fps_shadow[g_cameraOrder.indexOf('h')].data());
    if(g_cameraOrder.contains('l')) engine.rootContext()->setContextProperty("fp_l_shadow", fps_shadow[g_cameraOrder.indexOf('l')].data());
    if(g_cameraOrder.contains('r')) engine.rootContext()->setContextProperty("fp_r_shadow", fps_shadow[g_cameraOrder.indexOf('r')].data());

    const QUrl url(QStringLiteral("qrc:/NewUISet/Entry.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    auto mainctrl = new MainControl;
    mainctrl->setfp(fps, fps_shadow);


    QObject::connect(mainctrl, &MainControl::serverLoginOK, &guiInterface, &GUIInterface::serverLoginOK);
    QObject::connect(mainctrl, &MainControl::updateAppList, &guiInterface, &GUIInterface::updateAppList);
    QObject::connect(&guiInterface, &GUIInterface::uploadCureRecord, mainctrl, &MainControl::uploadCureRecord);

    QObject::connect(&guiInterface, &GUIInterface::iniTimersSignal, mainctrl, &MainControl::iniTimers);
    QObject::connect(&guiInterface, &GUIInterface::startIniDevSignal, mainctrl, &MainControl::iniControls);

    QObject::connect(mainctrl, &MainControl::showWarningMsg, &guiInterface, &GUIInterface::showWarningMsg);
    QObject::connect(mainctrl, &MainControl::confirmMsgL, &guiInterface, &GUIInterface::confirmMsgL);
    QObject::connect(mainctrl, &MainControl::confirmMsgR, &guiInterface, &GUIInterface::confirmMsgR);
    QObject::connect(mainctrl, &MainControl::confirmMsgCore, &guiInterface, &GUIInterface::confirmMsgCore);
    QObject::connect(mainctrl, &MainControl::getOpenFile, &guiInterface, &GUIInterface::getOpenFile, Qt::BlockingQueuedConnection);

    //非GUI线程更新ui只能通过gui线程内c++对象
    QObject::connect(&guiInterface, &GUIInterface::guiCallSig, mainctrl, &MainControl::onGUICall);
    QObject::connect(mainctrl, &MainControl::iniForCareVersion, &guiInterface, &GUIInterface::iniForCareVersion);
    QObject::connect(mainctrl, &MainControl::switchPoseOK, &guiInterface, &GUIInterface::switchPoseOK);
    QObject::connect(mainctrl, &MainControl::identifyOK, &guiInterface, &GUIInterface::identifyOK);
    QObject::connect(mainctrl, &MainControl::cureOK, &guiInterface, &GUIInterface::cureOK);
    QObject::connect(mainctrl, &MainControl::pauseTreatmentOK, &guiInterface, &GUIInterface::pauseTreatmentOK);
    QObject::connect(mainctrl, &MainControl::sigUpdateProgress, &guiInterface, &GUIInterface::sigUpdateProgress);
    QObject::connect(mainctrl, &MainControl::updateBedStat, &guiInterface, &GUIInterface::updateBedStat);
    QObject::connect(mainctrl, &MainControl::updateRbPos1, &guiInterface, &GUIInterface::updateRbPos1);
    QObject::connect(mainctrl, &MainControl::updateRbPos2, &guiInterface, &GUIInterface::updateRbPos2);
    QObject::connect(mainctrl, &MainControl::updateRbStatus, &guiInterface, &GUIInterface::updateRbStatus);
    QObject::connect(mainctrl, &MainControl::blockUI, &guiInterface, &GUIInterface::blockUI);
    QObject::connect(mainctrl, &MainControl::switchDevSta, &guiInterface, &GUIInterface::switchDevSta);

    QObject::connect(mainctrl, &MainControl::refreshCodeImg, CodeImage, &ShowImage::initData);

    //开始走加载进度 初始化所有设备
    QThread mainT;
    mainctrl->moveToThread(&mainT);
    QObject::connect(&mainT, &QThread::finished, mainctrl, &QObject::deleteLater);
    mainT.start();
    guiInterface.iniTimers();
    guiInterface.startIniDev();

    signal(SIGTERM, [](int){
        qApp->exit(-11);
    });


    int ret = app.exec();
    if(ret == 999)
    {
        QProcess::startDetached(qApp->applicationFilePath(), QStringList());
        return 0;
    }
    else if(ret == -11)
    {
        LOG_INFO << "shutdown...";
        guiInterface.guiCall({{"func", "resetRbPos"}});
        QThread::sleep(10);
        mainT.quit();
        mainT.wait();
    }

    return ret;
}
