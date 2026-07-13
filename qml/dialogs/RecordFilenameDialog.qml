import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

GlassDialog {
    id: dialog
    dialogTitle: "Start Recording"
    dialogWidth: 460

    onOpened: {
        field.text = defaultFilename();
        field.forceActiveFocus();
        field.selectAll();
    }

    function commit() {
        if (recorder.start(field.text))
            dialog.close();
    }

    function stripInvalid(name) {
        return name.replace(/[\/\\:*?"<>|]+/g, "").trim();
    }

    function defaultFilename() {
        var stationName = "";
        var presetIdx = configStore.findPreset(radio.frequencyMhz);
        if (presetIdx >= 0)
            stationName = stripInvalid(configStore.presetName(presetIdx));
        if (stationName.length === 0 && radio.rdsStationName.length > 0)
            stationName = stripInvalid(radio.rdsStationName);
        if (stationName.length === 0)
            stationName = radio.frequencyMhz.toFixed(2);

        function pad(n) { return String(n).padStart(2, "0"); }
        var now = new Date();
        var dateStr = now.getFullYear() + "-" + pad(now.getMonth() + 1) + "-" + pad(now.getDate());
        var timeStr = pad(now.getHours()) + "-" + pad(now.getMinutes()) + "-" + pad(now.getSeconds());
        return stationName + "_" + dateStr + "_" + timeStr;
    }

    Controls.Label { text: "FILENAME"; font.pointSize: 8; font.bold: true; color: Theme.textTertiary }
    RowLayout {
        Layout.fillWidth: true
        Controls.TextField {
            id: field
            Layout.fillWidth: true
            font.family: Theme.fontMono
            onAccepted: dialog.commit()
        }
        Controls.Label {
            text: recorder.formatExtension
            font.family: Theme.fontMono
            color: Theme.textTertiary
        }
    }

    Rectangle { Layout.fillWidth: true; Layout.topMargin: 8; Layout.bottomMargin: 8; height: 1; color: Theme.glassBorder }

    RowLayout {
        Layout.fillWidth: true
        Controls.Label { text: "Format"; color: Theme.textSecondary; Layout.fillWidth: true }
        Controls.Label {
            text: recorder.availableFormatNames[recorder.format] + " · " + recorder.sampleRate
                + " Hz · " + (recorder.stereo ? "Stereo" : "Mono")
            font.family: Theme.fontMono
            color: Theme.cyan
        }
    }

    footerData: [
        AccentButton { text: "Cancel"; variant: "ghost"; onClicked: dialog.close() },
        AccentButton {
            text: "Record"
            icon: "record"
            variant: "danger"
            enabled: field.text.length > 0
            onClicked: dialog.commit()
        }
    ]
}
