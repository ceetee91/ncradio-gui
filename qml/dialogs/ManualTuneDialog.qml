import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

GlassDialog {
    id: dialog
    dialogTitle: "Manual Tune"
    dialogWidth: 420

    onOpened: {
        field.text = radio.frequencyMhz.toFixed(2);
        field.selectAll();
        field.forceActiveFocus();
    }

    function commit() {
        const mhz = parseFloat(field.text);
        if (!isNaN(mhz) && mhz >= radio.freqMinMhz && mhz <= radio.freqMaxMhz) {
            radio.tuneTo(mhz);
            dialog.close();
        }
    }

    Controls.Label {
        text: "FREQUENCY (MHz)"
        font.pointSize: 8
        font.bold: true
        color: Theme.textTertiary
    }
    Controls.TextField {
        id: field
        Layout.fillWidth: true
        font.family: Theme.fontMono
        font.pointSize: 20
        horizontalAlignment: Text.AlignHCenter
        validator: DoubleValidator { bottom: radio.freqMinMhz; top: radio.freqMaxMhz; decimals: 2 }
        onAccepted: dialog.commit()
    }
    Controls.Label {
        text: "Range: " + radio.freqMinMhz.toFixed(2) + " – " + radio.freqMaxMhz.toFixed(2) + " MHz"
        color: Theme.textTertiary
        font.pointSize: 9
    }

    footerData: [
        AccentButton { text: "Cancel"; variant: "ghost"; onClicked: dialog.close() },
        AccentButton { text: "Tune"; variant: "primary"; onClicked: dialog.commit() }
    ]
}
