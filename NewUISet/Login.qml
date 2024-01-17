import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4

import TaoQuick 1.0
import QtGraphicalEffects 1.12

Rectangle {
    id:loginid
    width: 1920
    height: 1080
    anchors.fill: parent

    onVisibleChanged: pwdInput.text = ""

    MouseArea{
        anchors.fill: parent
    }

    Connections{
        target: guiIntf
        onIniLoginUsr:{
            nameInput.text = id
//            pwdInput.text = pwd
            if(id != "" || pwd != "")
                checkBox.checked = true
        }
    }

    Image {
        id: backimage
        width: loginid.width
        height:loginid.height
        source: "../Resource/new_ui/login_bg.jpg"
    }


    Rectangle{
        id: rect2
        x: 628
        y: 532
        width: 594
        height: 178
        color: "#aceaf3"

        radius: 20
        Rectangle{
            id:loginname
            width: 62+240
            x:38
            y:38
            height: 42
            color: "transparent"
            Text{
                id:namelbl
                width: 62
                height: loginname.height-20
                y:10
                text: qsTr("账  号:")
                color: "#333333"
                font.pixelSize: 18

            }

            Rectangle{
                id:inputrect1
                width: 240
                height: loginname.height
                color: "white"
                anchors.left: namelbl.right
                anchors.leftMargin: 0
                TextField{
                    id:nameInput
                    width: inputrect1.width
                    height: inputrect1.height
                    font.pointSize: 15
                    focus: true
                    placeholderText:qsTr("请输入账号")
                    echoMode: TextInput.Normal
                    selectByMouse: true
                    onAccepted: {
                        if(loginid.visible == true && loadingPage.visible == false)
                            dengluBtn.clicked()
                    }
                    enabled: loginid.visible==true
                }
            }


        }
        Rectangle{
            id:loginpwd
            width: 62+240
            x:38
            anchors.top: loginname.bottom
            anchors.topMargin: 15
            height: 42
            color: "transparent"
            Text{
                id:pwdlbl
                width: 62
                height: loginpwd.height-20
                y:10
                text: qsTr("密  码:")
                color: "#333333"
                font.pixelSize: 18
            }

            Rectangle{
                id:inputpwdrect1
                width: 240
                height: loginpwd.height
                color: "white"
                anchors.left: pwdlbl.right
                anchors.leftMargin: 0

                TextField{
                    id:pwdInput
                    width: inputrect1.width
                    height: inputrect1.height
                    font.pointSize: 15
                    focus: true
                    placeholderText:qsTr("请输入密码")
                    echoMode: TextInput.Password
                    selectByMouse: true
                    onAccepted: {
                        if(loginid.visible == true && loadingPage.visible == false)
                            dengluBtn.clicked()
                    }
                    enabled: loginid.visible==true
                }
            }


        }
        Rectangle{
            id: rectButton
            width: 165
            height: rect2.height
            x:rect2.width-rectButton.width
            color: "#ededed"
            //                     color: "#E9E9E9"
            radius: 20
            Button{
                id:dengluBtn
                width: rectButton.width
                height: rectButton.height
                font.pointSize:20
                background: Rectangle{
                    width: dengluBtn.width
                    height: dengluBtn.height
                    //                             color: "#E9E9E9" // same color as left area
                    color: "white"
                    radius: 20
                    Image {
                        id: backImage
                        width: rectButton.width
                        height: rectButton.height
                        source: dengluBtn.hovered ? "../Resource/images/denglu2.png" : "../Resource/images/denglu.png"
                        anchors.centerIn: parent
                        property bool rounded: true
                        property bool adapt: true
                        layer.enabled: rounded

                        layer.effect: OpacityMask{
                            maskSource: Item{
                                width: backImage.width
                                height: backImage.height
                                Rectangle{
                                    anchors.centerIn: parent
                                    width: backImage.adapt ? backImage.width:Math.min(backImage.width,backImage.height)
                                    height: backImage.adapt ? backImage.height : width
                                    radius: Math.max(width,height)
                                }
                            }

                        }
                    }
                }
                onClicked:{
                    if(homePage.version === 1)//登录服务端
                    {
                        if(nameInput.text == "offline" || nameInput.text == "admin")
                        {
                            loginid.visible = false
                            if(checkBox.checked === true)
                            {
                                guiIntf.setSysCfg("DEVICE/CACHE-USER", nameInput.text)
                                guiIntf.setSysCfg("DEVICE/CACHE-PWD", pwdInput.text)
                            }
                            else
                            {
                                guiIntf.setSysCfg("DEVICE/CACHE-USER", "")
                                guiIntf.setSysCfg("DEVICE/CACHE-PWD", "")
                                nameInput.text = ""
                                pwdInput.text = ""
                            }
                            homePage.user_id = ""
                            homePage.login_role = nameInput.text
//                            homePage.loadMeridians("手厥阴心包经 足阳明胃经 足太阴脾经1 手少阳三焦经 足太阳膀胱经1 足太阳膀胱经2", "", "", "")
                            if(homePage.login_role == "operator")
                                homePage.toPage(2)
                            else if(homePage.login_role == "doctor")
                                homePage.toPage(0)
                            else if(homePage.login_role == "root")
                                homePage.toPage(66)
                            else if(homePage.login_role == "admin")
                                homePage.toPage(4)
                            else
                                homePage.toPage(1)
                        }
                        else
                        {
                            var args = {"func": "login", "arg_str": nameInput.text + "|" + pwdInput.text}
                            guiIntf.guiCall(args)
                        }
                    }
                    else//本地登录
                    {
                        guiIntf.login(nameInput.text, pwdInput.text)
                        homePage.password = pwdInput.text
                    }
                }
            }
            Connections{
                target: guiIntf
                onLocalLoginOK:{
                    loginid.visible = false
                    if(checkBox.checked === true)
                    {
                        guiIntf.setSysCfg("DEVICE/CACHE-USER", nameInput.text)
                        guiIntf.setSysCfg("DEVICE/CACHE-PWD", pwdInput.text)
                    }
                    else
                    {
                        guiIntf.setSysCfg("DEVICE/CACHE-USER", "")
                        guiIntf.setSysCfg("DEVICE/CACHE-PWD", "")
                        nameInput.text = ""
                        pwdInput.text = ""
                    }
                    homePage.login_role = role
                    homePage.user_name = name
                    homePage.user_id = id
                    homePage.loadSettings(settings)
                    if(homePage.login_role == "operator")
                        homePage.toPage(2)
                    else if(homePage.login_role == "doctor")
                        homePage.toPage(0)
                    else if(homePage.login_role == "root")
                        homePage.toPage(66)
                    else if(homePage.login_role == "admin")
                        homePage.toPage(4)
                    else
                        homePage.toPage(1)
                }
            }
            Connections{
                target: guiIntf
                onServerLoginOK:{
                    if(result === true)
                    {
                        loginid.visible = false
                        if(checkBox.checked === true)
                        {
                            guiIntf.setSysCfg("DEVICE/CACHE-USER", nameInput.text)
                            guiIntf.setSysCfg("DEVICE/CACHE-PWD", pwdInput.text)
                        }
                        else
                        {
                            guiIntf.setSysCfg("DEVICE/CACHE-USER", "")
                            guiIntf.setSysCfg("DEVICE/CACHE-PWD", "")
                            nameInput.text = ""
                            pwdInput.text = ""
                        }
                        homePage.user_id = nameInput.text
                        homePage.login_role = "sv_" + role
                    }
                    else
                        tips.showTip({"message": "账号或密码错误", "type": "error", "show": true})
                }
            }
        }

        CheckBox {
            id: checkBox
            x: 324
            y: 150
            width: 105
            height: 23
            text: qsTr("记住我")
            scale: 0.7
            padding: 3
            leftPadding: 3
            topPadding: 3
            font.pointSize: 11
        }

    }

    Timer {
        id: quitTimer
        interval: 2000
        onTriggered: Qt.quit()
    }

    CusButton {
        id: quitBtn
        x: 1713
        width: 40
        height: 40
        text: "×"
        textColor: "white"
        fontsize: 25
        radius: 20
        anchors.right: parent.right
        anchors.top: parent.top
        backgroundColorNormal: "red"
        display: AbstractButton.TextOnly
        anchors.rightMargin: 50
        anchors.topMargin: 50
        visible: false
        onClicked: {
            var args = {"func": "flushLog"}
            guiIntf.guiCall(args)
            blockPage.showBlockingMsg("正在退出,请稍候...", 0)
            quitTimer.start()
        }
    }

    Text {
        id: name2
        x: 792
        y: 796
        visible: false
        color: "#000000"
        text: qsTr("发布版本：V1")
        font.letterSpacing: 5
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenterOffset: -74
        anchors.horizontalCenter: parent.horizontalCenter
        font.strikeout: false
        font.italic: false
        font.pointSize: 20
        font.bold: false
    }

    Text {
        id: name1
        x: 755
        y: 744
        visible: false
        color: "#000000"
        text: qsTr("完整版本：V1.0.0.0")
        font.letterSpacing: 5
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenterOffset: -26
        anchors.horizontalCenter: parent.horizontalCenter
        font.strikeout: false
        font.italic: false
        font.pointSize: 20
        font.bold: false
    }
}



/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}
}
##^##*/
