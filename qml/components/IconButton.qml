import QtQuick
import QtQuick.Controls as Controls

// Glass icon-only button matching Penpot's K.iconButton: transparent by
// default, glass fill on hover, cyan glass+border when active. Built as a
// plain Item rather than styling QQC2 ToolButton, since Breeze's style
// internals expect its own IconLabel-based contentItem API.
Item {
    id: root
    property string icon: ""
    property bool active: false
    property real size: 40
    property string tooltipText: ""
    // Idle icon tint; override for higher contrast where textSecondary reads
    // too dim against a surface (e.g. the minimal view's glass panel).
    property color iconColor: Theme.textSecondary
    signal clicked()

    implicitWidth: size
    implicitHeight: size
    opacity: enabled ? 1 : 0.4

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusMd
        color: root.active ? Theme.withAlpha(Theme.cyan, 0.18)
             : hoverHandler.hovered ? Theme.glass : "transparent"
        border.width: root.active ? 1 : 0
        border.color: Theme.withAlpha(Theme.cyan, 0.6)
    }

    AppIcon {
        anchors.centerIn: parent
        width: root.size * 0.46
        height: root.size * 0.46
        name: root.icon
        color: root.active ? Theme.cyan : root.iconColor
    }

    HoverHandler { id: hoverHandler; enabled: root.enabled }
    // Deferred for the same reason as AccentButton — avoids click-through
    // to whatever's underneath when this button closes a Popup.
    TapHandler { enabled: root.enabled; onTapped: Qt.callLater(root.clicked) }

    Controls.ToolTip {
        text: root.tooltipText
        visible: hoverHandler.hovered && text.length > 0
        delay: 500
    }
}
