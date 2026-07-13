import QtQuick
import QtQuick.Layouts

// Slim header for secondary pages (Settings, Equalizer): back button +
// title + optional trailing controls, matching the Penpot "miniHeader".
Item {
    id: root
    property string title: ""
    default property alias trailingData: trailingRow.data
    signal backRequested()

    implicitHeight: 56

    Rectangle { anchors.fill: parent; color: Theme.glass }
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.glassBorder }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 12

        IconButton { icon: "seek-back"; tooltipText: "Back"; onClicked: root.backRequested() }
        Text {
            text: root.title
            color: Theme.textPrimary
            font.family: Theme.fontUi
            font.weight: Font.Bold
            font.pointSize: 14
        }
        Item { Layout.fillWidth: true }
        RowLayout { id: trailingRow; spacing: 8 }
    }
}
