import QtQuick
import QtQuick.Controls as Controls

// Sidebar preset row: frequency (mono), name, tuned marker, selected state.
Rectangle {
    id: root
    property string freqLabel: ""
    property string name: ""
    property bool tuned: false
    property bool selected: false
    property bool showActions: true
    signal clicked()
    signal editRequested()
    signal deleteRequested()

    implicitHeight: 44
    radius: Theme.radiusMd
    color: selected ? Theme.withAlpha(Theme.cyan, 0.14) : "transparent"
    border.width: selected ? 1 : 0
    border.color: Theme.withAlpha(Theme.cyan, 0.6)

    Rectangle {
        visible: root.tuned
        width: 3
        height: 18
        radius: 2
        color: Theme.green
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
    }

    Text {
        id: freqText
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        text: root.freqLabel
        color: root.tuned ? Theme.green : Theme.textPrimary
        font.family: Theme.fontMono
        font.weight: Font.DemiBold
        font.pointSize: 11
    }

    Text {
        anchors.left: freqText.right
        anchors.leftMargin: 14
        anchors.right: actions.visible ? actions.left : parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        text: root.name.length > 0 ? root.name : "—"
        color: root.selected ? Theme.textPrimary : Theme.textSecondary
        font.family: Theme.fontUi
        font.weight: root.selected ? Font.DemiBold : Font.Normal
        font.pointSize: 10
        elide: Text.ElideRight
    }

    Row {
        id: actions
        visible: root.selected && root.showActions
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0

        IconButton { icon: "edit"; size: 28; onClicked: root.editRequested() }
        IconButton { icon: "trash"; size: 28; onClicked: root.deleteRequested() }
    }

    MouseArea {
        anchors.fill: parent
        z: -1
        onClicked: root.clicked()
    }
}
