import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

// Text/icon button matching Penpot's K.button variants: primary (filled
// cyan + glow), secondary (glass + border), ghost (text only), danger
// (red glass + border). Built as a plain Item, like IconButton, to avoid
// fighting Breeze's QQC2 style internals.
Item {
    id: root
    property string text: ""
    property string icon: ""
    property string variant: "primary" // primary | secondary | ghost | danger
    signal clicked()

    implicitWidth: rowLayout.implicitWidth + 36
    implicitHeight: 40
    opacity: enabled ? 1 : 0.5

    readonly property bool glow: variant === "primary" && enabled
    readonly property color bgColor: variant === "primary" ? Theme.cyan
        : variant === "secondary" ? Theme.glassElevated
        : variant === "danger" ? Theme.withAlpha(Theme.red, 0.16)
        : (hoverHandler.hovered ? Theme.glass : "transparent")
    readonly property color borderColor: variant === "secondary" ? Theme.glassBorderStrong
        : variant === "danger" ? Theme.withAlpha(Theme.red, 0.7)
        : "transparent"
    readonly property color textColor: variant === "primary" ? Theme.bgBase
        : variant === "secondary" ? Theme.cyan
        : variant === "danger" ? Theme.red
        : Theme.textSecondary

    MultiEffect {
        source: bg
        anchors.fill: bg
        visible: root.glow
        shadowEnabled: root.glow
        shadowColor: Theme.cyan
        shadowBlur: 0.7
        shadowOpacity: 0.5
        blurMax: 0
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: Theme.radiusMd
        color: root.bgColor
        border.width: (root.variant === "secondary" || root.variant === "danger") ? 1 : 0
        border.color: root.borderColor
    }

    RowLayout {
        id: rowLayout
        anchors.centerIn: parent
        spacing: 8

        AppIcon {
            visible: root.icon.length > 0
            name: root.icon
            color: root.textColor
            Layout.preferredWidth: 15
            Layout.preferredHeight: 15
        }
        Text {
            visible: root.text.length > 0
            text: root.text
            color: root.textColor
            font.family: Theme.fontUi
            font.weight: Font.DemiBold
            font.pointSize: 10.5
        }
    }

    HoverHandler { id: hoverHandler; enabled: root.enabled }
    // Defer emitting clicked() to the next event-loop turn: if the handler
    // closes a Popup this button lives in (e.g. Cancel/Save), doing that
    // synchronously mid-tap can let the same click leak through to
    // whatever's revealed underneath once the popup disappears.
    TapHandler { enabled: root.enabled; onTapped: Qt.callLater(root.clicked) }
}
