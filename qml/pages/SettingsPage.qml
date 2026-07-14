import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs as Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../components"
import "../dialogs"

// Full-screen Settings page: mini-header + left category nav + scrollable
// detail pane covering every field in Config — matches the Penpot
// "Settings" board (replaces the earlier dialog-based version).
Item {
    id: root
    signal backRequested()

    // Which theme the Appearance color editor is currently targeting. Bumped
    // revision forces the swatch bindings (which call Theme.themeColor) to
    // re-evaluate after an edit.
    property string editTarget: ""
    property int editRevision: 0

    // Human-labelled editable color slots — must match ThemeController's
    // editableKeys() order/names.
    readonly property var colorSlots: [
        { key: "bgBase",        label: "Background" },
        { key: "bgBase2",       label: "Background (deep)" },
        { key: "cyan",          label: "Primary accent" },
        { key: "green",         label: "Success accent" },
        { key: "amber",         label: "Warning accent" },
        { key: "red",           label: "Danger accent" },
        { key: "textPrimary",   label: "Text — primary" },
        { key: "textSecondary", label: "Text — secondary" },
    ]

    // Select `name` as the edit target and make it the live preview: activate
    // it for its mode and switch to that mode so edits show across the app.
    function focusTheme(name) {
        if (name.length === 0)
            return;
        root.editTarget = name;
        if (Theme.themeIsDark(name)) {
            Theme.activeDarkThemeName = name;
            Theme.dark = true;
        } else {
            Theme.activeLightThemeName = name;
            Theme.dark = false;
        }
    }

    Component.onCompleted: {
        audio.refreshDevices();
        editTarget = Theme.dark ? Theme.activeDarkThemeName : Theme.activeLightThemeName;
    }

    // Mirrors ncradio.c's M_SETTINGS mode: Esc or 'o' goes back to the
    // main screen.
    Shortcut {
        sequence: "Escape"
        enabled: root.Controls.StackView.status === Controls.StackView.Active
        onActivated: root.backRequested()
    }
    Shortcut {
        sequence: "O"
        enabled: root.Controls.StackView.status === Controls.StackView.Active
        onActivated: root.backRequested()
    }

    MiniHeader {
        id: header
        anchors.top: parent.top
        width: parent.width
        title: "Settings"
        onBackRequested: root.backRequested()
    }

    RowLayout {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        // ---- Category nav rail ----
        Item {
            Layout.preferredWidth: 240
            Layout.fillHeight: true

            Rectangle { anchors.fill: parent; color: Theme.glassElevated }
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.glassBorder }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 4

                Repeater {
                    model: [
                        { label: "General / Tuning", icon: "settings", anchor: generalSection },
                        { label: "Audio", icon: "volume", anchor: audioSection },
                        { label: "Recording", icon: "record", anchor: recordingSection },
                        { label: "Appearance", icon: "theme", anchor: appearanceSection },
                    ]
                    delegate: Item {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40

                        Rectangle {
                            anchors.fill: parent
                            radius: Theme.radiusMd
                            color: navMouse.containsMouse ? Theme.withAlpha(Theme.cyan, 0.12) : "transparent"
                        }
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            spacing: 10
                            AppIcon { name: modelData.icon; color: Theme.textSecondary; Layout.preferredWidth: 16; Layout.preferredHeight: 16 }
                            Text { text: modelData.label; color: Theme.textSecondary; font.family: Theme.fontUi; font.pointSize: 10 }
                        }
                        MouseArea {
                            id: navMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: scrollView.contentItem.contentY = Math.max(0, modelData.anchor.y - 8)
                        }
                    }
                }
            }
        }

        // ---- Detail pane ----
        Controls.ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth

            Kirigami.FormLayout {
                width: scrollView.availableWidth - 64
                x: 32

                // Kirigami.FormData labels/section headers are rendered by
                // Kirigami itself using Kirigami.Theme.*, which follows the
                // system Plasma color scheme — not our own light/dark
                // toggle — so without this override they stay whatever
                // (often near-white) the system scheme dictates,
                // unreadable against our light-mode background.
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.textColor: Theme.textPrimary
                Kirigami.Theme.disabledTextColor: Theme.textTertiary

                Item { id: generalSection; Kirigami.FormData.isSection: true; Kirigami.FormData.label: "General / Tuning" }

                Controls.ComboBox {
                    Kirigami.FormData.label: "Scan step:"
                    model: ["0.05 MHz", "0.10 MHz", "0.20 MHz"]
                    readonly property var values: [0.05, 0.10, 0.20]
                    Component.onCompleted: currentIndex = values.indexOf(Math.round(configStore.scanStepMhz * 1000) / 1000)
                    onActivated: configStore.scanStepMhz = values[currentIndex]
                }
                Controls.SpinBox {
                    Kirigami.FormData.label: "Signal threshold:"
                    from: 5; to: 95; stepSize: 5
                    value: configStore.signalThresholdPct
                    textFromValue: (v) => v + "%"
                    onValueModified: configStore.signalThresholdPct = value
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Save RDS names:"
                    checked: configStore.rdsNames
                    onToggled: configStore.rdsNames = checked
                }

                Item { id: audioSection; Kirigami.FormData.isSection: true; Kirigami.FormData.label: "Audio" }

                Controls.Label {
                    Kirigami.FormData.label: "Status:"
                    text: audio.statusText + (audio.errorMessage.length > 0 ? " — " + audio.errorMessage : "")
                    color: audio.running ? Theme.green : Theme.textSecondary
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Audio output:"
                    checked: audio.enabled
                    onToggled: audio.enabled = checked
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Capture device:"
                    model: audio.captureDevices.map(d => d.description)
                    Component.onCompleted: {
                        const i = audio.captureDevices.findIndex(d => d.name === audio.captureDevice);
                        if (i >= 0) currentIndex = i;
                    }
                    onActivated: audio.captureDevice = audio.captureDevices[currentIndex].name
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Playback device:"
                    model: audio.playbackDevices.map(d => d.description)
                    Component.onCompleted: {
                        const i = audio.playbackDevices.findIndex(d => d.name === audio.playbackDevice);
                        if (i >= 0) currentIndex = i;
                    }
                    onActivated: audio.playbackDevice = audio.playbackDevices[currentIndex].name
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Buffer size:"
                    readonly property var values: audio.bufferSizeOptions
                    model: values.map(v => audio.bufferSizeLabel(v))
                    Component.onCompleted: currentIndex = values.indexOf(audio.bufferFrames)
                    onActivated: audio.bufferFrames = values[currentIndex]
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Mute while scanning:"
                    checked: configStore.audioMuteScan
                    onToggled: configStore.audioMuteScan = checked
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Mute while seeking:"
                    checked: configStore.audioMuteSeek
                    onToggled: configStore.audioMuteSeek = checked
                }

                Item { id: recordingSection; Kirigami.FormData.isSection: true; Kirigami.FormData.label: "Recording" }

                RowLayout {
                    Kirigami.FormData.label: "Destination folder:"
                    Layout.fillWidth: true
                    spacing: 8
                    Controls.TextField {
                        id: destinationField
                        Layout.fillWidth: true
                        font.family: Theme.fontMono
                        Component.onCompleted: text = recorder.destinationFolder
                        onEditingFinished: recorder.destinationFolder = text
                    }
                    AccentButton {
                        text: "Browse…"
                        variant: "secondary"
                        onClicked: folderDialog.open()
                    }
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Don't ask for filename:"
                    checked: recorder.skipFilenamePrompt
                    onToggled: recorder.skipFilenamePrompt = checked
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Record format:"
                    model: recorder.availableFormatNames
                    currentIndex: recorder.format
                    onActivated: recorder.format = currentIndex
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Stereo:"
                    checked: recorder.stereo
                    onToggled: recorder.stereo = checked
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Sample rate:"
                    model: ["22050 Hz", "44100 Hz", "48000 Hz"]
                    readonly property var values: [22050, 44100, 48000]
                    Component.onCompleted: currentIndex = values.indexOf(recorder.sampleRate)
                    onActivated: recorder.sampleRate = values[currentIndex]
                }
                RowLayout {
                    Kirigami.FormData.label: recorder.format === 1 ? "Bitrate:"
                        : recorder.format === 2 ? "Quality:"
                        : recorder.format === 3 ? "Compression:" : "Quality:"
                    Layout.fillWidth: true
                    spacing: 12
                    enabled: recorder.format !== 0

                    // Snap to the actual supported values per format, rather
                    // than an arbitrary continuous range.
                    readonly property var mp3Values: [64, 96, 128, 160, 192, 224, 256, 320]
                    readonly property var oggValues: [-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                    readonly property var flacValues: [0, 1, 2, 3, 4, 5, 6, 7, 8]
                    readonly property var activeValues: recorder.format === 1 ? mp3Values
                        : recorder.format === 2 ? oggValues
                        : recorder.format === 3 ? flacValues : [0]

                    function nearestIndex(arr, val) {
                        let bestIdx = 0, bestDiff = Infinity;
                        for (let i = 0; i < arr.length; i++) {
                            const diff = Math.abs(arr[i] - val);
                            if (diff < bestDiff) { bestDiff = diff; bestIdx = i; }
                        }
                        return bestIdx;
                    }

                    Controls.Slider {
                        id: qualitySlider
                        Layout.fillWidth: true
                        from: 0
                        to: parent.activeValues.length - 1
                        stepSize: 1
                        snapMode: Controls.Slider.SnapAlways
                        value: parent.nearestIndex(parent.activeValues, recorder.quality)
                        onMoved: recorder.quality = parent.activeValues[Math.round(value)]
                    }
                    Controls.Label {
                        text: recorder.qualityLabel
                        color: Theme.cyan
                        font.family: Theme.fontMono
                        Layout.preferredWidth: 90
                    }
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Apply EQ to recordings:"
                    checked: recorder.applyEqToRecs
                    onToggled: recorder.applyEqToRecs = checked
                }

                Item { id: appearanceSection; Kirigami.FormData.isSection: true; Kirigami.FormData.label: "Appearance" }

                Controls.ComboBox {
                    Kirigami.FormData.label: "Dark theme:"
                    model: Theme.darkThemes
                    currentIndex: Theme.darkThemes.indexOf(Theme.activeDarkThemeName)
                    onActivated: Theme.activeDarkThemeName = Theme.darkThemes[currentIndex]
                }
                Controls.ComboBox {
                    Kirigami.FormData.label: "Light theme:"
                    model: Theme.lightThemes
                    currentIndex: Theme.lightThemes.indexOf(Theme.activeLightThemeName)
                    onActivated: Theme.activeLightThemeName = Theme.lightThemes[currentIndex]
                }
                Controls.Switch {
                    Kirigami.FormData.label: "Preview dark mode:"
                    checked: Theme.dark
                    onToggled: Theme.dark = checked
                }

                Item { Kirigami.FormData.isSection: true; Kirigami.FormData.label: "Customize" }

                RowLayout {
                    Kirigami.FormData.label: "Edit theme:"
                    Layout.fillWidth: true
                    spacing: 8

                    Controls.ComboBox {
                        id: editCombo
                        Layout.fillWidth: true
                        model: Theme.allThemes
                        currentIndex: Theme.allThemes.indexOf(root.editTarget)
                        onActivated: root.focusTheme(Theme.allThemes[currentIndex])
                    }
                    AccentButton {
                        text: "Duplicate…"
                        variant: "secondary"
                        onClicked: {
                            themeNameDialog.mode = "duplicate";
                            themeNameDialog.sourceName = root.editTarget;
                            themeNameDialog.open();
                        }
                    }
                }

                RowLayout {
                    Kirigami.FormData.label: " "
                    spacing: 8
                    visible: root.editTarget.length > 0 && !Theme.isBuiltin(root.editTarget)

                    AccentButton {
                        text: "Rename…"
                        variant: "ghost"
                        onClicked: {
                            themeNameDialog.mode = "rename";
                            themeNameDialog.sourceName = root.editTarget;
                            themeNameDialog.open();
                        }
                    }
                    AccentButton {
                        text: "Delete"
                        icon: "trash"
                        variant: "danger"
                        onClicked: deleteThemeDialog.open()
                    }
                }

                Controls.Label {
                    Kirigami.FormData.label: " "
                    visible: root.editTarget.length > 0 && Theme.isBuiltin(root.editTarget)
                    text: "Built-in preset — Duplicate it to customize colors."
                    color: Theme.textTertiary
                    font.pointSize: 9
                    wrapMode: Text.WordWrap
                }

                // Curated 8-color editor. Each row: label + click-to-edit
                // swatch. Enabled only for custom themes; built-ins are shown
                // read-only.
                Repeater {
                    model: root.colorSlots
                    delegate: RowLayout {
                        required property var modelData
                        Kirigami.FormData.label: modelData.label + ":"
                        Layout.fillWidth: true
                        spacing: 12
                        readonly property bool editable: root.editTarget.length > 0 && !Theme.isBuiltin(root.editTarget)

                        Rectangle {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 28
                            radius: Theme.radiusSm
                            border.width: 1
                            border.color: Theme.glassBorderStrong
                            // editRevision forces re-evaluation after edits.
                            color: (root.editRevision, root.editTarget.length > 0
                                ? Theme.themeColor(root.editTarget, modelData.key) : "transparent")
                            opacity: parent.editable ? 1 : 0.5

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: parent.parent.editable ? Qt.PointingHandCursor : Qt.ArrowCursor
                                enabled: parent.parent.editable
                                onClicked: {
                                    colorDialog.editKey = modelData.key;
                                    colorDialog.selectedColor = Theme.themeColor(root.editTarget, modelData.key);
                                    colorDialog.open();
                                }
                            }
                        }
                        Controls.Label {
                            // editRevision forces re-evaluation after edits.
                            text: (root.editRevision, root.editTarget.length > 0
                                ? Theme.themeColor(root.editTarget, modelData.key).toString().toUpperCase() : "")
                            color: Theme.textTertiary
                            font.family: Theme.fontMono
                            font.pointSize: 9
                        }
                    }
                }

                Item { Kirigami.FormData.isSection: false; height: 32 }
            }
        }
    }

    ThemeNameDialog {
        id: themeNameDialog
        onAccepted: (name) => {
            if (mode === "rename") {
                Theme.renameCustomTheme(root.editTarget, name);
                // renameCustomTheme uniquifies; re-resolve to the active name.
                root.editTarget = Theme.dark ? Theme.activeDarkThemeName : Theme.activeLightThemeName;
            } else {
                const created = Theme.duplicateTheme(root.editTarget, name);
                if (created.length > 0)
                    root.editTarget = created;
            }
        }
    }

    DeleteConfirmDialog {
        id: deleteThemeDialog
        dialogTitle: "Delete Theme"
        message: "Delete the custom theme \"" + root.editTarget + "\"? This cannot be undone."
        onConfirmed: {
            Theme.deleteCustomTheme(root.editTarget);
            root.editTarget = Theme.dark ? Theme.activeDarkThemeName : Theme.activeLightThemeName;
        }
    }

    Dialogs.ColorDialog {
        id: colorDialog
        property string editKey: ""
        onAccepted: {
            Theme.setThemeColor(root.editTarget, editKey, selectedColor);
            root.editRevision++;
        }
    }

    Dialogs.FolderDialog {
        id: folderDialog
        currentFolder: Qt.resolvedUrl("file://" + recorder.destinationFolder)
        onAccepted: {
            recorder.setDestinationFolderFromUrl(selectedFolder);
            destinationField.text = recorder.destinationFolder;
        }
    }
}
