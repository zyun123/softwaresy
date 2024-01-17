import QtQuick 2.12
import QtQml 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import TaoQuick 1.0
import QtMultimedia 5.12
import "./qml_basic"
//import QtQuick.VirtualKeyboard 2.4
//import QtQuick.VirtualKeyboard.Settings 2.2

Window{
    id: entrywnd
    width: 1920
    height: 1080
    visibility: "FullScreen"
    visible: true

    property int chkCali: 0
    property int contact_mod: 0

    onClosing:{close.accepted = false}

//    InputPanel {
//        id: inputPanel
//        z: 99
//        x: 560
//        y: entrywnd.height
//        width: 600
//        height: 300

//        states: State {
//            name: "visible"
//            when: inputPanel.active
//            PropertyChanges {
//                target: inputPanel
//                y: entrywnd.height - inputPanel.height
//            }
//        }
//        transitions: Transition {
//            from: ""
//            to: "visible"
//            reversible: true
//            ParallelAnimation {
//                NumberAnimation {
//                    properties: "y"
//                    duration: 250
//                    easing.type: Easing.InOutQuad
//                }
//            }
//        }
//        Component.onCompleted: {
//            VirtualKeyboardSettings.activeLocales = ["en_US","zh_CN"]
//            VirtualKeyboardSettings.locale = "en_US"
//        }
//    }

    property bool screenLocked: false
    property int lockInterval: 20 * 60 * 1000
    property bool enableScreenLock: true

    function releaseScreenLock()
    {
        if(screenLocked)
        {
            screenLockImg.visible=false
            screenLocked = false;
            var args = {"func": "pauseCamStream"}
            guiIntf.guiCall(args)
        }
        screenLockTimer.restart()
    }

    //解决qt bug: qml文本控件无法按中文输入法输入
    property bool fixinput: false
    onActiveFocusItemChanged:
    {
        if(activeFocusItem != null && fixinput == false)
        {
            activeFocusItem.forceActiveFocus()
            fixinput = true
        }
    }

    //锁屏
    MouseArea{
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: entrywnd.releaseScreenLock()
        onPositionChanged: entrywnd.releaseScreenLock()
        Component.onCompleted: screenLockTimer.start()
    }
    Timer{
        id: screenLockTimer
        interval: lockInterval > 0 ? lockInterval : 60 * 60 * 1000
        running: true
        onTriggered:{
            if(!screenLocked && lockInterval > 0 && enableScreenLock == true)
            {
                screenLockImg.visible=true
                screenLocked = true;
                var args = {"func": "pauseCamStream"}
                guiIntf.guiCall(args)
            }
        }
    }
    Rectangle {
        id: screenLockImg
        visible: false
        anchors.fill: parent
        color: "green"
        z:999
        Image {
            id: name
            anchors.fill: parent
            source: "../Resource/new_ui/login_bg.jpg"
        }
        MouseArea{
            id: msarea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: entrywnd.releaseScreenLock()
            onPositionChanged: entrywnd.releaseScreenLock()
        }
    }

    //主要页面
    Loading {
        id: loadingPage
        anchors.fill: parent
        z: 2
        Connections
        {
            target: guiIntf
            onSigUpdateProgress: loadingPage.progress = curpro
        }
    }
    Login {
        id: loginPage
        anchors.fill: parent
        z: 1
    }
    Home {
        id: homePage
        anchors.fill: parent
    }

    //悬浮窗口
    Blocking {
        id: blockPage
        anchors.fill: parent
        visible: false
        z: 98
        Connections
        {
            target: guiIntf
            onBlockUI: blockPage.showBlockingMsg(s, t)
        }
    }
    Warning {
        id: warnmsg
        anchors.fill: parent
        visible: false
        z: 99
    }
    MyMessageBox{
        id: msg_rb_left
        anchors.fill: parent
        visible: false
        z: 99
        reject_enable: false
        onAccepted: {
            var args = {"func": "releaseRbLeftBlock"}
            guiIntf.guiCall(args)
        }
        Connections
        {
            target: guiIntf
            onConfirmMsgL: msg_rb_left.showWarn("提示", s1, s2)
        }
    }
    MyMessageBox{
        id: msg_rb_right
        anchors.fill: parent
        visible: false
        z: 99
        reject_enable: false
        onAccepted: {
            var args = {"func": "releaseRbRightBlock"}
            guiIntf.guiCall(args)
        }
        Connections
        {
            target: guiIntf
            onConfirmMsgR: msg_rb_right.showWarn("提示", s1, s2)
        }
    }
    MyMessageBox{
        id: msg_core_th
        anchors.fill: parent
        visible: false
        z: 99
        reject_enable: false
        onAccepted: {
            var args = {"func": "releaseCoreBlock"}
            guiIntf.guiCall(args)
        }
        Connections
        {
            target: guiIntf
            onConfirmMsgCore: msg_core_th.showWarn("提示", s1, s2)
        }
    }

    Tips {
        id: tips
        z: 100
    }

    //音乐 语音
    MediaPlayer {
        id: bgmplayer
        source: "../Resource/audios/bgm_gaoshanliushui.mp3"
        loops: MediaPlayer.Infinite
        Component.onCompleted: play()
    }
    MediaPlayer {
        id: tipplayer
        source: ""
        function playtip(val){
            source = val
            play()
        }
    }

    //快捷键
    Shortcut{
        sequence: "F1"
        property bool idmemark: false
        onActivated: {
            var args = {"func": "switchFakeIdentify"}
            idmemark = idmemark ? false : true;
            guiIntf.guiCall(args)
            warnmsg.showWarn("提示", "", idmemark ? "已切换为图片识别" : "已切换为正常识别")
        }
    }
    Rectangle{
        x:0
        y:0
        width: 1920
        height: 150
        color: "black"
        opacity: 0.8
        z: 99
        MouseArea{
            anchors.fill: parent
        }
        visible: chkCali != 0
    }
    Rectangle{
        x:1920 - 300
        y:150
        width: 300
        height: 1080 - 150
        color: "black"
        opacity: 0.8
        z: 99
        MouseArea{
            anchors.fill: parent
        }
        visible: chkCali != 0
    }
    Shortcut{
        id: switchChkCali0
        sequence: "F3"
        onActivated: {
            if(chkCali === 0 && blockPage.visible === false && (homePage.login_role == "root" || homePage.login_role == "super"))
            {
                homePage.toPage(1)
                chkCali = 1
                var args = {"func": "iniRedPoint"}
                guiIntf.guiCall(args)
                warnmsg.showWarn("提示", "", "已进入左机械臂标定校验")
            }
        }
    }
    Shortcut{
        id: switchChkCali1
        sequence: "F4"
        onActivated: {
            if(chkCali === 0 && blockPage.visible === false && (homePage.login_role == "root" || homePage.login_role == "super"))
            {
                homePage.toPage(1)
                chkCali = 2
                var args = {"func": "iniRedPoint"}
                guiIntf.guiCall(args)
                warnmsg.showWarn("提示", "", "已进入右机械臂标定校验")
            }
        }
    }
    Shortcut{
        id: goChkCali
        sequence: "F5"
        onActivated: {
            if(chkCali !== 0 && blockPage.visible === false && (homePage.login_role == "root" || homePage.login_role == "super"))
            {
                chkCali = 0
                var args = {"func": "goToRedPoint", "arg_obj": {"contact_mod": contact_mod}}
                guiIntf.guiCall(args)
            }
        }
    }
    Shortcut{
        sequence: "F6"
        onActivated: {
            var args = {"func": "camshot",
                        "arg_str": "up_nei"}
            guiIntf.guiCall(args)
        }
    }
    Shortcut{
        sequence: "F7"
        onActivated: {
            var args = {"func": "camshot",
                        "arg_str": "down_wai"}
            guiIntf.guiCall(args)
        }
    }
    Shortcut{
        sequence: "F8"
        onActivated: {
            var args = {"func": "camshotInDiffPos",
                        "arg_str": "up_nei"}
            guiIntf.guiCall(args)
        }
    }
    Shortcut{
        sequence: "F9"
        onActivated: {
            var args = {"func": "camshotInDiffPos",
                        "arg_str": "down_wai"}
            guiIntf.guiCall(args)
        }
    }
//    Shortcut{
//        sequence: "F8"
//        property bool mark3d: false
//        onActivated: {
//            var args = {"func": "switchUsing3D"}
//            mark3d = mark3d ? false : true;
//            guiIntf.guiCall(args)
//            warnmsg.showWarn("提示", "", mark3d ? "已切换为3d识别" : "已切换为2d识别")
//        }
//    }
    Shortcut{
        sequence: "F10"
        onActivated: {
            var args = {"func": "comPlot"}
            guiIntf.guiCall(args)
        }
    }
    Shortcut{
        sequence: "Ctrl+Q"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                Qt.quit()
            }
        }
    }
    Shortcut{
        id: switchemc
        sequence: "Ctrl+E"
        onActivated: {
            var r = guiIntf.switchEMC()
            if(r === true)
                warnmsg.showWarn("提示", "", "已开启EMC检测模式")
            else
                warnmsg.showWarn("提示", "", "已关闭EMC检测模式")
        }
    }
    Shortcut{
        id: switchCaliTestMod
        sequence: "Ctrl+T"
        onActivated: {
            var r = guiIntf.switchTestCaliMod()
            if(r === 0)
                warnmsg.showWarn("提示", "", "标定验证已改为模式0")
            else
                warnmsg.showWarn("提示", "", "标定验证已改为模式1")
        }
    }
    Shortcut{
        id: switchContact
        sequence: "Ctrl+Down"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 1
                warnmsg.showWarn("提示", "", "标定接触已改为竖直向下")
            }
        }
    }
    Shortcut{
        id: switchContact4
        sequence: "Ctrl+RIGHT"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 2
                warnmsg.showWarn("提示", "", "标定接触已改为从左至右")
            }
        }
    }
    Shortcut{
        id: switchContact1
        sequence: "ALT+RIGHT"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 3
                warnmsg.showWarn("提示", "", "标定接触已改为从左上至右下")
            }
        }
    }
    Shortcut{
        id: switchContact2
        sequence: "ALT+LEFT"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 4
                warnmsg.showWarn("提示", "", "标定接触已改为从右上至左下")
            }
        }
    }
    Shortcut{
        id: switchContact3
        sequence: "Ctrl+LEFT"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 5
                warnmsg.showWarn("提示", "", "标定接触已改为从右至左")
            }
        }
    }
    Shortcut{
        id: switchContact6
        sequence: "Ctrl+UP"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 6
                warnmsg.showWarn("提示", "", "标定接触已改为从床头至床尾")
            }
        }
    }
    Shortcut{
        id: switchContact7
        sequence: "Ctrl+1"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 0
                warnmsg.showWarn("提示", "", "标定接触已改为刚性移动")
            }
        }
    }
    Shortcut{
        id: switchContact8
        sequence: "Ctrl+2"
        onActivated: {
            if(homePage.login_role == "root" || homePage.login_role == "super")
            {
                contact_mod = 7
                warnmsg.showWarn("提示", "", "标定接触已改为连续刚性移动")
            }
        }
    }
    Shortcut{
        id: quicklogin
        sequence: "Alt+1"
        onActivated: guiIntf.login("super", "sy666")
    }

    Connections{
        target: guiIntf
        onWarnMsgToUI:
        {
            warnmsg.showWarn("警告", title, msg)
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}
}
##^##*/
