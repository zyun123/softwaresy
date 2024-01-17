#ifndef BASECLIENT_H
#define BASECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class BaseClient : public QObject
{
    Q_OBJECT
public:
    explicit BaseClient(QString ip, quint16 port, qint64 readBufSize = 0, QObject *parent = nullptr);
    ~BaseClient();

    void addSndMsg(QByteArray msg);//添加待发消息至发送队列
    QByteArray getRcvMsg();//获取收到的最新消息
    void clearRcvMsg(){m_rcvMsg.clear();}//清空已有消息
    bool wait(int timeout = 1000);//挂起线程等待新包到来 timeout超时ms 等到数据返回true 超时返回false
    void iniConnection();//初始化连接
    void iniHeartBeatMsg(QString snd, QString rcv);//设置心跳检测
    void setRetSig(bool flag){m_retMsg = flag;}

signals:
  void informPauseTreatment();
  void warn(QString title, QString msg);
  void conChange(bool);
  void retMsg(QByteArray);

public slots:
    virtual void processMsgRcv();
    virtual void processDisCon();

protected:
    QTcpSocket *m_socket;
    QMutex m_mutex;
    QWaitCondition m_cond;
    QMutex m_cond_mutex;
    QByteArray m_rcvMsg;//最近一次接收消息
    QString m_ip;
    quint16 m_port;
    QString m_hbSndMsg;//心跳发送字符串
    QString m_hbRcvMsg;//心跳接受字符串
    QTimer *m_hbChkTimer;//心跳检测计时器
    QTimer *m_hbSndTimer;//心跳发送计时器
    QAtomicInteger<bool> m_connected;//连接状态

private:
    qint64 m_readBuffSize;
    bool m_keepConnecting;
    QTimer *m_reconTimer;//重连间隔
    QQueue<QByteArray> m_sndQ;//发送消息缓存
    QTimer *m_sndTimer;//发送间隔
    bool m_retMsg;//是否收到数据直接返回

    void connectToHost();
};

#endif // BASECLIENT_H
