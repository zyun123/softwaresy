#ifndef MYFILE_H
#define MYFILE_H

#include <QObject>
#include <QFile>

class MyFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit MyFile(QObject *parent = nullptr);

    QString source(){return m_source;}
    QString text(){return m_text;}
    void setSource(QString);
    void setText(QString);

signals:
    void sourceChanged();
    void textChanged();

private:
    QString m_source;
    QString m_text;
};

#endif // MYFILE_H
