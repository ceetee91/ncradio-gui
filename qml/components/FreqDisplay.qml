import QtQuick

// Large "LCD" style frequency readout — the hero element of the Now
// Playing screen.
Row {
    id: root
    property real mhz: 0
    property color accentColor: Theme.cyan
    spacing: 8

    Text {
        text: root.mhz.toFixed(2)
        color: root.accentColor
        font.family: Theme.fontMono
        font.weight: Font.Bold
        font.pointSize: 44
    }
    Text {
        text: "MHz"
        color: Theme.textSecondary
        font.family: Theme.fontUi
        font.weight: Font.DemiBold
        font.pointSize: 12
        font.letterSpacing: 1.5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
    }
}
