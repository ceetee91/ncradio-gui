#include "RadioController.h"
#include "ConfigStore.h"

#include <QDebug>
#include <cstring>

RadioController::RadioController(ConfigStore *configStore, QObject *parent)
    : QObject(parent)
    , m_configStore(configStore)
{
    m_pollTimer.setInterval(500);
    connect(&m_pollTimer, &QTimer::timeout, this, &RadioController::poll);

    m_statusTimer.setSingleShot(true);
    connect(&m_statusTimer, &QTimer::timeout, this, [this] {
        m_statusMessage.clear();
        emit statusMessageChanged();
    });
}

RadioController::~RadioController()
{
    if (m_ready) {
        radio_stop_scan(&m_radio);
        radio_stop_seek(&m_radio);
        radio_mute(&m_radio, 1);
        radio_close(&m_radio);
    }
}

bool RadioController::open(const QString &device)
{
    const QByteArray devUtf8 = device.toUtf8();
    if (radio_open(&m_radio, devUtf8.constData()) != 0) {
        m_errorMessage = QStringLiteral("Could not open %1").arg(device);
        m_ready = false;
        emit errorChanged();
        emit readyChanged();
        return false;
    }

    m_ready = true;
    emit readyChanged();

    // Restore saved volume/frequency, matching ncradio.c's startup sequence.
    const Config &cfg = m_configStore->data();
    if (cfg.volume > 0)
        radio_set_volume(&m_radio, cfg.volume);
    if (cfg.last_freq_hz >= m_radio.freq_min_hz && cfg.last_freq_hz <= m_radio.freq_max_hz)
        radio_set_freq(&m_radio, cfg.last_freq_hz);

    emit frequencyChanged();
    emit volumeChanged();

    poll();
    m_pollTimer.start();
    return true;
}

void RadioController::poll()
{
    if (!m_ready)
        return;

    if (m_radio.scanning) {
        pthread_mutex_lock(&m_radio.mutex);
        const double range = double(m_radio.freq_max_hz) - double(m_radio.freq_min_hz);
        m_scanProgress = range > 0
            ? (double(m_radio.scan_pos_hz) - double(m_radio.freq_min_hz)) / range
            : 0.0;
        refreshFoundStationsLocked();
        pthread_mutex_unlock(&m_radio.mutex);
        emit scanProgressChanged();
    } else if (m_radio.scan_started) {
        finishScan();
    }

    if (!m_radio.seeking && m_radio.seek_started) {
        finishSeek();
    }

    if (!m_radio.scanning && !m_radio.seeking) {
        const int sig = radio_get_signal(&m_radio);
        if (sig != m_signalPercent) {
            m_signalPercent = sig;
            emit signalChanged();
        } else {
            emit signalChanged(); // stereo may have changed even if % didn't
        }
        radio_read_rds(&m_radio);
        const QString ps = m_radio.rds.ps_ready ? QString::fromUtf8(m_radio.rds.ps).trimmed() : QString();
        const QString rt = m_radio.rds.rt_ready ? QString::fromUtf8(m_radio.rds.rt).trimmed() : QString();
        if (ps != m_rdsStationName || rt != m_rdsRadioText) {
            m_rdsStationName = ps;
            m_rdsRadioText = rt;
            emit rdsChanged();
        }
    }
}

void RadioController::setStatusMessage(const QString &msg, int timeoutMs)
{
    m_statusMessage = msg;
    emit statusMessageChanged();
    m_statusTimer.start(timeoutMs);
}

void RadioController::refreshFoundStationsLocked()
{
    // Caller holds m_radio.mutex.
    QVariantList list;
    list.reserve(m_radio.found_count);
    for (int i = 0; i < m_radio.found_count; ++i) {
        QVariantMap entry;
        entry[QStringLiteral("freqHz")] = m_radio.found_freqs[i];
        entry[QStringLiteral("freqMhz")] = m_radio.found_freqs[i] / 1000000.0;
        entry[QStringLiteral("name")] = QString::fromUtf8(m_radio.found_names[i]).trimmed();
        list.append(entry);
    }
    m_foundStations = list;
    emit foundStationsChanged();
}

void RadioController::tuneTo(double mhz)
{
    tuneToHz(static_cast<quint32>(mhz * 1000000.0 + 0.5));
}

void RadioController::tuneToHz(quint32 hz)
{
    if (!m_ready || m_radio.scanning || m_radio.seeking)
        return;
    radio_set_freq(&m_radio, hz);
    m_rdsStationName.clear();
    m_rdsRadioText.clear();
    emit rdsChanged();
    emit frequencyChanged();
}

void RadioController::stepUp()
{
    if (!m_ready || m_radio.scanning || m_radio.seeking)
        return;
    const uint32_t step = m_configStore->data().scan_step_hz;
    quint32 next = m_radio.freq_hz + step;
    if (next > m_radio.freq_max_hz)
        next = m_radio.freq_max_hz;
    tuneToHz(next);
}

void RadioController::stepDown()
{
    if (!m_ready || m_radio.scanning || m_radio.seeking)
        return;
    const uint32_t step = m_configStore->data().scan_step_hz;
    quint32 next = (m_radio.freq_hz > m_radio.freq_min_hz + step) ? m_radio.freq_hz - step : m_radio.freq_min_hz;
    tuneToHz(next);
}

void RadioController::seekForward()
{
    if (!m_ready || m_radio.scanning || m_radio.seeking)
        return;
    m_seekingForward = true;
    m_preSeekFreqHz = m_radio.freq_hz;
    const Config &cfg = m_configStore->data();
    const int threshold = int(cfg.signal_threshold_pct * 65535 / 100);
    radio_start_seek(&m_radio, 1, cfg.scan_step_hz, threshold);
    emit seekingChanged();
}

void RadioController::seekBackward()
{
    if (!m_ready || m_radio.scanning || m_radio.seeking)
        return;
    m_seekingForward = false;
    m_preSeekFreqHz = m_radio.freq_hz;
    const Config &cfg = m_configStore->data();
    const int threshold = int(cfg.signal_threshold_pct * 65535 / 100);
    radio_start_seek(&m_radio, 0, cfg.scan_step_hz, threshold);
    emit seekingChanged();
}

void RadioController::cancelSeek()
{
    radio_stop_seek(&m_radio);
}

void RadioController::finishSeek()
{
    radio_stop_seek(&m_radio);
    if (m_radio.seek_result_hz != 0) {
        radio_set_freq(&m_radio, m_radio.seek_result_hz);
        setStatusMessage(QStringLiteral("Station found"));
    } else {
        radio_set_freq(&m_radio, m_preSeekFreqHz);
        setStatusMessage(QStringLiteral("No station found"));
    }
    m_rdsStationName.clear();
    m_rdsRadioText.clear();
    emit rdsChanged();
    emit frequencyChanged();
    emit seekingChanged();
}

void RadioController::startScan()
{
    if (!m_ready || m_radio.scanning || m_radio.seeking)
        return;
    m_scanDiscard = false;
    m_foundStations.clear();
    emit foundStationsChanged();
    const Config &cfg = m_configStore->data();
    m_radio.scan_step_hz = cfg.scan_step_hz;
    m_radio.scan_threshold = int(cfg.signal_threshold_pct * 65535 / 100);
    m_radio.scan_rds_names = cfg.rds_names;
    radio_start_scan(&m_radio);
    emit scanningChanged();
}

void RadioController::stopScanAndSave()
{
    m_scanDiscard = false;
    radio_stop_scan(&m_radio);
}

void RadioController::stopScanAndDiscard()
{
    m_scanDiscard = true;
    radio_stop_scan(&m_radio);
}

void RadioController::finishScan()
{
    radio_stop_scan(&m_radio);

    pthread_mutex_lock(&m_radio.mutex);
    refreshFoundStationsLocked();
    const int count = m_radio.found_count;
    uint32_t freqs[MAX_PRESETS];
    char names[MAX_PRESETS][NAME_MAX_LEN + 1];
    std::memcpy(freqs, m_radio.found_freqs, sizeof(freqs));
    std::memcpy(names, m_radio.found_names, sizeof(names));
    pthread_mutex_unlock(&m_radio.mutex);

    if (!m_scanDiscard) {
        Config &cfg = m_configStore->data();
        cfg.count = count;
        for (int i = 0; i < count; ++i) {
            cfg.freqs[i] = freqs[i];
            std::strncpy(cfg.names[i], names[i], NAME_MAX_LEN);
            cfg.names[i][NAME_MAX_LEN] = '\0';
        }
        m_configStore->commit();
        emit m_configStore->presetsChanged();
        setStatusMessage(QStringLiteral("Scan complete — %1 stations found").arg(count));
    } else {
        setStatusMessage(QStringLiteral("Scan discarded"));
    }

    emit scanningChanged();
}

void RadioController::setVolume(int v)
{
    if (!m_ready || v == m_radio.volume)
        return;
    radio_set_volume(&m_radio, v);
    emit volumeChanged();
}

void RadioController::setMuted(bool m)
{
    if (!m_ready || m == (m_radio.muted != 0))
        return;
    radio_mute(&m_radio, m ? 1 : 0);
    emit mutedChanged();
}
