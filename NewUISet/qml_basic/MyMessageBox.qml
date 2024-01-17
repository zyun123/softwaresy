import QtQuick 2.0
import TaoQuick 1.0

Rectangle {
    id:msgbox
    color: "black"
    opacity: 0.8
    width: 1920
    height: 1080

    property int dispWidth: 552
    property int dispHeight: 274
    property string str1: ""
    property string str2: ""
    property string str3: ""
    property bool reject_enable: true

    signal accepted()
    signal rejected()

    function showWarn(s1, s2, s3){
        str1 = s1
        str2 = s2
        str3 = s3
        visible = true
    }

    MouseArea{
        anchors.fill: parent
        propagateComposedEvents: false
        hoverEnabled: true
        onClicked: {}
        onReleased: {}
        onPressed: {}
    }
    Rectangle{
        id:jingluodispRect
        width: dispWidth
        height: dispHeight
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        x:(warn.width-jingluodispRect.width)/2.0
        y:(warn.height-jingluodispRect.height)/2.0
        Rectangle{
            id:jingluofailHead
            width: jingluodispRect.width
            height: 52
            color: "#2b579a"
            Image {
                id: shibieshibaiIcon
                width: 27
                height: 27
                x:18
                y:(jingluofailHead.height-shibieshibaiIcon.height)/2.0
                source: "../../Resource/images/ys-shibieshibai.png"
            }
            Text{
                id:shibiefailLbl
                anchors.left: shibieshibaiIcon.right
                anchors.leftMargin: 10
                y:19
                text: str1
                color: "white"
                font.pixelSize: 16
                height: shibieshibaiIcon.height
                verticalAlignment: Text.AlignVCenter
            }
        }
        Text{
            id:sbibiefailmess
            x:52
            anchors.top: jingluofailHead.bottom
            anchors.topMargin: 30
            text: str2
            color: "#BF0000"
            font.bold: true
            font.pixelSize: 22
        }
        Text {
            id: shibieflaimess1
            x:52
            text: str3
            anchors.top: sbibiefailmess.bottom
            anchors.topMargin: 20
            color: "#000000"
            font.pixelSize: 16
        }
        CusButton{
            id:shibiefailBtn
            text: qsTr("确定")
            x:reject_enable ? 309 : 420
            width:100
            height:30
            anchors.top: shibieflaimess1.bottom
            anchors.topMargin: 50
            font.pixelSize: 18
            backgroundColor: "#6300bf"
            textColor: enabled ? pressed ? "black" : "white" : "#949494"
            radius: 4
            onClicked: {
                accepted()
                msgbox.visible = false
            }
        }

        CusButton {
            id: shibiefailBtn1
            x: 420
            y: 197
            width: 100
            height: 30
            radius: 4
            text: qsTr("取消")
            anchors.top: shibieflaimess1.bottom
            font.pixelSize: 18
            anchors.topMargin: 50
            backgroundColor: "#6300bf"
            textColor: enabled ? pressed ? "black" : "white" : "#949494"
            visible: reject_enable
            onClicked: {
                rejected()
                msgbox.visible = false
            }
        }
    }

}


