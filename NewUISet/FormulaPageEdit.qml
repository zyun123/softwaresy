import QtQuick 2.0
import TaoQuick 1.0
import QtQuick.Controls 2.3

Rectangle {
    id: tplmenu
    width: 1920
    height: 1080
    color: "black"
    opacity: 0.8

    onVisibleChanged:
    {
        cusButton0.checked = false
        cusButton1.checked = false
        cusButton14.checked = false
        cusButton3.checked = false
        cusButton4.checked = false
        cusButton5.checked = false
        cusButton6.checked = false
        cusButton7.checked = false
        cusButton8.checked = false
        cusButton10.checked = false
        cusButton11.checked = false
        cusButton12.checked = false
        cusButton13.checked = false
        cusButton15.checked = false
        flag = false
    }

    signal updatelist()

    property bool flag: false

    MouseArea{
        anchors.fill: parent
        propagateComposedEvents: false
        hoverEnabled: true
        onClicked: {}

        onReleased: {}

        onPressed: {}
    }

    Rectangle {
        id: rectangle
        x: 541
        y: 173
        width: 660
        height: 384
        color: "#f2f2f2"
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        function getCheckedButtonsText(){
            const buttons = [cusButton0,cusButton1,cusButton3,
                             cusButton4,cusButton5,cusButton6,cusButton7,
                             cusButton8,cusButton10,cusButton11,
                             cusButton12,cusButton13,cusButton14,cusButton15]
            const texts=[]
            for(var i = 0;i<buttons.length;i++){
                if(buttons[i].checked===true){
                    texts.push(buttons[i].text)
                }
            }
            let txt = texts.join(", ")
            return txt
        }

        CusButton {
            id: cusButton0
            x: 47
            y: 36
            width: 100
            height: 33
            text: "手厥阴心包经"
            checked: false
            fontsize: 13
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton1
            x: 278
            y: 36
            width: 100
            height: 33
            text: "足太阴脾经"
            checked: false
            fontsize: 13
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton3
            x: 392
            y: 140
            width: 100
            height: 33
            visible: true
            text: "足少阳胆经"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton4
            x: 392
            y: 89
            width: 100
            height: 33
            visible: true
            text: "足厥阴肝经"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton5
            x: 163
            y: 140
            width: 100
            height: 33
            visible: true
            text: "手太阴肺经"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton6
            x: 163
            y: 36
            width: 100
            height: 33
            text: "足阳明胃经"
            checked: false
            fontsize: 13
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton7
            x: 392
            y: 36
            width: 100
            height: 33
            visible: true
            text: "手少阴心经"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton8
            x: 163
            y: 89
            width: 100
            height: 33
            text: "足太阳膀胱经"
            checked: false
            fontsize: 13
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton10
            x: 47
            y: 89
            width: 100
            height: 33
            text: "手少阳三焦经"
            checked: false
            fontsize: 13
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton11
            x: 47
            y: 140
            width: 100
            height: 33
            visible: true
            text: "手阳明大肠经"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton12
            x: 278
            y: 140
            width: 100
            height: 33
            visible: true
            text: "手太阳小肠经"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton13
            x: 278
            y: 89
            width: 100
            height: 33
            visible: true
            text: "足少阴肾经"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton14
            x: 508
            y: 36
            width: 100
            height: 33
            visible: true
            text: "任脉"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        CusButton {
            id: cusButton15
            x: 508
            y: 89
            width: 100
            height: 33
            visible: true
            text: "督脉"
            checked: false
            fontsize: 11
            checkable: true
            backgroundColor: checked ? "#6300bf" : "#d7d7d7"
            hoverEnabled: false
            textColor: enabled ? checked ? "white" : "black" : "#949494"
        }

        TextField {
            id: textField
            x: 447
            y: 210
            width: 194
            height: 35
            text: qsTr("")
            font.pointSize: 13
            placeholderText: "请输入方案名称"
            selectByMouse: true
        }

        CusButton {
            id: cusButton16
            x: 475
            y: 325
            width: 63
            height: 30
            text: "保存"
            fontsize: 13
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked:
            {
                var tpl_meridians = rectangle.getCheckedButtonsText()
                if(tpl_meridians !== "" && textField.text !== "" && textField1.text !== "")
                {
                    var tmpstr = textField.text
                    if(tmpstr.replace(/[^\x00-\xff]/g,'__').length > 20)
                    {
                        warnmsg.showWarn("提示", "新建方案错误", "方案名称过长")
                        return
                    }
                    var args = []
                    args.push(textField.text)
                    args.push(tpl_meridians)
                    args.push(textField1.text)
                    if(homePage.login_role == "super")
                        args.push("1")
                    else
                        args.push("2")
                    if(formulaTpl_mod.insertData(args) === false)
                        warnmsg.showWarn("错误", "", "保存调理方案失败，请确认方案名称是否已存在")
                    updatelist()
                    tplmenu.visible = false
                }
                else
                    warnmsg.showWarn("提示", "新建方案错误", "未选择经络")
            }
        }

        CusButton {
            id: cusButton17
            x: 568
            y: 325
            width: 63
            height: 30
            text: "取消"
            fontsize: 13
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked: tplmenu.visible = false
        }

        CusButton {
            id: cusButton18
            x: 52
            y: 325
            width: 91
            height: 30
            text: "全选/不选"
            fontsize: 13
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked: {
                if(flag === true)
                {
                    flag = false
                    cusButton0.checked = false
                    cusButton1.checked = false
                    cusButton14.checked = false
                    cusButton3.checked = false
                    cusButton4.checked = false
                    cusButton5.checked = false
                    cusButton6.checked = false
                    cusButton7.checked = false
                    cusButton8.checked = false
                    cusButton10.checked = false
                    cusButton11.checked = false
                    cusButton12.checked = false
                    cusButton13.checked = false
                    cusButton15.checked = false
                }
                else
                {
                    flag = true
                    cusButton0.checked = true
                    cusButton1.checked = true
                    cusButton14.checked =true
                    cusButton3.checked = true
                    cusButton4.checked = true
                    cusButton5.checked = true
                    cusButton6.checked = true
                    cusButton7.checked = true
                    cusButton8.checked = true
                    cusButton10.checked =true
                    cusButton11.checked =true
                    cusButton12.checked =true
                    cusButton13.checked =true
                    cusButton15.checked =true
                }
            }
        }

        TextField {
            id: textField1
            x: 447
            y: 258
            width: 194
            height: 35
            text: qsTr("")
            font.pointSize: 13
            selectByMouse: true
            placeholderText: "请输入配伍名称"
        }

        Text {
            id: text20
            x: 317
            y: 213
            color: "#000000"
            text: qsTr("    方案名称：")
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 16
            font.bold: false
        }

        Text {
            id: text21
            x: 317
            y: 261
            color: "#000000"
            text: qsTr("    经络配伍：")
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 16
            font.bold: false
        }
    }

}
