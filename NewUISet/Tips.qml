import QtQuick 2.0

Item {
    id:messageBox
    width: msgBox.width
    y:50
    anchors{
        horizontalCenter: parent.horizontalCenter
    }

    property alias text: msg.text
    property bool show: false
    property string type: "info"
    visible: show

    function showTip(data) {
        if (data && data.show) {
            show = data.show
        }
        if (data && data.message) {
            text = data.message
        }
        if (data && data.type) {
            type = data.type
        }
        if (data && data.type && data.type !== 'success'
                && data.type !== 'error' && data.type !== 'info') {
            return false
        }
        timerId.start()
        messageBox.y=80
    }

    Behavior on y {
        NumberAnimation{
            duration: 200
        }
    }
    Timer{
        id:timerId
        interval: 3000
        repeat: false
        onTriggered: {
            messageBox.y=50
            timerHide.start()
        }
    }
    Timer{
        id:timerHide
        interval: 300
        repeat: false
        onTriggered: {
            show=false
        }
    }
    Rectangle{
        id:msgBox
        clip: true
        width: 300/*{
            if ($Utils.boxWidth(img)+$Utils.boxWidth(msg)<300){
                return 300
            }else
                return $Utils.boxWidth(img)+$Utils.boxWidth(msg)>600?600:$Utils.boxWidth(img)+$Utils.boxWidth(msg)+15
        }*/
        height: 50
        color: type==="info"?"#fdfdec":type==="success" ? "#f0f9eb":"#fef0f0"
        border.color: type==="info"?"#faecd8":type==="success"?"#e1f3d8":"#fde2e2"
        radius: 5
        Image {
            id: img
            source:type=== "info"?"../Resource/images/remind.png":type==="success"?"../Resource/images/success.png":"../Resource/images/failicon.png"
            width: 24
            height: 24
            anchors{

                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin:15
            }
        }
        Text{
            id:msg
            color: type==="info"?"#e6a23c":type==="success"?"#67c23a":"#f56c6c"
            font.pixelSize: 16
            anchors{

                verticalCenter: img.verticalCenter
                left: img.right
                leftMargin: 10
            }
        }
    }
}
