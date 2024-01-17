import QtQuick 2.0
import TaoQuick 1.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0

Item {
    id:devicebiaodingmanager
    property string pickrobotname: "1号机器人"
    property string robotpickleft: "左侧机器人"
    property string pickcameraname: "左侧相机"
    property int deviceindex: 0
    property int cameraindex: 0
    width: 1600
    height: 900

    Rectangle{
        id:devicebiaodinginputRect
        color: "#00bef3f8"
        radius: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.rightMargin: 626
        anchors.bottomMargin: 616
        anchors.topMargin: 55
        anchors.leftMargin: 55
        opacity: 0.8
        Rectangle{
            id:devicexuanze
            width: 478
            height: 146
            color: "transparent"
            radius: 5
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 40
            anchors.topMargin: 40

            Text {
                id: camerapicklbl
                x: 17
                y: -6
                width: 100
                color: "#ffffff"
                text: qsTr("选择机器人：")
                font.pixelSize: 16
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }

            CusComboBox {
                id: ccb_robot
                x: 123
                y: -6
                width: 207
                height: 30
                displayText: qsTr(currentText)
                model: ["左侧机器人", "右侧机器人"]
            }

            Text {
                id: cameraindexlbl
                x: 17
                y: 63
                width: 100
                height: 30
                color: "#ffffff"
                text: qsTr("选择摄像头：")
                font.pixelSize: 16
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }

            CusComboBox {
                id: ccb_cameraindex
                x: 123
                y: 63
                width: 207
                height: 30
                displayText: qsTr(currentText)
                model: ["顶部摄像头", "左侧摄像头", "右侧摄像头", "头部摄像头"]
            }

            CusButton {
                id: checkBtn7
                x: 379
                y: -6
                width: 91
                height: 30
                text: "开始标定"
                Layout.preferredWidth: 160
                Layout.preferredHeight: 64
                fontsize: 18
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    var args = {"func": "calibrateStart",
                        "arg_obj": {"robot_direction": ccb_robot.currentText, "cam": ccb_cameraindex.currentText}}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: checkBtn1
                x: 506
                y: -16
                width: 129
                height: 50
                text: "校验左臂标定"
                Layout.preferredWidth: 160
                Layout.preferredHeight: 64
                onClicked: {
                    var args = {"func": "checkCaliStart",
                        "arg_str": "0|" + ccb_cameraindex.currentText}
                    guiIntf.guiCall(args)
                }
                fontsize: 18
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                visible: true
            }

            CusButton {
                id: checkBtn2
                x: 506
                y: 48
                width: 132
                height: 50
                text: "校验右臂标定"
                Layout.preferredWidth: 160
                Layout.preferredHeight: 64
                onClicked: {
                    var args = {"func": "checkCaliStart",
                        "arg_str": "1|" + ccb_cameraindex.currentText}
                    guiIntf.guiCall(args)
                }
                fontsize: 18
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                visible: true
            }

            CusButton {
                id: checkBtn8
                x: 668
                y: -18
                width: 147
                height: 49
                text: "校准左臂传感器"
                fontsize: 18
                Layout.preferredWidth: 160
                Layout.preferredHeight: 64
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    var args = {"func": "caliSensor",
                        "arg_str": "0"}
                    guiIntf.guiCall(args)
                }
                visible: true
            }

            CusButton {
                id: checkBtn9
                x: 668
                y: 49
                width: 159
                height: 49
                text: "校准右臂传感器"
                fontsize: 18
                Layout.preferredWidth: 160
                Layout.preferredHeight: 64
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    var args = {"func": "caliSensor",
                        "arg_str": "1"}
                    guiIntf.guiCall(args)
                }
                visible: true
            }


        }

    }

    CusButton {
        id: checkBtn10
        x: 477
        y: 155
        width: 91
        height: 30
        text: "精度矫正"
        fontsize: 18
        Layout.preferredWidth: 160
        Layout.preferredHeight: 64
        textColor: pressed ? "white" : "black"
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        onClicked: {
            var args = {"func": "genCaliBias"}
            guiIntf.guiCall(args)
        }
    }


}







/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}
}
##^##*/
