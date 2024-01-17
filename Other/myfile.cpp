#include "myfile.h"
#include "Other/Logger.h"

MyFile::MyFile(QObject *parent) : QObject(parent)
{
    m_source = "";
    m_text = "";
}

void MyFile::setSource(QString filepath)
{
    m_source = filepath;
    QFile file(filepath);
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray rawdata = file.readAll();
        m_text = QString(rawdata);
        file.close();
    }
    else
        LOG_EXCEPTION_SYS << "file open failed";
}

void MyFile::setText(QString txt)
{
    QFile file(m_source);
    if(file.open(QIODevice::WriteOnly))
    {
        QByteArray rawdata = txt.toLocal8Bit();
        file.write(rawdata);
        file.close();
    }
    else
        LOG_EXCEPTION_SYS << "file open failed";
    m_text = txt;
}
