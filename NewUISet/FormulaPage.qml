import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls 1.4 as Qcontrol1
import QtQuick.Controls 2.12 as Qcontrol2
import QtQml 2.12
import TaoQuick 1.0

import UserManageModel 1.0
import PatientManageModel 1.0
import FormulaTplManageModel 1.0
import FormulaManageModel 1.0
import QtQuick.Layouts 1.0

import "./qml_basic"

Item {
    width: 1920
    height: 975

    property string cur_formula_mod: ""
    property string data_to_remove: ""

    function clearPage()
    {
        clearBtn.clicked()
        txt_patient_name.text = ""
    }

    MyMessageBox{
        id: msg_box
        x: 0
        y: 975 - 1080
        width: 1920
        height: 1080
        z: 99
        visible: false
    }

    Connections{
        target: msg_box
        onAccepted:{
            if(data_to_remove == "formula_mod")
            {
                formula_mod.removeData(formula_view.currentRow)
                gobtn.enabled = false
            }
            if(data_to_remove == "formulaTpl_mod")
            {
                formulaTpl_mod.removeData(formulaTpl_view.currentRow)
            }
        }
        onRejected:{

        }
    }

    FormulaPageEdit{
        id: tpledit
        z: 99
        visible: false
        onUpdatelist: {
            formulaTpl_mod.resetData()
            formulaTpl_view.selection.clear()
            formulaTpl_view.selection.select(formulaTpl_view.rowCount - 1)
            formulaTpl_view.currentRow = formulaTpl_view.rowCount - 1
        }
    }

    Rectangle {
        id: rectangle
        anchors.fill: parent
        color: "transparent"

        Rectangle {
            id: rectangle1
            width: 540
            color: "#f2f2f2"
            radius: 5
            border.color: "#add8e6"
            border.width: 0
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 30
            anchors.bottomMargin: 40
            anchors.topMargin: 40

            Rectangle {
                id: rectangle3
                x: 39
                y: 164
                width: 104
                height: 35
                color: "#0061b89d"
                radius: 5
                Text {
                    id: text2
                    color: "#000000"
                    text: qsTr("建立日期：")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }
            }

            TextField {
                id: txt_patient_name
                x: 39
                y: 99
                width: 316
                height: 35
                placeholderText: "姓名/身份证号"
                font.pointSize: 13
                enabled: true
            }

            CusButton{
                id: button
                x: 362
                y: 99
                width: 77
                height: 35
                text: "查询处方"
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked:
                {
                    var patientInfo = patient_mod.find("name", txt_patient_name.text)
                    if(patientInfo.length > 0)
                        formula_mod.filter(patientInfo[0]["idcard"], ["idcard"], time_from, time_to, "serial_num")
                    else
                        formula_mod.filter(txt_patient_name.text, ["idcard"], time_from, time_to, "serial_num")
                }
            }

            Rectangle {
                id: rectangle9
                x: 328
                y: 161
                width: 33
                height: 35
                color: "#0061b89d"
                radius: 5
                Text {
                    id: text8
                    color: "#000000"
                    text: qsTr("至")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.rightMargin: 2
                    anchors.bottomMargin: -1
                    anchors.leftMargin: -2
                    anchors.topMargin: 1
                    font.pointSize: 16
                    font.bold: false
                }
            }

            Rectangle {
                id: tabbg
                y: 233
                height: 632
                color: "white"
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 30
                anchors.rightMargin: 17
                anchors.leftMargin: 22
                MyTableView{
                    id: formula_view
                    y: 190
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    model: FormulaManageModel {
                        id: formula_mod
                    }
                    onVisibleChanged: formula_mod.resetData()
                    col_width: [100, 100, 150, 100, 200]
                    rowHeight: 50
                    onRowCountChanged: {
                        formula_view.selection.clear()
                        formula_view.selection.select(formula_view.rowCount - 1)
                        formula_view.currentRow = formula_view.rowCount - 1
                    }
                    TableViewColumn {
                        width: 100
                        title: "患者"
                        role: "patient_name"
                    }
                    TableViewColumn {
                        width: 100
                        title: "调理方案"
                        role: "formula_mod"
                    }
                    TableViewColumn {
                        width: 150
                        title: "创建日期"
                        role: "create_time"
                    }
                    TableViewColumn {
                        width: 100
                        title: "创建者"
                        role: "creator"
                    }
                    TableViewColumn {
                        width: 200
                        title: "流水号"
                        role: "serial_num"
                    }
                    onClicked: {
                        var rowdat = formula_mod.getRowDat(currentRow)
                        var patient_info = patient_mod.find("idcard", rowdat["patient_id"])
                        txt_idcard.text = patient_info[0]["idcard"]
                        txt_diagnosis.text = rowdat["diagnosis"]
                        txt_tocure_meridians.text = rowdat["meridians"]
                        txt_note.text = rowdat["note"]
                        txt_phone.text =  patient_info[0]["phone"]
                        txt_age.text = patient_info[0]["age"]
                        txt_name.text = rowdat["patient_name"]
                        cusComboBox.currentIndex = (patient_info[0]["gender"] === "男") ? 0 : 1
                        cureModBox.currentIndex = (rowdat["cure_mode"] === "生息模式") ? 0 : 1
                        cur_formula_mod = rowdat["formula_mod"]
                        cureModBox1.currentIndex = rowdat["cure_force"] === "标准" ? 0 : rowdat["cure_force"] === "稍强" ? 1 : 2
                        txt_ctimes.text = rowdat["cure_times"]
                        txt_idcard1.text = rowdat["mode_wtf"]

                        txt_name.enabled = false
                        txt_age.enabled = false
                        txt_idcard.enabled = false
                        txt_phone.enabled = false
                        cusComboBox.enabled = false

                        gobtn.enabled = true
                    }
                }
            }

            TextField {
                id: txt_patient_name1
                x: 169
                y: 164
                width: 153
                height: 35
                font.pointSize: 13
                placeholderText: ""
                enabled: true
                inputMask: "9999-99-99"
                text: (new Date()).getFullYear() + ('0' + ((new Date()).getMonth() + 1)).substr(-2) + "01"
            }

            TextField {
                id: txt_patient_name2
                x: 362
                y: 164
                width: 153
                height: 35
                font.pointSize: 13
                enabled: true
                placeholderText: ""
                inputMask: "9999-99-99"
                text: (new Date()).getFullYear() + ('0' + ((new Date()).getMonth() + 1)).substr(-2) + ('0' + (new Date()).getDate()).substr(-2)
            }

            Text {
                id: text22
                x: 315
                width: 203
                height: 51
                color: "#000000"
                text: qsTr("处方笺列表")
                anchors.top: parent.top
                horizontalAlignment: Text.AlignHCenter
                anchors.topMargin: 20
                font.pointSize: 24
                font.bold: true
                anchors.horizontalCenterOffset: 1
                anchors.horizontalCenter: parent.horizontalCenter
            }

            CusButton {
                id: button1
                x: 446
                y: 99
                width: 77
                height: 35
                text: "删除"
                textColor: pressed ? "white" : "black"
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                onClicked: {
                    if(formula_view.currentRow !== -1)
                    {
                        data_to_remove = "formula_mod"
                        msg_box.showWarn("提示", "", "请确认是否删除该处方")
                    }
                }
            }
        }

        Rectangle {
            id: meridians_buttons_rect
            x: 584
            y: 40
            width: 320
            height: 895
            color: "#f2f2f2"
            radius: 5
            border.color: "#ffffff"
            border.width: 2

            Rectangle {
                id: tabbg1
                x: 10
                y: 150
                width: 300
                height: 702
                color: "#ffffff"
                MyTableView{
                    id: formulaTpl_view
                    y: 190
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    model: FormulaTplManageModel {
                        id: formulaTpl_mod
                    }
                    onVisibleChanged: formulaTpl_mod.resetData()
                    col_width: [300]
                    rowHeight: 50
                    TableViewColumn {
                        width: 300
                        role: "name"
                        title: "方案名称"
                    }
//                    TableViewColumn {
//                        width: 220
//                        role: "meridians"
//                        title: "包含经络"
//                    }
//                    TableViewColumn {
//                        width: 200
//                        role: "detail"
//                        title: "备注"
//                    }
                }
            }

            CusButton {
                id: button5
                x: 16
                y: 100
                width: 63
                height: 33
                text: "创建"
                backgroundColor: button5.pressed ? "#6300bf" : "#d7d7d7"
                onClicked: {
                    tpledit.visible = true
                }
                textColor: enabled ? pressed ? "white" : "black" : "#949494"
                checked: true
                fontsize: 13
                checkable: true
                hoverEnabled: false
            }

            CusButton {
                id: button6
                x: 92
                y: 100
                width: 63
                height: 33
                text: "删除"
                backgroundColor: button6.pressed ? "#6300bf" : "#d7d7d7"
                onClicked: {
                    if(formulaTpl_view.currentRow >= 0 && formulaTpl_view.currentRow < formulaTpl_view.rowCount)
                    {
                        var rowdat = formulaTpl_mod.getRowDat(formulaTpl_view.currentRow)
                        if(rowdat["detail"] === "1")
                            warnmsg.showWarn("提示", "", "内置调理方案无法删除")
                        else
                        {
                            data_to_remove = "formulaTpl_mod"
                            msg_box.showWarn("提示", "", "请确认是否删除该调理方案")
                        }
                    }
                }
                textColor: enabled ? pressed ? "white" : "black" : "#949494"
                checked: true
                fontsize: 13
                checkable: true
                hoverEnabled: false
            }

            CusButton {
                id: button3
                x: 168
                y: 100
                width: 63
                height: 33
                text: "调用"
                backgroundColor: button3.pressed ? "#6300bf" : "#d7d7d7"
                onClicked: {
                    if(formulaTpl_view.currentRow > -1)
                    {
                        var tmpdat = formulaTpl_mod.getRowDat(formulaTpl_view.currentRow)
                        cur_formula_mod = tmpdat["name"]
                        txt_tocure_meridians.text = tmpdat["meridians"]
                        txt_idcard1.text = tmpdat["name_wtf"]
                    }
                }
                textColor: pressed ? "white" : "black"
                checked: true
                fontsize: 13
                checkable: true
                hoverEnabled: false
            }

            Text {
                id: text21
                x: 315
                width: 203
                height: 51
                color: "#000000"
                text: qsTr("调理方案")
                anchors.top: parent.top
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenterOffset: 1
                anchors.topMargin: 20
                font.pointSize: 24
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        Rectangle {
            id: rectangle13
            x: 916
            y: 40
            width: 958
            height: 895
            color: "#f2f2f2"
            radius: 5
            border.color: "#ffffff"
            border.width: 2
            CusButton {
                id: svbtn
                x: 239
                y: 104
                width: 100
                height: 35
                text: "保存处方"
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    //校验字段是否为空
                    if(txt_name.text === "" || txt_age.text === "" || txt_idcard.text === "" || txt_phone.text === "" ||
                            txt_diagnosis.text === "" || txt_tocure_meridians.text === "")
                    {
                        warnmsg.showWarn("提示", "保存处方失败", "请确认处方信息填写完整")
                        return
                    }
                    //校验身份证号
                    var tmpidcard = txt_idcard.text
                    if(tmpidcard.length === 18)
                    {
                        for(var i = 0; i < 17; i++)
                        {
                            if(isNaN(tmpidcard[i]) === true)
                            {
                                warnmsg.showWarn("提示", "保存处方失败", "身份证号格式错误")
                                return
                            }
                        }
                    }
                    else
                    {
                        warnmsg.showWarn("提示", "保存处方失败", "身份证号长度应为18位")
                        return
                    }
                    //校验手机号
                    var tmpphone = txt_phone.text
                    if(tmpphone.length !== 11)
                    {
                        warnmsg.showWarn("提示", "保存处方失败", "手机号长度应为11位")
                        return
                    }
                    //校验姓名
                    var tmpname = txt_name.text
                    if(tmpname.replace(/[^\x00-\xff]/g,'__').length > 15)
                    {
                        warnmsg.showWarn("提示", "保存处方失败", "患者姓名长度应在15个字符以内")
                        return
                    }
                    //校验年龄
                    var tmpage = parseInt(txt_age.text)
                    if(!(tmpage >= 18 && tmpage <= 70))
                    {
                        warnmsg.showWarn("提示", "保存处方失败", "患者年龄需在18~70岁")
                        return
                    }
                    //校验调理次数
                    var tmptimes = parseInt(txt_ctimes.text)
                    if(!(tmptimes >= 1 && tmptimes <= 99))
                    {
                        warnmsg.showWarn("提示", "保存处方失败", "调理次数应在1到99之间")
                        return
                    }
                    //保存患者信息和处方
                    var plandata = []
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
                    var patientInfo = patient_mod.find("idcard",txt_idcard.text)
                    if(patientInfo.length===0){
                        var new_patient=[]
                        new_patient.push(txt_name.text)
                        new_patient.push(txt_idcard.text)
                        new_patient.push(cusComboBox.displayText)
                        new_patient.push(txt_age.text)
                        new_patient.push(txt_phone.text)
                        new_patient.push(homePage.user_id)
                        new_patient.push(time_str)
                        new_patient.push(txt_note.text)
                        if(patient_mod.insertData(new_patient) === false)
                        {
                            warnmsg.showWarn("错误", "保存新患者信息失败", "请确认该患者信息是否与已录入信息冲突")
                            return
                        }
                    }
                    else
                    {
                        var genderindex = (patientInfo[0]["gender"] === "男") ? 0 : 1
                        if(patientInfo[0]["name"] !== txt_name.text || patientInfo[0]["age"] !== txt_age.text ||
                                genderindex !== cusComboBox.currentIndex || patientInfo[0]["phone"] !== txt_phone.text)
                        {
                            warnmsg.showWarn("提示", "患者信息错误", "患者信息与已录入信息冲突，请确认或修改后重新提交处方")
                            return
                        }
                    }
                    plandata.push(timestamp)
                    plandata.push(txt_idcard.text)
                    plandata.push(txt_diagnosis.text)
                    plandata.push(txt_tocure_meridians.text)
                    plandata.push(cureModBox.currentText)
                    plandata.push(cureModBox1.currentText)
                    plandata.push(txt_ctimes.text)
                    plandata.push(txt_note.text)
                    plandata.push(homePage.user_id)
                    plandata.push(time_str)
                    plandata.push(cur_formula_mod)
                    if(formula_mod.insertData(plandata) === false)
                    {
                        warnmsg.showWarn("错误", "", "保存处方信息失败")
                        return
                    }
                    gobtn.enabled = true
                }
                UserManageModel {
                    id: user_mod
                }

                PatientManageModel {
                    id: patient_mod
                }
                font.pointSize: 15
                font.bold: true
                enabled: homePage.login_role == "doctor" || homePage.login_role == "super"
            }

            CusButton {
                id: gobtn
                x: 806
                y: 104
                width: 100
                height: 35
                text: "开始调理"
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: enabled ? pressed ? "white" : "black" : "#949494"
                onClicked: {
                    var plandata = {}
                    plandata["name"]=txt_name.text
                    plandata["meridians"] = txt_tocure_meridians.text
                    plandata["cure_times"] = txt_ctimes.text
                    plandata["idcard"] = txt_idcard.text
                    plandata["mode"] = cureModBox.currentIndex
                    plandata["formula_mod"] = cur_formula_mod
                    plandata["mode_wtf"] = cureModBox1.currentIndex
                    homePage.cureStart(plandata)
                    homePage.toPage(1)
                }
                font.pointSize: 15
                font.bold: true
                enabled: false
            }

            CusButton {
                id: clearBtn
                x: 122
                y: 104
                width: 100
                height: 35
                text: "新建"
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    txt_name.text = ""
                    txt_age.text = ""
                    txt_phone.text = ""
                    txt_idcard.text = ""
                    txt_tocure_meridians.text = ""
                    txt_diagnosis.text = ""
                    txt_note.text = ""

                    txt_name.enabled = true
                    txt_age.enabled = true
                    txt_idcard.enabled = true
                    txt_phone.enabled = true
                    cusComboBox.enabled = true

                    gobtn.enabled = false

                    txt_ctimes.text = "1"
                }
                font.pointSize: 15
                font.bold: true
            }

            Text {
                id: text9
                x: 315
                width: 203
                height: 51
                color: "#000000"
                text: qsTr("处方笺")
                anchors.top: parent.top
                horizontalAlignment: Text.AlignHCenter
                anchors.topMargin: 20
                font.pointSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
                font.bold: true
            }

            Rectangle {
                id: rectangle15
                width: 784
                height: 180
                color: "#f2f2f2"
                radius: 5
                border.color: "#8b8b8b"
                border.width: 1
                anchors.left: parent.left
                anchors.top: text9.bottom
                anchors.leftMargin: 122
                anchors.topMargin: 91
                CusButton {
                    id: complBtn
                    x: 670
                    y: 21
                    width: 57
                    height: 35
                    visible: false
                    text: "补全"
                    backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                    textColor: pressed ? "white" : "black"
                    onClicked: {
                        var patientInfo
                        if(txt_phone.text != "")
                            patientInfo = patient_mod.find("phone", txt_phone.text)
                        else if(txt_idcard.text != "")
                            patientInfo = patient_mod.find("idcard", txt_idcard.text)
                        else if(txt_name.text != "")
                            patientInfo = patient_mod.find("name", txt_name.text)
                        if(patientInfo.length !== 0)
                        {
                            txt_name.text = patientInfo[0]["name"]
                            txt_idcard.text = patientInfo[0]["idcard"]
                            txt_age.text = patientInfo[0]["age"]
                            txt_phone.text = patientInfo[0]["phone"]
                            cusComboBox.currentIndex = (patientInfo[0]["gender"] === "男") ? 0 : 1
                        }
                    }
                    font.pointSize: 15
                    font.bold: true
                    enabled: true
                }

                Text {
                    id: text10
                    x: 66
                    y: 23
                    color: "#000000"
                    text: qsTr("姓名：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                TextField {
                    id: txt_name
                    x: 135
                    y: 23
                    width: 156
                    height: 35
                    font.pointSize: 13
                    onTextChanged: gobtn.enabled = false
                    placeholderText: ""
                }

                Text {
                    id: text11
                    x: 24
                    y: 107
                    color: "#000000"
                    text: qsTr("身份证号：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                TextField {
                    id: txt_idcard
                    x: 135
                    y: 107
                    width: 238
                    height: 35
                    validator: RegExpValidator{regExp: /\d{1,17}[0-9a-z]{0,1}/}
                    onFocusChanged: if(focus === true) select(0, 0)
                    font.pointSize: 13
                    onTextChanged: gobtn.enabled = false
                    placeholderText: ""
                }

                Text {
                    id: text12
                    x: 334
                    y: 21
                    color: "#000000"
                    text: qsTr("性别：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                CusComboBox {
                    id: cusComboBox
                    x: 408
                    y: 21
                    width: 70
                    height: 35
                    onCurrentIndexChanged: gobtn.enabled = false
                    model: ["男", "女"]
                }

                Text {
                    id: text13
                    x: 498
                    y: 21
                    color: "#000000"
                    text: qsTr("年龄：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                TextField {
                    id: txt_age
                    x: 567
                    y: 21
                    width: 97
                    height: 35
                    validator: RegExpValidator{regExp: /\d{1,2}/}
                    onFocusChanged: if(focus === true) select(0, 0)
                    font.pointSize: 13
                    onTextChanged: gobtn.enabled = false
                    placeholderText: ""
                }

                Text {
                    id: text14
                    x: 433
                    y: 105
                    color: "#000000"
                    text: qsTr("联系方式：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                TextField {
                    id: txt_phone
                    x: 544
                    y: 105
                    width: 156
                    height: 35
                    validator: RegExpValidator{regExp: /\d{1,11}/}
                    onFocusChanged: if(focus === true) select(0, 0)
                    font.pointSize: 13
                    onTextChanged: gobtn.enabled = false
                    placeholderText: ""
                }
            }

            Rectangle {
                id: rectangle16
                x: 122
                y: 357
                width: 784
                height: 163
                color: "#f2f2f2"
                radius: 5
                border.color: "#8b8b8b"
                border.width: 1
                Rectangle {
                    id: rectangle21
                    x: 51
                    y: 18
                    width: 706
                    height: 126
                    color: "#ffffff"
                    Qcontrol1.ScrollView {
                        x: 0
                        y: 0
                        width: 706
                        height: 126
                        contentItem: TextArea {
                            id: txt_diagnosis
                            x: 0
                            y: 0
                            width: 706
                            height: 126
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: 13
                            onTextChanged: {
                                if (length > 1000)
                                    remove(1000, length);
                                else
                                {
                                    gobtn.enabled = false
                                    text1.text = parseInt(length) + "/1000"
                                }
                            }

                            Text {
                                id: text1
                                x: 643
                                y: 105
                                width: 63
                                height: 21
                                text: qsTr("0/1000")
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: rectangle17
                x: 122
                y: 539
                width: 784
                height: 313
                color: "#f2f2f2"
                radius: 5
                border.color: "#8b8b8b"
                border.width: 1
                Rectangle {
                    id: rectangle211
                    x: 158
                    y: 20
                    width: 600
                    height: 106
                    visible: false
                    color: "#ffffff"
                    Qcontrol1.ScrollView {
                        x: 0
                        y: 0
                        width: 600
                        height: 106
                        contentItem: TextArea {
                            id: txt_tocure_meridians
                            x: 0
                            y: 0
                            width: 600
                            height: 106
                            visible: true
                            font.pointSize: 13
                            onTextChanged: gobtn.enabled = false
                            enabled: false
                        }
                    }
                }

                Text {
                    id: text18
                    x: 32
                    y: 20
                    color: "#000000"
                    text: qsTr("    经络配伍：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                Text {
                    id: text19
                    x: 35
                    y: 217
                    visible: true
                    color: "#000000"
                    text: qsTr("            备注：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                Rectangle {
                    id: rectangle212
                    x: 161
                    y: 217
                    width: 600
                    height: 60
                    visible: true
                    color: "#ffffff"
                    Qcontrol1.ScrollView {
                        x: 0
                        y: 0
                        width: 600
                        height: 60
                        contentItem: TextArea {
                            id: txt_note
                            x: 0
                            y: 0
                            width: 600
                            height: 60
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: 13
                            onTextChanged: {
                                if (length > 500)
                                    remove(500, length);
                                else
                                {
                                    gobtn.enabled = false
                                    text3.text = parseInt(length) + "/500"
                                }
                            }

                            Text {
                                id: text3
                                x: 537
                                y: 39
                                width: 63
                                height: 21
                                text: qsTr("0/500")
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }

                Qcontrol2.ComboBox {
                    id: cureModBox
                    x: 168
                    y: 94
                    width: 130
                    height: 34
                    anchors.leftMargin: 0
                    background: Rectangle {
                        color: "#f2f2f2"
                        radius: 5
                        border.color: "#8b8b8b"
                        border.width: 1
                        anchors.fill: parent
                    }
                    onCurrentIndexChanged: gobtn.enabled = false
                    currentIndex: 1
                    model: ["生息模式", "姿势模式"]
                    anchors.bottomMargin: 10
                }

                Text {
                    id: text20
                    x: 32
                    y: 94
                    color: "#000000"
                    text: qsTr("    流注次序：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                TextField {
                    id: txt_idcard1
                    x: 168
                    y: 20
                    width: 356
                    height: 40
                    font.pointSize: 13
                    placeholderText: ""
                    enabled: false
                }

                Text {
                    id: text23
                    x: 32
                    y: 157
                    color: "#000000"
                    text: qsTr("    阴阳表里：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                Qcontrol2.ComboBox {
                    id: cureModBox1
                    x: 168
                    y: 157
                    width: 130
                    height: 34
                    background: Rectangle {
                        color: "#f2f2f2"
                        radius: 5
                        border.color: "#8b8b8b"
                        border.width: 1
                        anchors.fill: parent
                    }
                    anchors.bottomMargin: 10
                    anchors.leftMargin: 0
                    model: ["标准", "稍强", "强"]
                    currentIndex: 0
                }

                Text {
                    id: text24
                    x: 362
                    y: 94
                    color: "#000000"
                    text: qsTr("    调理次数：")
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 16
                    font.bold: false
                }

                TextField {
                    id: txt_ctimes
                    x: 512
                    y: 93
                    width: 97
                    height: 35
                    validator: RegExpValidator{regExp: /\d{1,2}/}
                    font.pointSize: 13
                    placeholderText: ""
                    text: "1"
                }
            }

            Rectangle {
                id: rectangle12
                x: 56
                y: 162
                width: 44
                height: 180
                color: "#2b579a"
                Text {
                    id: text15
                    color: "#ffffff"
                    text: qsTr("患\n者\n信\n息")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 17
                }
            }

            Rectangle {
                id: rectangle18
                x: 56
                y: 357
                width: 44
                height: 163
                color: "#2b579a"
                Text {
                    id: text16
                    color: "#ffffff"
                    text: qsTr("诊\n断")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 17
                }
            }

            Rectangle {
                id: rectangle19
                x: 56
                y: 539
                width: 44
                height: 313
                color: "#2b579a"
                Text {
                    id: text17
                    color: "#ffffff"
                    text: qsTr("调\n理\n处\n方")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 17
                }
            }
        }
    }

}











/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}D{i:66}
}
##^##*/
