import QtQuick 2.0
import QtQuick.Window 2.12
import TaoQuick 1.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.5

Item {
    id: item1
    width: 1920
    height: 1280
    Text {
        id: text3
        x: 308
        y: 224
        color: "#ffffff"
        text: qsTr("产品名称： 数字中医循经治疗系统")
        anchors.horizontalCenterOffset: -1
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 25
    }

    Text {
        id: text1
        x: 308
        y: 357
        color: "#ffffff"
        text: qsTr("完整版本：V1.0.2.2")
        anchors.horizontalCenterOffset: -1
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 20
    }

    Text {
        id: text2
        x: 308
        y: 411
        color: "#ffffff"
        text: qsTr("发布版本：V1")
        anchors.horizontalCenterOffset: 1
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 20
    }

    Text {
        id: text4
        x: 307
        y: 292
        color: "#ffffff"
        text: qsTr("产品型号： SY-3")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 25
    }

    Text {
        id: text5
        x: 307
        y: 510
        color: "#ffffff"
        text: qsTr("公司网址： https://www.zksylf.com/")
        anchors.horizontalCenterOffset: 0
        font.pointSize: 20
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text6
        x: 316
        y: 457
        color: "#000000"
        text: qsTr("版权所有")
        font.pointSize: 20
        anchors.horizontalCenterOffset: 244
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text7
        x: 309
        y: 457
        color: "#ffffff"
        text: qsTr("中科尚易(北京)健康科技有限公司")
        font.pointSize: 20
        anchors.horizontalCenterOffset: -23
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text8
        x: 302
        y: 678
        width: 1251
        height: 40
        color: "#ffffff"
        text: qsTr("警告： 本软件程序受版权法和国际公约的保护，如未经授权而擅自复制或传播本软件程序或其中的任何部分，将受到严厉的民事及刑事制裁，还将在法律许可的范围内受到最大程度的起诉。")
        wrapMode: Text.WrapAnywhere
        font.pointSize: 20
        anchors.horizontalCenterOffset: 43
        anchors.horizontalCenter: parent.horizontalCenter
    }


}





/*##^##
Designer {
    D{i:0;formeditorZoom:0.33}D{i:6}D{i:7}D{i:8}
}
##^##*/
