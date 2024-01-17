import QtQuick 2.0
import QtQuick.Controls 2.3 as Qcontrol2
import QtQuick.Controls 1.4
import "./qml_basic/"
import AppointmentListModel 1.0

Item {
    Rectangle {
        id: tabbg
        color: "#f5f6f7"
//        anchors.top: textField.bottom
        anchors.fill: parent
        anchors.rightMargin: 40
        anchors.leftMargin: 40
        anchors.bottomMargin: 40
        anchors.topMargin: 40
        MyTableView{
            id: tabview
            model: AppointmentListModel{
                id:tabmod
            }
            rowHeight: 70
            col_info: [7]
            btn_txt: "开始调理"
            sortIndicatorVisible: false
            onBtn_clicked: {
                var rowdat = tabmod.getRowDat(tabview.currentRow)
                homepage.loadMeridians(rowdat["meridians"], rowdat["patient_id"], rowdat["serial_num"], rowdat["name"])
                homepage.toCurePage()
            }
            Connections{
                target: guiIntf
                onUpdateAppList:{
                    tabmod.setModelDat(applist)
                }
            }

            TableViewColumn {
                width: 0
                role: "serial_num"
            }
            TableViewColumn {
                width: 0
                role: "patient_id"
            }
            TableViewColumn {
                width: 0
                role: "meridians"
            }
            TableViewColumn {
                width: 200
                title: "姓名"
                role: "name"
            }
            TableViewColumn {
                width: 200
                title: "联系方式"
                role: "phone"
            }
            TableViewColumn {
                width: 500
                title: "诊断"
                role: "diagnosis"
            }
            TableViewColumn {
                width: 200
                title: "预约时间"
                role: "time"
            }
            TableViewColumn {
                width: 200
                title: ""
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
