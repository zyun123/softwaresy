import QtQuick 2.0
import QtQuick.Controls 2.3
import UserManageModel 1.0
import TaoQuick 1.0

Item {
    id: settings
    width: 1920
    height: 1000

    function loadSettings(args)
    {
        slider.value = args["music"]
        slider1.value = args["volume"]
        comboBox.currentIndex = args["screen_lock"]
    }

    function getSettings()
    {
        var args = {}
        args["music"] = slider.value
        args["volume"] = slider1.value
        args["screen_lock"] = comboBox.currentIndex
        return args
    }

    Rectangle {
        id: rectangle
        color: "transparent"
        anchors.fill: parent

        Text {
            id: text1
            width: 162
            height: 48
            color: "#add8e6"
            text: qsTr("音量设置：")
            anchors.left: parent.left
            anchors.top: parent.top
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.topMargin: 30
            anchors.leftMargin: 20
            font.bold: true
        }

        Text {
            id: text2
            width: 130
            height: 26
            text: qsTr("背景音乐：")
            anchors.left: parent.left
            anchors.top: text1.bottom
            font.pixelSize: 15
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.topMargin: 10
            anchors.leftMargin: 60
            color: "white"
        }

        Text {
            id: text3
            width: 130
            height: 26
            text: qsTr("提示语音：")
            anchors.left: parent.left
            anchors.top: text2.bottom
            font.pixelSize: 15
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.topMargin: 15
            anchors.leftMargin: 60
            color: "white"
        }

        Slider {
            id: slider
            width: 200
            height: 23
            anchors.left: text2.right
            anchors.top: text2.top
            anchors.topMargin: 0
            anchors.leftMargin: 0
            value: 1
            background: Rectangle{
                width: parent.availableWidth
                height: 10
                anchors.centerIn: parent
                radius: 5
                color: "lightgray"
                Rectangle{
                    width: slider.value * parent.width
                    height: 10
                    anchors.left: parent.left
                    radius: 5
                    color: "green"
                }
            }
            onValueChanged: bgmplayer.volume = value
        }

        Slider {
            id: slider1
            width: 200
            height: 23
            anchors.left: text3.right
            anchors.top: text3.top
            anchors.topMargin: 0
            background: Rectangle {
                width: parent.availableWidth
                height: 10
                color: "#d3d3d3"
                radius: 5
                Rectangle {
                    width: slider1.value * parent.width
                    height: 10
                    color: "#008000"
                    radius: 5
                    anchors.left: parent.left
                }
                anchors.centerIn: parent
            }
            value: 1
            anchors.leftMargin: 0
            onValueChanged: tipplayer.volume = value
        }

        Text {
            id: text4
            width: 162
            height: 48
            color: "#add8e6"
            text: qsTr("息屏设置：")
            anchors.left: parent.left
            anchors.top: text3.bottom
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.topMargin: 20
            anchors.leftMargin: 20
            font.bold: true
        }

        Text {
            id: text5
            width: 130
            height: 26
            text: qsTr("自动息屏：")
            anchors.left: parent.left
            anchors.top: text4.bottom
            font.pixelSize: 15
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.topMargin: 10
            anchors.leftMargin: 60
            color: "white"
        }

        ComboBox {
            id: comboBox
            width: 141
            height: 32
            anchors.left: text5.right
            anchors.top: text5.top
            anchors.topMargin: 0
            anchors.leftMargin: 0
            background: Rectangle{
                anchors.fill: parent
                color: "white"
                radius: 5
            }
            model: ["10分钟", "20分钟", "40分钟", "80分钟", "从不(不推荐)"]
            currentIndex: 1
            onCurrentIndexChanged: {
                if(currentIndex == 0)
                    entrywnd.lockInterval = 10 * 60 * 1000
                else if(currentIndex == 1)
                    entrywnd.lockInterval = 20 * 60 * 1000
                else if(currentIndex == 2)
                    entrywnd.lockInterval = 40 * 60 * 1000
                else if(currentIndex == 3)
                    entrywnd.lockInterval = 80 * 60 * 1000
                else
                    entrywnd.lockInterval = -1
            }
        }

        Text {
            id: text6
            x: 20
            y: 313
            width: 162
            height: 48
            color: "#add8e6"
            text: qsTr("密码设置：")
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.bold: true
        }

        CusButton {
            id: button
            x: 190
            y: 372
            width: 80
            height: 35
            text: qsTr("修改密码")
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked: {
                modifyedit.loaddat({"userid":homePage.user_id, "password":homePage.password, "name":homePage.user_name, "role":homePage.login_role})
                modifyedit.visible = true
            }
            visible: homePage.login_role !== "super"
        }

        UserManageModel{
            id: usrMod
        }

        UserManageRowEdit{
            id: modifyedit
            z: 99
            visible: false
            onRetData: {
                if(usrMod.modifyData(["password", "name", "role", "userid"], [editdat[1], editdat[2], editdat[3], editdat[0]]) === true)
                {
                    homePage.password = editdat[1]
                    homePage.user_name = editdat[2]
                    guiIntf.qmllog(homePage.user_id + "已设置新密码")
                }
                else
                    warnmsg.showWarn("错误", "修改密码失败", "请联系管理员处理")
            }
        }
    }

}
