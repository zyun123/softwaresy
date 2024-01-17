import QtQuick 2.0
import QtQuick.Controls 2.3
import TaoQuick 1.0

Item {
    id: item1

    Connections {
        target: guiIntf
        onUpdateBedStat:{
            textInput1.text = pos
        }
    }

    Text {
        id: text1
        width: 90
        height: 33
        text: qsTr("目标位置")
        anchors.left: parent.left
        anchors.top: parent.top
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.topMargin: 40
        anchors.leftMargin: 40
        font.pointSize: 13
    }

    Text {
        id: text2
        width: 90
        height: 33
        text: qsTr("当前位置")
        anchors.left: parent.left
        anchors.top: text1.bottom
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.topMargin: 20
        anchors.leftMargin: 40
        font.pointSize: 13
    }

    Rectangle {
        id: rectangle
        width: 78
        height: 33
        color: "#ffffff"
        anchors.left: text1.right
        anchors.top: text1.top
        anchors.leftMargin: 10
        anchors.topMargin: 0
        border.width: 1
        TextInput {
            id: textInput
            text: qsTr("4000")
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
            selectByMouse: true
        }
    }

    Rectangle {
        id: rectangle1
        width: 78
        height: 33
        color: "#ffffff"
        anchors.left: text1.right
        anchors.top: text1.top
        border.width: 1
        TextInput {
            id: textInput1
            text: qsTr("0")
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
            selectByMouse: true
        }
        anchors.leftMargin: 10
        anchors.topMargin: 53
    }

    CusButton {
        id: button
        width: 56
        height: 33
        text: qsTr("移动")
        anchors.left: rectangle.right
        anchors.top: text1.top
        anchors.topMargin: 0
        anchors.leftMargin: 10
        backgroundColor: "white"
        onClicked: {
            var args = {"func": "mvbed",
                        "arg_str": textInput.text}
            guiIntf.guiCall(args)
        }
    }

    CusButton {
        id: button1
        width: 56
        height: 33
        text: qsTr("刷新")
        anchors.left: rectangle.right
        anchors.top: text1.top
        anchors.leftMargin: 10
        anchors.topMargin: 53
        backgroundColor: "white"
        onClicked: {
            var args = {"func": "updateBedPos"}
            guiIntf.guiCall(args)
        }
    }

    CusButton {
        id: button2
        width: 56
        height: 33
        text: qsTr("停止")
        anchors.left: button.right
        anchors.top: text1.top
        anchors.leftMargin: 10
        anchors.topMargin: 0
        backgroundColor: "white"
        onClicked: {
            var args = {"func": "mvbed",
                        "arg_str": "9999"}
            guiIntf.guiCall(args)
        }
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1}D{i:2}D{i:4}D{i:3}D{i:5}D{i:7}D{i:9}
D{i:11}
}
##^##*/
