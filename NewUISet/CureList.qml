import QtQuick 2.12
import QtQuick.Controls 2.0
import QtQuick.Controls 2.5
import QtQuick.Controls 1.4
import TaoQuick 1.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtMultimedia 5.12
import QtQuick.Layouts 1.0
import QtQml 2.12
import CureRecordModel 1.0

Item {
    id:rightcontrol
    anchors.fill: parent
    width: 300
    height: 870

    property string app_serial: ""
    property string phone: ""

    property string patient_id: ""
    property string patient_name: ""
    property string start_cure_time: ""
    property string end_cure_time: ""

    property variant tianliform: { "足少阴肾经2":"shen2","手厥阴心包经":"xinbao","手少阳三焦经":"sanjiao","足少阳胆经1":"dan1","足少阳胆经2":"dan2",
        "足厥阴肝经1":"gan1", "足厥阴肝经2":"gan2", "手太阴肺经":"fei", "手阳明大肠经":"dachang","足阳明胃经1":"wei1","足阳明胃经2":"wei2","足阳明胃经3":"wei3", "足太阴脾经1":"pi1","足太阴脾经2":"pi2",
        "手少阴心经":"xin",  "手太阳小肠经":"xiaochang", "足太阳膀胱经1":"pangguang1","足太阳膀胱经2":"pangguang2", "足少阴肾经1":"shen1", "任脉":"ren", "督脉":"du" }
    property bool isSwitching: false
    property var startCureT: 0
    property int ctimes: 1
    property var mod_shengxi: [{"jingluo": "足少阴肾经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手厥阴心包经", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手少阳三焦经", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足少阳胆经1", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足少阳胆经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足厥阴肝经1", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足厥阴肝经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手太阴肺经", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手阳明大肠经", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足阳明胃经1", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足阳明胃经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足阳明胃经3", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足太阴脾经1", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足太阴脾经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手少阴心经", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手太阳小肠经", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足太阳膀胱经1", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足太阳膀胱经2", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足少阴肾经1", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "督脉", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "任脉", "tiwei": "仰卧手向上", "checked": false}]
    property var mod_zishi: [{"jingluo": "足少阴肾经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手厥阴心包经", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足少阳胆经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足厥阴肝经1", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足厥阴肝经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手太阴肺经", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足阳明胃经1", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足阳明胃经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足阳明胃经3", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足太阴脾经1", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "足太阴脾经2", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手少阴心经", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "任脉", "tiwei": "仰卧手向上", "checked": false},
                        {"jingluo": "手少阳三焦经", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足少阳胆经1", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "手阳明大肠经", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "手太阳小肠经", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足太阳膀胱经1", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足太阳膀胱经2", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "足少阴肾经1", "tiwei": "俯卧手向下", "checked": false},
                        {"jingluo": "督脉", "tiwei": "俯卧手向下", "checked": false}]
    property var dict: []

    Component.onCompleted: {
        dict = mod_zishi
        resetJingLuoStep()
        cureStart.connect(onCureStart)
        cureStart_zhenduan.connect(onCureStart_zhenduan)
    }

    CureRecordModel{
        id: cureRecord_mod
    }

    Shortcut{
        id: verswitch
        sequence: "F2"
        onActivated: {
            if(starttiaoli === false && regButton.enabled == true)
            {
                isrecognizesuccess = false
//                homePage.resetJL()
                clearJingLuoStep()
                homePage.cfg_ver = guiIntf.switchAppVer()
            }
        }
    }

    function onCureStart(plan){//本地登录版本
        var limit_ver = ["足少阳胆经1", "足厥阴肝经2", "足阳明胃经1", "足阳明胃经3"]
        for(let i = 0; i < dict.length; i++)
            dict[i]["checked"] = false
        var mer_str = plan.meridians
        var reg = /[\t\r\f\n\s]*/g;
        var trimStr = mer_str.replace(reg,'');
        var meridians = trimStr.split(",")
        for(let i = 0; i < dict.length; i++){
            for(let j = 0; j < meridians.length; j++)
            {
                if(dict[i]["jingluo"].indexOf(meridians[j]) !== -1 && (homePage.cfg_ver == 0 || limit_ver.indexOf(dict[i]["jingluo"]) === -1))
                    dict[i]["checked"] = true
            }
        }
        resetJingLuoStep()
        cureModBox.currentIndex = plan.mode
        patient_id = plan.idcard
        patient_name = plan.name
        if(plan.name !== "")
            text6.text = plan.name
        else
            text6.text = ""
        if(plan.formula_mod !== "")
            text9.text = plan.formula_mod
        else
            text9.text = ""
        cureModBox1.currentIndex = plan.mode_wtf
        cureTimes.value = parseInt(plan.cure_times)
        text7.text = ""
    }

    function onCureStart_zhenduan(plan){//诊断导入
        var limit_ver = ["足少阳胆经1", "足厥阴肝经2", "足阳明胃经1", "足阳明胃经3"]
        for(let i = 0; i < dict.length; i++)
            dict[i]["checked"] = false
        for(let i = 0; i < dict.length; i++){
            for(let j = 0; j < plan["plan"].length; j++)
            {
                if(dict[i]["jingluo"].indexOf(plan["plan"][j]) !== -1 && (homePage.cfg_ver == 0 || limit_ver.indexOf(dict[i]["jingluo"]) === -1))
                    dict[i]["checked"] = true
            }
        }
        resetJingLuoStep()
        cureModBox.currentIndex = 1
        patient_id = ""
        text6.text = plan.name
        text9.text = plan.planname
        cureModBox1.currentIndex = 0
        cureTimes.value = parseInt(plan.num)
        text7.text = ""
    }

    function loadMeridians(raw, id, serial, name){//登录服务端版本
        for(let i = 0; i < dict.length; i++)
            dict[i]["checked"] = false
        var list = raw.split(' ')
        for(let i = 0; i < dict.length; i++)
        {
            if(list.indexOf(dict[i]["jingluo"]) !== -1)
                dict[i]["checked"] = true
        }
        resetJingLuoStep()
        patient_id = id
        app_serial = serial
        patient_name = name
        if(name !== "")
            set_patient_name("当前患者：  " + name)
        else
            set_patient_name("")
    }

    function set_patient_name(str)
    {
        patienttreatname.text = str
    }

    function set_dict_checkstate(str, state)
    {
        for(let i = 0; i < dict.length; i++)
        {
            if(dict[i]["jingluo"] === str)
                dict[i]["checked"] = state
        }
    }

    function genRecordTime(start)
    {
        uploadRecordTime.stop()
        var recorddat = []
        const currentTime= new Date();
        var currentYear = currentTime.getFullYear();
        var currentMonth = ('0'+(currentTime.getMonth() + 1)).substr(-2);
        var currentDay = ('0'+currentTime.getDate()).substr(-2);
        var currentHours = ('0'+currentTime.getHours()).substr(-2);
        var currentMinutes = ('0'+currentTime.getMinutes()).substr(-2);
        var currentSeconds = ('0'+currentTime.getSeconds()).substr(-2);
        var currentMillionSeconds = ('00'+currentTime.getMilliseconds()).substr(-3);
        var time_str = currentYear + "." + currentMonth + "." + currentDay + " " +
                currentHours + ":" + currentMinutes + ":" + currentSeconds
        var timestamp = currentYear + currentMonth + currentDay +
                currentHours  + currentMinutes + currentSeconds + currentMillionSeconds

        if(start === true)
        {
            if(start_cure_time == "")
            {
                start_cure_time = time_str
                startCureT = (new Date()).valueOf()
                updateUsingTime.start()
            }
        }
        else
        {
            if(start_cure_time != "")
            {
                updateUsingTime.stop()
                if(patient_id !== "")
                {
                    var meridians = ""
                    for(var i=0;i<dict.length;i++)
                    {
                        if(dict[i]["checked"] === true)
                        {
                            meridians += dict[i]["jingluo"]
                            meridians += ","
                        }
                    }
                    recorddat.push(timestamp)
                    recorddat.push(patient_id)
                    recorddat.push(patient_name)
                    recorddat.push(homePage.user_id)
                    recorddat.push(meridians.substring(0, meridians.length - 1))
                    recorddat.push(start_cure_time)
                    recorddat.push(time_str)
                    recorddat.push(patient_id)
                    recorddat.push(homePage.device_id)
                    recorddat.push(text7.text)
                    if(cureRecord_mod.insertData(recorddat) === false)
                        warnmsg.showWarn("错误", "", "保存调理记录失败")
                }
                start_cure_time = ""
            }
        }
    }

    onVisibleChanged: {
        if(visible == false && start_cure_time != "")
            genRecordTime(false)
    }

    function clearJingLuoStep() {
        patient_id = ""
        patient_name = ""
        text6.text = ""
        text7.text = ""
        text9.text = ""
        cureModBox.currentIndex = 1
        cureModBox1.currentIndex = 0
        cureTimes.value = 1
        jingluoplanStep.clear();
    }

    function resetJingLuoStep(){
        jingluoplanStep.clear();
        var tiwei="";
        for (var i=0;i<dict.length;i++){
            var datatmp=dict[i]
            if(datatmp["checked"] === false)
                continue
            var jingluoname=datatmp["jingluo"]
            var tiweiname=datatmp["tiwei"]
            var soundname="";
            var posturebox=""
            if (tiweiname==="仰卧手向下"){
                soundname="pose1.mp3"
                posturebox="1号姿势"
            }
            if (tiweiname==="仰卧手向上"){
                soundname="pose2.mp3"
                posturebox="2号姿势"
            }
            if (tiweiname==="俯卧手向下"){
                soundname="pose3.mp3"
                posturebox="3号姿势"
            }
            if (tiweiname==="俯卧脚内八"){
                soundname="pose4.mp3"
                posturebox="4号姿势"
            }
            if (tiweiname!==tiwei){
                tiwei=tiweiname
                jingluoplanStep.append({"name":jingluoname,"colorCode":"green","soundname":soundname,"zishiname":tiweiname,"posturebox":posturebox})
            }else{
                jingluoplanStep.append({"name":jingluoname,"colorCode":"green","soundname":soundname,"zishiname":"","posturebox":posturebox})
            }
        }
        listView.currentIndex=0;
        recognizeActionPlay() //放音体位动作
        isrecognizesuccess=false
        starttiaoli=false
    }
    function recognizeActionStart(){
        tipplayer.playtip("../Resource/audios/start_iden.mp3")
        var currmodel=jingluoplanStep.get(listView.currentIndex)
        var posture=currmodel.posturebox
        var soundname=currmodel.soundname
        var jlnames = ""
        var tiwei = posture === "1号姿势" ? "仰卧手向下" :
                    posture === "2号姿势" ? "仰卧手向上" :
                    posture === "3号姿势" ? "俯卧手向下" : "俯卧脚内八"
        for(var i = 0; i < dict.length; i++){
            var datatmp = dict[i]
            if(datatmp["checked"] === true && datatmp["tiwei"] === tiwei)
                jlnames += tianliform[datatmp["jingluo"]] + ","
        }
        var args = {"func": "recognizeJingLuo",
            "arg_str": posture + "|" + jlnames}
        guiIntf.guiCall(args)
        if (!realtimeCorrect.visible){//识别全部切换到实时画面
            subPagIndex = 0
        }
    }
    function recognizeActionPlay(){//开始识别 跳到实时画面
        if (listView.currentIndex<jingluoplanStep.rowCount()){
            var currmodel=jingluoplanStep.get(listView.currentIndex)
            var posture=currmodel.posturebox
            var soundname=currmodel.soundname
            if (soundname!==""){ //有体位信息 发送socket 信息 先放音-》发送socket
                tipplayer.playtip("../Resource/audios/"+soundname)
                isrecognizesuccess=false //变换姿势，重新开始
                starttiaoli=false
            }
        }
    }
    function tiaoliActionStart(){
        //调理开始禁用顶部页面切换和退出登录 暂停调理或调理结束后恢复
        menu_enable = false
        regButton.enabled = false
        enableScreenLock = false
        verswitch.enabled = false
        tipplayer.playtip("../Resource/audios/start_care.mp3")
        ctimes = cureTimes.value
        var currmodel=jingluoplanStep.get(listView.currentIndex)
        var posture=currmodel.posturebox
        var meridianbox=tianliform[ currmodel.name]
        genRecordTime(true)
        var args = {"func": "startTreatment",
            "arg_str": meridianbox + "|false"}
        guiIntf.guiCall(args)
    }
    function tiaoliActionNext(){
        if (listView.currentIndex<jingluoplanStep.rowCount()){//一条经络调理完成，进入下一个经络
            var currmodel=jingluoplanStep.get(listView.currentIndex)
            var posture=currmodel.posturebox
            var zishi=currmodel.zishiname
            var meridianbox=tianliform[ currmodel.name]
            if (zishi==="" || ctimes > 0){
                var args = {"func": "startTreatment",
                    "arg_str": meridianbox + "|true"}
                guiIntf.guiCall(args)
            }else{
                isSwitching = true
                regButton.enabled = false
                enableScreenLock = false
                verswitch.enabled = false
                regButton1.enabled = false
                args = {"func": "switchPose"}
                guiIntf.guiCall(args)
                uploadRecordTime.start()
            }
        }
        else{//调理全部完成
            isSwitching = false
            args = {"func": "switchPose"}
            guiIntf.guiCall(args)
            listView.currentIndex = 0;
            //上传调理记录
            if(homePage.version === 1)
            {
                end_cure_time = Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm:ss")
                if(homepage.login_role != "offline")
                {
                    var arg = {}
                    arg["table"] = "cure_record"
                    arg["type"] = "insert"
                    arg["start_time"] = start_cure_time
                    arg["end_time"] = end_cure_time
                    arg["patient_id"] = patient_id
                    arg["app_serial"] = app_serial
                    arg["operator"] = homepage.user_id
                    arg["patient_name"] = patient_name
                    arg["phone"] = phone
                    guiIntf.callonUploadCureRecord(arg)
                }
            }
            else
                genRecordTime(false)
        }
    }
    function afterPauseTreatment()//调理中止后恢复按钮状态并播放语音提示
    {
        genRecordTime(false)
        menu_enable = true
        regButton.enabled = true
        verswitch.enabled = true
        regButton1.enabled = true
        starttiaoli=false
        tipplayer.playtip("../Resource/audios/cure_pause.mp3")
        enableScreenLock = true
        screenLockTimer.restart()
    }
    Timer{
        id:updateUsingTime
        interval: 500
        running: false
        repeat: true
        onTriggered: {
            var time = new Date()
            var dur = (startCureT == 0) ? 0 : time.valueOf() - startCureT
            var showT = new Date(parseInt(dur))
            text7.text = Qt.formatDateTime(showT, "mm:ss")
        }
    }
    Timer{
        id:uploadRecordTime
        interval: 10 * 60 * 1000
        running: false
        repeat: false
        onTriggered: {
            if(start_cure_time != "")
                genRecordTime(false)
        }
    }
    ComboBox{
        id: cureModBox
        x: 160
        y: 552
        width: 115
        height: 30
        currentIndex: 1
        enabled: starttiaoli === false && menu_enable === true
        model: ["生息模式", "姿势模式"]
        background: Rectangle{
            anchors.fill: parent
            color: "white"
            radius: 5
            border.color: "#d7d7d7"
            border.width: 1
        }
        onCurrentIndexChanged: {
            if(currentIndex === 0 || currentIndex === 1)
            {
                var tmp = dict
                if(currentIndex == 0)
                    dict = mod_shengxi
                else if(currentIndex == 1)
                    dict = mod_zishi
                for(let i = 0; i < tmp.length; i++)
                {
                    for(let j = 0; j < dict.length; j++)
                    {
                        if(dict[j]["jingluo"] === tmp[i]["jingluo"])
                        {
                            dict[j]["checked"] = tmp[i]["checked"]
                            break
                        }
                    }
                }
                resetJingLuoStep()
            }
        }
    }
    CusButton{
        id: regButton//识别按钮
        width:250
        height:80
        anchors.bottom: regButton1.top
        anchors.horizontalCenter: parent.horizontalCenter
        fontsize: 25
        anchors.bottomMargin: 10
        text: "调理规划"
        textColor: enabled ? "white" : "#949494"
        backgroundColor: enabled ? "#6300bf" : "#d7d7d7"
        onClicked:{//开始识别 先要播放声音 ，并发送消息给机器
            if (starttiaoli){
                tips.showTip({
                                  "message":"正在调理，无法进行识别！",
                                  "type":'info',
                                  "show":true
                              }
                              )
                return
            }
            if(listView.currentIndex >= jingluoplanStep.rowCount() || jingluoplanStep.rowCount() === 0)
            {
                if(homePage.login_role == "admin" || homePage.login_role == "root")
                {
                    tips.showTip({
                                     "message":"请先选择经络再识别",
                                     "type":'info',
                                     "show":true
                                 })
                }
                else
                {
                    tips.showTip({
                                     "message":"请先开处方再识别",
                                     "type":'info',
                                     "show":true
                                 })
                }
                return
            }
            regButton.enabled = false
            enableScreenLock = false
            verswitch.enabled = false
            menu_enable = false
            isrecognizesuccess=false
            recognizeActionStart()
            starttiaoli=false
            subPagIndex = 0
        }
    }
    CusButton{
        id: regButton1//开始调理/暂停调理按钮
        width:250
        height:80
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        fontsize: 25
        anchors.bottomMargin: 20
        text: starttiaoli ? "终止调理" : "开始调理"
        textColor: (enabled && (starttiaoli || isrecognizesuccess)) ? "white" : "#949494"
        backgroundColor: (enabled && (starttiaoli || isrecognizesuccess)) ? "#6300bf" : "#d7d7d7"
        onClicked:{
            if (starttiaoli){//暂停调理
                regButton1.enabled = false
                var args = {"func": "pauseTreatment"}
                guiIntf.guiCall(args, false, "操作员主动暂停调理")
                return
            }
            if (isrecognizesuccess){//开始调理
                tiaoliActionStart()
                subPagIndex = 0
                starttiaoli=true
            }
        }
    }
    Connections{
        target: guiIntf
        onIdentifyOK: {
            regButton.enabled = true
            verswitch.enabled = true
            menu_enable = true
            if(errCode == 0)//识别成功
            {
                subPagIndex = 1
                isrecognizesuccess = true
                tipplayer.playtip("../Resource/audios/identify_done.mp3")
//                regButton1.clicked()
                tips.showTip({"message": "识别成功", "type": "success", "show": true})
            }
            else if(errCode == -2)//姿势错误
            {
                warnmsg.showWarn("提示", "识别经络失败", "请调整为仰卧姿势且手心向上")
                tipplayer.playtip("../Resource/audios/tip_up_error.mp3")
                tips.showTip({"message": "识别失败", "type": "error", "show": true})
            }
            else if(errCode == -3)//姿势错误
            {
                warnmsg.showWarn("提示", "识别经络失败", "请调整为俯卧姿势且手心向下")
                tipplayer.playtip("../Resource/audios/tip_down_error.mp3")
                tips.showTip({"message": "识别失败", "type": "error", "show": true})
            }
            else if(errCode == -4)//机械臂超限
            {
                warnmsg.showWarn("提示", "识别经络失败", "请尽量调整人体位置到床中间后尝试重新识别")
                tipplayer.playtip("../Resource/audios/tip_position_error.mp3")
                tips.showTip({"message": "识别失败", "type": "error", "show": true})
            }
            else if(errCode === -5)//未标定相机
            {
                warnmsg.showWarn("提示", "识别经络失败", "请联系运维人员标定相机")
                tips.showTip({"message": "识别失败", "type": "error", "show": true})
            }
            else//其它错误
            {
                warnmsg.showWarn("提示", "识别经络失败", "请调整姿势后尝试重新识别")
                tipplayer.playtip("../Resource/audios/identify_error.mp3")
                tips.showTip({"message": "识别失败", "type": "error", "show": true})
            }
            enableScreenLock = true
            screenLockTimer.restart()
        }
    }
    Connections{
        target: guiIntf
        onCureOK: {
            if(suc)//继续调理下一条
            {
                listView.currentIndex++
                if(listView.currentIndex >= jingluoplanStep.rowCount() || jingluoplanStep.get(listView.currentIndex).zishiname !== "")
                {
                    ctimes--
                    if(ctimes > 0)
                    {
                        for(let i = listView.currentIndex - 1; i >= 0; i--)
                        {
                            if(jingluoplanStep.get(i).zishiname !== "")
                            {
                                listView.currentIndex = i
                                break
                            }
                        }
                    }
                }
                tiaoliActionNext()
            }
            else//调理过程报错
            {
                afterPauseTreatment()
            }
        }
    }
    Connections{
        target: guiIntf
        onSwitchPoseOK:{//收到床移动结束switchpos才算结束
            menu_enable = true
            regButton.enabled = true
            verswitch.enabled = true
            regButton1.enabled = true
            if(isSwitching)
            {
                subPagIndex = 0
                recognizeActionPlay()
            }
            else
            {
                isrecognizesuccess=false;
                starttiaoli=false;
                //播放调理结束提示语音
                tipplayer.playtip("../Resource/audios/cure_success.mp3")
            }
            enableScreenLock = true
            screenLockTimer.restart()
        }
    }
    Connections{
        target: guiIntf
        onPauseTreatmentOK:{
            afterPauseTreatment()
        }
    }

    Rectangle {
        id: whitedisp
        x: 25
        width: 250
        color: "#ffffff"
        anchors.top: rectangle3.bottom
        anchors.bottom: rectangle.top
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 15
        border.width: 1
        Rectangle{
            id: headline
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50
            color: "#2b579a"
            Text {
                id: name
                text: qsTr("调理进程")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pointSize: 15
                color: "white"
            }
        }
        ListView {
            id: listView
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: headline.bottom
            anchors.bottom: parent.bottom
            model: ListModel {
                id: jingluoplanStep
                ListElement {
                    name: "default"
                    soundname: ""
                    zishiname: ""
                    posturebox: ""
                    colorCode: "green"
                }
            }
            clip: true
            delegate: Rectangle {
                id: listviewcell
                width: listView.width
                height: 60
                color: tempcolor
                Rectangle {
                    id: listLeftLine
                    x: 14
                    y: index===0?listviewcell.height/2.0:0
                    width: 8
                    height: index===0 || jingluoplanStep.rowCount()-1==index? listviewcell.height/2.0: listviewcell.height
                    color: "#ededed"
                }

                Rectangle {
                    id: rightText
                    width: zishi.contentWidth+iconimg.width+10
                    height: 20
                    color: zishiname===""?"transparent":"#5e5e5e"
                    anchors.left: listLeftLine.right
                    Text {
                        id: zishi
                        color: "#ffffff"
                        text: zishiname
                        font.pixelSize: 14
                    }

                    Image {
                        id: iconimg
                        visible: zishiname===""?false:true
                        anchors.left: zishi.right
                        source: "../Resource/images/ys-laba.png"
                        anchors.leftMargin: 10
                    }
                    anchors.leftMargin: 10
                }

                Row {
                    id: row1
                    height: 40
                    anchors.top: rightText.bottom
                    anchors.fill: parent
                    // 圆点
                    Rectangle {
                        x: listView.currentIndex === index ?0:4
                        width: listView.currentIndex === index ?18:10
                        height: listView.currentIndex === index ?18:10
                        color: listView.currentIndex === index ?"#2b579a":"#999999"
                        radius: width/2
                        anchors.verticalCenter: row1.verticalCenter
                    }

                    Text {
                        id: jl_name
                        height: row1.height
                        visible: true
                        color: listView.currentIndex === index?"#2b579a":"#999999"
                        text: name
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                        font.pointSize: listView.currentIndex === index ? 18 : 14
                    }
                    anchors.leftMargin: 10
                    spacing: 10
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(starttiaoli === false && regButton.enabled === true)
                        {
                            var lastmodel = jingluoplanStep.get(listView.currentIndex)
                            var curmodel = jingluoplanStep.get(index)
                            listView.currentIndex = index
                            if(lastmodel.posturebox !== curmodel.posturebox)
                            {
                                isSwitching = true
                                regButton.enabled = false
                                enableScreenLock = false
                                verswitch.enabled = false
                                regButton1.enabled = false
                                isrecognizesuccess = false
                                var args = {"func": "switchPose"}
                                warnmsg.showWarnAndExe("提示", "请注意", "机械臂即将收回，请确认人体在安全位置", args)
                            }
                        }
                    }
                    onEntered: {
                        tempcolor = "#008ec9"
                    }
                    onExited: {
                        tempcolor = "transparent"
                    }
                    hoverEnabled: true
                }
                property color tempcolor: "#00000000"
            }
            highlightFollowsCurrentItem: true
            spacing: 0
            focus: true
        }
    }

    CusButton {
        id: curetypebtn2
        x: 30
        y: 70
        width: 80
        height: 30
        text: qsTr("选择经络")
        font.pointSize: 11
        Layout.preferredHeight: 36
        Layout.preferredWidth: 104
        enabled: regButton.enabled == true && regButton1.enabled == true && starttiaoli == false
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
        onClicked: {
            jlMenu.switchVer(homePage.cfg_ver)
            jlMenu.visible = true
        }

        visible: homePage.login_role == "root" || homePage.login_role == "admin"  || homePage.login_role == "super"
    }

    JingLuoMenu {
        id: jlMenu
        x: -(homePage.width - parent.width)
        y: -(homePage.height - parent.height)
        width: homePage.width
        height: homePage.height
        z: 99
        visible: false
    }

    ComboBox {
        id: cureModBox1
        x: 160
        y: 602
        width: 115
        height: 30
        currentIndex: 0
        background: Rectangle {
            color: "white"
            radius: 5
            anchors.fill: parent
            border.color: "#d7d7d7"
            border.width: 1
        }
        model: homePage.login_role === "super" ? ["标准", "稍强", "强", "1", "2"] : ["标准", "稍强", "强"]
        onCurrentTextChanged: {
            if(currentIndex < 3)
                guiIntf.setCureForceVal(parseInt(cureModBox1.currentIndex + 3))
            else
                guiIntf.setCureForceVal(parseInt(cureModBox1.currentIndex - 2))
        }
    }

    CusButton {
        id: curetypebtn4
        x: 116
        y: 70
        width: 80
        height: 30
        text: qsTr("系统重置")
        font.pointSize: 11
        Layout.preferredHeight: 36
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
        enabled: regButton.enabled == true && regButton1.enabled == true && starttiaoli == false
        Layout.preferredWidth: 104
        onClicked: {
            var args = {"func": "pauseTreatment"}
            guiIntf.guiCall(args, false, "操作员主动重置机械臂")
        }
        visible: homePage.login_role == "root" || homePage.login_role == "admin"  || homePage.login_role == "super"
    }

    AddPatientInfo{
        id: addinfo
        x: -(homePage.width - parent.width)
        y: -(homePage.height - parent.height)
        width: homePage.width
        height: homePage.height
        z: 99
        visible: false
    }

    Rectangle {
        id: rectangle
        x: 25
        y: 550
        width: 101
        height: 34
        color: "#2b579a"

        Text {
            id: text2
            x: 74
            y: 81
            color: "#ffffff"
            text: qsTr("流注次序：")
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: 11
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Rectangle {
        id: rectangle1
        x: 25
        y: 600
        width: 101
        height: 34
        color: "#2b579a"
        Text {
            id: text3
            x: 74
            y: 81
            color: "#ffffff"
            text: qsTr("阴阳表里：")
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: 11
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Rectangle {
        id: rectangle2
        x: 25
        y: 21
        width: 101
        height: 34
        color: "#2b579a"
        Text {
            id: text4
            x: 74
            y: 81
            color: "#ffffff"
            text: qsTr("当前患者：")
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: 11
            anchors.horizontalCenter: parent.horizontalCenter
        }
        visible: homePage.login_role !== "root" && homePage.login_role !== "admin" && homePage.login_role !== "super"
    }

    Rectangle {
        id: rectangle3
        x: 25
        y: 106
        width: 101
        height: 34
        color: "#2b579a"
        Text {
            id: text5
            x: 74
            y: 81
            color: "#ffffff"
            text: qsTr("调理时间：")
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: 11
            anchors.horizontalCenter: parent.horizontalCenter
        }
        visible: homePage.login_role !== "root" && homePage.login_role !== "admin" && homePage.login_role !== "super"
    }

    Rectangle {
        id: rectangle4
        x: 145
        y: 21
        width: 124
        height: 34
        color: "#ffffff"
        Text {
            id: text6
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "black"
            text: qsTr("")
            font.pointSize: 11
        }
        visible: homePage.login_role !== "root" && homePage.login_role !== "admin" && homePage.login_role !== "super"
    }

    Rectangle {
        id: rectangle5
        x: 145
        y: 106
        width: 124
        height: 34
        color: "#ffffff"
        Text {
            id: text7
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "black"
            text: qsTr("")
            font.pointSize: 11
        }
        visible: homePage.login_role !== "root" && homePage.login_role !== "admin" && homePage.login_role !== "super"
    }

    Rectangle {
        id: rectangle6
        x: 25
        y: 64
        width: 101
        height: 34
        visible: homePage.login_role !== "root" && homePage.login_role !== "admin" && homePage.login_role !== "super"
        color: "#2b579a"
        Text {
            id: text8
            x: 74
            y: 81
            color: "#ffffff"
            text: qsTr("调理方案：")
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: 11
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Rectangle {
        id: rectangle7
        x: 145
        y: 64
        width: 124
        height: 34
        visible: homePage.login_role !== "root" && homePage.login_role !== "admin" && homePage.login_role !== "super"
        color: "#ffffff"
        Text {
            id: text9
            color: "#000000"
            text: qsTr("")
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
        }
    }

    SpinBox{
        id: cureTimes
        x: 160
        y: 654
        width: 110
        height: 30
        visible: true
        from: 0
        to: 99
        value: 1
        enabled: starttiaoli === false
        editable: true
        contentItem: TextInput{
            selectByMouse: true
            text: cureTimes.textFromValue(cureTimes.value, cureTimes.locale)
            onTextChanged: cureTimes.value = text
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            validator: RegExpValidator{regExp: /\d{1,2}/}
        }
        up.indicator: Rectangle{
            x: parent.width - width
            width: parent.height * 0.7
            height: parent.height
            color: "#dfdfdf"
            Text {
                text: "+"
                font.bold: true
                font.pointSize: 9
                anchors.centerIn: parent
            }
        }
        down.indicator: Rectangle{
            x: 0
            width: parent.height * 0.7
            height: parent.height
            color: "#dfdfdf"
            Text {
                text: "-"
                font.bold: true
                font.pointSize: 9
                anchors.centerIn: parent
            }
        }
    }

    Rectangle {
        id: rectangle8
        x: 25
        y: 652
        width: 101
        height: 34
        color: "#2b579a"
        Text {
            id: text10
            x: 74
            y: 81
            color: "#ffffff"
            text: qsTr("调理次数：")
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: 11
        }
    }

}

/*##^##
Designer {
    D{i:0;height:978;width:300}D{i:57}
}
##^##*/
