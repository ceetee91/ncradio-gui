import QtQuick

// Small pill badge for status indicators: [ST]/[MO], [A]/[A!], [EQ], REC.
Rectangle {
    id: root
    property string text: ""
    property string variant: "neutral" // cyan | green | amber | red | neutral
    property int pointSize: 10

    readonly property color accent: variant === "cyan" ? Theme.cyan
        : variant === "green" ? Theme.green
        : variant === "amber" ? Theme.amber
        : variant === "red" ? Theme.red
        : Theme.textSecondary

    implicitWidth: label.implicitWidth + 20
    implicitHeight: label.implicitHeight + 8
    radius: Theme.radiusPill
    color: Theme.withAlpha(accent, 0.14)
    border.width: 1
    border.color: Theme.withAlpha(accent, 0.6)

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: root.accent
        font.family: Theme.fontUi
        font.weight: Font.Bold
        font.pointSize: root.pointSize
        font.letterSpacing: 0.6
        font.capitalization: Font.AllUppercase
    }
}
