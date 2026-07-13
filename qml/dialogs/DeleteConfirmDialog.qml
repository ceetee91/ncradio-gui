import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

GlassDialog {
    id: dialog
    property string message: "This cannot be undone."
    signal confirmed()

    dialogWidth: 420

    Controls.Label {
        Layout.fillWidth: true
        text: dialog.message
        color: Theme.textSecondary
        wrapMode: Text.WordWrap
    }

    footerData: [
        AccentButton { text: "Cancel"; variant: "ghost"; onClicked: dialog.close() },
        AccentButton {
            text: "Delete"
            icon: "trash"
            variant: "danger"
            onClicked: { dialog.confirmed(); dialog.close(); }
        }
    ]
}
