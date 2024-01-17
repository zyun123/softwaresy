import QtQuick 2.0
import TaoQuick 1.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.0

Rectangle {
    id: infoPage
    width: 1920
    height: 1000
    color: "transparent"

    property int curPageIndex: 0

    function clearPage()
    {
        patientManage.clearPage()
        cureRecord.clearPage()
    }


    Rectangle{
        id:devicemanagerrect
        height: 40
        color: "transparent"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.rightMargin: 30
        anchors.leftMargin: 30

        RowLayout {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            spacing: 0
            anchors.bottomMargin: 0
            anchors.topMargin: 0
            anchors.leftMargin: 0

            CusTextButton{
                id:devicemanagerheader0
                font.pointSize:18
                text:"患者信息"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 126
                textColor: selected ? "black" : "white"
                backgroundColorNormal:"#2b579a"
                backgroundColorPressed:"#ffffff"
                selected:curPageIndex==0
                onClicked:{
                    curPageIndex=0
                }
            }
            CusTextButton {
                id: devicemanagerheader1
                text: "调理记录"
                Layout.preferredHeight: 40
                textColor: selected ? "black" : "white"
                backgroundColorNormal:"#2b579a"
                backgroundColorPressed:"#ffffff"
                Layout.preferredWidth: 126
                font.pointSize: 18
                selected: curPageIndex==1
                onClicked:{
                    curPageIndex=1
                }
            }
        }
    }

    onVisibleChanged: curPageIndex=0

    PatientManage {
        id: patientManage
        visible: curPageIndex == 0
        anchors.top: devicemanagerrect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    CureRecord {
        id: cureRecord
        visible: curPageIndex == 1
        anchors.top: devicemanagerrect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

    }

}
