#include "bedcontrol.h"
#include <QThread>

BedControl::BedControl(QObject *parent) : QObject(parent)
{
    if(g_runArgs["BedVer"] == "1")
    {
        m_move_threhold_head = 0.4;
        m_move_threhold_tail = 0.4;
    }
    else
    {
        m_move_threhold_head = 0.4;
        m_move_threhold_tail = 0.5;
    }
    m_stopWaitForMove = false;
    m_headcam = nullptr;
    m_reconnect_flag = false;
}

BedControl::~BedControl()
{
    m_socketTh->quit();
    m_socketTh->wait();
}

void BedControl::iniCon()
{
    //调理床连接client
    m_bedClient = new BaseClient(g_BedIP, g_BedPort, 256);
    connect(m_bedClient, &BaseClient::conChange, this, [=](bool sta){
        m_isConnected = sta;
        if(!g_testEMC)
        {
            emit switchDevSta(6);
            if(!sta)
            {
                LOG_EXCEPTION_BED << "bed disconnected";
                emit informPauseTreatment();
                m_reconnect_flag = true;
            }
            else
            {
                LOG_INFO << "bed connected";
                if(m_reconnect_flag)
                {
                    lightsOnOff(LIGHT_ALL_ON);
                    elecOnOff(true, true, 0x15, 0x15);
                    CameraControl::restartCamProcess();//床重连后重启相机
                    m_reconnect_flag = false;
                }
            }
        }
    });
    m_bedClient->iniConnection();
    m_bedClient->iniHeartBeatMsg("heartbeat", "heartbeat");

    m_socketTh = new QThread();
    m_bedClient->moveToThread(m_socketTh);
    connect(m_socketTh, &QThread::finished, m_bedClient, &QObject::deleteLater);
    m_socketTh->start();
}

void BedControl::iniDev()
{
    if(g_runArgs["BedVer"] != "1")
    {
        restart();
        QThread::msleep(2000);
    }
    stop();
    moveTo(0, 1);
    lightsOnOff(LIGHT_ALL_ON);
    elecOnOff(true, false, 0x15, 0x15);
}

bool BedControl::waitForMove(double posInMeter, int timeout)
{
    int des = posInMeter * 10000;
    int passT = 0;
    int last_pos = 99999;
    int cnt = 0;
    int getStatErrCnt = 0;
    bool passProtect = false;

    while(!m_stopWaitForMove && getStatErrCnt < 10)
    {
        auto [bstat, suc] = getBedStatus();
        if(suc)
        {
            int cur = bstat.pos * 10000;
//            LOG_INFO << "cur bed pos: " << cur << " " << des;

            if (qAbs(cur - last_pos) < 2)//约等于上一个位置
                cnt++;
            else
            {
                last_pos = cur;
                if (cnt > 0)
                    cnt = 0;
            }

            //检测调理床碰撞
//            if(g_testEMC == true)
//            {
//                if(m_headcam != nullptr && des <= -3000 && cur > -3000)
//                {
//                    if(m_headcam->checkBedCollision())
//                    {
//                        stop();
//                        LOG_EXCEPTION_BED << "bed collision detected";
//                        return false;
//                    }
//                }
//            }

            if (qAbs(cur - des) < 20.0 && cnt >= 5)//超过5周期都处于目标位置判定到达
            {
                LOG_INFO << "bed arrived pos";
                return true;
            }
            else
            {
                if(g_runArgs["BedVer"] == "1")
                {
                    //判断是否到达后又远离目标位置
                    if(qAbs(cur - des) < 20.0)
                        passProtect = true;
                    else
                    {
                        if(qAbs(cur - des) > 40.0 && passProtect)
                        {
                            LOG_EXCEPTION_BED << "bed move over des: " << des << " cur: " << cur << " but has been stopped";
                            stop();
                            return true;
                        }
                    }
                }
                //超时
                if(passT > timeout)
                {
                    LOG_EXCEPTION_BED << "bed move timeout. cur pos: " << cur << " des: " << des;
                    return false;
                }
                else
                {
                    if(g_runArgs["BedVer"] == "1")
                    {
                        passT += 200;
                        QThread::msleep(200);
                    }
                    else
                    {
                        passT += 500;
                        QThread::msleep(500);
                    }
                }
            }
        }
        else
        {
            LOG_EXCEPTION_BED << "bed get stat error";
            getStatErrCnt++;
            continue;
        }
    }
    if(m_stopWaitForMove)
    {
        LOG_INFO << "bed move wait called off";
        return true;//主动停止不返回异常
    }
    return false;
}

std::tuple<BedStatus, bool> BedControl::getBedStatus()
{
    BedStatus bstatus;
    if(!m_getStaLock.try_lock())
        return std::make_tuple(bstatus, false);
    bool ret = false;
    QByteArray tmp = transToBytes({0x55, 0x01, 0x10});
    m_bedClient->addSndMsg(tmp);
    if(m_bedClient->wait())
    {
        QByteArray reply = m_bedClient->getRcvMsg();
        int resize = 8;
        quint8 nouse = 0x06;
        if(g_runArgs["BedVer"] != "1")
        {
            resize = 9;
            nouse = 0x07;
//            if(reply.size() < 9 || (quint8)(reply.at(8)) != 0)
//                LOG_EXCEPTION_BED << "bed alarm!!! " << reply.at(8);
        }
        if(reply.size() == resize && (quint8)(reply.at(0)) == 0xaa && (quint8)(reply.at(1)) == nouse && (quint8)(reply.at(2)) == 0xff)
        {
            int tmppos = 0;
            memcpy(&tmppos, reply.data() + 3, sizeof(int));
            if(g_runArgs["BedVer"] == "2")
                bstatus.pos = -tmppos * 10.0 / BED_POS_ARG / 10000.0;
            else if(g_runArgs["BedVer"] == "3")
                bstatus.pos = tmppos * 10.0 / 131072.0 / 10000.0;
            else
                bstatus.pos = tmppos / 10000.0;
            quint8 rawbyte = reply.at(7);
            bstatus.isMoving = ((quint8)(rawbyte & 0x01) == 1) ? true : false;
            bstatus.isElecOn1 = ((quint8)(rawbyte & 0x02) == 1) ? true : false;
            bstatus.isElecOn2 = ((quint8)(rawbyte & 0x04) == 1) ? true : false;
            bstatus.isHeatOn1 = ((quint8)(rawbyte & 0x08) == 1) ? true : false;
            bstatus.isHeatOn2 = ((quint8)(rawbyte & 0x10) == 1) ? true : false;
            bstatus.isNodeOn = ((quint8)(rawbyte & 0x80) == 1) ? true : false;

//            QString msglog = "replyofbed:---------------------------------------------";
//            for(int i = 0; i < reply.size(); i++)
//                msglog += QString::number((quint8)reply[i],16) + " ";
//            LOG_INFO << msglog;
            ret = true;
        }
        else
            ret = false;
    }
    else
        ret = false;
    m_getStaLock.unlock();
    return std::make_tuple(bstatus, ret);
}

bool BedControl::elecOnOff(bool vibrateOn, bool heatOn, quint8 l_rate, quint8 r_rate)
{
    quint8 vib = vibrateOn ? 4 : 0;
    quint8 heat = heatOn ? 1 : 0;
    QByteArray tmp;
    if(g_runArgs["BedVer"] != "1")
        tmp = transToBytes({ 0x55, 0x05, 0x11, vib, heat, l_rate, r_rate });
    else
        tmp = transToBytes({ 0x55, 0x03, 0x11, vib, heat, l_rate, r_rate });
    m_bedClient->addSndMsg(tmp);
    auto ret = waitForReply(tmp);
    if(g_runArgs["BedVer"] != "1")
        QThread::msleep(100);
    return ret;
}

bool BedControl::lightsOnOff(quint8 type)
{
    QByteArray tmp = transToBytes({0x55, 0x02, 0x14, type});
    m_bedClient->addSndMsg(tmp);
    auto ret = waitForReply(tmp);
    if(g_runArgs["BedVer"] != "1")
        QThread::msleep(100);
    return ret;
}

bool BedControl::moveTo(double posInMeter, double speedlvl, int timeout)
{
    stop();
    QThread::msleep(200);
    if(m_stopWaitForMove)
    {
        LOG_INFO << "move bed called off!!";
        return true;
    }
    LOG_INFO << "bed to " << posInMeter;
    Q_ASSERT(posInMeter >= -m_move_threhold_head && posInMeter <= m_move_threhold_tail);//任何情况下都不应移动床至超限位置
    int pos = posInMeter * 10000;
    QByteArray tmp;
    if(g_runArgs["BedVer"] != "1")
    {
        if(g_runArgs["BedVer"] == "2")
        {
            pos = pos * BED_POS_ARG / 10;
            pos = -pos;
        }
        else
            pos = pos * 131072.0 / 10;
        quint16 speed = DEFAULT_BED_SPEED * speedlvl * R_MIN_PER_MM_SEC;//0.1r/min
        tmp = transToBytes({0x55, 0x07, 0x12, (quint8)(pos), (quint8)(pos>>8), (quint8)(pos>>16), (quint8)(pos>>24),
                            (quint8)(speed>>8), (quint8)(speed)});
    }
    else
        tmp = transToBytes({0x55, 0x05, 0x12, (quint8)(pos), (quint8)(pos>>8), (quint8)(pos>>16), (quint8)(pos>>24)});
    QString msglog = "";
    for(int i = 0; i < tmp.size(); i++)
        msglog += QString::number((quint8)tmp[i],16) + " ";
    LOG_INFO << msglog;

    m_bedClient->addSndMsg(tmp);
    waitForReply(tmp);
    auto ret = waitForMove(posInMeter, timeout);
    if(!ret)
    {
        emit informPauseTreatment();
        emit warn(QString("错误").toLocal8Bit(), QString("床移动错误").toLocal8Bit());
    }
    if(g_runArgs["BedVer"] != "1")
        QThread::msleep(100);
    emit mvover();//防止主线程调用阻塞
    return ret;
}

bool BedControl::stop()
{
    QByteArray tmp = transToBytes({0x55, 0x01, 0x13});
    m_bedClient->addSndMsg(tmp);
    auto ret = waitForReply(tmp);
    if(g_runArgs["BedVer"] != "1")
        QThread::msleep(100);
    return ret;
}

bool BedControl::restart()
{
    QByteArray tmp = transToBytes({0x55, 0x01, 0x09});
    m_bedClient->addSndMsg(tmp);
    auto ret = waitForReply(tmp);
    if(g_runArgs["BedVer"] != "1")
        QThread::msleep(100);
    return ret;
}

bool BedControl::usbOnOff(quint8 type)
{
    QByteArray tmp = transToBytes({0x55, 0x02, 0x0f, type});
    m_bedClient->addSndMsg(tmp);
    auto ret = waitForReply(tmp);
    if(g_runArgs["BedVer"] != "1")
        QThread::msleep(100);
    return ret;
}

QByteArray BedControl::transToBytes(QList<quint8> list)
{
    QByteArray ret;
    ret.resize(list.size());
    for(int i = 0; i < list.size(); i++)
        ret[i] = list[i];
    return ret;
}

bool BedControl::waitForReply(QByteArray sndMsg, int timeout)
{
    sndMsg[0] = 0xaa;
    QByteArray reply;
    if(m_bedClient->wait(timeout))
        reply = m_bedClient->getRcvMsg();
    bool ret = false;
    if(sndMsg == reply)
        ret = true;
    else if(reply.size() > 0 && (quint8)(sndMsg.at(0)) == 0xaa && (quint8)(sndMsg.at(1)) == 0x03 && (quint8)(sndMsg.at(2)) == 0x11 && sndMsg.chopped(2) == reply)
        ret = true;
    else if(reply.size() > 4 && (quint8)(sndMsg.at(0)) == 0xaa && (quint8)(sndMsg.at(1)) == 0x05 && (quint8)(sndMsg.at(2)) == 0x12 && sndMsg.chopped(4) == reply.chopped(4))
        ret = true;
    if(!ret)
        LOG_EXCEPTION_BED << "bed reply err: expected: " << sndMsg << "reply: " << reply;
    return ret;
}
