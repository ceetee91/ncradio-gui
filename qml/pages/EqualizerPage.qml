import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"
import "../dialogs"

// Full-screen Equalizer page: mini-header + full-width 11-band view,
// matching the Penpot "Equalizer" board (replaces the earlier dialog).
Item {
    id: root
    signal backRequested()

    readonly property bool pageActive: root.Controls.StackView.status === Controls.StackView.Active

    function combinedPresetNames() {
        return eq.builtinPresetNames.concat(eq.customPresetNames);
    }
    function loadPresetAt(idx) {
        if (idx < eq.builtinPresetNames.length) eq.loadBuiltinPreset(idx);
        else eq.loadCustomPreset(idx - eq.builtinPresetNames.length);
    }
    function cyclePreset(delta) {
        const names = combinedPresetNames();
        if (names.length === 0) return;
        let idx = names.indexOf(eq.activePresetName);
        if (idx < 0) idx = 0;
        loadPresetAt((idx + delta + names.length) % names.length);
    }
    function deleteCurrentPreset() {
        const idx = eq.customPresetNames.indexOf(eq.activePresetName);
        if (idx >= 0) {
            deleteEqDialog.pendingIndex = idx;
            deleteEqDialog.message = "This will permanently remove the custom preset “"
                + eq.activePresetName + "”. This cannot be undone.";
            deleteEqDialog.open();
        }
    }

    // Mirrors ncradio.c's M_EQ mode key bindings.
    Shortcut { sequence: "Escape"; enabled: root.pageActive; onActivated: root.backRequested() }
    Shortcut { sequence: "Shift+E"; enabled: root.pageActive; onActivated: root.backRequested() }
    Shortcut { sequence: "Space"; enabled: root.pageActive; onActivated: eq.enabled = !eq.enabled }
    Shortcut { sequence: "["; enabled: root.pageActive; onActivated: root.cyclePreset(-1) }
    Shortcut { sequence: "]"; enabled: root.pageActive; onActivated: root.cyclePreset(1) }
    Shortcut { sequence: "0"; enabled: root.pageActive; onActivated: eq.loadFlat() }
    Shortcut { sequence: "S"; enabled: root.pageActive; onActivated: savePresetDialog.open() }
    Shortcut {
        sequence: "Delete"
        enabled: root.pageActive && eq.customPresetNames.includes(eq.activePresetName)
        onActivated: root.deleteCurrentPreset()
    }

    MiniHeader {
        id: header
        anchors.top: parent.top
        width: parent.width
        title: "Equalizer"
        onBackRequested: root.backRequested()
    }

    GlassPanel {
        id: card
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 40
        elevated: true
        glow: true

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 32
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Controls.Switch {
                    checked: eq.enabled
                    onToggled: eq.enabled = checked
                }
                Text { text: "Equalizer Enabled"; color: Theme.textPrimary; font.family: Theme.fontUi; font.pointSize: 11 }

                Item { Layout.fillWidth: true }

                Controls.ComboBox {
                    id: presetCombo
                    Layout.preferredWidth: 180
                    model: eq.builtinPresetNames.concat(eq.customPresetNames)
                    currentIndex: -1
                    displayText: eq.activePresetName.length > 0 ? eq.activePresetName : "Custom"
                    onActivated: {
                        if (currentIndex < eq.builtinPresetNames.length)
                            eq.loadBuiltinPreset(currentIndex);
                        else
                            eq.loadCustomPreset(currentIndex - eq.builtinPresetNames.length);
                    }
                }
                AccentButton {
                    text: "Save As…"
                    variant: "secondary"
                    onClicked: savePresetDialog.open()
                }
                IconButton {
                    icon: "trash"
                    size: 36
                    enabled: eq.customPresetNames.includes(eq.activePresetName)
                    onClicked: root.deleteCurrentPreset()
                }
                AccentButton {
                    text: "Flat"
                    variant: "ghost"
                    onClicked: eq.loadFlat()
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.glassBorder }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true
                spacing: 28

                Repeater {
                    model: eq.bandCount
                    delegate: ColumnLayout {
                        required property int index
                        Layout.fillHeight: true
                        Layout.preferredWidth: 48
                        spacing: 4

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: (eq.gains[index] > 0 ? "+" : "") + eq.gains[index].toFixed(1)
                            font.family: Theme.fontMono
                            font.pointSize: 10
                            color: eq.gains[index] >= 0 ? Theme.cyan : Theme.amber
                        }

                        EqBandSlider {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillHeight: true
                            value: eq.gains[index]
                            onMoved: (v) => eq.setGain(index, v)
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: eq.bandLabels[index]
                            font.family: Theme.fontUi
                            font.pointSize: 10
                            color: Theme.textSecondary
                        }
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: "32 Hz – 16 kHz  ·  ±12 dB per band  ·  Q ≈ 1.41 (1-octave)"
                color: Theme.textTertiary
                font.family: Theme.fontUi
                font.pointSize: 10
            }
        }
    }

    GlassDialog {
        id: savePresetDialog
        dialogTitle: "Save EQ Preset"
        dialogWidth: 360

        function commit() {
            if (nameField.text.length > 0) {
                eq.saveCustomPreset(nameField.text);
                savePresetDialog.close();
            }
        }

        Controls.TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: "Preset name"
            onAccepted: savePresetDialog.commit()
        }

        footerData: [
            AccentButton { text: "Cancel"; variant: "ghost"; onClicked: savePresetDialog.close() },
            AccentButton { text: "Save"; variant: "primary"; onClicked: savePresetDialog.commit() }
        ]
    }

    DeleteConfirmDialog {
        id: deleteEqDialog
        property int pendingIndex: -1
        dialogTitle: "Delete EQ Preset?"
        onConfirmed: eq.deleteCustomPreset(pendingIndex)
    }
}
