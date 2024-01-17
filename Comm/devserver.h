#ifndef DEVSERVER_H
#define DEVSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>

class DevServer : public QObject
{
    Q_OBJECT
public:
    explicit DevServer(QString ip, quint16 port, QObject *parent = nullptr);
    void addSndMsg(QByteArray msg);//添加待发消息至发送队列
    QByteArray getRcvMsg();//获取收到的最新消息
    void clearRcvMsg()//清空已有消息
    {
        m_mutex.lock();
        m_rcvMsg.clear();
        m_mutex.unlock();
    }
    int wait(int timeout = 1000);//挂起线程等待新包到来 timeout超时ms 返回值：正常0 超时1 连接断开2
    bool iniServer();//开始监听端口
    void iniHeartBeatMsg(QString snd, QString rcv);

signals:
    void conChange(bool);

private:
    QTcpServer *m_server;
    QTcpSocket *m_socket;
    QString m_ip;
    quint16 m_port;
    QByteArray m_rcvMsg;//最近一次接收消息
    QQueue<QByteArray> m_sndCache;//发送消息缓存
    QTimer *m_sndTimer;//发送间隔
    QMutex m_mutex;
    QWaitCondition m_cond;
    QMutex m_cond_mutex;

    QString m_hbSndMsg;//心跳发送字符串
    QString m_hbRcvMsg;//心跳接受字符串
    QAtomicInteger<bool> m_connected;//连接状态

    QTimer *m_hbChkTimer;//心跳检测计时器
    QTimer *m_hbSndTimer;//心跳发送计时器
};

#endif // DEVSERVER_H
