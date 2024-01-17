import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import TaoQuick 1.0
import QtMultimedia 5.12

Item {
    id:patienttreatmanager
    width: 1920
    height: 1000

    property int rightCellWidth: 300
    property int leftcellOffset: 30
    property int topcellOffset: 27
    property int cellSpace: 15
    property int subPagIndex: 0
    property string musicname: ""
    property bool isrecognizesuccess: false
    property bool starttiaoli: false

    function toReconize(){
        if(isrecognizesuccess)
            subPagIndex = 1
    }

    function resetJL(){
        curelist.resetJingLuoStep()
    }

    function clearJL(){
        curelist.clearJingLuoStep()
    }

    function loadMeridians(raw, id, serial, name){
        curelist.loadMeridians(raw, id, serial, name)
    }

    Rectangle{
        id:background
        x: 20
        y:topcellOffset + 35
        width: 1570
        height: (patienttreatmanager.height-topcellOffset*2 - 35)
        color: "#f2f2f2"
    }
    Rectangle{
        id:treathead
        x: 20
        y:topcellOffset
        width: background.width
        height: 35
        color: "transparent"
        Row{
            CusButton {
                id: treathead1
                x:10
                width: 110
                height: treathead.height
                text: qsTr("实时画面")
                backgroundColorNormal:subPagIndex===0?"white":"#2b579a"
                backgroundColorPressed:subPagIndex===0?"#2b579a":"white"
                textColor:subPagIndex===0?"black":"white"
                hoverEnabled: false
                //selected:"true"
                onClicked: subPagIndex = 0
            }
            CusButton{
                id:treathead2
                x:10+110+2
                width: 110
                height: treathead.height
                text: qsTr("规划结果")
                backgroundColorNormal:subPagIndex===1?"white":"#2b579a"
                backgroundColorPressed:subPagIndex===1?"#2b579a":"white"
                textColor:subPagIndex===1?"black":"white"
                hoverEnabled: false
                onClicked: {
                    if(isrecognizesuccess)
                        subPagIndex = 1
                }
                visible: homePage.login_role != "operator"
            }
            CusButton{
                id:treathead3
                x:10+110+2+110+2
                width: 110
                height: treathead.height
                text: qsTr("动态识别")
                backgroundColorNormal:subPagIndex===2?"black":"#70c0cc"
                backgroundColorPressed:subPagIndex===2?"#70c0cc":"black"
                textColor:"white"
                hoverEnabled: false
                visible: false
                onClicked: {
                    if(isrecognizesuccess)
                        subPagIndex = 2
                }
            }
        }
    }
    Rectangle{
        id:realtimeCorrect//实时画面页面
        anchors.top: treathead.bottom
        anchors.bottom: parent.bottom
        color: "transparent"
        width: background.width
        height: background.height-treathead.height-cellSpace
        visible: subPagIndex == 0
        RTCamView{
            width: realtimeCorrect.width
            height: realtimeCorrect.height

        }
    }
    Rectangle{
        id:realtimeRecognize//整体识别画面
        anchors.top: treathead.bottom
        anchors.bottom: parent.bottom
        color: "transparent"
        width: background.width
        height: background.height-treathead.height-cellSpace
        visible: subPagIndex == 1
        IdentifyReuslt{
            width: realtimeRecognize.width
            height: realtimeRecognize.height

        }
    }
    Rectangle{
        id:curePtsPg//局部识别画面
        anchors.top: treathead.bottom
        anchors.bottom: parent.bottom
        color: "transparent"
        width: background.width
        height: background.height-treathead.height-cellSpace
        visible: subPagIndex == 2
        RTIdentifyResult{
            width: curePtsPg.width
            height: curePtsPg.height

        }
    }
    Rectangle{
        id:rightdisp
        x: 1600
        width: rightCellWidth
        anchors.top: background.top
        anchors.bottom: background.bottom
        color: "#f2f2f2"
        CureList{
            id: curelist
            anchors.fill: parent
        }

    }
    onVisibleChanged:{
        if (visible){
            subPagIndex = 0
        }
    }
}



/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}
}
##^##*/
