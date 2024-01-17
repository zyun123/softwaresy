import QtQuick 2.0
import TaoQuick 1.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.0

Rectangle {
    id:devpage
    width: 1920
    height: 1000
    color:"#F2F2F2"

    property int curPageIndex: 0

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
                text:"左机械臂"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 126
                textColor:"white"
                backgroundColorNormal:"#78C8E2"
                backgroundColorPressed:"#007393"
                selected:curPageIndex==0
                onClicked:{
                    curPageIndex=0
                }
            }

            CusTextButton{
                id:devicemanagerheader1
                font.pointSize:18
                text:"右机械臂"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 126
                textColor:"white"
                backgroundColorNormal:"#78C8E2"
                backgroundColorPressed:"#007393"
                selected:curPageIndex==1
                onClicked:{
                    curPageIndex=1
                }
            }

            CusTextButton{
                id:devicemanagerheader2
                font.pointSize:18
                text:"调理床"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 126
                textColor:"white"
                backgroundColorNormal:"#78C8E2"
                backgroundColorPressed:"#007393"
                selected:curPageIndex==2
                onClicked:{
                    curPageIndex=2
                }
            }

            CusTextButton{
                id:devicemanagerheader3
                font.pointSize:18
                text:"相机"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 126
                textColor:"white"
                backgroundColorNormal:"#78C8E2"
                backgroundColorPressed:"#007393"
                selected:curPageIndex==3
                onClicked:{
                    curPageIndex=3
                }
            }

            CusTextButton {
                id: devicemanagerheader4
                text: "其它"
                Layout.preferredHeight: 40
                backgroundColorPressed: "#007393"
                textColor: "#ffffff"
                backgroundColorNormal: "#78c8e2"
                Layout.preferredWidth: 126
                font.pointSize: 18
                selected: curPageIndex==4
                onClicked:{
                    curPageIndex=4
                }
            }
        }
    }
    onVisibleChanged: curPageIndex=0

    Robotctl {
        id: leftrbPage
        rbindex: "0"
        visible: curPageIndex == 0
        anchors.top: devicemanagerrect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    Robotctl {
        id: rightrbPage
        rbindex: "1"
        visible: curPageIndex == 1
        anchors.top: devicemanagerrect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    Bedctl {
        id: bedPage
        visible: curPageIndex == 2
        anchors.top: devicemanagerrect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    DevOthers {
        id: otherPage
        visible: curPageIndex == 4
        anchors.top: devicemanagerrect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}D{i:2}D{i:1}
}
##^##*/
