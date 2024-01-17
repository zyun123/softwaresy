import QtQuick 2.0
import QtQuick.Controls 2.3
import TaoQuick 1.0

Rectangle {
    id: rectangle1
    width: 1920
    height: 1080
    color: "black"
    opacity: 0.8

    signal retData1(var editdat)
    signal retData2(var editdat)

    function loaddat(rowdat)
    {
        text_name.text = rowdat["name"]
        text_id.text = rowdat["idcard"]
        comboBox.currentIndex = rowdat["gender"] === "男" ? 0 : 1
        text_age.text = rowdat["age"]
        text_phone.text = rowdat["phone"]
        text_note.text = rowdat["note"]
        text_phone.enabled = false
    }
    function cleardat()
    {
        text_name.text = ""
        text_id.text = ""
        text_age.text = ""
        text_phone.text = ""
        text_note.text = ""
        text_phone.enabled = true
    }

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
        x: 293
        y: 109
        width: 575
        height: 487
        color: "#f2f2f2"
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        TextField {
            id: text_name
            x: 134
            y: 19
            width: 350
            height: 50
            text: qsTr("")
            placeholderText: "请输入姓名"
            font.pointSize: 13
            selectByMouse: true
            KeyNavigation.tab: text_age
        }

        Text {
            id: text1
            x: 55
            y: 19
            width: 73
            height: 50
            text: qsTr("姓名")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        Text {
            id: text5
            x: 48
            y: 284
            width: 73
            height: 50
            text: qsTr("身份证号")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }
        TextField {
            id: text_id
            x: 134
            y: 284
            width: 350
            height: 50
            text: qsTr("")
            font.pointSize: 13
            placeholderText: "请输入身份证号"
            selectByMouse: true
            enabled: true
            validator: RegExpValidator{regExp: /\d{1,17}[0-9a-z]{0,1}/}
            onFocusChanged: if(focus === true) select(0, 0)
            KeyNavigation.tab: text_name
        }
        TextField {
            id: text_phone
            x: 134
            y: 85
            width: 350
            height: 50
            text: qsTr("")
            font.pointSize: 13
            placeholderText: "请输入手机号"
            selectByMouse: true
            validator: RegExpValidator{regExp: /\d{1,11}/}
            onFocusChanged: if(focus === true) select(0, 0)
            KeyNavigation.tab: text_note
        }

        Text {
            id: text3
            x: 48
            y: 85
            width: 73
            height: 50
            text: qsTr("手机号")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        TextField {
            id: text_note
            x: 134
            y: 350
            width: 350
            height: 50
            text: qsTr("")
            selectByMouse: true
            placeholderText: "请输入备注"
            font.pointSize: 13
        }

        Text {
            id: text4
            x: 55
            y: 350
            width: 73
            height: 50
            text: qsTr("备注")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        CusButton {
            id: button
            x: 379
            y: 430
            width: 80
            height: 35
            text: qsTr("确定")
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked: {
                if(text_name.text == "")
                {
                    tips.showTip({
                                      "message":"姓名不能为空！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                if(text_id.text == "")
                {
                    tips.showTip({
                                      "message":"身份证号不能为空！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                if(text_phone.text == "")
                {
                    tips.showTip({
                                      "message":"手机号不能为空！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                if(text_age.text == "")
                {
                    tips.showTip({
                                      "message":"年龄不能为空！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                //校验身份证号
                var tmpidcard = text_id.text
                if(tmpidcard.length === 18)
                {
                    for(var i = 0; i < 17; i++)
                    {
                        if(isNaN(tmpidcard[i]) === true)
                        {
                            warnmsg.showWarn("提示", "", "身份证号格式错误")
                            return
                        }
                    }
                }
                else
                {
                    warnmsg.showWarn("提示", "", "身份证号长度错误")
                    return
                }
                //校验手机号
                var tmpphone = text_phone.text
                if(tmpphone.length !== 11)
                {
                    warnmsg.showWarn("提示", "", "手机号长度错误")
                    return
                }
                var dat = []
                if(text_phone.enabled == true)
                {
                    dat.push(text_name.text)
                    dat.push(text_id.text)
                    dat.push(comboBox.currentIndex == 0 ? "男" : "女")
                    dat.push(text_age.text)
                    dat.push(text_phone.text)
                    dat.push(homePage.user_id)

                    const currentTime= new Date();
                    var currentYear = currentTime.getFullYear();
                    var currentMonth = ('0'+(currentTime.getMonth() + 1)).substr(-2);
                    var currentDay = ('0'+currentTime.getDate()).substr(-2);
                    var currentHours = ('0'+currentTime.getHours()).substr(-2);
                    var currentMinutes = ('0'+currentTime.getMinutes()).substr(-2);
                    var currentSeconds = ('0'+currentTime.getSeconds()).substr(-2);
                    var currentMillionSeconds = ('00'+currentTime.getMilliseconds()).substr(-3);
                    var time_str = currentYear + "." + currentMonth + "." + currentDay + " " +
                            currentHours + ":" + currentMinutes + ":" + currentSeconds

                    dat.push(time_str)
                    dat.push(text_note.text)
                    retData2(dat)
                }
                else
                {
                    dat.push(text_name.text)
                    dat.push(text_age.text)
                    dat.push(text_id.text)
                    dat.push(text_note.text)
                    dat.push(text_phone.text)
                    retData1(dat)
                }
                rectangle1.visible = false
            }
        }

        CusButton {
            id: button1
            x: 470
            y: 431
            width: 80
            height: 35
            text: qsTr("取消")
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked: rectangle1.visible = false
        }

        TextField {
            id: text_age
            x: 134
            y: 219
            width: 350
            height: 50
            text: qsTr("")
            placeholderText: "请输入年龄"
            selectByMouse: true
            font.pointSize: 13
            validator: RegExpValidator{regExp: /\d{1,2}/}
            onFocusChanged: if(focus === true) select(0, 0)
            KeyNavigation.tab: text_phone
        }

        Text {
            id: text6
            x: 48
            y: 219
            width: 73
            height: 50
            text: qsTr("年龄")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        ComboBox {
            id: comboBox
            x: 134
            y: 158
            model: ["男", "女"]
        }

        Text {
            id: text7
            x: 55
            y: 153
            width: 73
            height: 50
            text: "性别"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

    }
}

