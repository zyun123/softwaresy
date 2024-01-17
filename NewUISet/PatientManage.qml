import QtQuick 2.12
import TaoQuick 1.0
import QtQuick.Controls 1.2
import QtQuick.Controls 2.12 as Qcontrol2
import PatientManageModel 1.0
import FormulaManageModel 1.0
import FormulaTplManageModel 1.0
import "./qml_basic"

Item {
    id: item1
    width: 1920
    height: 1080

    function clearPage()
    {
        textField.text = ""
    }

    MyMessageBox{
        id: msg_box
        anchors.fill: parent
        z: 99
        visible: false
    }

    Connections{
        target: msg_box
        onAccepted:{
            tabmod.removeData(tabview.currentRow)
        }
        onRejected:{

        }
    }

    FormulaManageModel{
        id:formula_mod
    }

    FormulaTplManageModel{
        id:formula_tpl_mod
    }

    onVisibleChanged: tabmod.resetData()

    Rectangle {
        id: tabbg
        color: "#f2f2f2"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: textField.bottom
        anchors.bottom: parent.bottom
        anchors.topMargin: 20
        anchors.bottomMargin: 40
        anchors.rightMargin: 40
        anchors.leftMargin: 40
        MyTableView {
            id: tabview
            y: 190
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            model: PatientManageModel {
                id: tabmod
            }
            onVisibleChanged: tabmod.resetData()
            col_width: [100, 200, 100, 100, 200, 100, 200, 100, 400]
            rowHeight: 50
            TableViewColumn {
                title: "姓名"
                role: "name"
                width: 100
            }
            TableViewColumn {
                title: "身份证号"
                role: "idcard"
                width: 200
            }
            TableViewColumn {
                title: "性别"
                role: "gender"
                width: 100
            }
            TableViewColumn {
                title: "年龄"
                role: "age"
                width: 100
            }
            TableViewColumn {
                title: "电话"
                role: "phone"
                width: 200
            }
            TableViewColumn {
                title: "创建者"
                role: "creator"
                width: 100
            }
            TableViewColumn {
                title: "创建时间"
                role: "create_time"
                width: 200
            }
            TableViewColumn {
                title: "调理方案"
                role: "formula_mod"
                width: 100
            }
            TableViewColumn {
                title: "经络配伍"
                role: "meridians"
                width: 400
            }
            onSortIndicatorOrderChanged: tabmod.cussort(sortIndicatorColumn, sortIndicatorOrder)
            onSortIndicatorColumnChanged: tabmod.cussort(sortIndicatorColumn, sortIndicatorOrder)
        }
    }

    TextField {
        id: textField
        width: 170
        height: 30
        text: qsTr("")
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        placeholderText: qsTr("请输入关键字")
        font.pointSize: 13
        selectByMouse: true
        onAccepted: button1.clicked()
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.top: parent.top
        anchors.topMargin: 40
    }
    Qcontrol2.Button{
        id: button1
        width: 70
        height: 30
        text: "搜索"
        anchors.left: textField.right
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.leftMargin: 20
        onClicked: textField.text === "" ? tabmod.resetData() : tabmod.filter(textField.text, ["name", "phone"])
        background: Rectangle{
            anchors.fill: parent
            radius: 5
            color: button1.pressed ? "#6300bf" : "#f2f2f2"
        }
    }
    Qcontrol2.Button{
        id: button5
        width: 82
        height: 30
        text: "开始调理"
        anchors.left: button1.right
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.leftMargin: 20
        onClicked: {
            if(tabview.currentRow < 0)
                warnmsg.showWarn("提示", "", "请选择患者")
            else
            {
                var rowdat = tabmod.getRowDat(tabview.currentRow)
                var finfos = formula_mod.find("patient_id", rowdat["idcard_wtf"])
                if(finfos.length === 0)
                    warnmsg.showWarn("提示", "", "请先为该患者添加处方")
                else
                {
                    var plandata = {}
                    plandata["name"] = rowdat["name"]
                    plandata["meridians"] = finfos[0]["meridians"]
                    plandata["cure_times"] = finfos[0]["cure_times"]
                    plandata["idcard"] = rowdat["idcard"]
                    plandata["mode"] = (finfos[0]["cure_mode"] === "姿势模式") ? 1 : 0
                    plandata["formula_mod"] = finfos[0]["formula_mod"]
                    plandata["mode_wtf"] = finfos[0]["cure_force"] === "标准" ? 0 : finfos[0]["cure_force"] === "稍强" ? 1 : 2
                    homePage.cureStart(plandata)
                    homePage.toPage(1)
                }
            }
        }
        background: Rectangle{
            anchors.fill: parent
            radius: 5
            color: button5.pressed ? "#6300bf" : "#f2f2f2"
        }
    }
    Qcontrol2.Button{
        id: button31
        x: 1630
        y: 40
        width: 70
        height: 30
        visible: false
        text: "新增"
        anchors.right: button3.left
        anchors.rightMargin: 20
        onClicked: {
            modifyedit.cleardat()
            modifyedit.visible = true
        }
        background: Rectangle{
            anchors.fill: parent
            radius: 5
            color: button31.pressed ? "#6300bf" : "#f2f2f2"
        }
    }
    Qcontrol2.Button{
        id: button3
        x: 1722
        y: 40
        width: 70
        height: 30
        visible: false
        text: "修改"
        anchors.right: button32.left
        anchors.rightMargin: 20
        onClicked: {
            if(tabview.currentRow != -1)
            {
                modifyedit.loaddat(tabmod.getRowDat(tabview.currentRow))
                modifyedit.visible = true
            }
        }
        background: Rectangle{
            anchors.fill: parent
            radius: 5
            color: button3.pressed ? "#6300bf" : "#f2f2f2"
        }
    }
    Qcontrol2.Button{
        id: button32
        x: 1810
        y: 40
        width: 70
        height: 30
        text: "删除"
        onClicked: {
            if(tabview.currentRow != -1)
                msg_box.showWarn("提示", "", "请确认是否删除该患者信息")
        }
        background: Rectangle{
            anchors.fill: parent
            radius: 5
            color: button32.pressed ? "#6300bf" : "#f2f2f2"
        }
        visible: homePage.login_role == "doctor" || homePage.login_role == "super"
    }
    PatientManageRowEdit{
        id: modifyedit
        z: 99
        visible: false
        onRetData1: {
//            tabmod.modifyData(editdat)
            tabmod.resetData()
        }
        onRetData2: {
            if(tabmod.insertData(editdat) === false)
                warnmsg.showWarn("错误", "", "新建患者信息失败，请确认该身份证号是否已录入")
            tabmod.resetData()
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}
}
##^##*/
