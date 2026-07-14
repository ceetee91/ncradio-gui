import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "../components"

// Small name-entry dialog reused for both duplicating a theme (creating a new
// custom theme from a preset/custom source) and renaming an existing custom
// theme. The caller wires up onAccepted with the entered text.
GlassDialog {
    id: dialog

    // "duplicate" | "rename"
    property string mode: "duplicate"
    // For duplicate: the source theme being copied. For rename: the theme
    // being renamed (also used to seed the field).
    property string sourceName: ""

    signal accepted(string name)

    dialogTitle: mode === "rename" ? "Rename Theme" : "Duplicate Theme"
    dialogWidth: 420

    onOpened: {
        field.text = mode === "rename" ? sourceName
            : (sourceName.length > 0 ? sourceName + " Copy" : "Custom Theme");
        field.forceActiveFocus();
        field.selectAll();
    }

    function commit() {
        if (field.text.trim().length === 0)
            return;
        dialog.accepted(field.text.trim());
        dialog.close();
    }

    Controls.Label {
        text: mode === "rename" ? "NEW NAME" : "THEME NAME"
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
        AccentButton {
            text: dialog.mode === "rename" ? "Rename" : "Duplicate"
            variant: "primary"
            onClicked: dialog.commit()
        }
    ]
}
