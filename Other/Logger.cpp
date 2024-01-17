#include "Logger.h"
#include "LoggerTemplate.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QDebug>
#include <QTextStream>
#include <string>
#include <QProcess>

namespace Logger {
static QString gLogDir;
static QTextStream async_textStream;
static QMutex async_mutex;

static void outputMessageAsync(QtMsgType type, const QMessageLogContext &context, const QString &msg);

void initLog(const QString &logPath)
{
    qInstallMessageHandler(outputMessageAsync);
    auto year = QDateTime::currentDateTime().date().year();
    auto month = QDateTime::currentDateTime().date().month();
    auto path = logPath + "/" + QString::number(year) + "." + QString::number(month);
    QProcess cmd;
    cmd.start("mkdir", {"-p", path});
    cmd.waitForFinished();
    QDir dir(path);
    Q_ASSERT(dir.exists());
    gLogDir = path;
}

void flushCurLog()
{
    async_mutex.lock();
    async_textStream.flush();
    async_mutex.unlock();
}

static void outputMessageAsync(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static const QString messageTemp = QStringLiteral("<div class=\"%1\">%2</div>\r\n");
    static QFile file;
    static uint count = 0;
    static const uint maxCount = 2 * 1024;//超过多少字节往文件刷新一次
    //解析日志类型
    auto rawmsg = msg;
    QString msgtype = "exception-sys";
    if(rawmsg.contains('@'))
    {
        auto tmp = rawmsg.split('@');
        msgtype = tmp[0];
        rawmsg = tmp[1];
    }
    //打包日志输出
    QString message;
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
    if(msgtype == "robotl" || msgtype == "robotr")
        message = QStringLiteral("%1 |%2\n").arg(timestamp).arg(rawmsg.remove('"'));
    else
        message = QStringLiteral("%1 %2 %3 %4\n").arg(timestamp).arg(QString(context.file)).arg(QString::number(context.line)).arg(rawmsg);
    QString htmlMessage = messageTemp.arg(msgtype).arg(message);
    //输出到缓存或本地文件
    QString fileNameDt = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_hh"));//每小时一个文件
    QString logfileName = QStringLiteral("%1/%2_log.html").arg(gLogDir).arg(fileNameDt);
    async_mutex.lock();
    if (file.fileName() != logfileName)//创建下一个日志文件
    {
        if(file.isOpen())
        {
            async_textStream.flush();
            file.close();
        }
        file.setFileName(logfileName);
        auto already_exist = file.exists();
        int cnt = 5;
        while(cnt--)
        {
            if(file.open(QIODevice::WriteOnly | QIODevice::Append))
                break;
        }
        Q_ASSERT(cnt > 0);//日志无法建立直接退出程序
        async_textStream.setDevice(&file);
        async_textStream.setCodec("UTF-8");
        if(!already_exist)
            async_textStream << logTemplate << "\r\n";
    }
    async_textStream << htmlMessage;
    count += htmlMessage.size();
    if (count >= maxCount)
    {
        count = 0;
        async_textStream.flush();
    }
    async_mutex.unlock();
    fprintf(stderr, "%s", message.toStdString().data());
}

} // namespace Logger
