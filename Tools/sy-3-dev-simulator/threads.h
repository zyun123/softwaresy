#ifndef THREADS_H
#define THREADS_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QThread>
#include <QTimer>

class base_server : public QObject
{
    Q_OBJECT
public:
    base_server(int port, QObject *parent = nullptr): QObject(parent)
    {
        m_sv = new QTcpServer(this);
        QObject::connect(m_sv, &QTcpServer::newConnection, this, [&](){
            m_socket = m_sv->nextPendingConnection();
            while(1)
            {
                if(m_socket != nullptr && m_socket->state() == QTcpSocket::ConnectedState)
                {
                    if(!m_con)
                    {
                        qDebug() << QString::number(m_port) + " connected";
                        m_con = true;
                    }
                    process();
                }
                else
                {
                    m_con = false;
                    break;
                }
            }
        });
        m_sv->listen(QHostAddress::LocalHost, port);
        m_port = port;
        m_socket = nullptr;
        m_con = false;
    }
protected:
    QTcpServer *m_sv;
    QTcpSocket *m_socket;
    int m_port;
    bool m_con;

    virtual void process() = 0;
};

class robot_dash : public base_server
{
    Q_OBJECT
public:
    robot_dash(int port, QObject *parent = nullptr): base_server(port, parent){}
    void process() override
    {
        if(m_socket->waitForReadyRead())
        {
            auto bytes = m_socket->readAll();
            m_socket->write("UR5");
        }
    }
};

class robot_rt : public base_server
{
    Q_OBJECT
public:
    robot_rt(int port, QObject *parent = nullptr): base_server(port, parent){}
    void process() override
    {
        while(1)
        {
            if(m_socket != nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
            {
                m_socket->write("123");
                m_socket->flush();
            }
            QThread::msleep(2000);
        }
    }
};

class bed_server : public base_server
{
    Q_OBJECT
public:
    bed_server(int port, QObject *parent = nullptr): base_server(port, parent){}
    void process() override
    {
        if(m_socket->waitForReadyRead())
        {
            auto bytes = m_socket->readAll();
            if((int)bytes[1] == 0x01 && (int)bytes[2] == 0x10)
            {
                QByteArray tmp;
                tmp.append(0xaa);
                tmp.append(0x06);
                tmp.append(0xff);
                for(int i = 0; i < 5; i++)
                    tmp.append('\0');
                m_socket->write(tmp);
            }
            else
            {
                if(bytes[0] == char(0x55))
                    bytes[0] = 0xaa;
                m_socket->write(bytes);
            }
        }
    }
};

class robot_urp : public QObject
{
    Q_OBJECT
public:
    robot_urp(int port, QObject *parent = nullptr): QObject(parent)
    {
        m_port = port;
        socket = new QTcpSocket(this);
        m_conTimer = new QTimer(this);
        m_conTimer->setInterval(1000);
        connect(m_conTimer, &QTimer::timeout, this, [=](){
            socket->setReadBufferSize(1024);
            socket->connectToHost(QHostAddress::LocalHost, m_port);
            socket->waitForConnected(1000);
        });
        connect(socket, &QTcpSocket::connected, this, [=](){
            qDebug() << QString::number(m_port) + " connected";
            m_conTimer->stop();
            while(1)
            {
                if(socket != nullptr && socket->state() == QTcpSocket::ConnectedState)
                {
                    if(socket->waitForReadyRead())
                    {
                        auto bytes = socket->readAll();
                        if(bytes == QString("[9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9]").toLocal8Bit())
                            socket->write("heartbeat_feedback");
                        else
                            socket->write("m");
                    }
                }
                else
                {
                    m_conTimer->start();
                    break;
                }
            }
        });
        m_conTimer->start();
    }
private:
    QTcpSocket *socket;
    QTimer *m_conTimer;
    int m_port;
};

#endif // THREADS_H
