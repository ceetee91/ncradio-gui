import QtQuick
import QtQuick.Layouts

// Four ascending bars + numeric readout, matching the Penpot "signal" icon
// treatment (used both as a small icon and, here, a live meter).
RowLayout {
    id: root
    property int percent: 0
    spacing: 3

    Repeater {
        model: 4
        delegate: Rectangle {
            required property int index
            readonly property real threshold: (index + 1) * 25
            Layout.preferredWidth: 4
            Layout.preferredHeight: 6 + index * 4
            Layout.alignment: Qt.AlignBottom
            radius: 1
            color: root.percent >= threshold ? Theme.cyan : Theme.trackBg
        }
    }

    Text {
        text: root.percent + "%"
        color: Theme.textPrimary
        font.family: Theme.fontMono
        font.pointSize: 10
        Layout.leftMargin: Theme.spacingXs
    }
}
