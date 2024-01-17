import QtQuick 2.0
import QtQuick.Controls 2.3

Button {
    id: button
    text: qsTr("")
    property string btn_txt: ""
    property var colors: ["#bcbcbc", "#bcbcbc", "#008ec9", "#9aa9f0", "white"]
    property var fontsize: 11
    property bool isChkBtn: false
    checkable: isChkBtn
    background: Rectangle{
        anchors.fill: parent
        radius: 5
        color: (isChkBtn && button.checked) ? colors[0] : (button.hovered || button.enabled === false) ? colors[1] : colors[2]
        Text {
            text: btn_txt
            font.pointSize: fontsize
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: (isChkBtn ? button.checked : (button.pressed || button.enabled === false)) ? colors[3] : colors[4]
        }
    }
}
