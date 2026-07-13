#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

extern "C" {
#include "radio.h"
}

class ConfigStore;

// Wraps radio.c/radio.h. Drives the backend from a GUI-thread QTimer,
// mirroring ncradio.c's own polling main loop (radio_get_signal /
// radio_read_rds every ~500ms, checking scanning/seeking completion) —
// the backend's documented thread-safety contract makes this safe without
// a dedicated QThread.
class RadioController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(double frequencyMhz READ frequencyMhz NOTIFY frequencyChanged)
    Q_PROPERTY(quint32 frequencyHz READ frequencyHz NOTIFY frequencyChanged)
    Q_PROPERTY(double freqMinMhz READ freqMinMhz NOTIFY readyChanged)
    Q_PROPERTY(double freqMaxMhz READ freqMaxMhz NOTIFY readyChanged)
    Q_PROPERTY(int signalPercent READ signalPercent NOTIFY signalChanged)
    Q_PROPERTY(bool stereo READ stereo NOTIFY signalChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(double scanProgress READ scanProgress NOTIFY scanProgressChanged)
    Q_PROPERTY(QVariantList foundStations READ foundStations NOTIFY foundStationsChanged)
    Q_PROPERTY(bool seeking READ seeking NOTIFY seekingChanged)
    Q_PROPERTY(bool seekingForward READ seekingForward NOTIFY seekingChanged)
    Q_PROPERTY(QString rdsStationName READ rdsStationName NOTIFY rdsChanged)
    Q_PROPERTY(QString rdsRadioText READ rdsRadioText NOTIFY rdsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit RadioController(ConfigStore *configStore, QObject *parent = nullptr);
    ~RadioController() override;

    Q_INVOKABLE bool open(const QString &device = QStringLiteral("/dev/radio0"));

    bool ready() const { return m_ready; }
    QString errorMessage() const { return m_errorMessage; }

    double frequencyMhz() const { return m_radio.freq_hz / 1000000.0; }
    quint32 frequencyHz() const { return m_radio.freq_hz; }
    double freqMinMhz() const { return m_radio.freq_min_hz / 1000000.0; }
    double freqMaxMhz() const { return m_radio.freq_max_hz / 1000000.0; }

    int signalPercent() const { return m_signalPercent; }
    bool stereo() const { return m_radio.stereo != 0; }

    int volume() const { return m_radio.volume; }
    void setVolume(int v);

    bool muted() const { return m_radio.muted != 0; }
    void setMuted(bool m);

    bool scanning() const { return m_radio.scanning != 0; }
    double scanProgress() const { return m_scanProgress; }
    QVariantList foundStations() const { return m_foundStations; }

    bool seeking() const { return m_radio.seeking != 0; }
    bool seekingForward() const { return m_seekingForward; }

    QString rdsStationName() const { return m_rdsStationName; }
    QString rdsRadioText() const { return m_rdsRadioText; }

    QString statusMessage() const { return m_statusMessage; }

public slots:
    void tuneTo(double mhz);
    void tuneToHz(quint32 hz);
    void stepUp();
    void stepDown();
    void seekForward();
    void seekBackward();
    void cancelSeek();
    void startScan();
    void stopScanAndSave();
    void stopScanAndDiscard();

signals:
    void readyChanged();
    void errorChanged();
    void frequencyChanged();
    void signalChanged();
    void volumeChanged();
    void mutedChanged();
    void scanningChanged();
    void scanProgressChanged();
    void foundStationsChanged();
    void seekingChanged();
    void rdsChanged();
    void statusMessageChanged();

private slots:
    void poll();

private:
    void setStatusMessage(const QString &msg, int timeoutMs = 3000);
    void finishSeek();
    void finishScan();
    void refreshFoundStationsLocked();

    ConfigStore *m_configStore;
    Radio m_radio{};
    QTimer m_pollTimer;
    QTimer m_statusTimer;

    bool m_ready = false;
    QString m_errorMessage;
    int m_signalPercent = 0;

    bool m_seekingForward = true;
    quint32 m_preSeekFreqHz = 0;

    bool m_scanDiscard = false;
    double m_scanProgress = 0.0;
    QVariantList m_foundStations;

    QString m_rdsStationName;
    QString m_rdsRadioText;
    QString m_statusMessage;
};
