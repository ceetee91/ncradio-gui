import QtQuick
import QtQuick.Layouts

// Dedicated scan-in-progress view: progress bar + live 2-column
// found-stations grid, matching the Penpot "Scan" board. Swapped in for
// the now-playing card in NowPlayingView while radio.scanning is true.
GlassPanel {
    id: root
    elevated: true
    glow: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 16

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 4
                Text {
                    text: "Scanning FM Band"
                    color: Theme.textPrimary
                    font.family: Theme.fontUi; font.weight: Font.Bold; font.pointSize: 17
                }
                Text {
                    text: radio.freqMinMhz.toFixed(2) + " – " + radio.freqMaxMhz.toFixed(2) + " MHz  ·  step "
                        + configStore.scanStepMhz.toFixed(3) + " MHz"
                    color: Theme.textSecondary
                    font.family: Theme.fontUi; font.pointSize: 10
                }
            }

            Item { Layout.fillWidth: true }

            AccentButton { text: "Discard"; variant: "ghost"; onClicked: radio.stopScanAndDiscard() }
            AccentButton { text: "Stop & Save"; icon: "check"; variant: "primary"; onClicked: radio.stopScanAndSave() }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            // Plain Item-based bar instead of Controls.ProgressBar: the
            // Breeze style's indeterminate-mode Repeater delegates bind to
            // `parent` before Qt Quick has attached them, which throws a
            // harmless but noisy "Cannot read property of null" warning
            // even though this bar is never indeterminate.
            Item {
                id: scanProgressBar
                Layout.fillWidth: true
                implicitHeight: 6

                Rectangle {
                    anchors.fill: parent
                    radius: height / 2
                    color: Theme.trackBg
                }
                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    radius: height / 2
                    width: Math.max(height, parent.width * radio.scanProgress)
                    color: Theme.amber
                }
            }
            Text {
                text: Math.round(radio.scanProgress * 100) + "%"
                color: Theme.amber
                font.family: Theme.fontMono; font.weight: Font.DemiBold; font.pointSize: 11
            }
        }

        Text {
            text: "FOUND STATIONS (" + radio.foundStations.length + ")"
            color: Theme.textTertiary
            font.family: Theme.fontUi; font.weight: Font.Bold; font.pointSize: 9
            font.letterSpacing: 1.4
        }

        GridView {
            id: foundGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: width / 2
            cellHeight: 48
            model: radio.foundStations
            // Keep the most recently found station in view as the scan
            // progresses down the band, instead of leaving the list
            // scrolled at whatever position it happened to be at.
            onCountChanged: positionViewAtEnd()

            delegate: Item {
                width: foundGrid.cellWidth
                height: foundGrid.cellHeight

                StationListItem {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    freqLabel: modelData.freqMhz.toFixed(2)
                    name: modelData.name
                    selected: index === radio.foundStations.length - 1
                    showActions: false
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: "Save RDS names: " + (configStore.rdsNames ? "On" : "Off")
                + " — collecting station names (≤1.5s each)"
            color: Theme.textTertiary
            font.family: Theme.fontUi; font.pointSize: 9
        }
    }
}
