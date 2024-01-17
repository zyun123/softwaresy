#ifndef MYSIGNALSPY_H
#define MYSIGNALSPY_H

#include <QObject>
#include <QMap>

class MySignalSpy : public QObject
{
public:
    explicit MySignalSpy(QObject *parent = nullptr);
    void setupDynamicConnections(QObject* obj);
    int qt_metacall(QMetaObject::Call c, int id, void **arguments) override;

private:
    QObject* m_target;
    QMap<int, int> m_dynamicSlotMapping;
};

#endif // MYSIGNALSPY_H
