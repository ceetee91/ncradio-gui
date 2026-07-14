import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

// Compact "mini player" view: a single glass card with frequency, RDS,
// signal + stereo, and a mute/volume bar, plus an always-on-top pin and a
// return button. Loaded (and destroyed) by Main.qml only while the window
// is in minimal mode, so its keyboard shortcuts exist only then and never
// collide with the Now Playing view's identical sequences.
Item {
    id: root
    signal exitRequested()
    signal toggleAlwaysOnTop()
    property bool alwaysOnTop: false

    // Tune to the preset before/after the current frequency, wrapping around.
    // If the current frequency isn't itself a preset, land on the first
    // (next) or last (previous) preset.
    function cyclePreset(delta) {
        if (configStore.presetCount === 0)
            return;
        let idx = configStore.findPreset(radio.frequencyMhz);
        if (idx < 0) {
            radio.tuneToHz(configStore.presetFreqHz(delta > 0 ? 0 : configStore.presetCount - 1));
            return;
        }
        idx = (idx + delta + configStore.presetCount) % configStore.presetCount;
        radio.tuneToHz(configStore.presetFreqHz(idx));
    }

    // Only the keys the spec allows in minimal mode — no scan, record,
    // manual tune, or preset editing.
    Shortcut { sequence: "Z"; onActivated: root.exitRequested() }
    Shortcut { sequence: "+"; onActivated: radio.volume = Math.min(100, radio.volume + 5) }
    Shortcut { sequence: "="; onActivated: radio.volume = Math.min(100, radio.volume + 5) }
    Shortcut { sequence: "-"; onActivated: radio.volume = Math.max(0, radio.volume - 5) }
    Shortcut { sequence: "_"; onActivated: radio.volume = Math.max(0, radio.volume - 5) }
    Shortcut { sequence: "M"; onActivated: radio.muted = !radio.muted }
    Shortcut { sequence: "<"; enabled: radio.ready; onActivated: radio.seekBackward() }
    Shortcut { sequence: ">"; enabled: radio.ready; onActivated: radio.seekForward() }
    Shortcut { sequence: ","; enabled: radio.ready; onActivated: radio.stepDown() }
    Shortcut { sequence: "."; enabled: radio.ready; onActivated: radio.stepUp() }
    Shortcut { sequence: "Up"; enabled: radio.ready; onActivated: root.cyclePreset(-1) }
    Shortcut { sequence: "Down"; enabled: radio.ready; onActivated: root.cyclePreset(1) }

    GlassPanel {
        anchors.fill: parent
        anchors.margins: 8
        elevated: true
        glow: true

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 8

            // ---- signal / stereo + window controls ----
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                StatusBadge { text: radio.stereo ? "ST" : "MO"; variant: radio.stereo ? "green" : "neutral" }
                SignalMeter { percent: radio.signalPercent }

                Item { Layout.fillWidth: true }

                IconButton {
                    icon: "pin"
                    size: 30
                    active: root.alwaysOnTop
                    iconColor: Theme.textPrimary
                    tooltipText: "Always on top"
                    onClicked: root.toggleAlwaysOnTop()
                }
                IconButton {
                    icon: "expand"
                    size: 30
                    iconColor: Theme.textPrimary
                    tooltipText: "Exit minimal mode"
                    onClicked: root.exitRequested()
                }
            }

            // ---- frequency + RDS ----
            FreqDisplay { mhz: radio.frequencyMhz; pointSize: 34 }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1
                Text {
                    Layout.fillWidth: true
                    text: radio.rdsStationName.length > 0 ? radio.rdsStationName : "—"
                    color: Theme.green
                    font.family: Theme.fontUi; font.weight: Font.Bold; font.pointSize: 12
                    elide: Text.ElideRight
                }
                Text {
                    Layout.fillWidth: true
                    text: radio.statusMessage.length > 0 ? radio.statusMessage : radio.rdsRadioText
                    color: Theme.textSecondary
                    font.family: Theme.fontUi; font.pointSize: 8
                    elide: Text.ElideRight
                }
            }

            Item { Layout.fillHeight: true }

            // ---- mute / volume ----
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                IconButton {
                    icon: radio.muted ? "mute" : "volume"
                    size: 30
                    active: radio.muted
                    onClicked: radio.muted = !radio.muted
                }
                VolumeSlider {
                    Layout.fillWidth: true
                    from: 0; to: 100
                    value: radio.volume
                    onMoved: (v) => radio.volume = v
                }
                Text {
                    text: radio.muted ? "MUTED" : radio.volume + "%"
                    color: radio.muted ? Theme.red : Theme.textPrimary
                    font.family: Theme.fontMono; font.pointSize: 9
                }
            }
        }
    }
}
