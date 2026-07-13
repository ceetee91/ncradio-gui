#include "AudioController.h"
#include "ConfigStore.h"

#include <cstring>

AudioController::AudioController(ConfigStore *configStore, QObject *parent)
    : QObject(parent)
    , m_configStore(configStore)
{
}

AudioController::~AudioController()
{
    audio_stop(&m_audio);
}

bool AudioController::enabled() const
{
    return m_configStore->data().audio_enabled != 0;
}

void AudioController::setEnabled(bool on)
{
    Config &cfg = m_configStore->data();
    if ((cfg.audio_enabled != 0) == on)
        return;
    cfg.audio_enabled = on ? 1 : 0;
    m_configStore->commit();
    emit enabledChanged();
    restart();
}

QString AudioController::statusText() const
{
    if (!m_audio.running)
        return QStringLiteral("Off");
    return QStringLiteral("On (%1Hz %2ch)").arg(m_audio.rate).arg(m_audio.channels);
}

QString AudioController::captureDevice() const
{
    return QString::fromUtf8(m_configStore->data().audio_device);
}

void AudioController::setCaptureDevice(const QString &dev)
{
    Config &cfg = m_configStore->data();
    const QByteArray devUtf8 = dev.toUtf8();
    std::strncpy(cfg.audio_device, devUtf8.constData(), sizeof(cfg.audio_device) - 1);
    cfg.audio_device[sizeof(cfg.audio_device) - 1] = '\0';
    m_configStore->commit();
    emit deviceChanged();
    restart();
}

QString AudioController::playbackDevice() const
{
    return QString::fromUtf8(m_configStore->data().audio_play_device);
}

void AudioController::setPlaybackDevice(const QString &dev)
{
    Config &cfg = m_configStore->data();
    const QByteArray devUtf8 = dev.toUtf8();
    std::strncpy(cfg.audio_play_device, devUtf8.constData(), sizeof(cfg.audio_play_device) - 1);
    cfg.audio_play_device[sizeof(cfg.audio_play_device) - 1] = '\0';
    m_configStore->commit();
    emit deviceChanged();
    restart();
}

int AudioController::bufferFrames() const
{
    return m_configStore->data().audio_buffer_frames;
}

void AudioController::setBufferFrames(int frames)
{
    Config &cfg = m_configStore->data();
    if (cfg.audio_buffer_frames == frames)
        return;
    cfg.audio_buffer_frames = frames;
    m_configStore->commit();
    emit bufferFramesChanged();
    restart();
}

QString AudioController::backendVersion() const
{
#ifdef HAVE_PIPEWIRE
    return QStringLiteral("PipeWire %1").arg(QString::fromUtf8(audio_pipewire_version()));
#else
    return QStringLiteral("ALSA %1").arg(QString::fromUtf8(audio_alsa_version()));
#endif
}

void AudioController::refreshDevices()
{
    char names[AUDIO_DEV_MAX][AUDIO_DEV_NAMELEN];
    char descs[AUDIO_DEV_MAX][AUDIO_DEV_DESCLEN];
    int count = 0;

    audio_enum_devices(names, descs, &count, AUDIO_DEV_MAX);
    m_captureDevices.clear();
    for (int i = 0; i < count; ++i) {
        QVariantMap entry;
        entry[QStringLiteral("name")] = QString::fromUtf8(names[i]);
        entry[QStringLiteral("description")] = QString::fromUtf8(descs[i]);
        m_captureDevices.append(entry);
    }

    audio_enum_play_devices(names, descs, &count, AUDIO_DEV_MAX);
    m_playbackDevices.clear();
    for (int i = 0; i < count; ++i) {
        QVariantMap entry;
        entry[QStringLiteral("name")] = QString::fromUtf8(names[i]);
        entry[QStringLiteral("description")] = QString::fromUtf8(descs[i]);
        m_playbackDevices.append(entry);
    }

    emit devicesEnumerated();
}

void AudioController::applyFromConfig(const QString &radioDevicePath)
{
    Config &cfg = m_configStore->data();
    m_audio.buffer_frames = cfg.audio_buffer_frames;

    if (!cfg.audio_enabled) {
        audio_stop(&m_audio);
        emit runningChanged();
        return;
    }

    if (cfg.audio_device[0] == '\0') {
        const QByteArray radioDevUtf8 = radioDevicePath.toUtf8();
        char detected[sizeof(cfg.audio_device)];
        if (audio_autodetect(radioDevUtf8.constData(), detected, sizeof(detected))) {
            std::strncpy(cfg.audio_device, detected, sizeof(cfg.audio_device) - 1);
            cfg.audio_device[sizeof(cfg.audio_device) - 1] = '\0';
            m_configStore->commit();
            emit deviceChanged();
        } else {
            return; // no device found; stay off
        }
    }

    std::strncpy(m_audio.play_device, cfg.audio_play_device, sizeof(m_audio.play_device) - 1);
    audio_start(&m_audio, cfg.audio_device);
    emit runningChanged();
}

void AudioController::restart()
{
    const Config &cfg = m_configStore->data();
    if (!cfg.audio_enabled) {
        audio_stop(&m_audio);
        emit runningChanged();
        return;
    }
    if (cfg.audio_device[0] == '\0')
        return;
    m_audio.buffer_frames = cfg.audio_buffer_frames;
    std::strncpy(m_audio.play_device, cfg.audio_play_device, sizeof(m_audio.play_device) - 1);
    audio_start(&m_audio, cfg.audio_device);
    emit runningChanged();
}
