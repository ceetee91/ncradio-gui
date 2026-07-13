#include "ConfigStore.h"

#include <cmath>
#include <cstring>

ConfigStore::ConfigStore(QObject *parent)
    : QObject(parent)
{
}

bool ConfigStore::load()
{
    const int rc = config_load(&m_config);
    emit presetsChanged();
    emit settingsChanged();
    return rc != 0;
}

bool ConfigStore::save()
{
    return config_save(&m_config) != 0;
}

void ConfigStore::setScanStepMhz(double mhz)
{
    m_config.scan_step_hz = static_cast<uint32_t>(std::lround(mhz * 1000000.0));
    commit();
}

void ConfigStore::setSignalThresholdPct(int pct)
{
    m_config.signal_threshold_pct = pct;
    commit();
}

void ConfigStore::setRdsNames(bool on)
{
    m_config.rds_names = on ? 1 : 0;
    commit();
}

void ConfigStore::setAudioMuteScan(bool on)
{
    m_config.audio_mute_scan = on ? 1 : 0;
    commit();
}

void ConfigStore::setAudioMuteSeek(bool on)
{
    m_config.audio_mute_seek = on ? 1 : 0;
    commit();
}

void ConfigStore::commit()
{
    save();
    emit settingsChanged();
}

uint32_t ConfigStore::presetFreqHz(int index) const
{
    if (index < 0 || index >= m_config.count)
        return 0;
    return m_config.freqs[index];
}

QString ConfigStore::presetName(int index) const
{
    if (index < 0 || index >= m_config.count)
        return {};
    return QString::fromUtf8(m_config.names[index]);
}

void ConfigStore::addPreset(double freqMhz, const QString &name)
{
    const auto hz = static_cast<uint32_t>(std::lround(freqMhz * 1000000.0));
    const QByteArray nameUtf8 = name.toUtf8();
    config_add(&m_config, hz, nameUtf8.constData());
    save();
    emit presetsChanged();
}

void ConfigStore::deletePreset(int index)
{
    if (index < 0 || index >= m_config.count)
        return;
    config_del(&m_config, index);
    save();
    emit presetsChanged();
}

void ConfigStore::renamePreset(int index, const QString &name)
{
    if (index < 0 || index >= m_config.count)
        return;
    const QByteArray nameUtf8 = name.toUtf8();
    std::strncpy(m_config.names[index], nameUtf8.constData(), NAME_MAX_LEN);
    m_config.names[index][NAME_MAX_LEN] = '\0';
    save();
    emit presetsChanged();
}

int ConfigStore::findPreset(double freqMhz) const
{
    const auto hz = static_cast<uint32_t>(std::lround(freqMhz * 1000000.0));
    return config_find(&m_config, hz);
}

int ConfigStore::addEqPreset(const QString &name, const QList<float> &gains)
{
    if (gains.size() != EQ_BANDS)
        return -1;
    float buf[EQ_BANDS];
    for (int i = 0; i < EQ_BANDS; ++i)
        buf[i] = gains.at(i);
    const QByteArray nameUtf8 = name.toUtf8();
    const int idx = config_eq_preset_add(&m_config, nameUtf8.constData(), buf);
    if (idx >= 0)
        save();
    return idx;
}

void ConfigStore::deleteEqPreset(int index)
{
    config_eq_preset_del(&m_config, index);
    save();
}
