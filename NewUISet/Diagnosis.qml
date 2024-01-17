import QtQuick 2.0
import QtQuick.Window 2.12
import TaoQuick 1.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.5

Item {
    id: item1
    CusButton {
        id: button
        width: 180
        height: 40
        text: qsTr("非特异性下腰疼诊断系统")
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.leftMargin: 40
        backgroundColor: "white"
        onClicked: {
            var args = {"func": "callOnQA"}
            guiIntf.guiCall(args)
        }
    }

    CusButton {
        id: button1
        width: 126
        height: 40
        text: qsTr("导入")
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 252
        backgroundColor: "#ffffff"
        anchors.topMargin: 40
        onClicked: {
            var doc = guiIntf.loadJson()
            homePage.cureStart_zhenduan(doc)
            homePage.toPage(1)
        }
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
