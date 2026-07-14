#include "RecordController.h"
#include "ConfigStore.h"
#include "AudioController.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

RecordController::RecordController(ConfigStore *configStore, AudioController *audioController, QObject *parent)
    : QObject(parent)
    , m_configStore(configStore)
    , m_audioController(audioController)
    , m_settings(QStringLiteral("ceetee91"), QStringLiteral("ncradio-gui"))
{
    m_elapsedTimer.setInterval(1000);
    connect(&m_elapsedTimer, &QTimer::timeout, this, [this] {
        ++m_elapsedSeconds;
        emit elapsedChanged();
    });
}

RecordController::~RecordController()
{
    if (m_record)
        stop();
}

int RecordController::formatFromConfig() const
{
    return m_configStore->data().record_format;
}

void RecordController::setFormat(int fmt)
{
    if (fmt < 0 || fmt >= RECORD_FMT_COUNT || fmt == formatFromConfig())
        return;
    m_configStore->data().record_format = fmt;
    m_configStore->commit();
    emit formatChanged();
    emit settingsChanged();
}

QStringList RecordController::availableFormatNames() const
{
    QStringList names;
    for (int i = 0; i < record_format_count; ++i)
        names.append(QString::fromUtf8(record_format_name(record_formats[i])));
    return names;
}

QString RecordController::formatExtension() const
{
    return QString::fromUtf8(record_extension(static_cast<RecordFormat>(formatFromConfig())));
}

bool RecordController::applyEqToRecs() const
{
    return m_configStore->data().record_eq_enabled != 0;
}

void RecordController::setApplyEqToRecs(bool on)
{
    Config &cfg = m_configStore->data();
    cfg.record_eq_enabled = on ? 1 : 0;
    m_configStore->commit();
    emit settingsChanged();
}

bool RecordController::stereo() const
{
    const Config &cfg = m_configStore->data();
    switch (formatFromConfig()) {
    case RECORD_FMT_WAV: return cfg.record_wav_stereo != 0;
    case RECORD_FMT_MP3: return cfg.record_stereo != 0;
    case RECORD_FMT_OGG: return cfg.record_ogg_stereo != 0;
    case RECORD_FMT_FLAC: return cfg.record_flac_stereo != 0;
    default: return true;
    }
}

void RecordController::setStereo(bool on)
{
    Config &cfg = m_configStore->data();
    switch (formatFromConfig()) {
    case RECORD_FMT_WAV: cfg.record_wav_stereo = on ? 1 : 0; break;
    case RECORD_FMT_MP3: cfg.record_stereo = on ? 1 : 0; break;
    case RECORD_FMT_OGG: cfg.record_ogg_stereo = on ? 1 : 0; break;
    case RECORD_FMT_FLAC: cfg.record_flac_stereo = on ? 1 : 0; break;
    }
    m_configStore->commit();
    emit settingsChanged();
}

int RecordController::sampleRate() const
{
    const Config &cfg = m_configStore->data();
    switch (formatFromConfig()) {
    case RECORD_FMT_WAV: return cfg.record_wav_samplerate;
    case RECORD_FMT_MP3: return cfg.record_samplerate;
    case RECORD_FMT_OGG: return cfg.record_ogg_samplerate;
    case RECORD_FMT_FLAC: return cfg.record_flac_samplerate;
    default: return 44100;
    }
}

void RecordController::setSampleRate(int rate)
{
    Config &cfg = m_configStore->data();
    switch (formatFromConfig()) {
    case RECORD_FMT_WAV: cfg.record_wav_samplerate = rate; break;
    case RECORD_FMT_MP3: cfg.record_samplerate = rate; break;
    case RECORD_FMT_OGG: cfg.record_ogg_samplerate = rate; break;
    case RECORD_FMT_FLAC: cfg.record_flac_samplerate = rate; break;
    }
    m_configStore->commit();
    emit settingsChanged();
}

double RecordController::quality() const
{
    const Config &cfg = m_configStore->data();
    switch (formatFromConfig()) {
    case RECORD_FMT_MP3: return cfg.record_bitrate;
    case RECORD_FMT_OGG: return cfg.record_ogg_quality;
    case RECORD_FMT_FLAC: return cfg.record_flac_level;
    default: return 0;
    }
}

void RecordController::setQuality(double q)
{
    Config &cfg = m_configStore->data();
    switch (formatFromConfig()) {
    case RECORD_FMT_MP3: cfg.record_bitrate = int(q); break;
    case RECORD_FMT_OGG: cfg.record_ogg_quality = float(q); break;
    case RECORD_FMT_FLAC: cfg.record_flac_level = int(q); break;
    default: return;
    }
    m_configStore->commit();
    emit settingsChanged();
}

QString RecordController::qualityLabel() const
{
    switch (formatFromConfig()) {
    case RECORD_FMT_MP3: return QStringLiteral("%1 kbps").arg(int(quality()));
    case RECORD_FMT_OGG: return QStringLiteral("Quality %1").arg(quality(), 0, 'f', 1);
    case RECORD_FMT_FLAC: return QStringLiteral("Level %1").arg(int(quality()));
    default: return QStringLiteral("N/A");
    }
}

QString RecordController::destinationFolder() const
{
    QString dir = m_settings.value(QStringLiteral("Recording/destinationFolder")).toString();
    if (dir.isEmpty()) {
        dir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        if (dir.isEmpty())
            dir = QDir::homePath();
    }
    return dir;
}

void RecordController::setDestinationFolder(const QString &path)
{
    if (path == destinationFolder())
        return;
    m_settings.setValue(QStringLiteral("Recording/destinationFolder"), path);
    emit settingsChanged();
}

void RecordController::setDestinationFolderFromUrl(const QUrl &url)
{
    setDestinationFolder(url.toLocalFile());
}

bool RecordController::skipFilenamePrompt() const
{
    return m_settings.value(QStringLiteral("Recording/skipFilenamePrompt"), false).toBool();
}

void RecordController::setSkipFilenamePrompt(bool on)
{
    if (on == skipFilenamePrompt())
        return;
    m_settings.setValue(QStringLiteral("Recording/skipFilenamePrompt"), on);
    emit settingsChanged();
}

QString RecordController::fullPath(const QString &name) const
{
    return QDir(destinationFolder()).filePath(name);
}

void RecordController::recordFeedTrampoline(void *ctx, const short *pcm, int frames, int /*channels*/)
{
    record_feed(static_cast<Record *>(ctx), pcm, frames);
}

bool RecordController::start(const QString &pathWithoutExt)
{
    if (m_record || !m_audioController->running())
        return false;

    Audio *audio = m_audioController->raw();
    const auto fmt = static_cast<RecordFormat>(formatFromConfig());
    const int outChannels = stereo() ? 2 : 1;
    const int outRate = sampleRate();
    const double q = quality();

    QString path = pathWithoutExt;
    const QString ext = formatExtension(); // already includes the leading dot (record_extension())
    if (!path.endsWith(ext, Qt::CaseInsensitive))
        path += ext;

    QDir().mkpath(QFileInfo(path).absolutePath());

    char errbuf[256] = {0};
    Record *r = record_open(fmt, path.toUtf8().constData(),
                             int(audio->rate), audio->channels,
                             outRate, outChannels, q,
                             errbuf, sizeof(errbuf));
    if (!r)
        return false;

    pthread_mutex_lock(&audio->rec_lock);
    audio->rec_fn = &RecordController::recordFeedTrampoline;
    audio->rec_ctx = r;
    audio->rec_eq_apply = applyEqToRecs() ? 1 : 0;
    pthread_mutex_unlock(&audio->rec_lock);

    m_record = r;
    m_currentFile = path;
    m_elapsedSeconds = 0;
    m_elapsedTimer.start();
    emit recordingChanged();
    emit elapsedChanged();
    return true;
}

void RecordController::stop()
{
    if (!m_record)
        return;

    Audio *audio = m_audioController->raw();
    pthread_mutex_lock(&audio->rec_lock);
    Record *r = static_cast<Record *>(audio->rec_ctx);
    audio->rec_fn = nullptr;
    audio->rec_ctx = nullptr;
    pthread_mutex_unlock(&audio->rec_lock);

    // record_close happens outside the lock, guaranteeing the audio thread
    // isn't mid-callback into it (documented safe sequence).
    record_close(r);

    m_record = nullptr;
    m_elapsedTimer.stop();
    emit recordingChanged();
}
