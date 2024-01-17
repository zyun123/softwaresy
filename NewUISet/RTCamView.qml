import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import TaoQuick 1.0
import QtMultimedia 5.12

Item {
    id:patienttreat
    anchors.fill: parent
    property bool qmlVisible: false
    property int censorCellHeight: 350
    property int rightCellWidth: 300
    property int leftcellOffset: 30
    property int topcellOffset: 27
    property int cellSpace: 15
    property var imgorder: ["m", "h", "l", "r"]
    width: 1620
    height: 920
    Rectangle{
        id:patientscroll
        y:cellSpace
        color: "white"
        width:patienttreat.width-cellSpace-15
        height: patienttreat.height-topcellOffset-30
        x:leftcellOffset+cellSpace
        Row{
            id: row
            spacing: 30
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            Rectangle{
                id:vid0_rect
                width: patientscroll.width/10*7
                height: vid0.width/16*9
                clip: true
                color: "transparent"
                VideoOutput{
                    id:vid0
                    width: patientscroll.width/10*7
                    height: vid0.width/16*9
                    source: fp_m
                }
                MouseArea{
                    anchors.fill: parent
                    drag.target: vid0
                    onWheel: {
                        var delta = wheel.angleDelta.y / 120
                        if (delta > 0)
                            vid0.scale = vid0.scale / 0.9
                        else if(vid0.scale >= 1)
                            vid0.scale = vid0.scale * 0.9
                    }
                    onDoubleClicked:{
                        vid0.width = vid0_rect.width
                        vid0.height = vid0_rect.height
                        vid0.x = 0
                        vid0.y = 0
                        vid0.scale = 1
                    }
                    onClicked: {
                        if(entrywnd.chkCali !== 0)
                        {
                            var args = {"func": "recordRedPoint",
                                "arg_obj": {"camid": imgorder[0], "pix_x": (mouseX - (-vid0.width * (vid0.scale - 1) / 2 + vid0.x)) / (vid0.width * vid0.scale) * 1280,
                                    "pix_y": (mouseY - (-vid0.height * (vid0.scale - 1) / 2 + vid0.y)) / (vid0.height * vid0.scale) * 720,
                                    "robotid": entrywnd.chkCali - 1, "preserve": textInput.text}}
                            guiIntf.guiCall(args)
                        }
                    }
                }
            }
            Rectangle{
                id: separater
                width: 1
                height: vid0.height + 150
                color: "black"

            }
            Column{
                id: column
                height: vid1.height*3+120
                spacing: 50

                VideoOutput {
                    id: vid1
                    width: patientscroll.width/10*2.5
                    height: width/16*9
                    source: fp_h
                    MouseArea{
                        id:area1
                        anchors.fill: parent
                        onDoubleClicked: {
                            var tmp = vid0.source
                            vid0.source = vid1.source
                            vid1.source = tmp
                            var tmp1 = imgorder[1]
                            imgorder[1] = imgorder[0]
                            imgorder[0] = tmp1
                        }
                    }
                }
                VideoOutput {
                    id: vid2
                    width: patientscroll.width/10*2.5
                    height: width/16*9
                    source: fp_l
                    MouseArea{
                        id:area2
                        anchors.fill: parent
                        onDoubleClicked: {
                            var tmp = vid0.source
                            vid0.source = vid2.source
                            vid2.source = tmp
                            var tmp1 = imgorder[2]
                            imgorder[2] = imgorder[0]
                            imgorder[0] = tmp1
                        }
                    }
                }
                VideoOutput {
                    id: vid3
                    width: patientscroll.width/10*2.5
                    height: width/16*9
                    source: fp_r
                    MouseArea{
                        id:area3
                        anchors.fill: parent
                        onDoubleClicked: {
                            var tmp = vid0.source
                            vid0.source = vid3.source
                            vid3.source = tmp
                            var tmp1 = imgorder[3]
                            imgorder[3] = imgorder[0]
                            imgorder[0] = tmp1
                        }
                    }
                }
            }
        }
    }

    TextInput {
        id: textInput
        x: 36
        y: 884
        width: 144
        height: 28
        text: qsTr("0.02")
        font.pointSize: 13
        selectByMouse: true
        visible: homePage.login_role === "super"
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}
}
##^##*/
