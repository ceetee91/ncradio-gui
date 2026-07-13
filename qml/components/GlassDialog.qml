import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

// Custom modal dialog matching the Penpot "dialogShell" treatment: a glass
// panel with glow, a title row with close button, a separator, then
// content and an optional footer row — replacing Kirigami.Dialog's default
// (opaque, non-glass) chrome.
Controls.Popup {
    id: root

    default property alias dialogContent: contentLayout.data
    property alias footerData: footerRow.data
    property alias contentSpacing: contentLayout.spacing
    property string dialogTitle: ""
    property real dialogWidth: 420

    parent: Controls.Overlay.overlay
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: dialogWidth
    padding: 0
    modal: true
    dim: true
    focus: true
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

    // Item-based TapHandlers (used by our custom buttons) don't stop event
    // delivery to items stacked below them the way MouseArea does, so a tap
    // inside this dialog can also land on a background button at the same
    // screen position. Disable the rest of the window for the duration.
    Connections {
        target: root
        function onOpened() { Theme.pushModal() }
        function onClosed() { Theme.popModal() }
    }

    background: GlassPanel {
        elevated: true
        glow: true
        color: Theme.modalSurface
    }

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 24
            Layout.bottomMargin: 16

            Text {
                text: root.dialogTitle
                color: Theme.textPrimary
                font.family: Theme.fontUi
                font.weight: Font.Bold
                font.pointSize: 13
                Layout.fillWidth: true
            }
            IconButton { icon: "close"; size: 32; onClicked: root.close() }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            height: 1
            color: Theme.glassBorder
        }

        ColumnLayout {
            id: contentLayout
            Layout.fillWidth: true
            Layout.margins: 24
            Layout.topMargin: 16
            spacing: 8
        }

        RowLayout {
            id: footerRow
            Layout.fillWidth: true
            Layout.margins: 24
            Layout.topMargin: 8
            Layout.alignment: Qt.AlignRight
            visible: children.length > 0
        }
    }
}
