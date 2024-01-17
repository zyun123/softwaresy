#include "maincontrol.h"
#include <QDebug>
#include <QTimer>
#include <math.h>
#include <QVector>
#include <QPixmap>
#include <QFile>
#include <QJsonArray>
#include <thread>
#include <QDateTime>
#include <QAbstractButton>
#include "Other/Logger.h"

MainControl::MainControl(QObject *parent) : QObject(parent)
{
    m_pauseLock = false;
    m_isSwitchingPos = false;
    //创建临时图片文件夹
    QProcess cmd;
    cmd.start("mkdir", {"cache"});
    cmd.waitForFinished();
    cmd.start("mkdir", {"saveimgs"});
    cmd.waitForFinished();
    cmd.start("mkdir", {"extrinsics"});
    cmd.waitForFinished();
    //初始化识别和调理界面加载图片
    QPixmap tmpImg(400, 300);
    tmpImg.save("cache/camera_identify_m.jpg");
    tmpImg.save("cache/camera_identify_l.jpg");
    tmpImg.save("cache/camera_identify_r.jpg");
    tmpImg.save("cache/camera_identify_h.jpg");

    m_camprocess = new QProcess(this);
    m_camProcessRestartFlag = true;
    connect(m_camprocess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus){
        if(m_camProcessRestartFlag)
        {
            LOG_EXCEPTION_CAM << "cam process crashed: " << exitCode << " " << exitStatus << " restarting...";
            m_startCamPTimer->start();
        }
    });
}

MainControl::~MainControl()
{
    m_camProcessRestartFlag = false;
    m_camprocess->close();
    for(int i = 0; i < m_camGetThreads.size(); i++)
    {
        m_camGetThreads[i]->requestInterruption();
        m_camGetThreads[i]->quit();
        m_camGetThreads[i]->wait();
    }
    m_coreProThread.quit();
    m_coreProThread.wait();
}

void MainControl::onGUICall(QJsonObject callinfo)
{
    QString func = callinfo.value("func").toString();
    QString arg_str = "";
    if(callinfo.contains("arg_str"))
        arg_str = callinfo.value("arg_str").toString();
    QJsonObject arg_obj;
    if(callinfo.contains("arg_obj"))
        arg_obj = callinfo.value("arg_obj").toObject();
    if(func == "restartApp") restartApp();
    else if(func == "recognizeJingLuo") recognizeJingLuo(arg_str);
    else if(func == "startTreatment")
    {
        auto list = arg_str.split("|");
        bool tmp;
        if(list[1] == "true")
            tmp = true;
        else
            tmp = false;
        startTreatment(list[0], tmp);
    }
    else if(func == "pauseTreatment")
    {
        if(m_pauseLock)
            return;
        m_pauseLock = true;
        std::thread t(&MainControl::pauseTreatment, this, true);
        t.detach();
    }
    else if(func == "switchPose") switchPose();
    else if(func == "freeDrive")
    {
        auto list = arg_str.split("|");
        freeDrive(list[0].toInt(), list[1] == "true" ? true : false);
    }
    else if(func == "robotPlay") robotPlay(arg_str.toInt());
    else if(func == "robotStop") robotStop(arg_str.toInt());
    else if(func == "safePos") safePos(arg_str.toInt());
    else if(func == "photoPos") photoPos(arg_str.toInt());
    else if(func == "robotsStop") robotsStop();
    else if(func == "robotsPause") robotsPause();
    else if(func == "robotsStart") robotsStart();
    else if(func == "oneBtnStart") oneBtnStart();
    else if(func == "endForce") endForce(arg_str.toInt());
    else if(func == "clearForce") clearForce(arg_str.toInt());
    else if(func == "reduce") reduce(arg_str.toInt());
    else if(func == "unlockProtectiveStop") unlockProtectiveStop(arg_str.toInt());
    else if(func == "powerOn") powerOn(arg_str.toInt());
    else if(func == "closePopUp") closePopUp(arg_str.toInt());
    else if(func == "movej")
    {
        auto list = arg_str.split("|");
        movej(list[0].toInt(), list[1].toInt(), list[2]);
    }
    else if(func == "movel")
    {
        auto list = arg_str.split("|");
        movel(list[0].toInt(), list[1]);
    }
    else if(func == "speedl")
    {
        auto list = arg_str.split("|");
        speedl(list[0].toInt(), list[1]);
    }
    else if(func == "speedj")
    {
        auto list = arg_str.split("|");
        speedj(list[0].toInt(), list[1]);
    }
    else if(func == "stopl")
    {
        auto list = arg_str.split("|");
        stopl(list[0].toInt(), list[1]);
    }
    else if(func == "stopj")
    {
        auto list = arg_str.split("|");
        stopj(list[0].toInt(), list[1]);
    }
    else if(func == "contact")
    {
        auto list = arg_str.split("|");
        contact(list[0].toInt(), list[1]);
    }
    else if(func == "rawCommand")
    {
        auto list = arg_str.split("|");
        rawCommand(list[0].toInt(), list[1]);
    }
    else if(func == "mvbed") mvbed(arg_str.toInt());
    else if(func == "vibrateOff") vibrateOff();
    else if(func == "viboff") viboff(arg_str.toInt());
    else if(func == "vibrateOn") vibrateOn();
    else if(func == "vibon") vibon(arg_str.toInt());
    else if(func == "heatingOff") heatingOff();
    else if(func == "heatingOn") heatingOn();
    else if(func == "calibrateStart") calibrateStart(arg_obj);
    else if(func == "checkCaliStart")
    {
        auto list = arg_str.split("|");
        checkCaliStart(list[0].toInt(), list[1]);
    }
    else if(func == "goUpDown")
    {
        auto list = arg_str.split("|");
        bool tmp;
        if(list[1] == "true")
            tmp = true;
        else
            tmp = false;
        goUpDown(list[0].toInt(), tmp, list[2]);
    }
    else if(func == "onPanelRBStateRefresh") onPanelRBStateRefresh(arg_str.toInt());
    else if(func == "pauseCamStream") pauseCamStream();
    else if(func == "goToEulerAngle")
    {
        auto list = arg_str.split("|");
        goToEulerAngle(list[0].toInt(), list[1]);
    }
    else if(func == "updateBedPos")
    {
        BedStatus stat;
        std::tie(stat, std::ignore) = m_bedCtrl->getBedStatus();
        emit updateBedStat(stat.pos * 10000);
    }
    else if(func == "switchFakeIdentify")switchFakeIdentify();
    else if(func == "switchUsing3D")switchUsing3D();
    else if(func == "camshot")camshot(arg_str);
    else if(func == "camshotInDiffPos")camshotInDiffPos(arg_str);
    else if(func == "setDampAndGain")
    {
        auto list = arg_str.split("|");
        auto sublist = list[1].split(",");
        m_robots[list[0].toInt()]->setDamp(sublist[0].toDouble());
        m_robots[list[0].toInt()]->setGain(sublist[1].toDouble());
    }
    else if(func == "comPlot")
    {
        m_robots[0]->comPlot();
        m_robots[1]->comPlot();
    }
    else if(func == "caliSensor") emit startCaliSensor(arg_str.toInt());
    else if(func == "flushLog")
    {
        if(g_runArgs["EnableLog"] == "true")
            Logger::flushCurLog();
    }
    else if(func == "callOnDiagnosis")
    {
        QProcess cmd;
        cmd.start("killall opencv_example_sweeper-x86_64.AppImage");
        cmd.waitForFinished();
        cmd.start("killall opencv_example_sweeper-x86_64.AppImage");
        cmd.waitForFinished();
        cmd.startDetached("./diagnosis/diagnosis.sh");
        cmd.waitForFinished();
    }
    else if (func == "callOnQA") {
        QProcess cmd;
        cmd.startDetached("firefox http://localhost:8088");
        cmd.waitForFinished();
    }
    else if(func == "login"){
        if(g_runArgs["LinkToServer"] == "true")
        {
            auto list = arg_str.split("|");
            QJsonObject sndobj;
            sndobj["table"] = "user_info";
            sndobj["type"] = "login";
            sndobj["user_id"] = list[0];
            sndobj["password"] = list[1];
            auto sndstr = QJsonDocument(sndobj).toBinaryData();
            m_dbclient->addSndMsg(sndstr);
        }
    }
    else if(func == "iniRedPoint")
        emit sig_iniRedPoint();
    else if(func == "recordRedPoint")
        emit sig_recordRedPoint(g_cameraOrder.indexOf(arg_obj["camid"].toString()),
                arg_obj["pix_x"].toDouble(), arg_obj["pix_y"].toDouble(), arg_obj["robotid"].toInt(), arg_obj["preserve"].toString().toDouble());
    else if(func == "goToRedPoint")
    {
        m_coreProWorker->resetStopTreat();
        emit sig_goToRedPoint(arg_obj["contact_mod"].toInt());
    }
    else if(func == "genCaliBias")
        emit sig_genCaliBias();
    else if(func == "releaseRbLeftBlock"){
        m_robots[0]->onConfirm();
    }
    else if(func == "releaseRbRightBlock"){
        m_robots[1]->onConfirm();
    }
    else if(func == "releaseCoreBlock"){
        g_coreBlockFlag = false;
    }
}

void MainControl::camshot(QString zishi)
{
    auto ts = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
    for(int i = 0; i < g_cameraNum; i++)
    {
        auto rgb_dat = m_cameras[i]->getBGRData();
        auto depth_dat = m_cameras[i]->getDepthData();
        CameraControl::savePic(g_saveImgPath + "saveimgs/camshot/" + zishi + "/" + QString(g_cameraOrder[i]) + "/color/",
                               QString(g_cameraOrder[i]) + "_" + zishi + "_" + ts + ".jpg", rgb_dat, CV_8UC3);
        CameraControl::savePic(g_saveImgPath + "saveimgs/camshot/" + zishi + "/" + QString(g_cameraOrder[i]) + "/depth/",
                               QString(g_cameraOrder[i]) + "_" + zishi + "_" + ts + ".png", depth_dat, CV_16UC1);
    }
    emit showWarningMsg("图片采集", "图片采集成功");
}

void MainControl::camshotInDiffPos(QString zishi)
{
    auto shot = [=](QString bedpos){
        auto ts = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
        for(int i = 0; i < g_cameraNum; i++)
        {
            auto rgb_dat = m_cameras[i]->getBGRData();
            auto depth_dat = m_cameras[i]->getDepthData();
            CameraControl::savePic(g_saveImgPath + "saveimgs/camshot/" + zishi + "/" + QString(g_cameraOrder[i]) + "/color/",
                                   QString(g_cameraOrder[i]) + "_" + zishi + "_" + ts + "_" + bedpos + ".jpg", rgb_dat, CV_8UC3);
            CameraControl::savePic(g_saveImgPath + "saveimgs/camshot/" + zishi + "/" + QString(g_cameraOrder[i]) + "/depth/",
                                   QString(g_cameraOrder[i]) + "_" + zishi + "_" + ts + "_" + bedpos + ".png", depth_dat, CV_16UC1);
        }
    };
    emit blockUI("采集中...", 0);
    double pos = 0.2;
    while(pos >= -0.4)
    {
        m_bedCtrl->moveTo(pos, 1);
        QThread::msleep(500);
        shot(QString::number((int)(pos * 10000)));
        pos -= 0.05;
    }
    emit blockUI("采集完毕", 3000);
}

void MainControl::onPanelRBStateRefresh(int index)
{
    auto stat = m_robots[index]->getStatus();
    auto pose = Global::fromArray(stat.toolVectorActual, 6);
    QString msg = "";
    for(int i = 0; i < 5; i++)
        msg += QString::number(pose[i], 'g', 3) + ",";
    msg += QString::number(pose[5], 'g', 3);
    auto jointdat = Global::fromArray(stat.qActual, 6);
    QString msg1 = "";
    for(int i = 0; i < 5; i++)
        msg1 += QString::number(jointdat[i], 'g', 3) + ",";
    msg1 += QString::number(jointdat[5], 'g', 3);
    emit updateRbPos1(index, msg);
    emit updateRbPos2(index, msg1);
    QJsonObject uistat;
    switch ((int)(stat.robotMode)) {
    case -1:
        uistat.insert("robotmode", "ROBOT_MODE_NO_CONTROLLER");
        break;
    case 0:
        uistat.insert("robotmode", "ROBOT_MODE_DISCONNECTED");
        break;
    case 1:
        uistat.insert("robotmode", "ROBOT_MODE_CONFIRM_SAFETY");
        break;
    case 2:
        uistat.insert("robotmode", "ROBOT_MODE_BOOTING");
        break;
    case 3:
        uistat.insert("robotmode", "ROBOT_MODE_POWER_OFF");
        break;
    case 4:
        uistat.insert("robotmode", "ROBOT_MODE_POWER_ON");
        break;
    case 5:
        uistat.insert("robotmode", "ROBOT_MODE_IDLE");
        break;
    case 6:
        uistat.insert("robotmode", "ROBOT_MODE_BACKDRIVE");
        break;
    case 7:
        uistat.insert("robotmode", "ROBOT_MODE_RUNNING");
        break;
    case 8:
        uistat.insert("robotmode", "ROBOT_MODE_UPDATING_FIRMWARE");
        break;
    default:
        uistat.insert("robotmode", "unknown value");
    }
    switch ((int)(stat.safetyStatus)) {
    case 13:
        uistat.insert("safetystatus", "SAFETY_STATUS_SYSTEM_THREE_POSITION_ENABLING_STOP");
        break;
    case 12:
        uistat.insert("safetystatus", "SAFETY_STATUS_AUTOMATIC_MODE_SAFEGUARD_STOP");
        break;
    case 11:
        uistat.insert("safetystatus", "SAFETY_STATUS_UNDEFINED_SAFETY_MODE");
        break;
    case 10:
        uistat.insert("safetystatus", "SAFETY_STATUS_VALIDATE_JOINT_ID");
        break;
    case 9:
        uistat.insert("safetystatus", "SAFETY_STATUS_FAULT");
        break;
    case 8:
        uistat.insert("safetystatus", "SAFETY_STATUS_VIOLATION");
        break;
    case 7:
        uistat.insert("safetystatus", "SAFETY_STATUS_ROBOT_EMERGENCY_STOP");
        break;
    case 6:
        uistat.insert("safetystatus", "SAFETY_STATUS_SYSTEM_EMERGENCY_STOP");
        break;
    case 5:
        uistat.insert("safetystatus", "SAFETY_STATUS_SAFEGUARD_STOP");
        break;
    case 4:
        uistat.insert("safetystatus", "SAFETY_STATUS_RECOVERY");
        break;
    case 3:
        uistat.insert("safetystatus", "SAFETY_STATUS_PROTECTIVE_STOP");
        break;
    case 2:
        uistat.insert("safetystatus", "SAFETY_STATUS_REDUCED");
        break;
    case 1:
        uistat.insert("safetystatus", "SAFETY_STATUS_NORMAL");
        break;
    default:
        uistat.insert("safetystatus", "unknown value");
    }
    QString tcpforce = "";
    for(int i = 0; i < 5; i++)
        tcpforce += QString::number(stat.tcpForce[i], 'g', 3) + ",";
    tcpforce += QString::number(stat.tcpForce[5], 'g', 3);
    uistat.insert("tcpforce", tcpforce);
    uistat.insert("programSta", QString::number(stat.programState));
    emit updateRbStatus(index, uistat);
}

void MainControl::iniTimers()
{
    //启动相机进程定时器
    m_startCamPTimer = new QTimer();
    connect(m_startCamPTimer, &QTimer::timeout, this, [this](){
        m_camprocess->start(QString("./camprocess/cam_process-") + g_runArgs["CamType"]);
        m_startCamPTimer->stop();
    });
    m_startCamPTimer->setInterval(3000);
}

void MainControl::iniControls()
{
    emit sigUpdateProgress(0.3);
    //检查设备是否都已连接
    auto testip = [=](QString ip)->bool{
        QProcess cmd;
        cmd.start("ping", {ip, "-c", "1", "-w", "1"});
        cmd.waitForReadyRead(1000);
        cmd.waitForFinished(1000);

        QString res = cmd.readAll();
        if (res.indexOf("ttl") == -1)
            return false;
        else
            return true;
    };
    if(g_runArgs["LinkRealBot"] == "true")
    {
        while(!(testip(g_BedIP) && testip(g_RobotrightIP) && testip(g_RobotleftIP)))
        {
            emit blockUI("设备ip不在同一网段内，请检查网络，15S后再次尝试连接...", 0);
            QThread::msleep(14500);
            emit blockUI("", 500);
        }
    }
    //设备控制对象初始化
    auto t1 = QDateTime::currentMSecsSinceEpoch();
    for(int i = 0; i < 2; i++)
        m_robots.append(QSharedPointer<RobotControl>(new RobotControl(i)));
    for(int i = 0; i < 2; i++)
    {
        connect(m_robots[i].data(), &RobotControl::warn, this, &MainControl::showWarningMsg);
        connect(m_robots[i].data(), &RobotControl::informPauseTreatment, this, &MainControl::onPause);
        connect(m_robots[i].data(), &RobotControl::switchDevSta, this, &MainControl::switchDevSta);
        connect(m_robots[i].data(), &RobotControl::blockUI, this, &MainControl::blockUI);
    }
    connect(m_robots[0].data(), &RobotControl::warn_b, this, &MainControl::confirmMsgL);
    connect(m_robots[1].data(), &RobotControl::warn_b, this, &MainControl::confirmMsgR);
    m_robots[0]->iniCon();
    m_robots[1]->iniCon();
    m_bedCtrl = QSharedPointer<BedControl>(new BedControl);
    connect(m_bedCtrl.data(), &BedControl::warn, this, &MainControl::showWarningMsg);
    connect(m_bedCtrl.data(), &BedControl::informPauseTreatment, this, &MainControl::onPause);
    connect(m_bedCtrl.data(), &BedControl::mvover, this, &MainControl::deal_mvover);
    connect(m_bedCtrl.data(), &BedControl::switchDevSta, this, &MainControl::switchDevSta);
    m_bedCtrl->iniCon();
    for(int i = 0; i < g_cameraNum; i++)
    {
        m_cameras.append(QSharedPointer<CameraControl>(new CameraControl(g_cameraSerialNums[i], i)));
        connect(m_cameras[i].data(), &CameraControl::frameArrived, m_fp[i].data(), &FrameProvider::onNewVideoContentReceived, Qt::BlockingQueuedConnection);
        connect(m_cameras[i].data(), &CameraControl::frameArrived_s, m_fp_shadow[i].data(), &FrameProvider::onNewVideoContentReceived, Qt::BlockingQueuedConnection);
        connect(m_cameras[i].data(), &CameraControl::getOpenFile, this, &MainControl::getOpenFile, Qt::BlockingQueuedConnection);
        connect(m_cameras[i].data(), &CameraControl::switchDevSta, this, &MainControl::switchDevSta);
    }
    for(int i = 0; i < m_cameras.size(); i++)
        m_cameras[i]->inishm();
    auto t2 = QDateTime::currentMSecsSinceEpoch();
    LOG_INFO << "设备连接耗时：" << t2 - t1;
    //清除可能的残余进程 TODO 暂可能需要2个相机来回切换
    CameraControl::restartCamProcess();
    //启动相机进程
    LOG_INFO << "starting cam_process...";
    m_camprocess->start(QString("./camprocess/cam_process-") + g_runArgs["CamType"]);
    //初始化调理床监控相机
    if(g_cameraOrder.contains('h'))
        m_bedCtrl->iniHeadCam(m_cameras[g_cameraOrder.indexOf('h')]);
    //子线程初始化
    //相机获取照片子线程 为节省线程资源只开一个线程读取四个相机的数据
    m_camGetWorkers.append(new CameraGetDataWorker(m_cameras));
    m_camGetThreads.append(new QThread);
    m_camGetWorkers[0]->moveToThread(m_camGetThreads[0]);
    connect(m_camGetThreads[0], &QThread::finished, m_camGetWorkers[0], &QObject::deleteLater);
    connect(this, &MainControl::setCamThreadStart, m_camGetWorkers[0], &CameraGetDataWorker::getAllImages);
    m_camGetThreads[0]->start();
    emit setCamThreadStart();
    //核心事务子线程
    m_coreProWorker = new CoreProcedureWorker(m_cameras, m_robots, m_bedCtrl);
    m_coreProWorker->moveToThread(&m_coreProThread);
    connect(&m_coreProThread, &QThread::finished, m_coreProWorker, &QObject::deleteLater);
    connect(this, &MainControl::startIdentify, m_coreProWorker, &CoreProcedureWorker::identifyJingLuo);
    connect(m_coreProWorker, &CoreProcedureWorker::identifyFinished, this, &MainControl::identifyOK);
    connect(m_coreProWorker, &CoreProcedureWorker::informPauseTreatment, this, &MainControl::onPause);
    connect(m_coreProWorker, &CoreProcedureWorker::warn, this, &MainControl::showWarningMsg);
    connect(m_coreProWorker, &CoreProcedureWorker::blockUI, this, &MainControl::blockUI);
    connect(m_coreProWorker, &CoreProcedureWorker::infromresetCams, this, &MainControl::resetCamStream);
    connect(this, &MainControl::startCure, m_coreProWorker, &CoreProcedureWorker::startCure);
    connect(m_coreProWorker, &CoreProcedureWorker::cureFinished, this, &MainControl::cureOK);
    connect(m_coreProWorker, &CoreProcedureWorker::refreshCodeImg, this, &MainControl::refreshCodeImg);
    connect(this, &MainControl::startCali, m_coreProWorker, &CoreProcedureWorker::goCalibrate);
    connect(this, &MainControl::startCaliSensor, m_coreProWorker, &CoreProcedureWorker::goCalibrateSensor);
    connect(this, &MainControl::checkCali, m_coreProWorker, &CoreProcedureWorker::goToChessBoard);
    connect(this, &MainControl::goUPAndDown, m_coreProWorker, &CoreProcedureWorker::upAndDown);
    connect(this, &MainControl::sig_genCaliBias, m_coreProWorker, &CoreProcedureWorker::genCaliBias);
    connect(this, &MainControl::sig_iniRedPoint, m_coreProWorker, &CoreProcedureWorker::iniRedPoint);
    connect(this, &MainControl::sig_recordRedPoint, m_coreProWorker, &CoreProcedureWorker::recordRedPoint);
    connect(this, &MainControl::sig_goToRedPoint, m_coreProWorker, &CoreProcedureWorker::goToRedPoint);
    connect(m_coreProWorker, &CoreProcedureWorker::warn_b, this, &MainControl::confirmMsgCore);
    m_coreProThread.start();
    //连接到服务器
    if(g_runArgs["LinkToServer"] == "true")
    {
        m_dbclient = new BaseClient(g_ServerIP, g_ServerPort, 1024);
        connect(m_dbclient, &BaseClient::retMsg, this, [=](QByteArray rcvmsg){
            m_rcvcache += rcvmsg;
            if(m_rcvcache.contains("##"))
            {
                auto msg = m_rcvcache.left(m_rcvcache.indexOf("##"));
                auto doc = QJsonDocument::fromBinaryData(msg);
                auto obj = doc.object();
                if(obj["type"] == "login" && obj["table"] == "user_info")
                {
                    if(obj["result"] == "ok")
                    {
                        auto raw_role = obj["role"].toString();
                        emit serverLoginOK(true, raw_role == "管理员" ? "admin" : "operator");
                    }
                    else
                        emit serverLoginOK(false, "");
                }
                if(obj["type"] == "select_list_of_device" && obj["table"] == "appointment_record")
                    emit updateAppList(obj["data_list"].toArray());
                m_rcvcache.clear();
            }
        });
        m_dbclient->iniConnection();
        m_dbclient->iniHeartBeatMsg("heartbeat", "heartbeat_reply");
        m_dbclient->setRetSig(true);
        m_dbclient->moveToThread(&m_dbClientThread);
        m_dbClientThread.start();
        QJsonObject sndobj;
        sndobj["table"] = "appointment_record";
        sndobj["type"] = "select_list_of_device";
        sndobj["device_id"] = g_deviceID;
        auto sndstr = QJsonDocument(sndobj).toBinaryData();
        m_refreshTimer = new QTimer;
        m_refreshTimer->setInterval(10000);
        connect(m_refreshTimer, &QTimer::timeout, this, [=](){
            m_dbclient->addSndMsg(sndstr);
        });
        m_dbclient->addSndMsg(sndstr);
        m_refreshTimer->start();
    }
    emit sigUpdateProgress(0.5);

    if(g_runArgs["LinkRealBot"] == "true")
    {
        std::thread iniDevT(&MainControl::iniDevices, this);
        iniDevT.detach();
    }
    else
        emit sigUpdateProgress(1);
}

void MainControl::iniDevices()
{
    //初始化顺序机械臂->床 避免同时移动
    auto t1 = QDateTime::currentMSecsSinceEpoch();
    oneBtnStart();
    emit sigUpdateProgress(0.7);
    m_bedCtrl->iniDev();
    emit sigUpdateProgress(1);
    emit iniForCareVersion();
    auto t2 = QDateTime::currentMSecsSinceEpoch();
    LOG_INFO << "设备初始化耗时：" << t2 - t1;
}

void MainControl::pauseCamStream()
{
    m_camProcessRestartFlag = m_camProcessRestartFlag ? false : true;
    if(!m_camProcessRestartFlag)
        m_camprocess->close();
    else
    {
        for(int i = 0; i < m_cameras.size(); i++)
            m_cameras[i]->inishm();
        m_camprocess->start(QString("./camprocess/cam_process-") + g_runArgs["CamType"]);
    }
}

void MainControl::resetCamStream()
{
    m_camprocess->close();
}

void MainControl::startTreatment(QString meridian, bool autonext)
{
    if(!autonext)//手动点开始调理重置暂停调理状态
        m_coreProWorker->resetStopTreat();
    emit startCure(meridian);
}

void MainControl::recognizeJingLuo(QString args)
{
    QStringList arglist = args.split("|");
    if(arglist.size() == 2)
        emit startIdentify(arglist[0], arglist[1]);
}

void MainControl::switchFakeIdentify()
{
    g_usingImg = g_usingImg ? false : true;
}

void MainControl::switchUsing3D()
{
    m_coreProWorker->setUsing3D();
}

void MainControl::onPause()
{
    if(g_runArgs["LinkRealBot"] == "false")
        return;
    if(m_pauseLock)
        return;
    m_pauseLock = true;
    std::thread t(&MainControl::pauseTreatment, this, false);
    t.detach();
}

void MainControl::pauseTreatment(bool normal)
{
    if(g_runArgs["LinkRealBot"] == "false")
        return;
    LOG_INFO << "触发中断...";
    if(normal)
        emit blockUI("正在中止调理，请稍候...", 0);
    else
        emit blockUI("设备异常，正在恢复系统状态，请稍候...", 0);
    bool istreating = false;
    if(m_coreProWorker->isTreating())
    {
        istreating = true;
        LOG_INFO << "正在调理，退出调理线程...";
        m_coreProWorker->stopTreat();
    }
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->stopWaitMove();
    m_bedCtrl->stopWaitMove();
    while(m_coreProWorker->isTreating())
        QThread::msleep(100);
    if(istreating)
        LOG_INFO << "调理线程已退出";
    //重置等待回复标志
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->resetStopWaitMove();
    m_bedCtrl->resetStopWaitMove();
    //停止设备
    m_bedCtrl->stop();
    auto stopbot = [=](int rbindex){
        if(m_robots[rbindex]->getStatus().programState == 2)
        {
            m_robots[rbindex]->robotStop();
            m_robots[rbindex]->vibrateOff();
        }
    };
    std::thread t_stop1(stopbot, 0);
    std::thread t_stop2(stopbot, 1);
    t_stop1.join();
    t_stop2.join();
    emit blockUI("重置机械臂状态...", 0);
    Global::parallel<void, RobotControl>(&RobotControl::robotReset, m_robots[0], m_robots[1]);
    emit blockUI("clear", 0);
    emit blockUI("校验调理床连接状态...", 0);
    if(m_bedCtrl->isConnected() == false)
        emit blockUI("调理床已断开连接...", 0);
    while(!m_bedCtrl->isConnected())
        QThread::msleep(500);
    Global::parallel<void, RobotControl>(&RobotControl::vibrateOff, m_robots[0], m_robots[1]);
    emit blockUI("clear", 0);
    if(normal)
        emit blockUI("已完成", 1500);
    else
        emit blockUI("已恢复，请人工将机械臂抬起后，确认患者姿势正确并尝试重新识别调理", 3000);
    LOG_INFO << "中断已完成";
    emit pauseTreatmentOK();
    m_pauseLock = false;
}

void MainControl::switchPose()
{
    m_isSwitchingPos = true;
    if(!m_robots[0]->isInPhotoPos())
        m_robots[0]->safePos();
    if(!m_robots[1]->isInPhotoPos())
        m_robots[1]->safePos();
    Global::parallel<void, RobotControl, bool*>(&RobotControl::photoPos, m_robots[0], m_robots[1], nullptr);
    std::thread t(&BedControl::moveTo, m_bedCtrl, 0, 1, 60 * 1000);
    t.detach();
}

void MainControl::deal_mvover()
{
    if(m_isSwitchingPos)
    {
        emit switchPoseOK();
        m_isSwitchingPos = false;
    }
}

void MainControl::freeDrive(int index, bool open)
{
    m_robots[index]->freeDrive(open, true);
}

void MainControl::robotPlay(int index)
{
    m_robots[index]->robotPlay(true);
}

void MainControl::robotStop(int index)
{
    m_robots[index]->robotStop(true);
}

void MainControl::endForce(int index)
{
    m_robots[index]->endforce();
}

void MainControl::clearForce(int index)
{
    m_robots[index]->clearforce();
}

void MainControl::reduce(int index)
{
    m_robots[index]->reduce(true);
}

void MainControl::safePos(int index)
{
    std::thread t(&RobotControl::safePos, m_robots[index], nullptr);
    t.detach();
}

void MainControl::photoPos(int index)
{
    std::thread t(&RobotControl::photoPos, m_robots[index], nullptr);
    t.detach();
}

void MainControl::mvbed(int pos)
{
    m_bedCtrl->stopWaitMove();
    m_bedCtrl->stop();
    m_bedCtrl->resetStopWaitMove();
    if(pos >= -m_bedCtrl->m_move_threhold_head * 10000 && pos <= m_bedCtrl->m_move_threhold_tail * 10000)
    {
        std::thread t(&BedControl::moveTo, m_bedCtrl, pos / 10000.0, 1, 60 * 1000);
        t.detach();
    }
    LOG_INFO << "mvbed finished...";
}

void MainControl::robotsStop()
{
    Global::parallel<void, RobotControl>(&RobotControl::vibrateOff, m_robots[0], m_robots[1]);
    Global::parallel<void, RobotControl, bool>(&RobotControl::robotStop, m_robots[0], m_robots[1], true);
    m_bedCtrl->stopWaitMove();
}

void MainControl::robotsPause()
{
    Global::parallel<void, RobotControl>(&RobotControl::robotPause, m_robots[0], m_robots[1]);
}

void MainControl::robotsStart()
{
    Global::parallel<void, RobotControl, bool>(&RobotControl::robotPlay, m_robots[0], m_robots[1], true);
}

void MainControl::unlockProtectiveStop(int index)
{
    m_robots[index]->unlockProtectiveStop(true);
}

void MainControl::closePopUp(int index)
{
    m_robots[index]->closePopUp(true);
}

void MainControl::powerOn(int index)
{
    m_robots[index]->powerOn(true);
}

void MainControl::contact(int index, QString args)
{
    auto list = args.split(',');
    if(list.size() != 8)
    {
        emit showWarningMsg("提示", "参数个数错误");
        return;
    }
    QVector<double> argvec;
    for(int i = 0; i < 8; i++)
    {
        bool suc = false;
        auto tmp = list[i].toDouble(&suc);
        if(!suc)
        {
            emit showWarningMsg("提示", "参数应为浮点数格式");
            return;
        }
        argvec.append(tmp);
    }
    m_robots[index]->contact(argvec.mid(0, 6), argvec[6], argvec[7]);
    std::thread t(&RobotControl::waitReplyFromTCPClient, m_robots[index], QStringList({"tool_contact"}), 15000);
    t.detach();
}

void MainControl::movej(int index, int type, QString args)
{
    auto list = args.split(',');
    if(list.size() != 10)
    {
        emit showWarningMsg("提示", "参数个数错误");
        return;
    }
    QVector<double> argvec;
    for(int i = 0; i < 10; i++)
    {
        bool suc = false;
        auto tmp = list[i].toDouble(&suc);
        if(!suc)
        {
            emit showWarningMsg("提示", "参数应为浮点数格式");
            return;
        }
        argvec.append(tmp);
    }
    m_robots[index]->moveToPos('j', type, argvec.mid(0, 6), argvec[6], argvec[7]);
    std::thread t(&RobotControl::waitReplyFromTCPClient, m_robots[index], QStringList({"movej"}), 15000);
    t.detach();
}

void MainControl::movel(int index, QString args)
{
    auto list = args.split(',');
    if(list.size() != 10)
    {
        emit showWarningMsg("提示", "参数个数错误");
        return;
    }
    QVector<double> argvec;
    for(int i = 0; i < 10; i++)
    {
        bool suc = false;
        auto tmp = list[i].toDouble(&suc);
        if(!suc)
        {
            emit showWarningMsg("提示", "参数应为浮点数格式");
            return;
        }
        argvec.append(tmp);
    }

    //为检测力控软保护
    m_robots[index]->setRTSoftProtectiveStopFlag(true);

    m_robots[index]->moveToPos('l', 1, argvec.mid(0, 6), argvec[6], argvec[7]);
    std::thread t(&RobotControl::waitReplyFromTCPClient, m_robots[index], QStringList({"movel"}), 15000);
    t.detach();
}

void MainControl::speedl(int index, QString args)
{
    auto list = args.split(',');
    if(list.size() != 10)
    {
        emit showWarningMsg("提示", "参数个数错误");
        return;
    }
    QVector<double> argvec;
    for(int i = 0; i < 10; i++)
    {
        bool suc = false;
        auto tmp = list[i].toDouble(&suc);
        if(!suc)
        {
            emit showWarningMsg("提示", "参数应为浮点数格式");
            return;
        }
        argvec.append(tmp);
    }
    std::thread t(&RobotControl::speedOn, m_robots[index], 'l', argvec.mid(0, 6), argvec[6], argvec[8]);
    t.detach();
}

void MainControl::speedj(int index, QString args)
{
    auto list = args.split(',');
    if(list.size() != 10)
    {
        emit showWarningMsg("提示", "参数个数错误");
        return;
    }
    QVector<double> argvec;
    for(int i = 0; i < 10; i++)
    {
        bool suc = false;
        auto tmp = list[i].toDouble(&suc);
        if(!suc)
        {
            emit showWarningMsg("提示", "参数应为浮点数格式");
            return;
        }
        argvec.append(tmp);
    }
    std::thread t(&RobotControl::speedOn, m_robots[index], 'j', argvec.mid(0, 6), argvec[6], argvec[8]);
    t.detach();
}

void MainControl::stopl(int index, QString args)
{
    //为检测力控软保护
    m_robots[index]->setRTSoftProtectiveStopFlag(false);

    bool suc = false;
    auto acc = args.toDouble(&suc);
    if(suc)
        m_robots[index]->stopMoving(acc, 'l');
    else
        emit showWarningMsg("提示", "参数应为浮点数格式");
}

void MainControl::stopj(int index, QString args)
{
    bool suc = false;
    auto acc = args.toDouble(&suc);
    if(suc)
        m_robots[index]->stopMoving(acc, 'j');
    else
        emit showWarningMsg("提示", "参数应为浮点数格式");
}

void MainControl::rawCommand(int index, QString args)
{
    auto arglist = args.split(',');
    if(arglist.size() != 28)
    {
        emit showWarningMsg("提示", "参数个数错误");
        return;
    }
    msg_comm msg;
    bool ok = true;
    for(int i = 0; i < 28; i++)
    {
        msg.args[i] = arglist.at(i).toDouble(&ok);
        if(!ok)
        {
            emit showWarningMsg("提示", "参数应为浮点数格式");
            return;
        }
    }
    if((g_runArgs["EnableLeftSensor"] == "true" && index == 0) ||
            (g_runArgs["EnableRightSensor"] == "true" && index == 1))
    {
        if(msg.args[0] == 2)//力控指令
        {
            auto o_zero = m_robots[index]->getZeroForce();
            QVector<double> t_vec;
            if(msg.args[5] == 0 && msg.args[6] == 0 && msg.args[7] == 0)
                t_vec = o_zero;
            else
            {
                QVector<double> r_vec = {msg.args[5], msg.args[6], msg.args[7]};
                t_vec = RobotControl::rotateVecByRotationVec(o_zero, r_vec);
            }
            for(int i = 0; i < 3; i++)
                msg.args[i + 14] -= t_vec[i];
        }
    }
    m_robots[index]->commRobotCtrl<msg_comm>(msg);
}

void MainControl::oneBtnStart()
{
    m_pauseLock = true;
    Global::parallel<void, RobotControl>(&RobotControl::robotReset, m_robots[0], m_robots[1]);
    emit blockUI("close", 0);
    Global::parallel<void, RobotControl, bool, bool>(&RobotControl::freeDrive, m_robots[0], m_robots[1], false, false);
    Global::parallel<void, RobotControl>(&RobotControl::vibrateOff, m_robots[0], m_robots[1]);
    if(!m_robots[0]->isInPhotoPos())
        m_robots[0]->safePos();
    if(!m_robots[1]->isInPhotoPos())
        m_robots[1]->safePos();
    Global::parallel<void, RobotControl, bool*>(&RobotControl::photoPos, m_robots[0], m_robots[1], nullptr);
    m_pauseLock = false;
}

void MainControl::vibrateOff()
{
    m_robots[0]->vibrateOff();
    m_robots[1]->vibrateOff();
}

void MainControl::viboff(int index)
{
    m_robots[index]->vibrateOff();
}

void MainControl::vibrateOn()
{
    m_robots[0]->vibrateOn();
    m_robots[1]->vibrateOn();
}

void MainControl::vibon(int index)
{
    m_robots[index]->vibrateOn();
}

void MainControl::heatingOff()
{
    m_bedCtrl->elecOnOff();
}

void MainControl::heatingOn()
{
    m_bedCtrl->elecOnOff(true, true);
}

void MainControl::calibrateStart(QJsonObject args)
{
    emit startCali(args);
}

void MainControl::checkCaliStart(int rbindex, QString camindex)
{
    emit checkCali(rbindex, camindex);
}

void MainControl::goUpDown(int rbindex, bool up, QString step)
{
    emit goUPAndDown(rbindex, up, step);
}

void MainControl::goToEulerAngle(int rbindex, QString msg)
{
    double pi = 3.1415926;
    auto list = msg.split(',');
    if(list.size() != 3)
    {
        emit showWarningMsg("提示", "参数个数错误");
        return;
    }
    QVector<double> argvec;
    for(int i = 0; i < 3; i++)
    {
        bool suc = false;
        auto tmp = list[i].toDouble(&suc);
        if(!suc)
        {
            emit showWarningMsg("提示", "参数应为浮点数格式");
            return;
        }
        else if(!(tmp >= -360 && tmp <= 360))
        {
            emit showWarningMsg("提示", "参数应为欧拉角度数");
            return;
        }
        argvec.append(tmp);
    }
    QVector<double> eulers = {argvec[0] * pi / 180, argvec[1] * pi / 180, argvec[2] * pi / 180};
    auto rvec = RobotControl::eulerAngleToRotationVec(eulers);
    auto curpose = Global::fromArray(m_robots[rbindex]->getStatus().toolVectorActual, 6);
    curpose[3] = rvec[0];
    curpose[4] = rvec[1];
    curpose[5] = rvec[2];
    m_robots[rbindex]->moveToPos('l', 1, curpose, 0.1, 0.1);
    std::thread t(&RobotControl::waitReplyFromTCPClient, m_robots[rbindex], QStringList({"movel"}), 15000);
    t.detach();
}

void MainControl::uploadCureRecord(QJsonObject obj)
{
    obj["device_id"] = g_deviceID;
    if(g_runArgs["LinkToServer"] == "true")
        m_dbclient->addSndMsg(QJsonDocument(obj).toBinaryData());
}
