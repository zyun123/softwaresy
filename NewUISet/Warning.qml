import QtQuick 2.0
import TaoQuick 1.0

Rectangle {
    id:warn
    color: "black"
    opacity: 0.8

    property string str1: ""
    property string str2: ""
    property string str3: ""
    property var q1: []
    property var q2: []
    property var q3: []
    property var todo: ({})
    property bool flag: false

    function showWarn(s1, s2, s3){
        if(visible === false)
        {
            str1 = s1
            str2 = s2
            str3 = s3
            visible = true
            flag = false
            q1 = []
            q2 = []
            q3 = []
        }
        else
        {
            q1.unshift(s1)
            q2.unshift(s2)
            q3.unshift(s3)
        }
    }

    function showWarnAndExe(s1, s2, s3, args){
        if(visible === false)
        {
            str1 = s1
            str2 = s2
            str3 = s3
            visible = true
            todo = args
            flag = true
            q1 = []
            q2 = []
            q3 = []
        }
    }

    MouseArea{
        anchors.fill: parent
        propagateComposedEvents: false
        hoverEnabled: true
        onClicked: {}

        onReleased: {}

        onPressed: {}
    }
    property int dispWidth: 552
    property int dispHeight: 274
    width: 1920
    height: 1080
    Rectangle{
        id:jingluodispRect
        width: dispWidth
        height: dispHeight
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
                source: "../Resource/images/ys-shibieshibai.png"
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
//            CusButton_Image{
//                id:shibiefailExit
//                x:jingluofailHead.width-30-20
//                width: 30
//                height: 30
//                y:(jingluofailHead.height-shibiefailExit.height)/2.0
//                btnImgUrl:"qrc:/Resource/images/ys-guanbi.png"
//                onClicked: {
//                    warnmsg.visible=false

//                }
//            }
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
            text: qsTr("我知道了")
            x:215
            width:100
            height:30
            anchors.top: shibieflaimess1.bottom
            anchors.topMargin: 50
            font.pixelSize: 18
            backgroundColor: "#6300bf"
            textColor: enabled ? pressed ? "black" : "white" : "#949494"
            radius: 4
            onClicked:{
                if(q1.length == 0)
                    warnmsg.visible = false
                else
                {
                    str1 = q1.shift()
                    str2 = q2.shift()
                    str3 = q3.shift()
                }

                if(flag === true)
                {
                    guiIntf.guiCall(todo)
                    flag = false
                }
            }
        }
    }

}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}
}
##^##*/
