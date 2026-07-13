import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

// Simple single-message, single-button info popup (e.g. "already exists"
// style validation errors) — the GlassDialog equivalent of a message box.
GlassDialog {
    id: dialog
    property string message: ""

    dialogWidth: 380

    Controls.Label {
        Layout.fillWidth: true
        text: dialog.message
        color: Theme.textSecondary
        wrapMode: Text.WordWrap
    }

    footerData: [
        AccentButton { text: "OK"; variant: "primary"; onClicked: dialog.close() }
    ]
}
