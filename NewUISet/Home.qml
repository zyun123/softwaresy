import QtQuick 2.0
import QtQuick.Controls 2.3
import TaoQuick 1.0
import QtQuick.Layouts 1.0
import QtQml 2.12
import UserManageModel 1.0

Item {
    id: homepage
    width: 1920
    height: 1080
    anchors.fill: parent

    property int version: 0

    property int cfg_ver: 0

    property string device_id: "sy3"
    property string user_id: ""
    property string login_role: ""
    property string user_name: ""
    property string password: ""

    property bool menu_enable: true

    signal cureStart(var plan)

    signal cureStart_zhenduan(var plan)

    function loadSettings(args)
    {
        settingPage.loadSettings(args)
    }

    function resetJL()
    {
        curePage.resetJL()
    }

    function loadMeridians(raw, id, serial, name)
    {
        curePage.loadMeridians(raw, id, serial, name)
    }

    function toPage(index)
    {
        if(index === 1)
            button1.checked = true
        else if(index === 2)
            button2.checked = true
        else if(index === 0)
            button.checked = true
        else if(index === 4)
            button4.checked = true
        else if(index === 66)
            button66.checked = true
    }

    UserManageModel{
        id: user_mod
    }

    Connections{
        target: guiIntf
        onIniDeviceID:{
            device_id = id
        }
    }

    Image {
        id: image
        anchors.fill: parent
        source: "../Resource/new_ui/normal_bg.jpg"
        fillMode: Image.Stretch
    }

    Rectangle {
        id: funcmenu
        height: 75
        color: "transparent"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0

        RowLayout {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 30
            anchors.bottomMargin: 5
            anchors.topMargin: 26
            spacing: 10

            Button {
                id: button2
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button2.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("患者管理")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button2.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable && (login_role == "doctor" || login_role == "operator" || login_role == "super")
                visible: login_role == "doctor" || login_role == "operator" || login_role == "super"
            }

            Button {
                id: button
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                autoExclusive: true
                checkable: true
                enabled: menu_enable && (login_role == "doctor" || login_role == "operator"  || login_role == "super")
                visible: login_role == "doctor" || login_role == "super"
                background: Rectangle {
                    width: 122
                    color: button.checked ? "white" : "transparent"
                    radius: 3
                    border.width: 1
                    border.color: "white"
                    Text {
                        text: qsTr("数字处方")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button.checked ? "black" : "white"
                    }
                }
            }

            Button {
                id: button22
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                autoExclusive: true
                checkable: true
                checked: false
                enabled: menu_enable && (login_role == "sv_operator" || login_role == "sv_admin")
                visible: login_role == "sv_operator" || login_role == "sv_admin"
                background: Rectangle {
                    width: 122
                    color: button22.checked ? "white" : "transparent"
                    radius: 3
                    border.width: 1
                    border.color: "white"
                    Text {
                        text: qsTr("预约列表")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button22.checked ? "black" : "white"
                    }
                }
            }

            Button {
                id: button1
                text: qsTr("")
                checked: true
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button1.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("调理控制")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button1.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable
            }

            Button {
                id: button4
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button4.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("用户管理")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button4.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable && (login_role == "admin" || login_role == "super")
                visible: login_role == "admin" || login_role == "super"
            }

            Button {
                id: button5
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button5.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("标定校准")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button5.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable && (login_role == "root" || login_role == "sv_admin" || login_role == "super")
                visible: login_role == "root" || login_role == "sv_admin" || login_role == "super"
            }

            Button {
                id: button66
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button66.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("运行维护")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button66.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable && (login_role == "root" || login_role == "sv_admin" || login_role == "super")
                visible: login_role == "root" || login_role == "sv_admin" || login_role == "super"
            }

            Button {
                id: button3
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button3.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("设置")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button3.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable
            }

            Button {
                id: button33
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button33.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("关于")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button33.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable
            }

            Button {
                id: button6
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button6.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("隐藏功能")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button6.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable && (login_role == "super" || login_role == "sv_admin")
                visible: login_role == "super"
            }

            Button {
                id: button8
                text: qsTr("")
                Layout.preferredHeight: 42
                Layout.preferredWidth: 122
                background: Rectangle {
                    width: 122
                    color: button8.checked ? "white" : "transparent"
                    radius: 3
                    border.color: "white"
                    border.width: 1
                    Text {
                        text: qsTr("诊断")
                        font.pointSize: 13
                        anchors.centerIn: parent
                        color: button8.checked ? "black" : "white"
                    }
                }
                checkable: true
                autoExclusive: true
                enabled: menu_enable
                visible: false
            }
        }
        Button {
            id: button7
            x: 1812
            y: 8
            width: 122
            height: 42
            text: qsTr("")
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 20
            anchors.bottomMargin: 5
            enabled: menu_enable
            background: Rectangle {
                anchors.fill: parent
                color: button7.pressed ? "#ededed" : "transparent"
                radius: 3
                border.width: 1
                border.color: "#ededed"
                Text {
                    text: qsTr("退出登录")
                    font.pointSize: 13
                    anchors.centerIn: parent
                    color: "white"
                }
            }
            onClicked: {
                if(login_role != "super")
                {
                    var settings = settingPage.getSettings()
                    user_mod.modifyData(["music", "volume", "screen_lock", "userid"],
                                   [settings["music"].toString(), settings["volume"].toString(), settings["screen_lock"].toString(), user_id])
                }
                guiIntf.qmllog("用户登出： " + user_id)
                curePage.clearJL()
                formulaPage.clearPage()
                infoPage.clearPage()
                loginPage.visible = true
                user_id = ""
                user_name = ""
                login_role = ""
                password = ""
            }
        }

        Text {
            id: text8
            anchors.top: button7.top
            anchors.right: button7.left
            anchors.rightMargin: 150
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "#ffffff"
            text: qsTr("当前用户： " + (login_role == "admin" ? "管理员" :
                                      login_role == "root" ? "运行维护":
                                      login_role == "doctor" ? "医生":
                                      login_role == "operator" ? "操作员" : "超级用户") + "  " + user_id)
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: 20
        }
    }

    AppointmentView {
        id: appPage
        visible: button22.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    FormulaPage {
        id: formulaPage
        visible: button.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    CurePage {
        id: curePage
        visible: button1.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    InfoPage {
        id: infoPage
        visible: button2.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Calibrate {
        id: caliPage
        visible: button5.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    About {
        id: abtPage
        visible: button33.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    DevOps {
        id: devPage
        visible: button6.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    DevMaintain {
        id: devManPage
        visible: button66.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Diagnosis {
        id: diagPage
        visible: button8.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Settings {
        id: settingPage
        visible: button3.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    UserManage {
        id: usermanage
        visible: button4.checked
        anchors.top: funcmenu.bottom
        anchors.bottom: sta_bar.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Rectangle {
        id: sta_bar
        height: 30
        color: "#2b579a"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Rectangle {
            id: sta_l
            y: 11
            width: 15
            height: 15
            color: "red"
            radius: 15
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: text1.right
            anchors.leftMargin: 10
        }

        Text {
            id: text1
            y: 8
            text: qsTr("L")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            font.pixelSize: 12
            anchors.leftMargin: 55
            color: "white"
        }

        Rectangle {
            id: sta_r
            y: 11
            width: 15
            height: 15
            color: "red"
            radius: 15
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: text2.right
            anchors.leftMargin: 10
        }

        Text {
            id: text2
            y: 8
            text: qsTr("R")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: sta_l.right
            font.pixelSize: 12
            anchors.leftMargin: 30
            color: "white"
        }

        Rectangle {
            id: sta_cam0
            y: 11
            width: 15
            height: 15
            color: "red"
            radius: 15
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: text3.right
            anchors.leftMargin: 10
        }

        Text {
            id: text3
            y: 8
            text: "cam0"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: sta_r.right
            font.pixelSize: 12
            anchors.leftMargin: 30
            color: "white"
        }

        Rectangle {
            id: sta_cam1
            y: 11
            width: 15
            height: 15
            color: "red"
            radius: 15
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: text4.right
            anchors.leftMargin: 10
        }

        Text {
            id: text4
            y: 8
            text: qsTr("cam1")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: sta_cam0.right
            font.pixelSize: 12
            anchors.leftMargin: 30
            color: "white"
        }

        Rectangle {
            id: sta_cam2
            y: 11
            width: 15
            height: 15
            color: "red"
            radius: 15
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: text5.right
            anchors.leftMargin: 10
        }

        Text {
            id: text5
            y: 8
            text: qsTr("cam2")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: sta_cam1.right
            font.pixelSize: 12
            anchors.leftMargin: 30
            color: "white"
        }

        Rectangle {
            id: sta_cam3
            y: 11
            width: 15
            height: 15
            color: "red"
            radius: 15
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: text6.right
            anchors.leftMargin: 10
        }

        Text {
            id: text6
            y: 8
            text: qsTr("cam3")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: sta_cam2.right
            font.pixelSize: 12
            anchors.leftMargin: 30
            color: "white"
        }

        Rectangle {
            id: sta_bed
            y: 11
            width: 15
            height: 15
            color: "red"
            radius: 15
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: text7.right
            anchors.leftMargin: 10
        }

        Text {
            id: text7
            y: 8
            text: qsTr("bed")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: sta_cam3.right
            font.pixelSize: 12
            anchors.leftMargin: 30
            color: "white"
        }

        Connections {
            target: guiIntf
            onSwitchDevSta:{
                if(index === 0)
                    sta_l.color = (sta_l.color.r === 1) ? "green" : "red"
                if(index === 1)
                    sta_r.color = (sta_r.color.r === 1) ? "green" : "red"
                if(index === 2)
                    sta_cam0.color = (sta_cam0.color.r === 1) ? "green" : "red"
                if(index === 3)
                    sta_cam1.color = (sta_cam1.color.r === 1) ? "green" : "red"
                if(index === 4)
                    sta_cam2.color = (sta_cam2.color.r === 1) ? "green" : "red"
                if(index === 5)
                    sta_cam3.color = (sta_cam3.color.r === 1) ? "green" : "red"
                if(index === 6)
                    sta_bed.color = (sta_bed.color.r === 1) ? "green" : "red"
            }
        }
    }
}


