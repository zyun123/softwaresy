#include "mysignalspy.h"
#include "Other/Logger.h"
#include <QMetaMethod>

MySignalSpy::MySignalSpy(QObject *parent) : QObject(parent)
{

}

void MySignalSpy::setupDynamicConnections(QObject* obj){
    m_target = obj;
    auto mo = obj->metaObject();
    auto offset = mo->methodOffset();
    m_dynamicSlotMapping.clear();
    for(auto i = offset; i != mo->methodCount(); ++i){
        auto m = mo->method(i);
        if(m.methodType() != QMetaMethod::Signal)
            continue;
        m_dynamicSlotMapping[i - offset] = i;
        QMetaObject::connect(obj, i, this, i, Qt::UniqueConnection);
    }
}

int MySignalSpy::qt_metacall(QMetaObject::Call c, int id, void **arguments){
    //id是槽函数的Index
    id = QObject::qt_metacall(c, id, arguments);
    if (id < 0 || c != QMetaObject::InvokeMetaMethod)
        return id;
    auto signalId = m_dynamicSlotMapping[id];
    //获得发送的信号元对象
    auto signalMethod = m_target->metaObject()->method(signalId);
    for (int i = 0; i != signalMethod.parameterCount(); ++i){
        auto type = signalMethod.parameterType(i);
        auto arg = arguments[i + 1];
        LOG_INFO << signalMethod.name() << " param" << i << "type:" << type << "value:" << arg;
    }
}
