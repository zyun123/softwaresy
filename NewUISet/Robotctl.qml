import QtQuick 2.0
import QtQuick.Window 2.12
import TaoQuick 1.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.5

Item {
    id: robotctl
    x: 10
    y: 10
    width: 1600
    height: 900

    property string rbindex: "0"
    property string pgcolor: rbindex == "0" ? "#86d2d9" : "#bef3f8"

    Connections{
        target: guiIntf
        onUpdateRbPos1:{
            if(rbindex == "0" && index == 0 || rbindex == "1" && index == 1)
                textInput37.text = msg
        }
        onUpdateRbPos2:{
            if(rbindex == "0" && index == 0 || rbindex == "1" && index == 1)
                textInput38.text = msg
        }
        onUpdateRbStatus:{
            if(rbindex == "0" && index == 0 || rbindex == "1" && index == 1)
            {
                textInput34.text = info["tcpforce"]
                textInput35.text = info["programSta"]
                textInput36.text = info["robotmode"]
            }
        }
    }

    Rectangle{
        color: pgcolor
        radius: 5
        anchors.fill: parent
        anchors.rightMargin: 60
        anchors.leftMargin: 30
        anchors.bottomMargin: 55
        anchors.topMargin: 55

        Rectangle {
            id: rectangle
            x: 50
            y: 46
            width: 355
            height: 127
            color: pgcolor
            border.width: 2

            CusButton {
                id: cusButton2
                x: 9
                y: 29
                width: 87
                height: 37
                text: "开自由驱动"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "freeDrive",
                                "arg_str": rbindex + "|true"}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton3
                x: 195
                y: 29
                width: 71
                height: 37
                text: "开震动"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "vibon",
                                "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton4
                x: 272
                y: 29
                width: 71
                height: 37
                text: "关震动"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "viboff",
                                "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton5
                x: 209
                y: 72
                width: 102
                height: 37
                text: "切换缩减模式"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "reduce",
                                "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton6
                x: 9
                y: 72
                width: 94
                height: 37
                text: "结束力控"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "endForce",
                                "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton7
                x: 109
                y: 72
                width: 94
                height: 37
                text: "力控清零"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "clearForce",
                        "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton21
                x: 102
                y: 29
                width: 87
                height: 37
                text: "关自由驱动"
                backgroundColor: "#ffffff"
                Layout.preferredWidth: 70
                Layout.preferredHeight: 40
                onClicked: {
                    var args = {"func": "freeDrive",
                                "arg_str": rbindex + "|false"}
                    guiIntf.guiCall(args)
                }
            }
        }

        Rectangle {
            id: rectangle1
            x: 42
            y: 31
            width: 58
            height: 32
            color: pgcolor

            Text {
                id: text1
                x: 23
                y: 16
                width: 64
                height: 38
                text: qsTr("功能控制")
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: parent.horizontalCenter
                clip: false
            }
        }

        Rectangle {
            id: rectangle20
            x: 50
            y: 198
            width: 785
            height: 258
            color: pgcolor
            border.width: 2

            Rectangle {
                id: rectangle8
                x: 34
                y: 40
                width: 291
                height: 145
                color: pgcolor
                border.width: 2

                Rectangle {
                    id: rectangle2
                    x: 33
                    y: 18
                    width: 237
                    height: 29
                    color: "#ffffff"
                    border.width: 2
                    TextInput {
                        id: textInput
                        text: qsTr("0,0,0,0,0,0")
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        anchors.leftMargin: 5
                        font.pointSize: 11
                        mouseSelectionMode: TextInput.SelectCharacters
                        selectByMouse: true
                    }
                }

                Text {
                    id: text17
                    x: 178
                    y: 62
                    width: 27
                    height: 21
                    text: qsTr("加速")
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                }

                Rectangle {
                    id: rectangle19
                    x: 210
                    y: 58
                    width: 60
                    height: 29
                    color: "#ffffff"
                    border.width: 2
                    TextInput {
                        id: textInput13
                        text: qsTr("0.1")
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: 11
                        mouseSelectionMode: TextInput.SelectCharacters
                        selectByMouse: true
                    }
                }

                Text {
                    id: text16
                    x: 33
                    y: 62
                    width: 27
                    height: 21
                    text: "移速"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                }

                Rectangle {
                    id: rectangle18
                    x: 66
                    y: 58
                    width: 60
                    height: 29
                    color: "#ffffff"
                    border.width: 2
                    TextInput {
                        id: textInput12
                        text: qsTr("0.1")
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: 11
                        mouseSelectionMode: TextInput.SelectCharacters
                        selectByMouse: true
                    }
                }

                Text {
                    id: text29
                    x: 8
                    y: 97
                    width: 52
                    height: 21
                    text: "返回时间"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                }

                Rectangle {
                    id: rectangle37
                    x: 66
                    y: 93
                    width: 60
                    height: 29
                    color: "#ffffff"
                    border.width: 2
                    TextInput {
                        id: textInput23
                        text: qsTr("0")
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: 11
                        mouseSelectionMode: TextInput.SelectCharacters
                        selectByMouse: true
                    }
                }

                Text {
                    id: text30
                    x: 163
                    y: 97
                    width: 27
                    height: 21
                    text: "交融半径"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                }

                Rectangle {
                    id: rectangle38
                    x: 210
                    y: 93
                    width: 60
                    height: 29
                    color: "#ffffff"
                    border.width: 2
                    TextInput {
                        id: textInput24
                        text: qsTr("0")
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: 11
                        mouseSelectionMode: TextInput.SelectCharacters
                        selectByMouse: true
                    }
                }
            }

            Rectangle {
                id: rectangle9
                x: 15
                y: 26
                width: 58
                height: 32
                color: pgcolor
                Text {
                    id: text8
                    x: 23
                    y: 16
                    width: 64
                    height: 38
                    text: "参数"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    clip: false
                }
            }

            CusButton {
                id: cusButton8
                x: 336
                y: 83
                width: 94
                height: 37
                text: "movej(位姿)"
                Layout.preferredHeight: 40
                backgroundColor: "#ffffff"
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "movej",
                                "arg_str": rbindex + "|1|" + textInput.text + "," +
                                           textInput13.text + "," + textInput12.text + "," + textInput23.text + "," + textInput24.text}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton9
                x: 436
                y: 83
                width: 60
                height: 37
                text: "movel"
                Layout.preferredHeight: 40
                backgroundColor: "#ffffff"
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "movel",
                                "arg_str": rbindex + "|" + textInput.text + "," +
                                           textInput13.text + "," + textInput12.text + "," + textInput23.text + "," + textInput24.text}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton10
                x: 336
                y: 40
                width: 113
                height: 37
                text: "movej(关节角)"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                onClicked: {
                    var args = {"func": "movej",
                                "arg_str": rbindex + "|2|" + textInput.text + "," +
                                           textInput13.text + "," + textInput12.text + "," + textInput23.text + "," + textInput24.text}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton17
                x: 34
                y: 201
                width: 94
                height: 37
                text: "安全位置"
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                onClicked: {
                    var args = {"func": "safePos",
                        "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton19
                x: 140
                y: 201
                width: 94
                height: 37
                text: "识别位置"
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                onClicked: {
                    var args = {"func": "photoPos",
                        "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
            }

            Rectangle {
                id: rectangle36
                x: 547
                y: 205
                width: 101
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput22
                    text: qsTr("0,0,0")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    mouseSelectionMode: TextInput.SelectCharacters
                    font.pointSize: 11
                    selectByMouse: true
                }
            }

            CusButton {
                id: cusButton20
                x: 665
                y: 201
                width: 101
                height: 37
                text: "欧拉角姿态"
                backgroundColor: "#ffffff"
                Layout.preferredWidth: 70
                Layout.preferredHeight: 40
                onClicked: {
                    var args = {"func": "goToEulerAngle",
                        "arg_str": rbindex + "|" + textInput22.text}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton22
                x: 402
                y: 126
                width: 60
                height: 37
                text: "speedl"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "speedl",
                                "arg_str": rbindex + "|" + textInput.text + "," +
                                           textInput13.text + "," + textInput12.text + "," + textInput23.text + "," + textInput24.text}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton23
                x: 336
                y: 126
                width: 60
                height: 37
                text: "speedj"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "speedj",
                                "arg_str": rbindex + "|" + textInput.text + "," +
                                           textInput13.text + "," + textInput12.text + "," + textInput23.text + "," + textInput24.text}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton24
                x: 336
                y: 169
                width: 60
                height: 37
                text: "stopl"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "stopl",
                        "arg_str": rbindex + "|" + textInput13.text}
                    guiIntf.guiCall(args)
                }
            }

            CusButton {
                id: cusButton25
                x: 402
                y: 169
                width: 60
                height: 37
                text: "stopj"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "stopj",
                        "arg_str": rbindex + "|" + textInput13.text}
                    guiIntf.guiCall(args)
                }
            }

            Rectangle {
                id: rectangle31
                x: 616
                y: 40
                width: 150
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput21
                    text: qsTr("0,0,-0.05,0,0,0")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                    font.pointSize: 11
                    anchors.leftMargin: 5
                }
            }

            Text {
                id: text27
                x: 583
                y: 44
                width: 27
                height: 21
                text: "方向"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Text {
                id: text31
                x: 584
                y: 79
                width: 27
                height: 21
                text: qsTr("加速")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle32
                x: 616
                y: 75
                width: 60
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput25
                    text: qsTr("0.1")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                    font.pointSize: 11
                }
            }

            Text {
                id: text32
                x: 584
                y: 114
                width: 27
                height: 21
                text: qsTr("减速")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle39
                x: 616
                y: 110
                width: 60
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput26
                    text: qsTr("1")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                    font.pointSize: 11
                }
            }

            CusButton {
                id: cusButton12
                x: 696
                y: 106
                width: 70
                height: 37
                text: "contact"
                Layout.preferredWidth: 70
                Layout.preferredHeight: 40
                backgroundColor: "#ffffff"
                onClicked: {
                    var args = {"func": "contact",
                        "arg_str": rbindex + "|" + textInput21.text + "," + textInput25.text + "," + textInput26.text}
                    guiIntf.guiCall(args)
                }
            }
        }

        Rectangle {
            id: rectangle21
            x: 42
            y: 185
            width: 58
            height: 32
            color: pgcolor
            Text {
                id: text18
                x: 23
                y: 16
                width: 64
                height: 38
                text: "移动指令"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: parent.horizontalCenter
                clip: false
            }
        }

        Rectangle {
            id: rectangle29
            x: 50
            y: 479
            width: 785
            height: 190
            color: pgcolor
            border.width: 2

            Text {
                id: text21
                x: 43
                y: 33
                width: 57
                height: 21
                text: "力控类型"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle24
                x: 106
                y: 29
                width: 87
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput16
                    text: qsTr("2")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                }
            }

            Text {
                id: text22
                x: 29
                y: 96
                width: 57
                height: 21
                text: "力控大小"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle25
                x: 92
                y: 92
                width: 200
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput17
                    text: qsTr("0,0,-3,0,0,0")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    anchors.leftMargin: 5
                    font.pointSize: 11
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                }
            }

            Text {
                id: text24
                x: 8
                y: 149
                width: 81
                height: 21
                text: "坐标系偏移"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle27
                x: 92
                y: 145
                width: 200
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput19
                    text: qsTr("0,0,0,0,0,0")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    anchors.leftMargin: 5
                    font.pointSize: 11
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                }
            }

            Text {
                id: text23
                x: 303
                y: 96
                width: 57
                height: 21
                text: "力控方向"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle26
                x: 366
                y: 92
                width: 200
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput18
                    text: qsTr("0,0,1,0,0,0")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    anchors.leftMargin: 5
                    font.pointSize: 11
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                }
            }

            Text {
                id: text25
                x: 303
                y: 149
                width: 57
                height: 21
                text: "寻力速度"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle28
                x: 366
                y: 145
                width: 200
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput20
                    text: qsTr("0.05,0.1,0.1,0.05,0.1,0.1")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    anchors.leftMargin: 5
                    font.pointSize: 11
                    mouseSelectionMode: TextInput.SelectCharacters
                    selectByMouse: true
                }
            }

            CusButton {
                id: cusButton11
                x: 215
                y: 25
                width: 77
                height: 37
                text: "开启力控"
                Layout.preferredHeight: 40
                backgroundColor: "#ffffff"
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "rawCommand",
                                "arg_str": rbindex + "|2,3," + textInput19.text + "," +
                                           textInput18.text + "," + textInput17.text + "," + textInput16.text + "," + textInput20.text + ",0"}
                    guiIntf.guiCall(args)
                }
            }

            Text {
                id: text19
                x: 398
                y: 33
                width: 27
                height: 21
                text: "阻尼"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle22
                x: 431
                y: 29
                width: 87
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput14
                    text: qsTr("0")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                    selectByMouse: true
                    mouseSelectionMode: TextInput.SelectCharacters
                }
            }

            Text {
                id: text20
                x: 529
                y: 33
                width: 27
                height: 21
                text: "增益"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 11
            }

            Rectangle {
                id: rectangle23
                x: 562
                y: 29
                width: 87
                height: 29
                color: "#ffffff"
                border.width: 2
                TextInput {
                    id: textInput15
                    text: qsTr("1")
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                    selectByMouse: true
                    mouseSelectionMode: TextInput.SelectCharacters
                }
            }

            CusButton {
                id: cusButton28
                x: 655
                y: 27
                width: 105
                height: 33
                text: "设置阻尼增益"
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 70
                onClicked: {
                    var args = {"func": "setDampAndGain",
                        "arg_str": rbindex + "|" + textInput14.text + "," + textInput15.text}
                    guiIntf.guiCall(args)
                }
            }
        }

        Rectangle {
            id: rectangle30
            x: 43
            y: 462
            width: 58
            height: 32
            color: pgcolor
            Text {
                id: text26
                x: 23
                y: 16
                width: 64
                height: 38
                text: "力控指令"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: parent.horizontalCenter
                clip: false
            }
        }

        Rectangle {
            id: rectangle33
            x: 916
            y: 11
            width: 6
            height: 720
            color: "#26282a"
        }

        CusButton {
            id: cusButton18
            x: 1405
            y: 46
            width: 94
            height: 37
            text: "刷新"
            backgroundColor: "#ffffff"
            Layout.preferredHeight: 40
            Layout.preferredWidth: 70
            onClicked: {
                var args = {"func": "onPanelRBStateRefresh",
                            "arg_str": rbindex}
                guiIntf.guiCall(args)
            }
        }

        Text {
            id: text44
            x: 994
            y: 50
            width: 85
            height: 21
            text: qsTr("tcp_force")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
        }

        Rectangle {
            id: rectangle54
            x: 1085
            y: 46
            width: 292
            height: 29
            color: "#ffffff"
            border.width: 2
            TextInput {
                id: textInput34
                text: qsTr("0.001,0.001,0.001,0.001,0.001,0.001")
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                mouseSelectionMode: TextInput.SelectCharacters
                selectByMouse: true
                font.pointSize: 11
            }
        }

        Text {
            id: text45
            x: 967
            y: 92
            width: 112
            height: 21
            text: qsTr("program_state")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
        }

        Rectangle {
            id: rectangle55
            x: 1085
            y: 88
            width: 292
            height: 29
            color: "#ffffff"
            border.width: 2
            TextInput {
                id: textInput35
                text: qsTr("0.001,0.001,0.001,0.001,0.001,0.001")
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                mouseSelectionMode: TextInput.SelectCharacters
                selectByMouse: true
                font.pointSize: 11
            }
        }

        Text {
            id: text46
            x: 988
            y: 135
            width: 91
            height: 21
            text: qsTr("robot_mode")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
        }

        Rectangle {
            id: rectangle56
            x: 1085
            y: 131
            width: 292
            height: 29
            color: "#ffffff"
            border.width: 2
            TextInput {
                id: textInput36
                text: qsTr("0.001,0.001,0.001,0.001,0.001,0.001")
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                mouseSelectionMode: TextInput.SelectCharacters
                selectByMouse: true
                font.pointSize: 11
            }
        }

        Text {
            id: text47
            x: 994
            y: 175
            width: 85
            height: 21
            text: qsTr("位姿")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
        }

        Rectangle {
            id: rectangle57
            x: 1085
            y: 171
            width: 292
            height: 29
            color: "#ffffff"
            border.width: 2
            TextInput {
                id: textInput37
                text: qsTr("0.001,0.001,0.001,0.001,0.001,0.001")
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                mouseSelectionMode: TextInput.SelectCharacters
                selectByMouse: true
                font.pointSize: 11
            }
        }

        Text {
            id: text48
            x: 994
            y: 215
            width: 85
            height: 21
            text: "关节角"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
        }

        Rectangle {
            id: rectangle58
            x: 1085
            y: 211
            width: 292
            height: 29
            color: "#ffffff"
            border.width: 2
            TextInput {
                id: textInput38
                text: qsTr("0.001,0.001,0.001,0.001,0.001,0.001")
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                mouseSelectionMode: TextInput.SelectCharacters
                selectByMouse: true
                font.pointSize: 11
            }
        }

        Rectangle {
            id: rectangle34
            x: 426
            y: 46
            width: 355
            height: 127
            color: pgcolor
            border.width: 2
            CusButton {
                id: cusButton14
                x: 9
                y: 29
                width: 94
                height: 37
                text: "上电"
                onClicked: {
                        var args = {"func": "powerOn",
                                    "arg_str": rbindex}
                        guiIntf.guiCall(args)
                    }
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
            }

            CusButton {
                id: cusButton15
                x: 109
                y: 29
                width: 129
                height: 37
                text: "解除保护性停止"
                onClicked: {
                        var args = {"func": "unlockProtectiveStop",
                                    "arg_str": rbindex}
                        guiIntf.guiCall(args)
                    }
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
            }

            CusButton {
                id: cusButton16
                x: 244
                y: 29
                width: 102
                height: 37
                text: "关闭警告"
                onClicked: {
                        var args = {"func": "closePopUp",
                                    "arg_str": rbindex}
                        guiIntf.guiCall(args)
                    }
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
            }

            CusButton {
                id: cusButton
                x: 9
                y: 72
                width: 94
                height: 37
                text: "加载程序"
                onClicked: {
                        var args = {"func": "robotPlay",
                                    "arg_str": rbindex}
                        guiIntf.guiCall(args)
                    }
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
            }

            CusButton {
                id: cusButton1
                x: 109
                y: 72
                width: 94
                height: 37
                text: "中止程序"
                onClicked: {
                    var args = {"func": "robotStop",
                        "arg_str": rbindex}
                    guiIntf.guiCall(args)
                }
                Layout.preferredWidth: 70
                backgroundColor: "#ffffff"
                Layout.preferredHeight: 40
            }
        }

        Rectangle {
            id: rectangle35
            x: 417
            y: 31
            width: 58
            height: 32
            color: pgcolor
            Text {
                id: text28
                x: 23
                y: 16
                width: 64
                height: 38
                text: "系统控制"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: parent.horizontalCenter
                clip: false
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}D{i:53}D{i:55}D{i:62}
}
##^##*/
