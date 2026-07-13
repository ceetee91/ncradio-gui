import QtQuick
import QtQuick.Effects

// Horizontal glow-track slider (volume, progress-style controls).
Item {
    id: root
    property real from: 0
    property real to: 100
    property real value: 0
    property color accentColor: Theme.cyan
    signal moved(real value)

    implicitHeight: 16

    readonly property real ratio: to > from ? Math.max(0, Math.min(1, (value - from) / (to - from))) : 0

    Rectangle {
        id: track
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        height: 6
        radius: 3
        color: Theme.trackBg
    }

    Rectangle {
        anchors.verticalCenter: track.verticalCenter
        anchors.left: track.left
        height: 6
        radius: 3
        width: Math.max(height, track.width * root.ratio)
        color: root.accentColor
    }

    MultiEffect {
        source: knob
        anchors.fill: knob
        shadowEnabled: true
        shadowColor: root.accentColor
        shadowBlur: 0.6
        shadowOpacity: 0.55
        blurMax: 0
    }

    Rectangle {
        id: knob
        width: 16
        height: 16
        radius: 8
        color: Theme.bgBase
        border.width: 2.5
        border.color: root.accentColor
        anchors.verticalCenter: track.verticalCenter
        x: Math.max(0, Math.min(track.width - width, track.width * root.ratio - width / 2))
    }

    MouseArea {
        anchors.fill: parent
        function updateFromX(x) {
            const r = Math.max(0, Math.min(1, x / track.width));
            root.value = root.from + r * (root.to - root.from);
            root.moved(root.value);
        }
        onPressed: (mouse) => updateFromX(mouse.x)
        onPositionChanged: (mouse) => { if (pressed) updateFromX(mouse.x); }
    }
}
