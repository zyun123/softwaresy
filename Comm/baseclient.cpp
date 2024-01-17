#include "baseclient.h"
#include "global.h"
#include <QProcess>
#include <QThread>
#include "Other/Logger.h"

#define RECON_INTERVAL 3000//连接失败尝试重连间隔时间ms
#define TCP_SEND_INTERVAL 10//tcp发包间隔
#define HEARTBEAT_SEND_INTERVAL 1000//心跳发送间隔
#define HEARTBEAT_CHECK_INTERVAL 3500//心跳校验间隔

BaseClient::BaseClient(QString ip, quint16 port, qint64 readBufSize, QObject *parent) : QObject(parent)
{
    m_socket = nullptr;
    m_keepConnecting = true;
    m_readBuffSize = readBufSize;
    m_connected = false;
    m_hbSndMsg = "";
    m_hbRcvMsg = "";
    m_retMsg = false;

    if(g_runArgs["LinkRealBot"] == "true")
    {
        m_ip = ip;
        m_port = port;
    }
    else
    {
        m_ip = "localhost";
        if(ip == "192.168.1.102" && port == 29999)
            m_port = 9991;
        else if(ip == "192.168.1.102" && port == 30003)
            m_port = 9992;
        else if(ip == "192.168.1.101" && port == 29999)
            m_port = 9993;
        else if(ip == "192.168.1.101" && port == 30003)
            m_port = 9994;
        else if(ip == "192.168.1.22" && port == 1234)
            m_port = 9995;
        else
            m_port = port;
    }
}

void BaseClient::iniConnection()
{
    m_reconTimer = new QTimer(this);
    connect(m_reconTimer, &QTimer::timeout, this, [=]{
        if(m_keepConnecting)
            connectToHost();
    });

    m_hbSndTimer = new QTimer(this);
    m_hbSndTimer->setInterval(HEARTBEAT_SEND_INTERVAL);
    connect(m_hbSndTimer, &QTimer::timeout, this, [=]{
        if(m_connected)
            addSndMsg(m_hbSndMsg.toLocal8Bit());
    });

    m_hbChkTimer = new QTimer(this);
    m_hbChkTimer->setInterval(HEARTBEAT_CHECK_INTERVAL);
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

    connectToHost();

    m_sndTimer = new QTimer(this);
    connect(m_sndTimer, &QTimer::timeout, this, [=]{
        m_mutex.lock();
        if(m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState && !m_sndQ.empty())
        {
            QByteArray msg = m_sndQ.dequeue();
            m_socket->write(msg);
            m_socket->flush();
            if(m_hbSndTimer->isActive())
                m_hbSndTimer->start();
        }
        else
            m_sndQ.clear();
        m_mutex.unlock();
    });
    m_sndTimer->start(TCP_SEND_INTERVAL);
}

void BaseClient::iniHeartBeatMsg(QString snd, QString rcv)
{
    m_hbSndMsg = snd;
    m_hbRcvMsg = rcv;
    if(snd != "")
        m_hbSndTimer->start();
    if(rcv != "")
        m_hbChkTimer->start();
}

BaseClient::~BaseClient()
{
    m_keepConnecting = false;
    m_socket->disconnectFromHost();
}

void BaseClient::connectToHost()
{
    m_mutex.lock();
    m_sndQ.clear();
    m_rcvMsg.clear();
    if(m_socket != nullptr)
    {
        delete m_socket;
        m_socket = nullptr;
    }
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::disconnected, this, &BaseClient::processDisCon);
    connect(m_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, [=](QAbstractSocket::SocketError e){
        LOG_EXCEPTION_SYS << QString("%1 %2 connect error: %3...reconnecting...").arg(m_ip).arg(m_port).arg(e);
        if(!m_reconTimer->isActive())
            m_reconTimer->start(RECON_INTERVAL);
        if(m_connected)
        {
            m_connected = false;
            emit conChange(false);
            m_hbChkTimer->stop();
            m_hbSndTimer->stop();
        }
    });
    connect(m_socket, &QTcpSocket::connected, this, [=]{
        if(m_reconTimer->isActive())
            m_reconTimer->stop();
        if(!m_connected)
        {
            m_connected = true;
            emit conChange(true);
        }
        if(m_hbSndMsg != "")
            m_hbSndTimer->start();
        if(m_hbRcvMsg != "")
            m_hbChkTimer->start();
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &BaseClient::processMsgRcv);
    m_socket->setReadBufferSize(m_readBuffSize);
    if(m_port == 30003)
        m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_socket->connectToHost(m_ip, m_port);
    m_socket->waitForConnected();
    m_mutex.unlock();
}

void BaseClient::addSndMsg(QByteArray msg)
{
    m_mutex.lock();
    if(m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
        m_sndQ.enqueue(msg);
    m_mutex.unlock();
}

void BaseClient::processMsgRcv()
{
    if(m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
    {
        auto raw = m_socket->readAll();
        m_mutex.lock();
        m_rcvMsg.clear();
        if(m_hbRcvMsg == "" || raw != m_hbRcvMsg.toLocal8Bit()  && m_ip != "192.168.1.22" ||
                raw.mid(2, raw.size() - 2) != m_hbRcvMsg.toLocal8Bit() && m_ip == "192.168.1.22")
        {
            if(m_retMsg)
                emit retMsg(raw);
            else
                m_rcvMsg = raw;
            m_cond_mutex.lock();
            m_cond.wakeAll();
            m_cond_mutex.unlock();
        }
        if(m_hbChkTimer->isActive())
            m_hbChkTimer->start();
        m_mutex.unlock();
    }
}

void BaseClient::processDisCon()
{
    if(!m_reconTimer->isActive())
        m_reconTimer->start(RECON_INTERVAL);
    if(m_connected)
    {
        m_connected = false;
        emit conChange(false);
        m_hbChkTimer->stop();
        m_hbSndTimer->stop();
    }
}

QByteArray BaseClient::getRcvMsg()
{
    QByteArray ret;
    m_mutex.lock();
    ret = m_rcvMsg;
    m_mutex.unlock();
    return ret;
}

bool BaseClient::wait(int timeout)
{
    bool ret = false;
    m_cond_mutex.lock();
    if(m_connected && m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
        ret = m_cond.wait(&m_cond_mutex, timeout);
    else
    {
        LOG_EXCEPTION_SYS << "already disconnected: " << m_ip << " " << m_port;
        QThread::msleep(timeout);
    }
    m_cond_mutex.unlock();
    return ret;
}
