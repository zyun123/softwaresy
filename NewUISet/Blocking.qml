import QtQuick 2.0

Rectangle{
    id: blockpg
    color: "black"
    opacity: 0.8

    property string msg: ""
    width: 1280
    height: 720

    function showBlockingMsg(s, t){
        if(s === "clear")
            msg = ""
        else if(s === "close")
        {
            blockpg.visible = false
            msg = ""
        }
        else
        {
            if(blockpg.visible === false)
                blockpg.visible = true
            if(msg === "")
                msg = s
            else
                msg += "\n" + s
            if(t > 0)
            {
                delayTimer.interval = t
                delayTimer.start()
            }
        }
    }

    Timer {
        id: delayTimer
        interval: 1000
        running: false
        onTriggered: {
            delayTimer.stop()
            blockpg.visible = false
            msg = ""
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        propagateComposedEvents: false
        hoverEnabled: true
        onClicked: {}
        onReleased: {}
        onPressed: {}

        Text {
            id: text1
            color: "#93c29a"
            text: msg
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: 20
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}D{i:2}
}
##^##*/
