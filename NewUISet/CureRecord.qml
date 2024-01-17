import QtQuick 2.12
import TaoQuick 1.0
import QtQuick.Controls 1.2
import QtQuick.Controls 2.12 as Qcontrol2
import QtQml 2.12
import CureRecordModel 1.0
import "./qml_basic"

Item {
    id: item1
    width: 1920
    height: 1080

    function clearPage()
    {
        textField.text = ""
    }

    onVisibleChanged: {
        if(visible === true)
            tabmod.resetData()
    }

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
            model: CureRecordModel {
                id: tabmod
            }
            onVisibleChanged: tabmod.resetData()
            col_width: [200, 150, 150, 400, 150, 200, 200, 150, 100, 100]
            rowHeight: 50
            TableViewColumn {
                width: 200
                title: "流水号"
                role: "serial_num"
            }
            TableViewColumn {
                width: 150
                title: "患者"
                role: "patient_name"
            }
            TableViewColumn {
                width: 150
                title: "操作员"
                role: "operator_id"
            }
            TableViewColumn {
                width: 400
                title: "调理经络"
                role: "meridians"
            }
            TableViewColumn {
                width: 150
                title: "调理方案"
                role: "formula_mod"
            }
            TableViewColumn {
                width: 200
                title: "开始时间"
                role: "start_time"
            }
            TableViewColumn {
                width: 200
                title: "结束时间"
                role: "end_time"
            }
            TableViewColumn {
                width: 150
                title: "调理时长"
                role: "dur_time"
            }
            TableViewColumn {
                width: 100
                title: "累计次数"
                role: "times"
            }
            TableViewColumn {
                width: 100
                title: "设备编号"
                role: "device_id"
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
        onClicked: tabmod.filter(textField.text, ["patient_name", "phone"])
        background: Rectangle{
            anchors.fill: parent
            radius: 5
            color: button1.pressed ? "#6300bf" : "#f2f2f2"
        }
    }
    Qcontrol2.Button{
        id: button5
        width: 70
        height: 30
        text: "重置"
        anchors.left: button1.right
        anchors.top: textField.top
        anchors.topMargin: 0
        anchors.leftMargin: 20
        onClicked: tabmod.resetData()
        background: Rectangle{
            anchors.fill: parent
            radius: 5
            color: button5.pressed ? "#6300bf" : "#f2f2f2"
        }
    }
}
