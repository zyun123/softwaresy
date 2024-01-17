#include "realtimemonclient.h"
#include "global.h"
#include "pubdef.h"
#include <QJsonArray>
#include <QtMath>

#define MESSAGE_LEN 1220
#define INFO_WORDS "qTarget[6];qdTarget[6];qddTarget[6];iTarget[6];mTarget[6];qActual[6];qdActual[6];iActual[6];iControl[6];toolVectorActual[6];\
tcpSpeedActual[6];tcpForce[6];toolVectorTarget[6];tcpSpeedTarget[6];digitalInputBits;motorTemperatures[6];controllerTimer;\
testValue;robotMode;jointModes[6];safetyModes;testValue2[6];toolAccelerometerValues[3];testValue3[6];speedScaling;\
linearMomentumNorm;testValue4;testValue5;vMain;vRobot;iRobot;vActual[6];digitalOutputs;programState;elbowPosition[3];\
elbowVelocity[3];safetyStatus;testValue6;testValue7;testValue8;payloadMass;payloadCoG[3];payloadInertia[6]"

using namespace std;

RealtimeMonClient::RealtimeMonClient(QString ip, QObject *parent)
    : BaseClient(ip, g_RobotRTPort, 2440, parent) {
    m_lastTS = 0;
    m_reduceFlag = false;
    m_plot_connected = false;
    m_plot_client = nullptr;
    m_instantStopFlag = false;
    m_freeDriveFLag = false;
    m_softProtectiveStopFlag = false;
    m_safetyModesFlag = false;
    m_progStateFlag = false;
    m_updateCnt = 0;
    m_recover_cnt = 0;
    m_last_toolpos = {0, 0, 0};
}

void RealtimeMonClient::connectToPlot()
{
    //连接plot
    m_plot_client = new QWebSocket();
    connect(m_plot_client, &QWebSocket::connected, this, [=](){
        LOG_INFO << m_ip << "connect to plot";
        m_plot_connected = true;
    });
    connect(m_plot_client, &QWebSocket::disconnected, this, [=](){
        LOG_INFO << m_ip << "disconnect from plot";
        m_plot_connected = false;
    });
    auto url = QString("ws://localhost:") + (m_ip == "192.168.1.102" ? "9990" : "9991");
    m_plot_client->open(QUrl(url));
}

void RealtimeMonClient::processMsgRcv()
{
    if (m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
    {
        auto raw = m_socket->readAll();
        m_mutex.lock();
        m_rcvMsg.clear();
        if(m_hbRcvMsg == "" || raw != m_hbRcvMsg.toLocal8Bit())
            m_rcvMsg = raw;
        if(m_hbChkTimer->isActive())
            m_hbChkTimer->start();
        m_mutex.unlock();
        if(raw.size() != MESSAGE_LEN)
            return;
        //解析30003端口接收数据
        RealtimeRobotStatus tmpInfo;
        memset(&tmpInfo, 0, sizeof(RealtimeRobotStatus));
        char *pData = raw.data();
        //大小端转换
        tmpInfo.messageSize = Global::fromBigEndian<int>(pData);
        if(tmpInfo.messageSize != MESSAGE_LEN)
        {
            LOG_EXCEPTION_BOT << "realtime client data messageSize error:" << tmpInfo.messageSize;
            return;
        }
        for(int i = 0; i < (MESSAGE_LEN - 4) / 8; i++)
            *(qreal*)(pData + 4 + i * 8) = Global::fromBigEndian<qreal>(pData + 4 + i * 8);
        //拷贝到结构体
        memcpy((char*)&tmpInfo + 8, pData + 4, MESSAGE_LEN - 4);
        m_statusMutex.lock();
        memcpy(&m_rbInfo, &tmpInfo, sizeof(RealtimeRobotStatus));
        m_statusMutex.unlock();
        //发给数据分析工具
        if(m_plot_connected)
        {
            QString parse_words = INFO_WORDS;
            QJsonObject obj;
            auto keylist = parse_words.split(";");
            pData = raw.data() + 12;
            for(int i = 0; i < keylist.size(); i++)
            {
                auto keystr = keylist[i];
                auto index = keystr.indexOf("[");
                if(index == -1)
                {
                    qreal val = 0;
                    memcpy(&val, pData, 8);
                    obj.insert(keystr, QJsonValue(val));
                    pData += 8;
                }
                else
                {
                    int siz = QString(keystr[index + 1]).toInt();
                    QJsonArray arr;
                    for(int j = 0; j < siz; j++)
                    {
                        qreal val = 0;
                        memcpy(&val, pData + 8 * j, 8);
                        arr.append(val);
                    }
                    obj.insert(keystr.chopped(3), arr);
                    pData += 8 * siz;
                }
            }
            //自定义值可在下面添加---------------------------------------------------

            //--------------------------------------------------------------------
            QJsonDocument doc(obj);
            m_plot_client->sendTextMessage(doc.toJson());
            m_plot_client->flush();
        }

        //30003数据反馈处理
        bool errFlag = false;
        QString warnMsg_1 = "";
        QString warnMsg_2 = "";
        //判断数据是否未更新
        if(tmpInfo.time == m_lastTS)
        {
            m_updateCnt++;
            if(m_updateCnt >= 100)
            {
                m_updateCnt = 0;
                LOG_EXCEPTION_BOT << m_ip << " 30003 timestamp repeat" << tmpInfo.time;
                m_lastTS = 0;
                m_connected = false;
                emit conChange(false);
                m_hbChkTimer->stop();
                m_hbSndTimer->stop();
                if(m_socket != nullptr)
                    m_socket->disconnectFromHost();
                return;
            }
        }
        else
        {
            m_lastTS = tmpInfo.time;
            m_updateCnt = 0;
        }
        //判断安全模式状态
        if(tmpInfo.safetyModes == 3 || tmpInfo.safetyModes > 4)
        {
            if(m_safetyModesFlag)
            {
                LOG_EXCEPTION_BOT << m_ip << "safetymodes" << tmpInfo.safetyModes;
                m_safetyModesFlag = false;
                errFlag = true;
            }
        }
        else
            m_safetyModesFlag = true;
        //判断是否有急停命令
        unsigned char di = tmpInfo.digitalInputBits;
        if((di & 0x02) > 0)
        {
            if(m_instantStopFlag)
            {
                LOG_INFO << "机械臂按钮主动停止调理:" << m_ip;
                m_instantStopFlag = false;
                errFlag = true;
            }
        }
        else
            m_instantStopFlag = true;
        //判断是否下发自由驱动
        if((di & 0x08) > 0)
        {
            if(!m_freeDriveFLag)
            {
                m_freeDriveFLag = true;
                enableFreeDrive(true);
            }
        }
        else
        {
            if(m_freeDriveFLag)
            {
                m_freeDriveFLag = false;
                enableFreeDrive(false);
            }
        }
        //判断机械臂脚本运行状态
        if(tmpInfo.programState != 2)
        {
            if(m_progStateFlag)
            {
                LOG_EXCEPTION_BOT << "robot: " << m_ip << "programState" << tmpInfo.programState;
                m_progStateFlag = false;
                errFlag = true;
            }
        }
        //判断工具力控
        //获取最近15ms内tcp的平均值
        auto avg_tcp = getAvgTcp(tmpInfo.tcpForce[0], tmpInfo.tcpForce[1], tmpInfo.tcpForce[2]);
        auto ifcond = [&avg_tcp](QString type, double threhold)->bool{
            if(type == ">")
                return qAbs(avg_tcp[0]) > threhold || qAbs(avg_tcp[1]) > threhold || qAbs(avg_tcp[2]) > threhold;
            else
                return qAbs(avg_tcp[0]) < threhold && qAbs(avg_tcp[1]) < threhold && qAbs(avg_tcp[2]) < threhold;
        };
        //获取最近速度的平均值
        auto avg_spd = getAvgSpd(tmpInfo.tcpSpeedActual[0], tmpInfo.tcpSpeedActual[1], tmpInfo.tcpSpeedActual[2]);
        //力控软保护
        if(ifcond(">", g_testSoftProtect))
        {
            if(m_softProtectiveStopFlag)
            {
                LOG_EXCEPTION_BOT << m_ip << "soft force protect: " << avg_tcp;
                m_softProtectiveStopFlag = false;
                errFlag = true;
            }
        }
        //中止调理或改变力控移动方向
        if(errFlag)
            emit informPauseTreatment();
        else
        {
            m_auto_mutex.lock();
            auto p1 = m_auto_startpt;
            auto p2 = m_auto_endpt;
            m_auto_mutex.unlock();
            if(p1.size() > 0 && p2.size() > 0)
            {
                double th_low = g_runArgs["AutoForceThrehold"].split(',').at(0).toDouble();
                double th_high = g_runArgs["AutoForceThrehold"].split(',').at(1).toDouble();
                if((m_ip == g_RobotrightIP && g_runArgs["EnableRightSensor"] == "true") ||
                        (m_ip == g_RobotleftIP && g_runArgs["EnableLeftSensor"] == "true"))
                {
                    th_low = 4;
                    th_high = 10;
                }

                QVector<double> v11 = {p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]};
                QVector<double> v22 = {tmpInfo.toolVectorActual[0] - p2[0], tmpInfo.toolVectorActual[1] - p2[1], tmpInfo.toolVectorActual[2] - p2[2]};
                auto k = v11[0] * v22[0] + v11[1] * v22[1] + v11[2] * v22[2];

                if(m_reduceFlag && m_recover_cnt <= 30)
                {
                    auto mod = [=](QVector<double> vec)->double{
                      return qSqrt(qPow(vec[0], 2) + qPow(vec[1], 2) + qPow(vec[2], 2));
                    };
                    QVector<double> v1 = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
                    QVector<double> v2 = {tmpInfo.toolVectorActual[0] - m_last_toolpos[0], tmpInfo.toolVectorActual[1] - m_last_toolpos[1], tmpInfo.toolVectorActual[2] - m_last_toolpos[2]};
                    auto k = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
                    m_last_toolpos[0] = tmpInfo.toolVectorActual[0];
                    m_last_toolpos[1] = tmpInfo.toolVectorActual[1];
                    m_last_toolpos[2] = tmpInfo.toolVectorActual[2];
                    if(mod(avg_spd) >= DEFAULT_CURE_SPEED * 1.4)
                        m_recover_cnt++;
                    else
                        m_recover_cnt = 0;
                }

                if(ifcond(">", th_high))
                {
                    if(!m_reduceFlag)
                    {
                        m_reduceFlag = true;
                        emit reduceForce();
                        m_recover_cnt = 0;
                    }
                }
                else if(ifcond("<", th_low) || m_recover_cnt > 30 || k < 0)
                {
                    if(m_reduceFlag)
                    {
                        m_reduceFlag = false;
                        if(m_recover_cnt > 30)
                            emit recoverForce("当前速度表明已悬空");
                        else if(k < 0)
                            emit recoverForce("当前位置超过目标点");
                        else
                            emit recoverForce("当前受力低于恢复阈值");
                        m_recover_cnt = 0;
                    }
                }
            }
        }
    }
}

RealtimeRobotStatus RealtimeMonClient::getRobotStatus() {
    RealtimeRobotStatus stat;
    m_statusMutex.lock();
    stat = m_rbInfo;
    m_statusMutex.unlock();
    return stat;
}

QVector<double> RealtimeMonClient::getAvgTcp(double x, double y, double z)
{
    if(m_tcplist.isEmpty())
    {
        QList<double> tmp1 = {x};
        QList<double> tmp2 = {y};
        QList<double> tmp3 = {z};
        m_tcplist.append(tmp1);
        m_tcplist.append(tmp2);
        m_tcplist.append(tmp3);
    }
    else
    {
        m_tcplist[0].append(x);
        m_tcplist[1].append(y);
        m_tcplist[2].append(z);
    }
    QVector<double> ret = {0, 0, 0};
    if(m_tcplist[0].size() < 75)
        return ret;
    else
    {
        for(int i = 0; i < 3; i++)
        {
            double sum = 0;
            for(int j = 0; j < m_tcplist[i].size(); j++)
                sum += m_tcplist[i][j];
            ret[i] = sum / m_tcplist[i].size();
        }
        for(int i = 0; i < 3; i++)
            m_tcplist[i].removeAt(0);
        return ret;
    }
}

QVector<double> RealtimeMonClient::getAvgSpd(double x, double y, double z)
{
    if(m_spdlist.isEmpty())
    {
        QList<double> tmp1 = {x};
        QList<double> tmp2 = {y};
        QList<double> tmp3 = {z};
        m_spdlist.append(tmp1);
        m_spdlist.append(tmp2);
        m_spdlist.append(tmp3);
    }
    else
    {
        m_spdlist[0].append(x);
        m_spdlist[1].append(y);
        m_spdlist[2].append(z);
    }
    QVector<double> ret = {0, 0, 0};
    if(m_spdlist[0].size() < 30)
        return ret;
    else
    {
        for(int i = 0; i < 3; i++)
        {
            double sum = 0;
            for(int j = 0; j < m_spdlist[i].size(); j++)
                sum += m_spdlist[i][j];
            ret[i] = sum / m_spdlist[i].size();
        }
        for(int i = 0; i < 3; i++)
            m_spdlist[i].removeAt(0);
        return ret;
    }
}

void RealtimeMonClient::setAutoArgs(QVector<double> p1, QVector<double> p2)
{
    m_auto_mutex.lock();
    m_auto_startpt = p1;
    m_auto_endpt = p2;
    m_auto_mutex.unlock();
    m_reduceFlag = false;
}
