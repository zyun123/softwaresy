import QtQuick 2.0
import TaoQuick 1.0

Item {
    id: jmenu
    width: 1280
    height: 720

    property int ver: 0

    function switchVer(v){
        ver = v
        if(v === 0)
        {
            cusButton12.enabled = true
            cusButton19.enabled = true
            cusButton20.enabled = true
            cusButton21.enabled = true
        }
        else
        {
            cusButton12.enabled = false
            cusButton19.enabled = false
            cusButton20.enabled = false
            cusButton21.enabled = false
        }
    }

    onVisibleChanged: {
        cusButton.checked = false
        cusButton2.checked = false
        cusButton3.checked = false
        cusButton4.checked = false
        cusButton5.checked = false
        cusButton6.checked = false
        cusButton7.checked = false
        cusButton8.checked = false
        cusButton9.checked = false
        cusButton12.checked = false
        cusButton13.checked = false
        cusButton14.checked = false
        cusButton15.checked = false
        cusButton16.checked = false
        cusButton17.checked = false
        cusButton18.checked = false
        cusButton19.checked = false
        cusButton20.checked = false
        cusButton21.checked = false
        cusButton22.checked = false
        cusButton23.checked = false
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }
    Rectangle {
        id: rectangle
        color: "#000000"
        anchors.fill: parent
        opacity: 0.8

        Rectangle {
            id: rectangle1
            x: 469
            y: 247
            width: 484
            height: 356
            color: "#ffffff"
            radius: 5
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            CusButton {
                id: cusButton
                x: 48
                y: 34
                width: 90
                height: 33
                text: "手厥阴心包经"
                fontsize: 11
                hoverEnabled: false
                checked: true
                checkable: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
            }

            CusButton {
                id: cusButton1
                x: 354
                y: 299
                width: 84
                height: 33
                text: "确定"
                fontsize: 11
                hoverEnabled: false
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    rightcontrol.set_dict_checkstate(cusButton.text, cusButton.checked)
                    rightcontrol.set_dict_checkstate(cusButton2.text, cusButton2.checked)
                    rightcontrol.set_dict_checkstate(cusButton3.text, cusButton3.checked)
                    rightcontrol.set_dict_checkstate(cusButton4.text, cusButton4.checked)
                    rightcontrol.set_dict_checkstate(cusButton5.text, cusButton5.checked)
                    rightcontrol.set_dict_checkstate(cusButton6.text, cusButton6.checked)
                    rightcontrol.set_dict_checkstate(cusButton7.text, cusButton7.checked)
                    rightcontrol.set_dict_checkstate(cusButton8.text, cusButton8.checked)
                    rightcontrol.set_dict_checkstate(cusButton9.text, cusButton9.checked)
                    rightcontrol.set_dict_checkstate(cusButton12.text, cusButton12.checked)
                    rightcontrol.set_dict_checkstate(cusButton13.text, cusButton13.checked)
                    rightcontrol.set_dict_checkstate(cusButton14.text, cusButton14.checked)
                    rightcontrol.set_dict_checkstate(cusButton15.text, cusButton15.checked)
                    rightcontrol.set_dict_checkstate(cusButton16.text, cusButton16.checked)
                    rightcontrol.set_dict_checkstate(cusButton17.text, cusButton17.checked)
                    rightcontrol.set_dict_checkstate(cusButton18.text, cusButton18.checked)
                    rightcontrol.set_dict_checkstate(cusButton19.text, cusButton19.checked)
                    rightcontrol.set_dict_checkstate(cusButton20.text, cusButton20.checked)
                    rightcontrol.set_dict_checkstate(cusButton21.text, cusButton21.checked)
                    rightcontrol.set_dict_checkstate(cusButton22.text, cusButton22.checked)
                    rightcontrol.set_dict_checkstate(cusButton23.text, cusButton23.checked)

                    rightcontrol.resetJingLuoStep()
                    jmenu.visible = false
                }
            }

            CusButton {
                id: cusButton2
                x: 145
                y: 34
                width: 90
                height: 33
                text: "足阳明胃经2"
                checked: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton3
                x: 244
                y: 34
                width: 90
                height: 33
                text: "足太阴脾经1"
                checked: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton4
                x: 340
                y: 34
                width: 90
                height: 33
                text: "手少阳三焦经"
                checked: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton5
                x: 49
                y: 73
                width: 90
                height: 33
                text: "足太阳膀胱经1"
                checked: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton6
                x: 146
                y: 73
                width: 90
                height: 33
                text: "足太阳膀胱经2"
                checked: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton7
                x: 244
                y: 73
                width: 90
                height: 33
                text: "手太阴肺经"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton8
                x: 342
                y: 73
                width: 90
                height: 33
                text: "足厥阴肝经1"
                checked: false
                fontsize: 11
                hoverEnabled: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
            }

            CusButton {
                id: cusButton9
                x: 50
                y: 112
                width: 90
                height: 33
                text: "足少阳胆经2"
                fontsize: 11
                checkable: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                checked: false
            }

            CusButton {
                id: cusButton10
                x: 49
                y: 299
                width: 84
                height: 33
                text: "全选"
                hoverEnabled: false
                fontsize: 11
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    cusButton.checked = true
                    cusButton2.checked = true
                    cusButton3.checked = true
                    cusButton4.checked = true
                    cusButton5.checked = true
                    cusButton6.checked = true
                    cusButton7.checked = true
                    cusButton8.checked = true
                    cusButton9.checked = true
                    cusButton13.checked = true
                    cusButton14.checked = true
                    cusButton15.checked = true
                    cusButton16.checked = true
                    cusButton17.checked = true
                    cusButton18.checked = true
                    cusButton22.checked = true
                    cusButton23.checked = true
                    if(ver === 0)
                    {
                        cusButton12.checked = true
                        cusButton19.checked = true
                        cusButton20.checked = true
                        cusButton21.checked = true
                    }
                }
            }

            CusButton {
                id: cusButton11
                x: 139
                y: 299
                width: 84
                height: 33
                text: "全不选"
                hoverEnabled: false
                fontsize: 11
                backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
                textColor: pressed ? "white" : "black"
                onClicked: {
                    cusButton.checked = false
                    cusButton2.checked = false
                    cusButton3.checked = false
                    cusButton4.checked = false
                    cusButton5.checked = false
                    cusButton6.checked = false
                    cusButton7.checked = false
                    cusButton8.checked = false
                    cusButton9.checked = false
                    cusButton12.checked = false
                    cusButton13.checked = false
                    cusButton14.checked = false
                    cusButton15.checked = false
                    cusButton16.checked = false
                    cusButton17.checked = false
                    cusButton18.checked = false
                    cusButton19.checked = false
                    cusButton20.checked = false
                    cusButton21.checked = false
                    cusButton22.checked = false
                    cusButton23.checked = false
                }
            }

            CusButton {
                id: cusButton12
                x: 146
                y: 112
                width: 90
                height: 33
                text: "足少阳胆经1"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton13
                x: 244
                y: 112
                width: 90
                height: 33
                text: "足少阴肾经1"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton14
                x: 342
                y: 112
                width: 90
                height: 33
                text: "足少阴肾经2"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton15
                x: 50
                y: 151
                width: 90
                height: 33
                text: "手阳明大肠经"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                checkable: true
                hoverEnabled: false
                fontsize: 11
            }

            CusButton {
                id: cusButton16
                x: 146
                y: 151
                width: 90
                height: 33
                text: "手太阳小肠经"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                checkable: true
                fontsize: 11
            }

            CusButton {
                id: cusButton17
                x: 244
                y: 151
                width: 90
                height: 33
                text: "手少阴心经"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                checkable: true
                fontsize: 11
            }

            CusButton {
                id: cusButton18
                x: 342
                y: 151
                width: 90
                height: 33
                text: "足太阴脾经2"
                checked: false
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                checkable: true
                fontsize: 11
            }

            CusButton {
                id: cusButton19
                x: 50
                y: 190
                width: 90
                height: 33
                text: "足阳明胃经1"
                checkable: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                fontsize: 11
                checked: false
            }

            CusButton {
                id: cusButton20
                x: 146
                y: 190
                width: 90
                height: 33
                text: "足阳明胃经3"
                checkable: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                fontsize: 11
                checked: false
            }

            CusButton {
                id: cusButton21
                x: 244
                y: 190
                width: 90
                height: 33
                text: "足厥阴肝经2"
                checkable: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                fontsize: 11
                checked: false
            }

            CusButton {
                id: cusButton22
                x: 342
                y: 190
                width: 90
                height: 33
                text: "任脉"
                checkable: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                fontsize: 11
                checked: false
            }

            CusButton {
                id: cusButton23
                x: 50
                y: 229
                width: 90
                height: 33
                text: "督脉"
                checkable: true
                backgroundColor: checked ? "#6300bf" : "#d7d7d7"
                textColor: checked ? "white" : "black"
                hoverEnabled: false
                fontsize: 11
                checked: false
            }
        }
    }

}
