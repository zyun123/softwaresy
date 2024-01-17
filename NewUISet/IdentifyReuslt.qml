import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import TaoQuick 1.0
Item {
    id:patienttreat
    objectName: "idPage"
    anchors.fill: parent
    property string mydatastr: ""
    property int indexMess: 0

    property int censorCellHeight: 350
    property int rightCellWidth: 300
    property int leftcellOffset: 30
    property int topcellOffset: 27
    property int cellSpace: 15
    width: 1620
    height: 920

    Rectangle{
        id:patientscroll
        y:cellSpace
        color: "transparent"
        width:patienttreat.width-cellSpace
        height: patienttreat.height-cellSpace-topcellOffset
        x:leftcellOffset+cellSpace
        GridView {
            anchors.fill: patientscroll
            clip: true
            model:  ListModel {
                id: theModel
                ListElement {
                    eachname: "俯\n视\n机\n位\n\n\n\n\n\n\n\n"
                    eachimage:"qrc:/new/prefix1/Image/xxx.jpg"//加载图片资源
                    eachid:4
                }
                ListElement {
                    eachname: "头\n部\n机\n位\n\n\n\n\n\n\n\n"
                    eachimage:"qrc:/new/prefix1/Image/xxx.jpg"
                    eachid:7
                }
                ListElement {
                    eachname: "左\n侧\n机\n位\n\n\n\n\n\n\n\n"
                    eachimage:"qrc:/new/prefix1/Image/xxx.jpg"
                    eachid:5
                }
                ListElement {
                    eachname: "右\n侧\n机\n位\n\n\n\n\n\n\n\n"
                    eachimage:"qrc:/new/prefix1/Image/xxx.jpg"
                    eachid:6
                }
            }
            cellWidth: patientscroll.width/2
            cellHeight: patientscroll.height/2
            delegate: numberDelegate

            Component {
                id: numberDelegate

                Rectangle {
                    id: wrapper
                    width: patientscroll.width/2-15
                    height: patientscroll.height/2-15
                    //x:5
                    //                    anchors.topMargin:10
                    //                    anchors.leftMargin: 10
                    color: "lightGreen"
                    border.color: "black"
                    border.width: 1

                    Rectangle{
                        id:leftname
                        width: 25
                        height:wrapper.height
                        color: "#363636"
                        Text {
                            id:landscape_name
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 20
                            font.pixelSize: 15
                            text: eachname
                            color: "#E3E3E3"
                            font.bold: true  		//字体是否加粗，缺省为false
                            verticalAlignment: Text.AlignTop 	//垂直居中，控件必须有height才可以使用
                            horizontalAlignment: Text.AlignVCenter 	//水平居中，控件必须有width才可以使用
                            // rotation: -90
                        }
                    }
                    Rectangle{  //使图片边角有圆弧
                        id:maskrect
                        radius: 10
                        color: "lightgray"
                        anchors.top: parent.top
                        anchors.left: leftname.right
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 0
                        Image {
                            id:theimage
                            anchors.fill: parent
                            // source: eachimage
                            visible: true
                        }
                        Connections{
                            target: CodeImage

                            onCallQmlRefeshImg: {
                                // console.log("callonrefrsh11");
                                //theimage.source = ""
                                if ((eachid)==index){
                                    theimage.source = "image://CodeImg/"+index+ Math.random()
                                }


                            }
                        }
                        Rectangle{
                            id:themask
                            anchors.fill: parent
                            radius: 10
                            visible: false
                        }
                        MouseArea{
                            id: cvshow
                            anchors.fill: parent
                            onDoubleClicked: {
                                if(scalerect.visible == false && theimage.source != "")
                                {
                                    scaleImg.source = theimage.source
                                    scaleImg.scale = 1
                                    scaleImg.x = 0
                                    scaleImg.y = 0
                                    scalerect.visible = true
                                }
                            }
                        }
                    }
                    // radius: 10

                }
            }
        }
    }

    Rectangle{
        id: scalerect
        x:leftcellOffset+cellSpace
        y:cellSpace
        width:patienttreat.width-cellSpace-cellSpace
        height: patienttreat.height-cellSpace-topcellOffset-cellSpace
        visible: false
        color: "gray"
        z: 99
        clip: true
        Image {
            id: scaleImg
            x:0
            y:0
            width: patienttreat.width-cellSpace-cellSpace
            height: patienttreat.height-cellSpace-topcellOffset-cellSpace
        }
        MouseArea {
            id: msArea
            anchors.fill: parent
            drag.target: scaleImg
            enabled: scalerect.visible == true
            onWheel: {
                var delta = wheel.angleDelta.y / 120
                if (delta > 0)
                    scaleImg.scale = scaleImg.scale / 0.9
                else if(scaleImg.scale >= 1)
                    scaleImg.scale = scaleImg.scale * 0.9
            }
            onDoubleClicked: {
                scalerect.visible = false
            }
        }
    }

    onVisibleChanged: {
        if (visible){
            CodeImage.initData(4);
            CodeImage.initData(5);
            CodeImage.initData(6);
            CodeImage.initData(7);
            scalerect.visible = false
        }
    }

}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}
}
##^##*/
