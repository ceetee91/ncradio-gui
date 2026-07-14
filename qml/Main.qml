import QtQuick
import QtCore
import QtQuick.Controls as Controls
import QtQuick.Effects
import org.kde.kirigami as Kirigami
import "components"
import "pages"

Kirigami.ApplicationWindow {
    id: root
    title: Qt.application.name + " v" + Qt.application.version
    // Smallest default size that keeps the layout intact: height sized so
    // ~8 station rows are fully visible in the sidebar without scrolling.
    width: 1040
    height: 540
    minimumWidth: 1040
    minimumHeight: 540
    visible: true

    // --- Minimal ("mini player") mode ---
    property bool minimalMode: false
    // Remembered across sessions (see Settings below); the on-top hint only
    // actually applies while minimal — it's a mini-player control.
    property bool alwaysOnTop: false
    // Full-size dimensions to restore on leaving minimal mode.
    property int savedWidth: width
    property int savedHeight: height

    flags: (minimalMode && alwaysOnTop) ? (Qt.Window | Qt.WindowStaysOnTopHint) : Qt.Window

    Settings {
        category: "MinimalMode"
        property alias alwaysOnTop: root.alwaysOnTop
    }

    onMinimalModeChanged: {
        if (minimalMode) {
            savedWidth = width;
            savedHeight = height;
            // Lower the minimum BEFORE shrinking, or Qt clamps against it.
            minimumWidth = 300;
            minimumHeight = 190;
            width = 340;
            height = 220;
        } else {
            // Restore the minimum first; the saved size is always ≥ it.
            minimumWidth = 1040;
            minimumHeight = 540;
            width = savedWidth;
            height = savedHeight;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.bgBase
        z: -10
    }

    // Clicking anywhere that isn't itself an interactive control (e.g. the
    // background behind the sidebar's search field) drops focus back to a
    // plain, non-text item — otherwise a TextField keeps active focus
    // indefinitely once clicked into, permanently disabling every keyboard
    // shortcut (which all check for a focused text field) even after the
    // user has clicked elsewhere.
    MouseArea {
        anchors.fill: parent
        z: -9
        onClicked: forceActiveFocus()
    }

    // ambient glow blobs (shared background across all pages) — real blur,
    // so translucent glass panels have soft light to visibly catch.
    Rectangle {
        x: -120; y: -160; width: 520; height: 520; radius: width / 2
        color: Theme.withAlpha(Theme.cyan, Theme.dark ? 0.22 : 0.16)
        visible: true
        layer.enabled: true
        layer.effect: MultiEffect { blurEnabled: true; blur: 1.0; blurMax: 96 }
    }
    Rectangle {
        x: root.width - 380; y: root.height - 420; width: 600; height: 600; radius: width / 2
        color: Theme.withAlpha(Theme.green, Theme.dark ? 0.16 : 0.12)
        visible: true
        layer.enabled: true
        layer.effect: MultiEffect { blurEnabled: true; blur: 1.0; blurMax: 96 }
    }

    // Mirrors ncradio.c: pause the audio pipe while scanning/seeking if the
    // corresponding "mute while..." setting is on, and resume it once done.
    Connections {
        target: radio
        function onScanningChanged() {
            if (!configStore.audioMuteScan)
                return;
            if (radio.scanning) audio.pauseStream();
            else audio.resumeStream();
        }
        function onSeekingChanged() {
            if (!configStore.audioMuteSeek)
                return;
            if (radio.seeking) audio.pauseStream();
            else audio.resumeStream();
        }
    }

    Controls.StackView {
        id: stackView
        anchors.fill: parent
        // Hidden in minimal mode; its NowPlayingView shortcuts also go dormant
        // via that view's `minimalMode`-folded pageActive gate.
        visible: !root.minimalMode
        // Disabled while any GlassDialog is open: see GlassDialog.qml for
        // why this is needed (Pointer Handlers don't respect Popup modality).
        enabled: Theme.modalDepth === 0 && !root.minimalMode
        initialItem: nowPlayingComponent

        popEnter: Transition {}
        popExit: Transition {}
        pushEnter: Transition {}
        pushExit: Transition {}
    }

    // Only instantiated while minimal, so MinimalView's keyboard shortcuts
    // exist only then and never collide with NowPlayingView's identical ones.
    Loader {
        anchors.fill: parent
        z: 10
        active: root.minimalMode
        sourceComponent: minimalComponent
    }

    Component {
        id: nowPlayingComponent
        NowPlayingView {
            minimalMode: root.minimalMode
            onSettingsRequested: stackView.push(settingsComponent)
            onEqualizerRequested: stackView.push(equalizerComponent)
            onMinimalModeRequested: root.minimalMode = true
        }
    }
    Component {
        id: minimalComponent
        MinimalView {
            alwaysOnTop: root.alwaysOnTop
            onToggleAlwaysOnTop: root.alwaysOnTop = !root.alwaysOnTop
            onExitRequested: root.minimalMode = false
        }
    }
    Component {
        id: settingsComponent
        SettingsPage { onBackRequested: stackView.pop() }
    }
    Component {
        id: equalizerComponent
        EqualizerPage { onBackRequested: stackView.pop() }
    }
}
