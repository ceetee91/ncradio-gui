import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../components"

GlassDialog {
    id: dialog
    dialogTitle: "About"
    dialogWidth: 440

    AppIcon {
        Layout.alignment: Qt.AlignHCenter
        name: "signal"
        width: 64
        height: 64
        color: Theme.cyan
    }
    Text {
        Layout.alignment: Qt.AlignHCenter
        text: Qt.application.name
        font.family: Theme.fontBrand
        font.weight: Font.Bold
        font.pointSize: 18
        color: Theme.textPrimary
    }
    Controls.Label {
        Layout.alignment: Qt.AlignHCenter
        text: "v" + Qt.application.version + " · based on ncradio 1.2"
        color: Theme.textSecondary
    }
    Controls.Label {
        Layout.alignment: Qt.AlignHCenter
        text: "by Constantinos Tsakiris"
        color: Theme.textSecondary
    }

    Rectangle { Layout.fillWidth: true; Layout.topMargin: 12; Layout.bottomMargin: 4; height: 1; color: Theme.glassBorder }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        columnSpacing: 16
        rowSpacing: 6

        Controls.Label { text: "Audio backend"; color: Theme.textSecondary }
        Controls.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: audio.backendVersion
            color: Theme.textPrimary
            font.family: Theme.fontMono
        }
        Controls.Label { text: "Device autodetect"; color: Theme.textSecondary }
        Controls.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: audio.deviceAutodetectMethod
            color: Theme.textPrimary
            font.family: Theme.fontMono
        }
        Controls.Label { text: "Recording formats"; color: Theme.textSecondary }
        Controls.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: recorder.availableFormatNames.join(" ")
            color: Theme.textPrimary
            font.family: Theme.fontMono
        }
        Controls.Label { text: "Resampling"; color: Theme.textSecondary }
        Controls.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: recorder.resamplingAvailable ? "libsamplerate" : "disabled (native rate only)"
            color: Theme.textPrimary
            font.family: Theme.fontMono
        }
        Controls.Label { text: "Config file"; color: Theme.textSecondary }
        Controls.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: "~/.ncradio.conf"
            color: Theme.textPrimary
            font.family: Theme.fontMono
        }
    }

    footerData: [
        AccentButton { text: "Close"; variant: "primary"; onClicked: dialog.close() }
    ]
}
