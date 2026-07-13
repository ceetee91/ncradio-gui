import QtQuick
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

    Rectangle {
        anchors.fill: parent
        color: Theme.bgBase
        z: -10
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
        // Disabled while any GlassDialog is open: see GlassDialog.qml for
        // why this is needed (Pointer Handlers don't respect Popup modality).
        enabled: Theme.modalDepth === 0
        initialItem: nowPlayingComponent

        popEnter: Transition {}
        popExit: Transition {}
        pushEnter: Transition {}
        pushExit: Transition {}
    }

    Component {
        id: nowPlayingComponent
        NowPlayingView {
            onSettingsRequested: stackView.push(settingsComponent)
            onEqualizerRequested: stackView.push(equalizerComponent)
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
