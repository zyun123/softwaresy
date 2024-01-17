#pragma once
#include <QString>
#include <QDebug>

#define LOG_INFO qDebug() << "info@"
#define LOG_MSG_BOTLEFT qDebug() << "robotl@"
#define LOG_MSG_BOTRIGHT qDebug() << "robotr@"
#define LOG_EXCEPTION_SYS qDebug() << "exception-sys@"
#define LOG_EXCEPTION_BOT qDebug() << "exception-bot@"
#define LOG_EXCEPTION_CAM qDebug() << "exception-cam@"
#define LOG_EXCEPTION_BED qDebug() << "exception-bed@"
#define LOG_EXCEPTION_AI qDebug() << "exception-ai@"

namespace Logger {
void initLog(const QString &logPath = QStringLiteral("Log"));
void flushCurLog();
}
