#pragma once

#include <QObject>
#include <QString>

extern "C" {
#include "config.h"
}

// Owns the single Config instance (presets, tuning/audio/recording/EQ
// settings) and persists it to ~/.ncradio.conf via the vendored config.c —
// the same file and format /data/ncradio's ncurses app reads and writes.
class ConfigStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int presetCount READ presetCount NOTIFY presetsChanged)
    Q_PROPERTY(double scanStepMhz READ scanStepMhz WRITE setScanStepMhz NOTIFY settingsChanged)
    Q_PROPERTY(int signalThresholdPct READ signalThresholdPct WRITE setSignalThresholdPct NOTIFY settingsChanged)
    Q_PROPERTY(bool rdsNames READ rdsNames WRITE setRdsNames NOTIFY settingsChanged)
    Q_PROPERTY(bool audioMuteScan READ audioMuteScan WRITE setAudioMuteScan NOTIFY settingsChanged)
    Q_PROPERTY(bool audioMuteSeek READ audioMuteSeek WRITE setAudioMuteSeek NOTIFY settingsChanged)

public:
    explicit ConfigStore(QObject *parent = nullptr);

    double scanStepMhz() const { return m_config.scan_step_hz / 1000000.0; }
    void setScanStepMhz(double mhz);
    int signalThresholdPct() const { return m_config.signal_threshold_pct; }
    void setSignalThresholdPct(int pct);
    bool rdsNames() const { return m_config.rds_names != 0; }
    void setRdsNames(bool on);
    bool audioMuteScan() const { return m_config.audio_mute_scan != 0; }
    void setAudioMuteScan(bool on);
    bool audioMuteSeek() const { return m_config.audio_mute_seek != 0; }
    void setAudioMuteSeek(bool on);

    Config &data() { return m_config; }
    const Config &data() const { return m_config; }

    bool load();
    bool save();

    int presetCount() const { return m_config.count; }
    Q_INVOKABLE uint32_t presetFreqHz(int index) const;
    Q_INVOKABLE QString presetName(int index) const;

    Q_INVOKABLE void addPreset(double freqMhz, const QString &name);
    Q_INVOKABLE void deletePreset(int index);
    Q_INVOKABLE void renamePreset(int index, const QString &name);
    Q_INVOKABLE int findPreset(double freqMhz) const;

    Q_INVOKABLE int addEqPreset(const QString &name, const QList<float> &gains);
    Q_INVOKABLE void deleteEqPreset(int index);

    // Generic setter used by controllers after mutating fields directly via
    // data(); persists and notifies in one step.
    Q_INVOKABLE void commit();

signals:
    void presetsChanged();
    void settingsChanged();

private:
    Config m_config{};
};
