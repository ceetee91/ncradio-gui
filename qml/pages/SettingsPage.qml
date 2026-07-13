import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../components"

// Full-screen Settings page: mini-header + left category nav + scrollable
// detail pane covering every field in Config — matches the Penpot
// "Settings" board (replaces the earlier dialog-based version).
Item {
    id: root
    signal backRequested()

    Component.onCompleted: audio.refreshDevices()

    MiniHeader {
        id: header
        anchors.top: parent.top
        width: parent.width
        title: "Settings"
        onBackRequested: root.backRequested()
    }

    RowLayout {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        // ---- Category nav rail ----
        Item {
            Layout.preferredWidth: 240
            Layout.fillHeight: true

            Rectangle { anchors.fill: parent; color: Theme.glassElevated }
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.glassBorder }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 4

                Repeater {
                    model: [
                        { label: "General / Tuning", icon: "settings", anchor: generalSection },
                        { label: "Audio", icon: "volume", anchor: audioSection },
                        { label: "Recording", icon: "record", anchor: recordingSection },
                    ]
                    delegate: Item {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40

                        Rectangle {
                            anchors.fill: parent
                            radius: Theme.radiusMd
                            color: navMouse.containsMouse ? Theme.withAlpha(Theme.cyan, 0.12) : "transparent"
                        }
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            spacing: 10
                            AppIcon { name: modelData.icon; color: Theme.textSecondary; Layout.preferredWidth: 16; Layout.preferredHeight: 16 }
                            Text { text: modelData.label; color: Theme.textSecondary; font.family: Theme.fontUi; font.pointSize: 10 }
                        }
                        MouseArea {
                            id: navMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: scrollView.contentItem.contentY = Math.max(0, modelData.anchor.y - 8)
                        }
                    }
                }
            }
        }

        // ---- Detail pane ----
        Controls.ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth

            Kirigami.FormLayout {
                width: scrollView.availableWidth - 64
                x: 32

                Item { id: generalSection; Kirigami.FormData.isSection: true; Kirigami.FormData.label: "General / Tuning" }

                Controls.ComboBox {
                    Kirigami.FormData.label: "Scan step:"
                    model: ["0.05 MHz", "0.10 MHz", "0.20 MHz"]
                    readonly property var values: [0.05, 0.10, 0.20]
                    Component.onCompleted: currentIndex = values.indexOf(Math.round(configStore.scanStepMhz * 1000) / 1000)
                    onActivated: configStore.scanStepMhz = values[currentIndex]
                }
                Controls.SpinBox {
                    Kirigami.FormData.label: "Signal threshold:"
                    from: 5; to: 95; stepSize: 5
                    value: configStore.signalThresholdPct
                    textFromValue: (v) => v + "%"
                    onValueModified: configStore.signalThresholdPct = value
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Save RDS names:"
                    checked: configStore.rdsNames
                    onToggled: configStore.rdsNames = checked
                }

                Item { id: audioSection; Kirigami.FormData.isSection: true; Kirigami.FormData.label: "Audio" }

                Controls.Label {
                    Kirigami.FormData.label: "Status:"
                    text: audio.statusText + (audio.errorMessage.length > 0 ? " — " + audio.errorMessage : "")
                    color: audio.running ? Theme.green : Theme.textSecondary
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Audio output:"
                    checked: audio.enabled
                    onToggled: audio.enabled = checked
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Capture device:"
                    model: audio.captureDevices.map(d => d.description)
                    Component.onCompleted: {
                        const i = audio.captureDevices.findIndex(d => d.name === audio.captureDevice);
                        if (i >= 0) currentIndex = i;
                    }
                    onActivated: audio.captureDevice = audio.captureDevices[currentIndex].name
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Playback device:"
                    model: audio.playbackDevices.map(d => d.description)
                    Component.onCompleted: {
                        const i = audio.playbackDevices.findIndex(d => d.name === audio.playbackDevice);
                        if (i >= 0) currentIndex = i;
                    }
                    onActivated: audio.playbackDevice = audio.playbackDevices[currentIndex].name
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Buffer size:"
                    readonly property var values: audio.bufferSizeOptions
                    model: values.map(v => audio.bufferSizeLabel(v))
                    Component.onCompleted: currentIndex = values.indexOf(audio.bufferFrames)
                    onActivated: audio.bufferFrames = values[currentIndex]
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Mute while scanning:"
                    checked: configStore.audioMuteScan
                    onToggled: configStore.audioMuteScan = checked
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Mute while seeking:"
                    checked: configStore.audioMuteSeek
                    onToggled: configStore.audioMuteSeek = checked
                }

                Item { id: recordingSection; Kirigami.FormData.isSection: true; Kirigami.FormData.label: "Recording" }

                Controls.ComboBox {
                    Kirigami.FormData.label: "Record format:"
                    model: recorder.availableFormatNames
                    currentIndex: recorder.format
                    onActivated: recorder.format = currentIndex
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Stereo:"
                    checked: recorder.stereo
                    onToggled: recorder.stereo = checked
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Sample rate:"
                    model: ["22050 Hz", "44100 Hz", "48000 Hz"]
                    readonly property var values: [22050, 44100, 48000]
                    Component.onCompleted: currentIndex = values.indexOf(recorder.sampleRate)
                    onActivated: recorder.sampleRate = values[currentIndex]
                }
                RowLayout {
                    Kirigami.FormData.label: recorder.format === 1 ? "Bitrate:"
                        : recorder.format === 2 ? "Quality:"
                        : recorder.format === 3 ? "Compression:" : "Quality:"
                    Layout.fillWidth: true
                    spacing: 12
                    enabled: recorder.format !== 0

                    // Snap to the actual supported values per format, rather
                    // than an arbitrary continuous range.
                    readonly property var mp3Values: [64, 96, 128, 160, 192, 224, 256, 320]
                    readonly property var oggValues: [-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                    readonly property var flacValues: [0, 1, 2, 3, 4, 5, 6, 7, 8]
                    readonly property var activeValues: recorder.format === 1 ? mp3Values
                        : recorder.format === 2 ? oggValues
                        : recorder.format === 3 ? flacValues : [0]

                    function nearestIndex(arr, val) {
                        let bestIdx = 0, bestDiff = Infinity;
                        for (let i = 0; i < arr.length; i++) {
                            const diff = Math.abs(arr[i] - val);
                            if (diff < bestDiff) { bestDiff = diff; bestIdx = i; }
                        }
                        return bestIdx;
                    }

                    Controls.Slider {
                        id: qualitySlider
                        Layout.fillWidth: true
                        from: 0
                        to: parent.activeValues.length - 1
                        stepSize: 1
                        snapMode: Controls.Slider.SnapAlways
                        value: parent.nearestIndex(parent.activeValues, recorder.quality)
                        onMoved: recorder.quality = parent.activeValues[Math.round(value)]
                    }
                    Controls.Label {
                        text: recorder.qualityLabel
                        color: Theme.cyan
                        font.family: Theme.fontMono
                        Layout.preferredWidth: 90
                    }
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Apply EQ to recordings:"
                    checked: recorder.applyEqToRecs
                    onToggled: recorder.applyEqToRecs = checked
                }

                Item { Kirigami.FormData.isSection: false; height: 32 }
            }
        }
    }
}
