import QtQuick 2.0
import QtQuick.Controls 2.3 as Qcontrol2
import QtQuick.Controls 1.4

TableView {
    id: tabview
    anchors.fill: parent
    anchors.margins: 10
    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    sortIndicatorVisible: true

    property real headerHeight: 50
    property real rowHeight: 50
    property string btn_txt: "查看详情"
    property var col_info: []
    signal btn_clicked(var col_index, var row_index)
    property var col_width: []
    property var row_height: ({})

    function getRealLen(str){
        return str.replace(/[^\x00-\xff]/g,'__').length
    }

    function refresh(){
        row_height[-1] = 17
    }

    onRowCountChanged: currentRow = -1

    Qcontrol2.ScrollBar {
        id: vbar
        hoverEnabled: true
        active: hovered || pressed
        orientation: Qt.Vertical
        size: tabview.height / tabview.flickableItem.contentHeight
        visible: size < 1
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 50
        contentItem: Rectangle {
            implicitWidth: (vbar.hovered || vbar.pressed) ? 10 : 2
            color: vbar.pressed ? "gray" : "#c2c2c2"
            radius: 5
        }
        onPositionChanged: {
            tabview.flickableItem.contentY = position * tabview.flickableItem.contentHeight - 50
        }
    }
    Qcontrol2.ScrollBar {
        id: hbar
        hoverEnabled: true
        active: hovered || pressed
        orientation: Qt.Horizontal
        size: tabview.width / tabview.flickableItem.contentWidth
        visible: size < 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentItem: Rectangle {
            implicitHeight: (hbar.hovered || hbar.pressed) ? 10 : 2
            color: "gray"
            radius: 5
        }
        onPositionChanged: {
            tabview.flickableItem.contentX = position * tabview.flickableItem.contentWidth
        }
    }
    MouseArea {
        anchors{left: parent.left; right: vbar.left; top: parent.top; bottom: parent.bottom}
        acceptedButtons: Qt.MidButton
        onWheel: {
            if(vbar.visible === true)
            {
                var delta = wheel.angleDelta.y / (parent.height - 50)
                var tmp = vbar.position
                if (delta < 0)
                    tmp += 0.2 * (1 - vbar.size)
                else
                    tmp -= 0.2 * (1 - vbar.size)
                if(tmp < 0)
                    tmp = 0
                if(tmp > 1 - vbar.size)
                    tmp = 1 - vbar.size
                vbar.position = tmp
            }
        }
    }
    property var row: null
    headerDelegate: Rectangle {
        border.width: 1
        border.color: "#bcbcbc"
        width: 150
        height: headerHeight
        color: "#2b579a"
        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            text: styleData.value
            font.pointSize: 13
            font.bold: true
            color: "white"
        }
    }
    itemDelegate: Rectangle {
        width: col_width[styleData.column]
        border.width: 1
        border.color: "#bcbcbc"
        color: "transparent"
        clip: true
        Text {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: col_width[styleData.column] > width ? (col_width[styleData.column] - width) / 2 : 5
            anchors.topMargin: (row_height.hasOwnProperty(styleData.row) === true || row_height[styleData.row] > implicitHeight) ? (row_height[styleData.row] - implicitHeight) / 2 : 5
            text: styleData.value === "admin" ? "管理员" : styleData.value === "root" ? "运行维护" :
                                                          styleData.value === "doctor" ? "医生" :
                                                          styleData.value === "operator" ? "操作员" : styleData.value
            font.pointSize: 11
            wrapMode: Text.WrapAnywhere
            onTextChanged: {
                width = getRealLen(text) * 10
                if(width > col_width[styleData.column])
                    width = col_width[styleData.column] - 10
            }
            onImplicitHeightChanged: {
                if(row_height.hasOwnProperty(styleData.row) === false || row_height[styleData.row] < implicitHeight + 10)
                {
                    row_height[styleData.row] = implicitHeight + 10
                }
            }
        }
        MyButton {
            visible: col_info.indexOf(styleData.column) != -1
            width: 75
            height: 30
            btn_txt: tabview.btn_txt
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 37.5
            anchors.topMargin: (tabview.rowHeight - 30) / 2
            onClicked: btn_clicked(styleData.column, styleData.row)
        }
    }
    rowDelegate: Rectangle{
        height: row_height.hasOwnProperty(styleData.row) === true ? row_height[styleData.row] : rowHeight
        color: styleData.selected ? "#008ec9" : styleData.row % 2 == 0 ? "white" : "#f5f6f7"
    }
}
