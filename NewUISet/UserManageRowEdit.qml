import QtQuick 2.0
import QtQuick.Controls 2.3
import TaoQuick 1.0

Rectangle {
    id: rectangle1
    width: 1920
    height: 1080
    color: "black"
    opacity: 0.8

    signal retData(var editdat)

    function loaddat(rowdat)
    {
        textField.text = rowdat["userid"]
        textField1.text = rowdat["password"]
        textField2.text = rowdat["name"]
        if(rowdat["role"] === "operator")
            comboBox.currentIndex = 0
        else if(rowdat["role"] === "doctor")
            comboBox.currentIndex = 1
        else if(rowdat["role"] === "admin")
            comboBox.currentIndex = 2
        else if(rowdat["role"] === "root")
            comboBox.currentIndex = 3
        else
            comboBox.currentIndex = 4
        textField.enabled = false
        comboBox.enabled = false
    }
    function cleardat()
    {
        textField.text = ""
        textField1.text = ""
        textField2.text = ""
        textField3.text = ""
        comboBox.currentIndex = 0
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
        width: 585
        height: 377
        color: "#f2f2f2"
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        TextField {
            id: textField
            x: 134
            y: 48
            width: 256
            height: 50
            text: qsTr("")
            placeholderText: "请输入用户名"
            font.pointSize: 13
            selectByMouse: true
        }

        Text {
            id: text1
            x: 48
            y: 48
            width: 73
            height: 50
            text: qsTr("用户名")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        TextField {
            id: textField1
            x: 134
            y: 111
            width: 256
            height: 50
            text: qsTr("")
            font.pointSize: 13
            placeholderText: "请输入密码"
            selectByMouse: true
            echoMode: "Password"
        }

        Text {
            id: text2
            x: 48
            y: 111
            width: 73
            height: 50
            text: "密码"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        TextField {
            id: textField2
            x: 134
            y: 245
            width: 256
            height: 50
            visible: false
            text: qsTr("nouse")
            font.pointSize: 13
            placeholderText: "请输入昵称"
            selectByMouse: true
        }

        Text {
            id: text3
            x: 48
            y: 245
            width: 73
            height: 50
            visible: false
            text: qsTr("昵称")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        Text {
            id: text4
            x: 48
            y: 240
            width: 73
            height: 50
            text: qsTr("账号权限")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 13
        }

        ComboBox {
            id: comboBox
            x: 134
            y: 245
            width: 114
            height: 40
            font.pointSize: 13
            model: ["操作员","医生","管理员","运行维护"]
            background: Rectangle{
                anchors.fill: parent
                color: "white"
                radius: 5
            }
        }

        CusButton {
            id: button
            x: 358
            y: 319
            width: 80
            height: 35
            text: qsTr("确定")
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked: {
                if(textField.text == "")
                {
                    tips.showTip({
                                      "message":"用户名不能为空！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                if(textField1.text == "")
                {
                    tips.showTip({
                                      "message":"密码不能为空！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                if(textField3.text == "")
                {
                    tips.showTip({
                                      "message":"密码确认不能为空！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                if(textField1.text != textField3.text)
                {
                    tips.showTip({
                                      "message":"密码输入不一致！",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                //校验id长度
                var tmpid = textField.text
                if(tmpid.replace(/[^\x00-\xff]/g,'__').length > 15)
                {
                    tips.showTip({
                                      "message":"用户名长度应为1~15个字符",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                //校验密码长度
                var tmppwd = textField1.text
                if(tmppwd.length > 15 || tmppwd.length < 5)
                {
                    tips.showTip({
                                      "message":"密码长度应为5~15个字符",
                                      "type":'info',
                                      "show":true
                                  }
                                 )
                    return
                }
                var dat = []
                dat.push(textField.text)
                dat.push(textField1.text)
                dat.push("nouse")
                if(comboBox.currentText == "操作员")
                    dat.push("operator")
                else if(comboBox.currentText == "医生")
                    dat.push("doctor")
                else if(comboBox.currentText == "管理员")
                    dat.push("admin")
                else
                    dat.push("root")
                dat.push(1)
                dat.push(1)
                dat.push(1)
                retData(dat)
                rectangle1.visible = false
            }
        }

        CusButton {
            id: button1
            x: 467
            y: 319
            width: 80
            height: 35
            text: qsTr("取消")
            backgroundColor: pressed ? "#6300bf" : "#d7d7d7"
            textColor: pressed ? "white" : "black"
            onClicked: rectangle1.visible = false
        }

        TextField {
            id: textField3
            x: 134
            y: 178
            width: 256
            height: 50
            text: qsTr("")
            font.pointSize: 13
            echoMode: "Password"
            placeholderText: "请再次输入密码"
            selectByMouse: true
        }
    }
}


