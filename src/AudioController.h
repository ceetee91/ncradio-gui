#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

extern "C" {
#include "audio.h"
}

class ConfigStore;

// Wraps audio.c/audio.h — the PipeWire/ALSA capture->playback pipe and
// device enumeration/autodetection.
class AudioController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY runningChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY runningChanged)
    Q_PROPERTY(QString captureDevice READ captureDevice WRITE setCaptureDevice NOTIFY deviceChanged)
    Q_PROPERTY(QString playbackDevice READ playbackDevice WRITE setPlaybackDevice NOTIFY deviceChanged)
    Q_PROPERTY(QVariantList captureDevices READ captureDevices NOTIFY devicesEnumerated)
    Q_PROPERTY(QVariantList playbackDevices READ playbackDevices NOTIFY devicesEnumerated)
    Q_PROPERTY(int bufferFrames READ bufferFrames WRITE setBufferFrames NOTIFY bufferFramesChanged)
    Q_PROPERTY(QString backendVersion READ backendVersion CONSTANT)
    Q_PROPERTY(QString deviceAutodetectMethod READ deviceAutodetectMethod CONSTANT)

public:
    explicit AudioController(ConfigStore *configStore, QObject *parent = nullptr);
    ~AudioController() override;

    Audio *raw() { return &m_audio; }
    void setEq(struct Eq *eq) { m_audio.eq = eq; }

    bool enabled() const;
    void setEnabled(bool on);

    bool running() const { return m_audio.running != 0; }
    QString statusText() const;
    QString errorMessage() const { return QString::fromUtf8(m_audio.errmsg); }

    QString captureDevice() const;
    void setCaptureDevice(const QString &dev);
    QString playbackDevice() const;
    void setPlaybackDevice(const QString &dev);

    QVariantList captureDevices() const { return m_captureDevices; }
    QVariantList playbackDevices() const { return m_playbackDevices; }

    int bufferFrames() const;
    void setBufferFrames(int frames);

    QString backendVersion() const;
    QString deviceAutodetectMethod() const {
#ifdef HAVE_UDEV
        return QStringLiteral("udev + sysfs fallback");
#else
        return QStringLiteral("sysfs");
#endif
    }

    // Mirrors ncradio.c's audio_apply(): autodetects a device if none is
    // configured yet, then starts the pipe if audio_enabled.
    Q_INVOKABLE void applyFromConfig(const QString &radioDevicePath);
    Q_INVOKABLE void refreshDevices();

    // Transient stop/restart for the "mute while scanning/seeking" settings
    // — mirrors ncradio.c's audio_stop()/audio_apply() calls around scan and
    // seek. Unlike setEnabled(false), these don't touch the persisted
    // audio_enabled config value.
    Q_INVOKABLE void pauseStream();
    Q_INVOKABLE void resumeStream();

signals:
    void enabledChanged();
    void runningChanged();
    void deviceChanged();
    void devicesEnumerated();
    void bufferFramesChanged();

private:
    void restart();

    Audio m_audio{};
    ConfigStore *m_configStore;
    QVariantList m_captureDevices;
    QVariantList m_playbackDevices;
};
