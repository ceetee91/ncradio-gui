#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

extern "C" {
#include "record.h"
}

class ConfigStore;
class AudioController;

// Wraps record.c/record.h + the format-specific settings in Config
// (record_wav_*/record_ogg_*/record_flac_*, legacy record_bitrate/
// record_stereo/record_samplerate for MP3). Installs itself as
// AudioController's Audio.rec_fn while recording, following the documented
// lock/swap/unlock-then-close sequence for stopping.
class RecordController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY recordingChanged)
    Q_PROPERTY(int elapsedSeconds READ elapsedSeconds NOTIFY elapsedChanged)
    Q_PROPERTY(int format READ format WRITE setFormat NOTIFY formatChanged)
    Q_PROPERTY(QStringList availableFormatNames READ availableFormatNames CONSTANT)
    Q_PROPERTY(QString formatExtension READ formatExtension NOTIFY formatChanged)
    Q_PROPERTY(bool applyEqToRecs READ applyEqToRecs WRITE setApplyEqToRecs NOTIFY settingsChanged)
    Q_PROPERTY(bool stereo READ stereo WRITE setStereo NOTIFY settingsChanged)
    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate NOTIFY settingsChanged)
    Q_PROPERTY(double quality READ quality WRITE setQuality NOTIFY settingsChanged)
    // qualityLabel depends on both format and quality; settingsChanged
    // covers both since setFormat() emits it in addition to formatChanged.
    Q_PROPERTY(QString qualityLabel READ qualityLabel NOTIFY settingsChanged)
    Q_PROPERTY(bool resamplingAvailable READ resamplingAvailable CONSTANT)

public:
    explicit RecordController(ConfigStore *configStore, AudioController *audioController, QObject *parent = nullptr);
    ~RecordController() override;

    bool recording() const { return m_record != nullptr; }
    QString currentFile() const { return m_currentFile; }
    int elapsedSeconds() const { return m_elapsedSeconds; }

    int format() const { return m_configStore ? formatFromConfig() : 0; }
    void setFormat(int fmt);
    QStringList availableFormatNames() const;
    QString formatExtension() const;

    bool applyEqToRecs() const;
    void setApplyEqToRecs(bool on);

    bool stereo() const;
    void setStereo(bool on);
    int sampleRate() const;
    void setSampleRate(int rate);
    double quality() const;
    void setQuality(double q);
    QString qualityLabel() const;
    bool resamplingAvailable() const {
#ifdef HAVE_SAMPLERATE
        return true;
#else
        return false;
#endif
    }

    Q_INVOKABLE bool start(const QString &pathWithoutExt);
    Q_INVOKABLE void stop();

signals:
    void recordingChanged();
    void elapsedChanged();
    void formatChanged();
    void settingsChanged();

private:
    int formatFromConfig() const;
    static void recordFeedTrampoline(void *ctx, const short *pcm, int frames, int channels);

    Record *m_record = nullptr;
    QString m_currentFile;
    QTimer m_elapsedTimer;
    int m_elapsedSeconds = 0;

    ConfigStore *m_configStore;
    AudioController *m_audioController;
};
