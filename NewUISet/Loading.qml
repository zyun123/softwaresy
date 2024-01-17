import QtQuick 2.0
import QtGraphicalEffects 1.12

Item {
    id: item1
    opacity: 1

    property real progress: 0

    MouseArea{
        anchors.fill: parent
    }

    Image {
        id: background
        anchors.fill: parent
        source: "../Resource/new_ui/screen_lock.jpg"
    }

    Rectangle {
        id: rectangle
        x: 138
        y: 129
        width: 1164
        height: 102
        color: "lightgrey"
        radius: 5
        border.width: 6
        border.color: "black"
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 179
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Rectangle {
        id: rectangle1
        width: rectangle.width * progress
        height: 90
//        opacity: 0.701
//        color: "#0f7d7d"
        anchors.left: rectangle.left
        anchors.top: rectangle.top
        anchors.topMargin: 5
        anchors.leftMargin: 6

        LinearGradient{
            anchors.fill: parent
            start: Qt.point(0,0)
            end: Qt.point(width, height)
            gradient:Gradient{
                GradientStop{position: 0.0;color: "#0f7d7d"}
                GradientStop{position: 1.0;color: "white"}
            }
        }

        Behavior on width {
            NumberAnimation{duration: 500}
        }

        onWidthChanged: {
            if(progress >= 1)
                parent.visible = false
        }
    }

    Text {
        id: text1
        y: 394
        width: 281
        height: 88
        visible: false
        text: "配置加载中，请稍候。。。"
        anchors.left: rectangle.left
        anchors.bottom: rectangle.top
        font.pixelSize: 24
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        clip: true

        NumberAnimation {
            id: numberAnimation
            target: text1
            property: "width"
            duration: 5000
            loops: -1
            running: true
            to: 281
            from: 217
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.75;height:1053;width:1848}D{i:4}
}
##^##*/
