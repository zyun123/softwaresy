import QtQuick 2.0
import QtQuick.Window 2.12
import TaoQuick 1.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.5

Item {
    id: item1
    CusButton {
        id: button
        width: 126
        height: 40
        text: qsTr("系统日志")
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.leftMargin: 40
        onClicked: guiIntf.openLog()
        backgroundColor: "white"
    }

    CusButton {
        id: button1
        width: 126
        height: 40
        text: qsTr("刷新日志文件")
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 179
        backgroundColor: "#ffffff"
        anchors.topMargin: 40
        onClicked: {
            var args = {"func": "flushLog"}
            guiIntf.guiCall(args)
        }
    }

}
