#include "devserver.h"
#include <QThread>
#include "global.h"

#define TCP_SEND_INTERVAL 10//tcp发包间隔
#define HEARTBEAT_SEND_INTERVAL 1000//心跳发送间隔
#define HEARTBEAT_CHECK_INTERVAL 3500//心跳校验间隔

DevServer::DevServer(QString ip, quint16 port, QObject *parent) : QObject(parent)
{
    if(g_runArgs["LinkRealBot"] == "true")
    {
        m_ip = ip;
        m_port = port;
    }
    else
    {
        m_ip = "localhost";
        m_port = port;
    }
    m_socket = nullptr;
    m_connected = false;
    m_hbSndMsg = "";
    m_hbRcvMsg = "";
}

void DevServer::iniHeartBeatMsg(QString snd, QString rcv)
{
    m_hbSndMsg = snd;
    m_hbRcvMsg = rcv;
    if(snd != "")
        m_hbSndTimer->start(HEARTBEAT_SEND_INTERVAL);
    if(rcv != "")
        m_hbChkTimer->start(HEARTBEAT_CHECK_INTERVAL);
}

bool DevServer::iniServer()
{
    m_sndTimer = new QTimer(this);
    connect(m_sndTimer, &QTimer::timeout, this, [=]{
        m_mutex.lock();
        if(m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState && !m_sndCache.empty())
        {
            QByteArray msg = m_sndCache.dequeue();
            m_socket->write(msg);
            m_socket->flush();
        }
        else
            m_sndCache.clear();
        m_mutex.unlock();
    });
    m_sndTimer->start(TCP_SEND_INTERVAL);

    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, [=]{
        m_socket = m_server->nextPendingConnection();
        connect(m_socket, &QTcpSocket::disconnected, this, [=]{
            if(m_connected)
            {
                m_connected = false;
                emit conChange(false);
                m_hbChkTimer->stop();
                m_hbSndTimer->stop();
            }
            m_mutex.lock();
            m_sndCache.clear();
            m_rcvMsg.clear();
            m_mutex.unlock();
        });
        connect(m_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, [=](QAbstractSocket::SocketError e){
            LOG_EXCEPTION_SYS << QString("%1 %2 connect error: %3").arg(m_ip).arg(m_port).arg(e);
            if(m_connected)
            {
                m_connected = false;
                emit conChange(false);
                m_hbChkTimer->stop();
                m_hbSndTimer->stop();
            }
        });
        connect(m_socket, &QTcpSocket::readyRead, this, [=]{
            QByteArray raw = m_socket->readAll();
            m_mutex.lock();
            m_rcvMsg.clear();
            if(m_hbRcvMsg == "" || raw != m_hbRcvMsg.toLocal8Bit())
            {
                m_rcvMsg = raw;
                m_cond_mutex.lock();
                m_cond.wakeAll();
                m_cond_mutex.unlock();
                if(m_port == g_Host1Port)
                    LOG_MSG_BOTLEFT << "rb_l | rcv |" << raw.replace('<', "").replace('>', "");
                else if(m_port == g_Host2Port)
                    LOG_MSG_BOTRIGHT << "rb_r | rcv |" << raw.replace('<', "").replace('>', "");
            }
            if(m_hbChkTimer->isActive())
                m_hbChkTimer->start(HEARTBEAT_CHECK_INTERVAL);
            m_mutex.unlock();
        });
        if(!m_connected)
        {
            m_connected = true;
            emit conChange(true);
        }
        if(m_hbSndMsg != "")
            m_hbSndTimer->start(HEARTBEAT_SEND_INTERVAL);
        if(m_hbRcvMsg != "")
            m_hbChkTimer->start(HEARTBEAT_CHECK_INTERVAL);
    });

    m_hbSndTimer = new QTimer(this);
    connect(m_hbSndTimer, &QTimer::timeout, this, [=]{
        if(m_connected)
            addSndMsg(m_hbSndMsg.toLocal8Bit());
    });

    m_hbChkTimer = new QTimer(this);
    connect(m_hbChkTimer, &QTimer::timeout, this, [=]{
        if(m_connected)
        {
            m_connected = false;
            emit conChange(false);
            m_hbChkTimer->stop();
            m_hbSndTimer->stop();
            if(m_socket != nullptr)
                m_socket->disconnectFromHost();
        }
    });

    if(!m_server->listen(QHostAddress(m_ip), m_port))
    {
        LOG_EXCEPTION_SYS << "listen failed: " << m_ip << " " << m_port;
        return false;
    }
    else
    {
        LOG_EXCEPTION_SYS << "listen established: " << m_ip << " " << m_port;
        return true;
    }
}

void DevServer::addSndMsg(QByteArray msg)
{
    m_mutex.lock();
    if(m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
        m_sndCache.enqueue(msg);
    m_mutex.unlock();
}

QByteArray DevServer::getRcvMsg()
{
    QByteArray ret;
    m_mutex.lock();
    ret = m_rcvMsg;
    m_mutex.unlock();
    return ret;
}

int DevServer::wait(int timeout)
{
    int ret = 0;
    m_cond_mutex.lock();
    if(m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
    {
        if(m_cond.wait(&m_cond_mutex, timeout))
            ret = 0;
        else
            ret = 1;
    }
    else
    {
        LOG_EXCEPTION_SYS << m_ip << " " << m_port << " client might be disconnected...";
        ret = 2;
    }
    m_cond_mutex.unlock();
    return ret;
}
