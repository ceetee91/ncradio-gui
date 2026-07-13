import QtQuick
import QtQuick.Effects

// Reusable translucent "glass" container: semi-transparent fill, 1px
// border, soft drop shadow (or accent glow) — matches the Penpot glass
// panel treatment used throughout the design. Children are added normally
// (Item's own built-in default "data" property) and render above bg/shadow
// since they're declared after them.
Item {
    id: root
    property bool elevated: false
    property bool glow: false
    property alias radius: bg.radius
    property alias color: bg.color
    property alias border: bg.border

    MultiEffect {
        source: bg
        anchors.fill: bg
        shadowEnabled: true
        shadowColor: root.glow ? Theme.cyan : Theme.shadowColor
        shadowBlur: root.glow ? 0.7 : 0.4
        shadowOpacity: root.glow ? 0.45 : 0.22
        shadowVerticalOffset: root.glow ? 0 : 8
        shadowHorizontalOffset: 0
        blurMax: 0
        autoPaddingEnabled: true
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: Theme.radiusXl
        color: root.elevated ? Theme.glassElevated : Theme.glass
        border.width: 1
        border.color: root.glow ? Theme.glassBorderStrong : Theme.glassBorder
    }

    // Subtle top-edge highlight — light catching the top of the glass —
    // to sell the surface as glass rather than a flat tinted rectangle.
    Rectangle {
        anchors.fill: bg
        radius: bg.radius
        gradient: Gradient {
            GradientStop { position: 0.0; color: Theme.withAlpha("#FFFFFF", Theme.dark ? 0.10 : 0.35) }
            GradientStop { position: 0.4; color: "transparent" }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
}
