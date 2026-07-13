import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

GlassDialog {
    id: dialog
    property bool addMode: false
    property int presetIndex: -1

    dialogTitle: addMode ? "Add Preset" : "Rename Preset"
    dialogWidth: 420

    onOpened: {
        if (addMode) {
            field.text = radio.rdsStationName.length > 0 ? radio.rdsStationName : "";
        } else if (presetIndex >= 0) {
            field.text = configStore.presetName(presetIndex);
        }
        field.forceActiveFocus();
        field.selectAll();
    }

    function commit() {
        if (addMode) {
            if (configStore.findPreset(radio.frequencyMhz) >= 0) {
                dialog.close();
                duplicateAlert.open();
                return;
            }
            configStore.addPreset(radio.frequencyMhz, field.text);
        } else if (presetIndex >= 0) {
            configStore.renamePreset(presetIndex, field.text);
        }
        dialog.close();
    }

    Controls.Label {
        text: (addMode ? radio.frequencyMhz.toFixed(2) : (configStore.presetFreqHz(dialog.presetIndex) / 1000000.0).toFixed(2)) + " MHz"
        font.family: Theme.fontMono
        color: Theme.textTertiary
    }
    Controls.Label {
        text: "NAME"
        font.pointSize: 8
        font.bold: true
        color: Theme.textTertiary
    }
    Controls.TextField {
        id: field
        Layout.fillWidth: true
        maximumLength: 32
        onAccepted: dialog.commit()
    }
    Controls.Label {
        text: field.text.length + " / 32 characters"
        color: Theme.textTertiary
        font.pointSize: 9
    }

    footerData: [
        AccentButton { text: "Cancel"; variant: "ghost"; onClicked: dialog.close() },
        AccentButton { text: addMode ? "Add" : "Save"; variant: "primary"; onClicked: dialog.commit() }
    ]

    AlertDialog {
        id: duplicateAlert
        dialogTitle: "Already Added"
        message: "Station is already in the preset list."
    }
}
