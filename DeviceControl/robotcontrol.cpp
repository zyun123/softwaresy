#include "robotcontrol.h"
#include <QtEndian>
#include <QDateTime>
#include <QtMath>
#include <thread>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "pubdef.h"
#include <NumCpp.hpp>

RobotControl::RobotControl(int rbindex, QObject *parent) : QObject(parent)
{
    m_stopWaitForReply_8080 = false;
    m_robotIndex = rbindex;
    m_reduce = false;
    m_auto_force_on = false;
    m_free_drive_flag = false;
    m_dashConnected = false;
    m_rtConnected = false;
    m_serverConnected = false;
    m_isRunningScript = false;
    m_isMoving = false;
    m_isForcing = false;
    m_blockFreeDrive = false;
}

RobotControl::~RobotControl()
{
    for(int i = 0; i < m_threads.size(); i++)
    {
        m_threads[i]->quit();
        m_threads[i]->wait();
    }
}

void RobotControl::iniCon()
{
    //29999连接
    QString ip = (m_robotIndex == 0) ? g_RobotleftIP : g_RobotrightIP;//左右机械臂顺序统一为0左1右
    m_DashClient = new BaseClient(ip, g_RobotDashPort, 256);
    connect(m_DashClient, &BaseClient::conChange, this, [=](bool sta){
       m_dashConnected = sta;
       if(m_rtConnected)
       {
           emit switchDevSta(m_robotIndex);
           if(!sta)
               emit informPauseTreatment();
       }
       if(!sta)
           LOG_EXCEPTION_BOT << "robot dashboard disconnected" << (m_robotIndex == 0 ? g_RobotleftIP : g_RobotrightIP);
       else
           LOG_INFO << "robot dashboard connected" << (m_robotIndex == 0 ? g_RobotleftIP : g_RobotrightIP);
    });
    m_DashClient->iniConnection();
    m_DashClient->iniHeartBeatMsg("get robot model\n", "UR5\n");
    addToThreadPool(m_DashClient);
    //30003连接
    m_RTClient = new RealtimeMonClient(ip);
    connect(m_RTClient, &BaseClient::conChange, this, [=](bool sta){
       m_rtConnected = sta;
       if(m_dashConnected)
       {
           emit switchDevSta(m_robotIndex);
           if(!sta)
               emit informPauseTreatment();
       }
       if(!sta)
           LOG_EXCEPTION_BOT << "robot realtime port disconnected" << (m_robotIndex == 0 ? g_RobotleftIP : g_RobotrightIP);
       else
           LOG_INFO << "robot realtime port connected" << (m_robotIndex == 0 ? g_RobotleftIP : g_RobotrightIP);
    });
    m_RTClient->iniConnection();
    m_RTClient->iniHeartBeatMsg("", "just_check");
    addToThreadPool(m_RTClient);
    connect(m_RTClient, &RealtimeMonClient::warn, this, &RobotControl::warn);
    connect(m_RTClient, &RealtimeMonClient::informPauseTreatment, this, &RobotControl::informPauseTreatment);
    connect(this, &RobotControl::connectToPlot, m_RTClient, &RealtimeMonClient::connectToPlot);
    connect(m_RTClient, &RealtimeMonClient::reduceForce, this, &RobotControl::onReduceForce);
    connect(m_RTClient, &RealtimeMonClient::recoverForce, this, &RobotControl::onRecoverForce);
    connect(m_RTClient, &RealtimeMonClient::enableFreeDrive, this, [=](bool flag){freeDrive(flag, false);});
    connect(this, &RobotControl::sigRTProgSFlag, m_RTClient, &RealtimeMonClient::setProgState, Qt::BlockingQueuedConnection);
    connect(this, &RobotControl::sigRTSPSFlag, m_RTClient, &RealtimeMonClient::setSoftProtectiveStop, Qt::BlockingQueuedConnection);
    connect(this, &RobotControl::sigSetAutoArgs, m_RTClient, &RealtimeMonClient::setAutoArgs, Qt::BlockingQueuedConnection);
    //urscript连接
    quint16 svport = (m_robotIndex == 0) ? g_Host1Port : g_Host2Port;
    m_robotServer = new DevServer(g_HostIP, svport);
    connect(m_robotServer, &DevServer::conChange, this, [=](bool sta){
       m_serverConnected = sta;
       if(m_dashConnected && m_rtConnected && !sta)
       {
           if(m_isRunningScript)//脚本正在运行 无法连接上位机
           {
               LOG_EXCEPTION_BOT << "urscript disconnected";
               emit informPauseTreatment();
           }
       }
       if(!sta)
           LOG_EXCEPTION_BOT << "robot URScript disconnected" << (m_robotIndex == 0 ? g_RobotleftIP : g_RobotrightIP);
       else
           LOG_INFO << "robot URScript connected" << (m_robotIndex == 0 ? g_RobotleftIP : g_RobotrightIP);
    });
    while(!m_robotServer->iniServer())
    {
        QThread::msleep(500);
        blockForConfirm("错误", "请确认端口8080/8081未被占用");
    }
    m_robotServer->iniHeartBeatMsg("[9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9]", "<heartbeat_feedback>");
    addToThreadPool(m_robotServer);
}

QVector<double> RobotControl::eulerAngleToRotationVec(QVector<double> eangle)
{
    //三个旋转角为内旋的zyx或者外旋的xyz
    cv::Mat R_x = (cv::Mat_<double>(3,3) <<
                   1, 0, 0,
                   0, qCos(eangle[0]), -qSin(eangle[0]),
                   0, qSin(eangle[0]), qCos(eangle[0]));
    cv::Mat R_y = (cv::Mat_<double>(3,3) <<
                   qCos(eangle[1]), 0, qSin(eangle[1]),
                   0, 1, 0,
                   -qSin(eangle[1]), 0, qCos(eangle[1]));
    cv::Mat R_z = (cv::Mat_<double>(3,3) <<
                   qCos(eangle[2]), -qSin(eangle[2]), 0,
                   qSin(eangle[2]), qCos(eangle[2]), 0,
                   0, 0, 1);
    cv::Mat R = R_z * R_y * R_x;
    cv::Mat res;
    cv::Rodrigues(R, res);
    return {res.at<double>(0, 0), res.at<double>(1, 0), res.at<double>(2, 0)};
}

QVector<double> RobotControl::vecToEulerAngle(QVector<double> origin_v, QVector<double> target_v)
{
    auto sq1 = qSqrt(qPow(origin_v[0], 2) + qPow(origin_v[1], 2) + qPow(origin_v[2], 2));
    auto sq2 = qSqrt(qPow(target_v[0], 2) + qPow(target_v[1], 2) + qPow(target_v[2], 2));
    auto origin_norm = nc::asarray<double>({origin_v[0] / sq1, origin_v[1] / sq1, origin_v[2] / sq1});
    auto target_norm = nc::asarray<double>({target_v[0] / sq2, target_v[1] / sq2, target_v[2] / sq2});
    auto c = nc::dot(origin_norm, target_norm);
    auto n_vector = nc::cross(origin_norm, target_norm);
    auto s = nc::norm(n_vector);
    auto n_vector_invert = nc::asarray<double>({{0,-n_vector[2],n_vector[1]},
                                                {n_vector[2],0,-n_vector[0]},
                                                {-n_vector[1],n_vector[0],0}});
    auto I = nc::eye<double>(3);
    if(c[0] == -1)//两向量反向
        return {3.1415926, 0, 0};
    auto R = I + n_vector_invert + nc::dot(n_vector_invert, n_vector_invert) / (1 + c[0]);
    auto sy = qSqrt(R(0, 0) * R(0, 0) + R(1, 0) * R(1, 0));
    double x,y,z;
    if(sy < 0.000001)
    {
        x = qAtan2(-R(1, 2), R(1, 1));
        y = qAtan2(-R(2, 0), sy);
        z = 0;
    }
    else
    {
        x = qAtan2(R(2, 1), R(2, 2));
        y = qAtan2(-R(2, 0), sy);
        z = qAtan2(R(1, 0), R(0, 0));
    }
    return {x, y, z};
}

QVector<double> RobotControl::xVec(QVector<double> a, QVector<double> b)
{
    return {a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]};
}

double RobotControl::dotVec(QVector<double> a, QVector<double> b)
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

QVector<double> RobotControl::rotateVecAlongAxis(QVector<double> vec, QVector<double> axis, double theta)
{
    auto sq1 = qSqrt(qPow(vec[0], 2) + qPow(vec[1], 2) + qPow(vec[2], 2));
    auto sq2 = qSqrt(qPow(axis[0], 2) + qPow(axis[1], 2) + qPow(axis[2], 2));
    for(int i = 0; i < vec.size(); i++)
        vec[i] /= sq1;
    for(int i = 0; i < axis.size(); i++)
        axis[i] /= sq2;

    double S    = qSin(theta * 3.14 / 180.f);
    double C    = qCos(theta * 3.14 / 180.f);
    double XX    = axis[0] * axis[0];
    double YY    = axis[1] * axis[1];
    double ZZ    = axis[2] * axis[2];
    double XY    = axis[0] * axis[1];
    double YZ    = axis[1] * axis[2];
    double ZX    = axis[2] * axis[0];
    double XS    = axis[0] * S;
    double YS    = axis[1] * S;
    double ZS    = axis[2] * S;
    double OMC    = 1.f - C;

    return {(OMC * XX + C ) * vec[0] + (OMC * XY - ZS) * vec[1] + (OMC * ZX + YS) * vec[2],
                (OMC * XY + ZS) * vec[0] + (OMC * YY + C ) * vec[1] + (OMC * YZ - XS) * vec[2],
                (OMC * ZX - YS) * vec[0] + (OMC * YZ + XS) * vec[1] + (OMC * ZZ + C ) * vec[2]};
}

QVector<double> RobotControl::rotateVecByRotationVec(QVector<double> vec, QVector<double> r_vec)
{
    auto theta = qSqrt(qPow(r_vec[0], 2) + qPow(r_vec[1], 2) + qPow(r_vec[2], 2));
    QVector<double> axis = {r_vec[0] / theta, r_vec[1] / theta, r_vec[2] / theta};
    auto t_vec = rotateVecAlongAxis(vec, axis, theta * 180 / 3.1415926);
    auto mod0 = qSqrt(qPow(vec[0], 2) + qPow(vec[1], 2) + qPow(vec[2], 2));
    auto mod = qSqrt(qPow(t_vec[0], 2) + qPow(t_vec[1], 2) + qPow(t_vec[2], 2));
    for(int i = 0; i < 3; i++)
        t_vec[i] *= mod0 / mod;
    return t_vec;
}

bool RobotControl::sndDashCmd(QString cmd, QStringList expected, int reSndTimes)
{
    int cnt = 0;
    while(cnt < reSndTimes)
    {
        m_DashClient->addSndMsg(cmd.toLocal8Bit());
        if(m_robotIndex == 0)
            LOG_MSG_BOTLEFT << "rb_l | d_snd |" << cmd;
        else
            LOG_MSG_BOTRIGHT << "rb_r | d_snd |" << cmd;
        if(waitReplyFromDashBoard(expected, 1))
            return true;
        QThread::msleep(500);
        cnt++;
    }
    return false;
}

RealtimeRobotStatus RobotControl::getStatus()
{
    RealtimeRobotStatus stat;
    if(m_rtConnected)
        stat = m_RTClient->getRobotStatus();
    return stat;
}

void RobotControl::addToThreadPool(QObject *obj)
{
    QThread *newThread = new QThread();
    obj->moveToThread(newThread);
    connect(newThread, &QThread::finished, obj, &QObject::deleteLater);
    m_threads.append(newThread);
    newThread->start();
}

template <typename T>
void RobotControl::commRobotCtrl(T &msg, QString log)
{
    double args[ARGS_LEN];
    memset(args, 0, sizeof(args));
    memcpy(args, &msg, sizeof(T));
    QString sndmsg = Global::arrToString(args, ARGS_LEN);
    m_robotServer->addSndMsg(sndmsg.toLocal8Bit());
    if(m_robotIndex == 0)
        LOG_MSG_BOTLEFT << QString("rb_l | snd | ") + log + "raw" + sndmsg;
    else
        LOG_MSG_BOTRIGHT << QString("rb_r | snd | ") + log + "raw" + sndmsg;
}
template void RobotControl::commRobotCtrl<msg_comm>(msg_comm &msg, QString log);//外部调用居然还要特化声明

void RobotControl::freeDrive(bool open, bool tip)
{
    if(!m_isMoving && !m_isForcing && !m_blockFreeDrive)
    {
        msg_ctrl_freedrive msg;
        if(!open)
            msg.enable = 2;
        QString log = QString("freedrive ") + (open ? "on " : "off ");
        commRobotCtrl(msg, log);
        QStringList ret_str;
        if(open)
            ret_str = QStringList({"freedrive_mode"});
        else
            ret_str = QStringList({"end_freedrive_mode"});
        if(waitReplyFromTCPClient(ret_str, 3000) == 0)
        {
            m_free_drive_flag = open;
            if(tip)
            {
                if(open)
                    emit warn(QString("提示").toLocal8Bit(), QString("已开启自由驱动：%1").arg(m_robotIndex).toLocal8Bit());
                else
                    emit warn(QString("提示").toLocal8Bit(), QString("已关闭自由驱动：%1").arg(m_robotIndex).toLocal8Bit());
            }
        }
        else
            emit warn(QString("错误").toLocal8Bit(), QString("自由驱动控制失败：%1").arg(m_robotIndex).toLocal8Bit());
    }
    else if(tip)
        emit warn(QString("错误").toLocal8Bit(), QString("调理进行中、移动及力控期间无法开启自由驱动：%1").arg(m_robotIndex).toLocal8Bit());
    else
        LOG_INFO << "freedrive fail" << m_isMoving << m_isForcing << m_blockFreeDrive;
}

void RobotControl::robotPlay(bool tip)
{
    if(!sndDashCmd("play\n", {"Starting program"}) || !queryDashBoard("running\n", {"Program running: true"}))
    {
        if(tip)
            emit warn("机械臂错误", "脚本启动失败");
    }
    else
    {
        if(g_runArgs["EnableLeftSensor"] == "true" && m_robotIndex == 0 || g_runArgs["EnableRightSensor"] == "true" && m_robotIndex == 1)
        {
            msg_ctrl_enable_fsensor msg;
            QString log = "enable_rtde ";
            commRobotCtrl(msg, log);
            waitReplyFromTCPClient({"ft_rtde_input_enable"});
        }
        setRTProgStatFlag(true);
        m_isRunningScript = true;
    }
}

void RobotControl::moveToPos(char type, int posetype, QVector<double> pos, double acc, double vel)
{
    if(m_free_drive_flag)
        freeDrive(false);
    if(m_free_drive_flag)
        return;
    msg_move msg;
    if(type == 'l')
        msg.mvtype = 1;
    else
        msg.mvtype = 2;
    for(int i = 0; i < 6; i++)
        msg.argvec[i] = pos[i];
    msg.acc = acc;
    msg.speed = vel;
    msg.posetype = posetype;
    QString log = QString("") + (type == 'l' ? "movel " : "movej ") +
                                "type=" + (posetype == 1 ? "pose" : "joint") + " " +
                                "p" + Global::arrToString(msg.argvec, 6) + " " +
                                "spd=" + QString::number(vel, 'g', 3) + " " +
                                "acc=" + QString::number(acc, 'g', 3) + " ";
    m_isMoving = true;
    commRobotCtrl(msg, log);
}

void RobotControl::speedOn(char type, QVector<double> spdv, double acc, double rt)
{
    if(m_free_drive_flag)
        freeDrive(false);
    if(m_free_drive_flag)
        return;
    msg_move msg;
    if(type == 'l')
        msg.mvtype = 3;
    else
        msg.mvtype = 4;
    msg.posetype = 0;
    for(int i = 0; i < 6; i++)
        msg.argvec[i] = spdv[i];
    msg.acc = acc;
    msg.funcRT = rt;
    QString log = QString("speed ");
    m_isMoving = true;
    commRobotCtrl(msg, log);
}

void RobotControl::stopMoving(double acc, char type)
{
    if(!m_free_drive_flag)
    {
        msg_move msg;
        msg.acc = acc;
        if(type == 'l')
            msg.mvtype = 11;
        else
            msg.mvtype = 12;
        QString log = QString("") + (type == 'l' ? "stopl " : "stopj ") + "acc=" + QString::number(acc, 'g', 3) + " ";
        commRobotCtrl(msg, log);
        if(type == 'l')
            waitReplyFromTCPClient({"stopl"});
        else
            waitReplyFromTCPClient({"stopj"});
    }
}

void RobotControl::safePos(bool *result)
{
    auto curpos = Global::fromArray(getStatus().toolVectorActual, 6);
    bool moveret = true;
    if(curpos[1] < -0.492 && curpos[2] < 0.203)
    {
        curpos[1] = curpos[1] + 0.1 > -0.492 ? -0.492 : curpos[1] + 0.1;
        curpos[2] = curpos[2] + 0.1 > 0.203 ? 0.203 : curpos[2] + 0.1;
        moveToPos('l', 1, curpos, 0.1, 0.1);
        if(waitReplyFromTCPClient({"movel"}) != 0)
        {
            moveret = false;
            LOG_EXCEPTION_BOT << m_robotIndex << "move to safe pos error1";
            emit warn("机械臂错误", "移动到安全位置失败");
            if(result != nullptr)
                *result = false;
            return;
        }
    }
    if(moveret)
    {
        if(m_robotIndex == 0)
            moveToPos('j', 2, LEFT_SAFE_POS, 0.3, 1.0);
        else
            moveToPos('j', 2, RIGHT_SAFE_POS, 0.3, 1.0);
        if(waitReplyFromTCPClient({"movej"}) != 0)
        {
            LOG_EXCEPTION_BOT << m_robotIndex << "move to safe pos error2";
            emit warn("机械臂错误", "移动到安全位置失败");
            if(result != nullptr)
                *result = false;
            return;
        }
    }
    if(result != nullptr)
        *result = true;
}

void RobotControl::photoPos(bool *result)
{
    if(m_robotIndex == 0)
        moveToPos('j', 2, LEFT_PHOTO_POS, 0.3, 1.0);
    else
        moveToPos('j', 2, RIGHT_PHOTO_POS, 0.3, 1.0);
    if(waitReplyFromTCPClient({"movej"}) != 0)
    {
        LOG_EXCEPTION_BOT << m_robotIndex << "move to photo pos failed";
        emit warn("机械臂错误", "移动到拍照位置失败");
        if(result != nullptr)
            *result = false;
        return;
    }
    if(result != nullptr)
        *result = true;
}

void RobotControl::calibratePos(QChar type)
{
    if(m_robotIndex == 0)
    {
        if(type == "m")
            moveToPos('j', 2, LEFT_CAM_M_CALI_POS, 0.3, 1.0);
        if(type == "h")
            moveToPos('j', 2, LEFT_CAM_H_CALI_POS, 0.3, 1.0);
        if(type == "l")
            moveToPos('j', 2, LEFT_CAM_L_CALI_POS, 0.3, 1.0);
        if(type == "r")
            moveToPos('j', 2, LEFT_CAM_R_CALI_POS, 0.3, 1.0);
    }
    else
    {
        if(type == "m")
            moveToPos('j', 2, RIGHT_CAM_M_CALI_POS, 0.3, 1.0);
        if(type == "h")
            moveToPos('j', 2, RIGHT_CAM_H_CALI_POS, 0.3, 1.0);
        if(type == "l")
            moveToPos('j', 2, RIGHT_CAM_L_CALI_POS, 0.3, 1.0);
        if(type == "r")
            moveToPos('j', 2, RIGHT_CAM_R_CALI_POS, 0.3, 1.0);
    }
    if(waitReplyFromTCPClient({"movej"}) != 0)
        emit warn("机械臂错误", "移动到标定位置失败");
}

void RobotControl::robotStop(bool tip)
{
    m_isRunningScript = false;
    setRTProgStatFlag(false);
    if(!sndDashCmd("stop\n", {"Stopped"}))
    {
        if(tip)
            emit warn("机械臂错误", "脚本运行中止失败");
    }
    else
    {
        m_isMoving = false;
        m_isForcing = false;
    }
}

void RobotControl::robotPause()
{
    if(!sndDashCmd("pause\n", {"Pausing program"}))
        emit warn("机械臂错误", "脚本暂停失败");
}

void RobotControl::unlockProtectiveStop(bool tip)
{
    auto ret = sndDashCmd("unlock protective stop\n", {"Protective stop releasing"});
    if(tip)
    {
        if(ret)
            emit warn(QString("提示").toLocal8Bit(), QString("已解除保护性停止：%1").arg(m_robotIndex).toLocal8Bit());
        else
            emit warn(QString("错误").toLocal8Bit(), QString("解除保护性停止失败：%1").arg(m_robotIndex).toLocal8Bit());
    }
}

void RobotControl::closePopUp(bool tip)
{
    auto ret = sndDashCmd("close popup\n", {"closing popup"});
    if(ret)
        ret = sndDashCmd("close safety popup\n", {"closing safety popup"});
    if(tip)
    {
        if(ret)
            emit warn(QString("提示").toLocal8Bit(), QString("已关闭下位机警告：%1").arg(m_robotIndex).toLocal8Bit());
        else
            emit warn(QString("错误").toLocal8Bit(), QString("关闭下位机警告失败：%1").arg(m_robotIndex).toLocal8Bit());
    }
}

void RobotControl::powerOn(bool tip)
{
    auto ret = sndDashCmd("power on", {"Powering on"});
    if(tip)
    {
        if(ret)
            emit warn(QString("提示").toLocal8Bit(), QString("已上电：%1").arg(m_robotIndex).toLocal8Bit());
        else
            emit warn(QString("错误").toLocal8Bit(), QString("上电失败：%1").arg(m_robotIndex).toLocal8Bit());
    }
}

void RobotControl::blockForConfirm(QString title, QString msg)
{
    m_block_flag = true;
    emit warn_b(title, msg);
    while(m_block_flag)
        QThread::msleep(500);
}

void RobotControl::robotReset()
{
    if(!m_rtConnected)
        emit blockUI(QString(m_robotIndex == 0 ? "左" : "右") + "机械臂已断开连接...", 0);
    while(!m_rtConnected)
        QThread::msleep(500);
    do
    {
        //处理急停
        bool instop = false;
        auto sta = getStatus();
        if(sta.safetyModes >= 5 && sta.safetyModes <= 7)
        {
            emit blockUI(QString(m_robotIndex == 0 ? "左" : "右") + "机械臂已急停，等待恢复中...", 0);
            instop = true;
        }
        while(sta.safetyModes >= 5 && sta.safetyModes <= 7)
        {
            QThread::msleep(500);
            sta = getStatus();
        }
        if(instop)
            blockForConfirm("提示", QString(m_robotIndex == 0 ? "左" : "右") + "机械臂急停已恢复");
        //处理故障
        bool fault = false;
        if(getStatus().safetyModes > 7)
        {
            emit blockUI(QString(m_robotIndex == 0 ? "左" : "右") + "机械臂故障，正在恢复...", 0);
            fault = true;
        }
        while(getStatus().safetyModes > 7)
        {
            while(!sndDashCmd("restart safety\n", {"Restarting safety"}))
                QThread::msleep(500);
            sta = getStatus();
            int maxcnt = 15;
            while((sta.safetyModes > 7 || sta.safetyModes <= 0) && maxcnt > 0)
            {
                QThread::msleep(500);
                sta = getStatus();
                maxcnt--;
            }
        }
        if(fault)
            blockForConfirm("提示", QString(m_robotIndex == 0 ? "左" : "右") + "机械臂故障已恢复");
        //处理保护性停止
        if(getStatus().safetyModes == 3)
            emit blockUI(QString(m_robotIndex == 0 ? "左" : "右") + "机械臂保护性停止，正在恢复...", 0);
        while(getStatus().safetyModes == 3)
            unlockProtectiveStop();
        //解除刹车
        while(getStatus().robotMode != 7)
        {
            if(getStatus().safetyModes != 1)
                break;
            sndDashCmd("brake release\n", {"Brake releasing"}, 1);
            QThread::msleep(1000);
        }
        //重启加载运行脚本
        while(!sndDashCmd("get loaded program\n", {"Loaded program: /programs/sy3_control.urp\n"}, 2))
            sndDashCmd("load sy3_control.urp\n", {"Loading program"});
        robotStop();
        robotPlay();
        //关闭示教界面弹窗
        closePopUp();
    } while(getStatus().safetyModes != 1 || getStatus().programState != 2);
}

bool RobotControl::isInPhotoPos()
{
    auto jointpose = Global::fromArray(getStatus().qActual, 6);
    QVector<double> target_l = LEFT_PHOTO_POS;
    QVector<double> target_r = RIGHT_PHOTO_POS;
    auto f = [](QVector<double> &v1, QVector<double> &v2)->bool{
        if(v1.size() != v2.size())
            return false;
        for(int i = 0; i < v1.size(); i++)
            if(qAbs(v1[i] - v2[i]) > 0.005)
                return false;
        return true;
    };
    if((m_robotIndex == 0 && f(jointpose, target_l)) || (m_robotIndex == 1 && f(jointpose, target_r)))
        return true;
    else
        return false;
}

void RobotControl::vibrateOff()
{
    msg_ctrl_io msg;
    msg.ioindex = 0;
    msg.value = 2;
    QString log = "vib off ";
    commRobotCtrl(msg, log);
    waitReplyFromTCPClient({"set_standard_digital_out"});
}

void RobotControl::vibrateOn()
{
    msg_ctrl_io msg;
    msg.ioindex = 0;
    msg.value = 1;
    QString log = "vib on ";
    commRobotCtrl(msg, log);
    waitReplyFromTCPClient({"set_standard_digital_out"});
}

void RobotControl::setforce(msg_force_startforce msg)
{
    if(m_free_drive_flag)
        freeDrive(false);
    if(m_free_drive_flag)
        return;
    if(g_runArgs["EnableLeftSensor"] == "true" && m_robotIndex == 0 || g_runArgs["EnableRightSensor"] == "true" && m_robotIndex == 1)
    {
        QVector<double> t_vec;
        if(msg.aixs_bias[3] == 0 && msg.aixs_bias[4] == 0 && msg.aixs_bias[5] == 0)
            t_vec = m_zero_force;
        else
        {
            QVector<double> r_vec = {msg.aixs_bias[3], msg.aixs_bias[4], msg.aixs_bias[5]};
            t_vec = RobotControl::rotateVecByRotationVec(m_zero_force, r_vec);
        }
        for(int i = 0; i < 3; i++)
            msg.force_value[i] -= t_vec[i];
    }
    QString log = QString("") + "setforce " +
                                "type=" + QString::number(msg.ftype) + " " +
                                "aixs" + Global::arrToString(msg.aixs_bias, 6) + " " +
                                "val" + Global::arrToString(msg.force_value, 6) + " " +
                                "dir" + Global::arrToString(msg.direction, 6) + " " +
                                "spd" + Global::arrToString(msg.speed, 6) + " ";
    m_isForcing = true;
    commRobotCtrl(msg, log);
    waitReplyFromTCPClient({"force_mode"});
}

void RobotControl::setDamp(double damp)
{
    msg_force_damp msg;
    msg.fdamp = damp;
    QString log = QString("setdamp ") + "d=" + QString::number(damp, 'g', 3) + " ";
    commRobotCtrl(msg, log);
    waitReplyFromTCPClient({"force_mode_set_damping"});
}

void RobotControl::setGain(double gain)
{
    msg_force_gain msg;
    msg.fgain = gain;
    QString log = QString("setgain ") + "g=" + QString::number(gain, 'g', 3) + " ";
    commRobotCtrl(msg, log);
    waitReplyFromTCPClient({"force_mode_set_gain_scaling"});
}

void RobotControl::startAutoForce(QVector<double> start_pos, QVector<double> target_pos, msg_force_startforce &normal_f, msg_force_startforce &climb_f, QVector<double> lift_dir)
{
    m_auto_pathpt = target_pos;
    m_auto_force_normal = normal_f;
    m_auto_force_climb = climb_f;
    auto v = DEFAULT_CURE_SPEED / qSqrt(qPow(lift_dir[0], 2) + qPow(lift_dir[1], 2) + qPow(lift_dir[2], 2));
    m_auto_lift_dir = {lift_dir[0] * v, lift_dir[1] * v, lift_dir[2] * v, 0, 0, 0};
    m_auto_force_on = true;
    emit sigSetAutoArgs(start_pos, target_pos);
}

void RobotControl::endAutoForce()
{
    emit sigSetAutoArgs({}, {});
    m_auto_force_on = false;
    //关闭自适应需等待当前指令结束
    int cnt = 0;
    while(cnt < 20)
    {
        if(m_mutex_automode.try_lock())
        {
            //自适应逻辑执行完毕
            m_mutex_automode.unlock();
            break;
        }
        QThread::msleep(10);
        cnt++;
    }
}

void RobotControl::onRecoverForce(QString reason)
{
    if(m_auto_force_on == true)
    {
        if(!m_mutex_automode.try_lock())
            return;
        LOG_INFO << "爬坡恢复" << m_robotIndex << "由于" << reason;
        //恢复默认力方向
        stopMoving(DEFAULT_CURE_ACC);
        endforce();
        setforce(m_auto_force_normal);
        //恢复向目标点移动
        moveToPos('l', 1, m_auto_pathpt, DEFAULT_CURE_ACC, DEFAULT_CURE_SPEED);
        m_mutex_automode.unlock();
    }
}

void RobotControl::onReduceForce()
{
    if(m_auto_force_on == true)
    {
        if(!m_mutex_automode.try_lock())
            return;
        LOG_INFO << "触发爬坡" << m_robotIndex;
        //调整力方向为行进方向
        stopMoving(DEFAULT_CURE_ACC);
        endforce();
        setforce(m_auto_force_climb);
        //行进方向改为力控反向
        speedOn('l', m_auto_lift_dir, DEFAULT_CURE_ACC);
        m_mutex_automode.unlock();
    }
}

void RobotControl::endforce()
{
    if(!m_free_drive_flag)
    {
        msg_force_endforce msg;
        QString log = QString("endforce ") + "acc=" + QString::number(msg.stopacc, 'g', 3) + " ";
        commRobotCtrl(msg, log);
        waitReplyFromTCPClient({"end_force_mode"});
    }
}

bool RobotControl::clearforce()
{
    if((g_runArgs["EnableLeftSensor"] == "true" && m_robotIndex == 0) ||
            (g_runArgs["EnableRightSensor"] == "true" && m_robotIndex == 1))
    {
        QThread::sleep(1);//外置传感器清零多等待1s
        auto sta = m_RTClient->getRobotStatus();
        m_zero_force = QVector<double>({sta.tcpForce[0], sta.tcpForce[1], sta.tcpForce[2]});
        LOG_INFO << "fsensor clear force finished: " << m_robotIndex;
        return true;
    }
    else
    {
        msg_force_clearforce msg;
        QString log = "clearforce ";
        commRobotCtrl(msg, log);
        return (waitReplyFromTCPClient({"zero_ftsensor"}) == 0);
    }
}

void RobotControl::reduce(bool tip)
{
    msg_ctrl_io msg;
    msg.ioindex = 1;
    if(m_reduce)
        msg.value = 2;
    commRobotCtrl(msg);
    if(waitReplyFromTCPClient({"set_standard_digital_out"}, 3000) == 0)
    {
        m_reduce = m_reduce ? false : true;
        if(tip)
        {
            if(m_reduce)
                emit warn(QString("提示").toLocal8Bit(), QString("已开启缩减模式：%1").arg(m_robotIndex).toLocal8Bit());
            else
                emit warn(QString("提示").toLocal8Bit(), QString("已关闭缩减模式：%1").arg(m_robotIndex).toLocal8Bit());
        }
    }
    else
        emit warn(QString("错误").toLocal8Bit(), QString("缩减模式切换失败：%1").arg(m_robotIndex).toLocal8Bit());
}

void RobotControl::setTCP(QVector<double> tcp)
{
    if(tcp.size() != 6)
        return;
    msg_ctrl_tcp msg;
    for(int i = 0; i < 6; i++)
        msg.tcp[i] = tcp[i];
    QString log = QString("setTCP ") + "tcp" + Global::arrToString(msg.tcp, 6) + " ";
    commRobotCtrl(msg, log);
    waitReplyFromTCPClient({"set_tcp"});
}

void RobotControl::setPayLoad(double mass, QVector<double> cog)
{   
    msg_ctrl_payload msg;
    msg.mass = mass;
    msg.cog[0] = cog[0];
    msg.cog[1] = cog[1];
    msg.cog[2] = cog[2];
    QString log = QString("setpayload ") + "cog" + Global::arrToString(msg.cog, 3) + " " + "mass=" + QString::number(mass, 'g', 3) + " ";
    commRobotCtrl(msg, log);
    waitReplyFromTCPClient({"set_target_payload"});
}

QVector<double> RobotControl::jsonarrayToVector(QJsonArray arr)
{
    QVector<double> ret;
    for(int i = 0; i < arr.size(); i++)
        ret.append(arr.at(i).toDouble());
    return ret;
}

bool RobotControl::queryDashBoard(QString query, QStringList expected)
{
    if(expected.size() == 0)
        return 0;
    bool ret = false;
    int count = 10;
    while(count > 0)
    {
        m_DashClient->addSndMsg(query.toLocal8Bit());
        if(m_robotIndex == 0)
            LOG_MSG_BOTLEFT << "rb_l | d_snd |" << query;
        else
            LOG_MSG_BOTRIGHT << "rb_r | d_snd |" << query;
        if(waitReplyFromDashBoard(expected, 1))
        {
            ret = true;
            break;
        }
        count--;
        QThread::msleep(100);
    }
    if(count <= 0)
        LOG_EXCEPTION_BOT << "timeout wait for query: " << query << " " << expected;
    return ret;
}

bool RobotControl::waitReplyFromDashBoard(QStringList expected, int times)
{
    if(expected.size() == 0)//无期待值等待一次接收后直接返回
    {
        m_DashClient->wait();
        return true;
    }
    auto contained = [=](QString msg)->bool{//模糊判断
      bool ret = false;
      for(int i = 0; i < expected.size(); i++)
      {
          if(msg.contains(expected[i]))
          {
              ret = true;
              break;
          }
      }
      return ret;
    };
    bool flag = false;
    for(int i = 0; i < times; i++)
    {
        if(m_DashClient->wait())
        {
            auto reply = m_DashClient->getRcvMsg();
            if(m_robotIndex == 0)
                LOG_MSG_BOTLEFT << "rb_l | d_rcv |" << reply;
            else
                LOG_MSG_BOTRIGHT << "rb_r | d_rcv |" << reply;
            if(reply.size() > 0 && contained(reply))
            {
                flag = true;
                break;
            }
        }
    }
    return flag;
}

int RobotControl::waitReplyFromTCPClient(QStringList expected, int timeout)
{
    if(expected.size() == 0)//无期待值等待一次接收后直接返回
    {
        m_robotServer->wait();
        return 0;
    }
    int ret = 0;
    int passT = 0;
    while(passT < timeout && !m_stopWaitForReply_8080)
    {
        auto waitret = m_robotServer->wait();
        if(waitret == 0)//正常返回
        {
            auto reply = m_robotServer->getRcvMsg();
            //可能会粘包
            QString rawmsg = reply;
            auto msglist = rawmsg.remove('<').split('>');
            bool flag = false;
            for(int i = 0; i < msglist.size(); i++)
            {
                auto msg = msglist[i];
                if(msg.size() > 0 && expected.contains(msg))
                {
                    ret = 0;
                    flag = true;
                    break;
                }
            }
            if(flag)
            {
                if(expected.contains("movel") || expected.contains("movej") ||
                        expected.contains("speedl") || expected.contains("speedj") ||
                        expected.contains("stopl") || expected.contains("stopj") ||
                        expected.contains("tool_contact"))
                    m_isMoving = false;
                if(expected.contains("end_force_mode"))
                    m_isForcing = false;
                break;
            }
        }
        else if(waitret == 1)//超时
        {
            ret = 1;
            passT += 1000;//只有超时保证1000ms
        }
        else//断开
        {
            ret = 2;
            break;
        }
    }
    if(m_stopWaitForReply_8080)
        ret = 0;//主动停止不返回异常
    return ret;
}

void RobotControl::contact(QVector<double> spd, double acc, double stop_acc)
{
    if(m_free_drive_flag)
        freeDrive(false);
    if(m_free_drive_flag)
        return;
    msg_ctrl_contact msg;
    for(int i = 0; i < 6; i++)
        msg.speed[i] = spd[i];
    msg.spdacc = acc;
    msg.stopacc = stop_acc;
    QString log = QString("contact ") + "spd" + Global::arrToString(msg.speed, 6) + " " +
            "acc=" + QString::number(acc, 'g', 3) + " " + "dec=" + QString::number(stop_acc, 'g', 3) + " ";
    m_isMoving = true;
    commRobotCtrl(msg, log);
}
