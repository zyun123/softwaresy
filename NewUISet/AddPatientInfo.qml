import QtQuick 2.0
import TaoQuick 1.0
import QtQuick.Controls 2.3

Item {
    id: jmenu
    width: 1280
    height: 720
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }
    Rectangle {
        id: rectangle
        color: "#000000"
        anchors.fill: parent
        opacity: 0.8

        Rectangle {
            id: rectangle1
            x: 469
            y: 247
            width: 438
            height: 238
            color: "#ffffff"
            radius: 5
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            CusButton {
                id: cusButton1
                x: 320
                y: 177
                width: 74
                height: 29
                text: "确定"
                fontsize: 18
                hoverEnabled: false
                backgroundColor: "#64d0d2"
                onClicked: {
                    rightcontrol.patient_name = textField.text
                    rightcontrol.phone = textField1.text
                    rightcontrol.set_patient_name("当前患者：  " + textField.text)
                    jmenu.visible = false
                }
            }

            Text {
                id: text1
                x: 80
                y: 56
                text: qsTr("姓名")
                anchors.verticalCenter: textField.verticalCenter
                font.pixelSize: 12
            }

            TextField {
                id: textField
                x: 119
                y: 45
                width: 242
                height: 40
                text: qsTr("")
            }

            Text {
                id: text2
                x: 56
                y: 92
                text: "联系方式"
                anchors.verticalCenter: textField1.verticalCenter
                font.pixelSize: 12
                anchors.verticalCenterOffset: 0
            }

            TextField {
                id: textField1
                x: 119
                y: 108
                width: 242
                height: 40
                text: qsTr("")
            }
        }
    }

}
