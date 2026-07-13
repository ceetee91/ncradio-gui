import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"
import "../dialogs"

// Main / Now Playing view: header + sidebar preset list + now-playing
// card. Pushed as the initial item of Main.qml's StackView.
Item {
    id: root
    signal settingsRequested()
    signal equalizerRequested()

    readonly property bool pageActive: Controls.StackView.status === Controls.StackView.Active
    readonly property bool typing: Window.activeFocusItem instanceof TextInput
    readonly property bool navGuard: pageActive && !typing && radio.ready && !radio.scanning
    readonly property bool presetSelected: presetList.currentIndex >= 0 && presetList.currentIndex < configStore.presetCount

    function presetLabel(idx) {
        const freqMhz = (configStore.presetFreqHz(idx) / 1000000.0).toFixed(2);
        const name = configStore.presetName(idx);
        return freqMhz + (name.length > 0 ? " · " + name : "");
    }

    // ---------------- Keyboard shortcuts (mirrors ncradio.c's M_NORMAL
    // key bindings) — disabled while typing in a text field, while this
    // page isn't the one on top of the StackView, or (for tuning actions)
    // while a scan is in progress, same as the equivalent buttons below.
    Shortcut { sequence: "S"; enabled: root.navGuard; onActivated: radio.startScan() }
    Shortcut { sequence: ","; enabled: root.navGuard; onActivated: radio.stepDown() }
    Shortcut { sequence: "."; enabled: root.navGuard; onActivated: radio.stepUp() }
    Shortcut { sequence: "<"; enabled: root.navGuard; onActivated: radio.seekBackward() }
    Shortcut { sequence: ">"; enabled: root.navGuard; onActivated: radio.seekForward() }
    Shortcut { sequence: "T"; enabled: root.navGuard; onActivated: manualTuneDialog.open() }
    Shortcut { sequence: "O"; enabled: root.pageActive && !root.typing; onActivated: root.settingsRequested() }
    Shortcut { sequence: "Shift+E"; enabled: root.pageActive && !root.typing; onActivated: root.equalizerRequested() }
    Shortcut { sequence: "R"; enabled: root.pageActive && !root.typing && audio.running; onActivated: recordDialog.open() }
    Shortcut { sequence: "+"; enabled: root.pageActive && !root.typing; onActivated: radio.volume = Math.min(100, radio.volume + 5) }
    Shortcut { sequence: "="; enabled: root.pageActive && !root.typing; onActivated: radio.volume = Math.min(100, radio.volume + 5) }
    Shortcut { sequence: "-"; enabled: root.pageActive && !root.typing; onActivated: radio.volume = Math.max(0, radio.volume - 5) }
    Shortcut { sequence: "_"; enabled: root.pageActive && !root.typing; onActivated: radio.volume = Math.max(0, radio.volume - 5) }
    Shortcut { sequence: "M"; enabled: root.pageActive && !root.typing; onActivated: radio.muted = !radio.muted }
    Shortcut { sequence: "A"; enabled: root.pageActive && !root.typing && !radio.scanning; onActivated: manualAddDialog.open() }
    Shortcut {
        sequence: "D"
        enabled: root.pageActive && !root.typing && root.presetSelected
        onActivated: {
            deletePresetDialog.pendingIndex = presetList.currentIndex;
            deletePresetDialog.message = "This will permanently remove “" + root.presetLabel(presetList.currentIndex)
                + "” from your presets. This cannot be undone.";
            deletePresetDialog.open();
        }
    }
    Shortcut {
        sequence: "E"
        enabled: root.pageActive && !root.typing && root.presetSelected
        onActivated: { renamePresetDialog.presetIndex = presetList.currentIndex; renamePresetDialog.open(); }
    }
    Shortcut { sequence: "Up"; enabled: root.pageActive && !root.typing; onActivated: presetList.decrementCurrentIndex() }
    Shortcut { sequence: "Down"; enabled: root.pageActive && !root.typing; onActivated: presetList.incrementCurrentIndex() }
    Shortcut {
        sequence: "Return"
        enabled: root.pageActive && !root.typing && root.presetSelected
        onActivated: radio.tuneToHz(configStore.presetFreqHz(presetList.currentIndex))
    }
    Shortcut {
        sequence: "Enter"
        enabled: root.pageActive && !root.typing && root.presetSelected
        onActivated: radio.tuneToHz(configStore.presetFreqHz(presetList.currentIndex))
    }
    Shortcut {
        sequence: "PgUp"
        enabled: root.pageActive && !root.typing && configStore.presetCount > 0
        onActivated: {
            const page = Math.max(1, Math.floor(presetList.height / 46) - 1);
            presetList.currentIndex = Math.max(0, presetList.currentIndex - page);
        }
    }
    Shortcut {
        sequence: "PgDown"
        enabled: root.pageActive && !root.typing && configStore.presetCount > 0
        onActivated: {
            const page = Math.max(1, Math.floor(presetList.height / 46) - 1);
            presetList.currentIndex = Math.min(configStore.presetCount - 1, presetList.currentIndex + page);
        }
    }
    Shortcut { sequence: "Q"; enabled: root.pageActive && !root.typing; onActivated: Qt.quit() }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---------------- Header ----------------
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 56

            Rectangle {
                anchors.fill: parent
                color: Theme.glass
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.glassBorder
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 16
                spacing: 12

                AppIcon { name: "signal"; width: 22; height: 22; color: Theme.cyan }
                Text {
                    text: Qt.application.name
                    color: Theme.textPrimary
                    font.family: Theme.fontBrand
                    font.weight: Font.Bold
                    font.pointSize: 14
                    font.letterSpacing: 1.5
                }

                Item { Layout.fillWidth: true }

                IconButton { icon: "eq"; tooltipText: "Equalizer"; onClicked: root.equalizerRequested() }
                IconButton { icon: "settings"; tooltipText: "Settings"; onClicked: root.settingsRequested() }
                IconButton { icon: "theme"; tooltipText: "Toggle theme"; onClicked: Theme.dark = !Theme.dark }
                IconButton { icon: "info"; tooltipText: "About"; onClicked: aboutDialog.open() }
            }
        }

        // ---------------- Body ----------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ---- Sidebar ----
            Item {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                opacity: radio.scanning ? 0.5 : 1
                enabled: !radio.scanning

                Rectangle { anchors.fill: parent; color: Theme.glassElevated }
                Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.glassBorder }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Text {
                        visible: radio.scanning
                        Layout.fillWidth: true
                        text: "Presets locked during scan"
                        color: Theme.textTertiary
                        font.family: Theme.fontUi
                        font.pointSize: 10
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        visible: !radio.scanning
                        Layout.fillWidth: true
                        spacing: 8
                        Controls.TextField {
                            id: searchField
                            Layout.fillWidth: true
                            placeholderText: "Search stations…"
                        }
                        IconButton { icon: "plus"; tooltipText: "Add current frequency as preset"; onClicked: manualAddDialog.open() }
                    }

                    Text {
                        text: "PRESETS (" + configStore.presetCount + ")"
                        color: Theme.textTertiary
                        font.family: Theme.fontUi
                        font.weight: Font.Bold
                        font.pointSize: 9
                        font.letterSpacing: 1.4
                    }

                    ListView {
                        id: presetList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 2
                        model: presetModel
                        Component.onCompleted: currentIndex = configStore.findPreset(radio.frequencyMhz)

                        delegate: StationListItem {
                            width: presetList.width
                            visible: searchField.text.length === 0
                                || model.name.toLowerCase().includes(searchField.text.toLowerCase())
                                || model.freqLabel.includes(searchField.text)
                            height: visible ? implicitHeight : 0
                            freqLabel: model.freqLabel
                            name: model.name
                            tuned: model.freqHz === radio.frequencyHz
                            selected: presetList.currentIndex === index
                            onClicked: {
                                presetList.currentIndex = index;
                                radio.tuneToHz(model.freqHz);
                            }
                            onEditRequested: {
                                renamePresetDialog.presetIndex = index;
                                renamePresetDialog.open();
                            }
                            onDeleteRequested: {
                                deletePresetDialog.pendingIndex = index;
                                deletePresetDialog.message = "This will permanently remove “"
                                    + model.freqLabel + (model.name.length > 0 ? " · " + model.name : "")
                                    + "” from your presets. This cannot be undone.";
                                deletePresetDialog.open();
                            }
                        }
                    }
                }
            }

            // ---- Content ----
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ScanView {
                    anchors.fill: parent
                    anchors.margins: 40
                    visible: radio.scanning
                }

                GlassPanel {
                    id: card
                    anchors.fill: parent
                    anchors.margins: 40
                    elevated: true
                    glow: true
                    visible: !radio.scanning

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 32
                        spacing: 20

                        // status row
                        RowLayout {
                            Layout.fillWidth: true
                            StatusBadge { text: radio.stereo ? "ST" : "MO"; variant: radio.stereo ? "green" : "neutral" }
                            Item { Layout.fillWidth: true }
                            SignalMeter { percent: radio.signalPercent }
                            StatusBadge {
                                text: audio.running ? "A" : (audio.enabled ? "A!" : "A")
                                variant: audio.running ? "green" : (audio.enabled ? "red" : "neutral")
                            }
                            StatusBadge { visible: eq.enabled; text: "EQ"; variant: "cyan" }
                        }

                        // frequency + RDS row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 48

                            FreqDisplay { mhz: radio.frequencyMhz }

                            ColumnLayout {
                                spacing: 4
                                Layout.fillWidth: true
                                Text {
                                    text: "STATION"
                                    color: Theme.textTertiary
                                    font.family: Theme.fontUi; font.weight: Font.Bold; font.pointSize: 8; font.letterSpacing: 1.6
                                }
                                Text {
                                    text: radio.rdsStationName.length > 0 ? radio.rdsStationName : "—"
                                    color: Theme.green
                                    font.family: Theme.fontUi; font.weight: Font.Bold; font.pointSize: 15
                                }
                                Text {
                                    text: "RADIOTEXT"
                                    color: Theme.textTertiary
                                    font.family: Theme.fontUi; font.weight: Font.Bold; font.pointSize: 8; font.letterSpacing: 1.6
                                    topPadding: 8
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: radio.statusMessage.length > 0 ? radio.statusMessage : radio.rdsRadioText
                                    color: Theme.textSecondary
                                    font.family: Theme.fontUi; font.pointSize: 10
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.glassBorder }

                        // transport row / seek banner
                        RowLayout {
                            Layout.fillWidth: true
                            visible: !radio.seeking

                            IconButton { icon: "seek-back"; enabled: radio.ready && !radio.scanning; onClicked: radio.seekBackward() }
                            IconButton { icon: "step-back"; enabled: radio.ready && !radio.scanning; onClicked: radio.stepDown() }
                            AccentButton {
                                text: "Tune"
                                icon: "search"
                                variant: "secondary"
                                enabled: radio.ready && !radio.scanning
                                onClicked: manualTuneDialog.open()
                            }
                            IconButton { icon: "step-fwd"; enabled: radio.ready && !radio.scanning; onClicked: radio.stepUp() }
                            IconButton { icon: "seek-fwd"; enabled: radio.ready && !radio.scanning; onClicked: radio.seekForward() }

                            Item { Layout.fillWidth: true }

                            AccentButton {
                                text: "Scan Band"
                                icon: "scan"
                                variant: "primary"
                                enabled: radio.ready
                                onClicked: radio.startScan()
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            visible: radio.seeking
                            StatusBadge { text: "SEEKING " + (radio.seekingForward ? "FORWARD ›" : "‹ BACKWARD"); variant: "amber" }
                            Item { Layout.fillWidth: true }
                            AccentButton { text: "Cancel"; variant: "ghost"; onClicked: radio.cancelSeek() }
                        }

                        // volume / record row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            IconButton {
                                icon: radio.muted ? "mute" : "volume"
                                active: radio.muted
                                onClicked: radio.muted = !radio.muted
                            }
                            VolumeSlider {
                                Layout.preferredWidth: 260
                                from: 0; to: 100
                                value: radio.volume
                                onMoved: (v) => radio.volume = v
                            }
                            Text {
                                text: radio.muted ? "MUTED" : radio.volume + "%"
                                color: radio.muted ? Theme.red : Theme.textPrimary
                                font.family: Theme.fontMono; font.pointSize: 11
                            }

                            Item { Layout.fillWidth: true }

                            Controls.Label {
                                visible: recorder.recording
                                text: "● REC " + Math.floor(recorder.elapsedSeconds / 60) + ":" + String(recorder.elapsedSeconds % 60).padStart(2, "0")
                                color: Theme.red
                                font.family: Theme.fontMono
                                font.bold: true
                            }
                            AccentButton {
                                text: recorder.recording ? "Stop" : "Record"
                                icon: "record"
                                variant: "danger"
                                enabled: audio.running || recorder.recording
                                onClicked: recorder.recording ? recorder.stop() : recordDialog.open()
                            }
                        }
                    }
                }
            }
        }
    }

    ManualTuneDialog { id: manualTuneDialog }
    RenamePresetDialog { id: manualAddDialog; addMode: true }
    RenamePresetDialog { id: renamePresetDialog; addMode: false }
    DeleteConfirmDialog {
        id: deletePresetDialog
        property int pendingIndex: -1
        dialogTitle: "Delete Preset?"
        onConfirmed: configStore.deletePreset(pendingIndex)
    }
    RecordFilenameDialog { id: recordDialog }
    AboutDialog { id: aboutDialog }
}
