import QtQuick 2.0
import QtQuick.Controls 2.3
import TaoQuick 1.0

Item {
    id: item1
    width: 1920
    height: 1080

    Connections {
        target: guiIntf
        onUpdateBedStat:{
            textInput1.text = pos
        }
    }

    Text {
        id: text1
        x: 200
        y: 41
        width: 90
        height: 33
        text: qsTr("目标位置")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 13
        color: "white"
    }

    Text {
        id: text2
        x: 200
        y: 88
        width: 90
        height: 33
        text: qsTr("当前位置")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 13
        color: "white"
    }

    Rectangle {
        id: rectangle
        x: 296
        y: 41
        width: 78
        height: 33
        color: "#ffffff"
        border.width: 1
        TextInput {
            id: textInput
            text: qsTr("4000")
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }
    }

    Rectangle {
        id: rectangle1
        x: 296
        y: 88
        width: 78
        height: 33
        color: "#ffffff"
        border.width: 1
        TextInput {
            id: textInput1
            text: qsTr("0")
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }
    }

    CusButton {
        id: button
        x: 394
        y: 41
        width: 56
        height: 33
        text: qsTr("移动")
        onClicked: {
            var args = {"func": "mvbed",
                "arg_str": textInput.text}
            guiIntf.guiCall(args)
        }
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }

    CusButton {
        id: button1
        x: 394
        y: 88
        width: 56
        height: 33
        text: qsTr("刷新")
        onClicked: {
            var args = {"func": "updateBedPos"}
            guiIntf.guiCall(args)
        }
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }

    CusButton {
        id: button2
        x: 463
        y: 41
        width: 56
        height: 33
        text: qsTr("停止")
        onClicked: {
            var args = {"func": "mvbed",
                "arg_str": "9999"}
            guiIntf.guiCall(args)
        }
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }

    Text {
        id: text3
        x: 42
        y: 33
        width: 162
        height: 48
        color: "#add8e6"
        text: qsTr("调理床：")
        font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
    }

    Text {
        id: text5
        x: 24
        y: 180
        width: 162
        height: 48
        color: "#add8e6"
        text: qsTr("机械臂示教：")
        font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
    }

    Text {
        id: text6
        x: 36
        y: 282
        width: 162
        height: 48
        color: "#add8e6"
        text: qsTr("日志查询：")
        font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
    }

    CusButton {
        id: button3
        x: 214
        y: 290
        width: 100
        height: 33
        text: qsTr("系统日志")
        onClicked: guiIntf.openLog()
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }

    CusButton {
        id: button4
        x: 350
        y: 290
        width: 100
        height: 33
        text: qsTr("刷新日志文件")
        onClicked: {
            var args = {"func": "flushLog"}
            guiIntf.guiCall(args)
        }
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }

    CusButton {
        id: button5
        x: 214
        y: 188
        width: 100
        height: 33
        text: qsTr("启动示教器")
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
        onClicked: {
            guiIntf.openVNC()
        }
    }

}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}D{i:11}D{i:12}D{i:13}D{i:16}
}
##^##*/
