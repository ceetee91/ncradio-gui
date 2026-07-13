import QtQuick
import QtQuick.Effects

// Vertical bipolar slider for one EQ band: range [-12, +12] dB, 0dB at
// vertical center — matches the Penpot equalizer band control.
Item {
    id: root
    property real minValue: -12
    property real maxValue: 12
    property real value: 0
    property color accentColor: value >= 0 ? Theme.cyan : Theme.amber
    signal moved(real value)

    implicitWidth: 20

    readonly property real mid: height / 2
    readonly property real ratio: Math.max(-1, Math.min(1, value / maxValue))

    Rectangle {
        id: track
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 6
        radius: 3
        color: Theme.trackBg
    }

    Rectangle {
        // 0dB centerline
        anchors.horizontalCenter: track.horizontalCenter
        y: track.y + root.mid - 0.5
        width: track.width + 8
        height: 1
        color: Theme.textTertiary
        opacity: 0.5
    }

    Rectangle {
        id: fill
        anchors.horizontalCenter: track.horizontalCenter
        width: track.width
        radius: 3
        color: root.accentColor
        y: track.y + (root.ratio >= 0 ? root.mid - root.mid * root.ratio : root.mid)
        height: Math.max(1, Math.abs(root.mid * root.ratio))
    }

    MultiEffect {
        source: knob
        anchors.fill: knob
        shadowEnabled: true
        shadowColor: root.accentColor
        shadowBlur: 0.5
        shadowOpacity: 0.5
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
        anchors.horizontalCenter: track.horizontalCenter
        y: track.y + root.mid - root.mid * root.ratio - height / 2
    }

    MouseArea {
        anchors.fill: parent
        function updateFromY(y) {
            const local = y - track.y;
            const r = Math.max(-1, Math.min(1, (root.mid - local) / root.mid));
            root.value = r * root.maxValue;
            root.moved(root.value);
        }
        onPressed: (mouse) => updateFromY(mouse.y)
        onPositionChanged: (mouse) => { if (pressed) updateFromY(mouse.y); }
    }
}
