import QtQuick 2.12
import TaoQuick 1.0
import QtQuick.Controls 1.2
import QtQuick.Controls 2.12 as Qcontrol2
import UserManageModel 1.0
import "./qml_basic"

Item {
    id: item1
    width: 1920
    height: 1080

    onVisibleChanged: tabmod.resetData()

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

    Rectangle {
        id: tabbg
        color: "white"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: textField.bottom
        anchors.bottom: parent.bottom
        anchors.topMargin: 20
        anchors.bottomMargin: 60
        anchors.rightMargin: 40
        anchors.leftMargin: 40
        MyTableView {
            id: tabview
            y: 190
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            model: UserManageModel {
                id: tabmod
            }
            onVisibleChanged: tabmod.resetData()
            col_width: [150, 150, 150]
            rowHeight: 50
            TableViewColumn {
                title: "账号id"
                role: "userid"
                width: 150
            }
            TableViewColumn {
                title: "密码"
                role: "password"
                width: 150
            }
            TableViewColumn {
                title: "角色权限"
                role: "role"
                width: 150
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
    CusButton{
        id: button1
        width: 70
        height: 30
        text: "搜索"
        anchors.left: textField.right
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.leftMargin: 20
        onClicked: tabmod.filter(textField.text, ["userid", "role"])
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }
    CusButton{
        id: button5
        width: 70
        height: 30
        text: "重置"
        anchors.left: button1.right
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.leftMargin: 20
        onClicked: tabmod.resetData()
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }
    CusButton{
        id: button2
        width: 70
        height: 30
        text: "添加"
        anchors.right: button4.left
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.rightMargin: 20
        onClicked: {
            insertedit.cleardat()
            insertedit.visible = true
        }
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }
    CusButton{
        id: button3
        width: 70
        height: 30
        text: "修改"
        anchors.right: button4.left
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.rightMargin: 20
        onClicked: {
            if(tabview.currentRow != -1)
            {
                modifyedit.loaddat(tabmod.getRowDat(tabview.currentRow))
                modifyedit.visible = true
            }
        }
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
        visible: false
    }
    CusButton{
        id: button4
        width: 70
        height: 30
        text: "删除"
        anchors.right: parent.right
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.rightMargin: 40
        onClicked: {
            if(tabview.currentRow != -1)
            {
                var usrdat = tabmod.getRowDat(tabview.currentRow)
                if(usrdat["userid"] === "admin")
                {
                    tips.showTip({"message": "初始管理员无法删除", "type": "info", "show": true})
                    return
                }
                msg_box.showWarn("提示", "", "请确认是否删除该用户")
            }
        }
        backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
        textColor: pressed ? "white" : "black"
    }
    UserManageRowEdit{
        id: insertedit
        z: 99
        visible: false
        onRetData: {
            if(tabmod.insertData(editdat) === false)
                warnmsg.showWarn("错误", "新建用户失败", "该用户ID已存在")
            else
                guiIntf.qmllog(homePage.user_id + "已添加新用户: " + editdat[0])
            tabmod.resetData()
        }
    }
    UserManageRowEdit{
        id: modifyedit
        z: 99
        visible: false
        onRetData: {
            if(tabmod.modifyData(["password", "name", "role", "userid"], [editdat[1], editdat[2], editdat[3], editdat[0]]) === true)
                guiIntf.qmllog(homePage.user_id + "已修改用户信息" + editdat[0])
            else
                warnmsg.showWarn("错误", "", "修改用户信息失败")
            tabmod.resetData()
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}D{i:2}D{i:1}D{i:13}D{i:14}
}
##^##*/
