import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

GlassDialog {
    id: dialog
    dialogTitle: "Start Recording"
    dialogWidth: 460

    onOpened: {
        field.text = "";
        field.forceActiveFocus();
    }

    function commit() {
        if (recorder.start(field.text))
            dialog.close();
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
