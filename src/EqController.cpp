#include "EqController.h"
#include "ConfigStore.h"

#include <cstring>

EqController::EqController(ConfigStore *configStore, QObject *parent)
    : QObject(parent)
    , m_configStore(configStore)
{
    // Seed at 48kHz like ncradio.c's startup; AudioController re-inits at
    // the negotiated rate once the audio pipe's format is known.
    eq_init(&m_eq, 48000.0);

    const Config &cfg = m_configStore->data();
    for (int i = 0; i < EQ_BANDS; ++i)
        eq_set_gain(&m_eq, i, cfg.eq_gains[i]);
    m_eq.enabled = cfg.eq_enabled;
    m_activePresetName = QString::fromUtf8(cfg.eq_active_preset);
}

void EqController::setEnabled(bool on)
{
    if ((m_eq.enabled != 0) == on)
        return;
    m_eq.enabled = on ? 1 : 0;
    Config &cfg = m_configStore->data();
    cfg.eq_enabled = m_eq.enabled;
    m_configStore->commit();
    emit enabledChanged();
}

QVariantList EqController::gains() const
{
    QVariantList list;
    list.reserve(EQ_BANDS);
    for (int i = 0; i < EQ_BANDS; ++i)
        list.append(double(m_eq.gains_db[i]));
    return list;
}

QStringList EqController::bandLabels() const
{
    QStringList labels;
    labels.reserve(EQ_BANDS);
    for (int i = 0; i < EQ_BANDS; ++i)
        labels.append(QString::fromUtf8(EQ_FREQ_LABELS[i]));
    return labels;
}

QStringList EqController::builtinPresetNames() const
{
    QStringList names;
    names.reserve(EQ_PRESET_COUNT);
    for (int i = 0; i < EQ_PRESET_COUNT; ++i)
        names.append(QString::fromUtf8(EQ_PRESET_NAMES[i]));
    return names;
}

QVariantList EqController::customPresetNames() const
{
    const Config &cfg = m_configStore->data();
    QVariantList names;
    for (int i = 0; i < cfg.eq_custom_count; ++i)
        names.append(QString::fromUtf8(cfg.eq_custom[i].name));
    return names;
}

void EqController::syncGainsFromEq()
{
    Config &cfg = m_configStore->data();
    for (int i = 0; i < EQ_BANDS; ++i)
        cfg.eq_gains[i] = m_eq.gains_db[i];
}

void EqController::persist()
{
    syncGainsFromEq();
    Config &cfg = m_configStore->data();
    const QByteArray nameUtf8 = m_activePresetName.toUtf8();
    std::strncpy(cfg.eq_active_preset, nameUtf8.constData(), EQ_CUSTOM_NAMELEN - 1);
    cfg.eq_active_preset[EQ_CUSTOM_NAMELEN - 1] = '\0';
    m_configStore->commit();
}

void EqController::setGain(int band, double dB)
{
    if (band < 0 || band >= EQ_BANDS)
        return;
    eq_set_gain(&m_eq, band, float(dB));
    m_activePresetName.clear(); // manual tweak invalidates the named preset, like the original app
    persist();
    emit gainsChanged();
    emit activePresetChanged();
}

void EqController::resetBand(int band)
{
    setGain(band, 0.0);
}

void EqController::loadBuiltinPreset(int index)
{
    if (index < 0 || index >= EQ_PRESET_COUNT)
        return;
    eq_load_preset(&m_eq, index);
    m_activePresetName = QString::fromUtf8(EQ_PRESET_NAMES[index]);
    persist();
    emit gainsChanged();
    emit activePresetChanged();
}

void EqController::loadCustomPreset(int index)
{
    const Config &cfg = m_configStore->data();
    if (index < 0 || index >= cfg.eq_custom_count)
        return;
    for (int i = 0; i < EQ_BANDS; ++i)
        eq_set_gain(&m_eq, i, cfg.eq_custom[index].gains[i]);
    m_activePresetName = QString::fromUtf8(cfg.eq_custom[index].name);
    persist();
    emit gainsChanged();
    emit activePresetChanged();
}

int EqController::saveCustomPreset(const QString &name)
{
    QList<float> gains;
    gains.reserve(EQ_BANDS);
    for (int i = 0; i < EQ_BANDS; ++i)
        gains.append(m_eq.gains_db[i]);
    const int idx = m_configStore->addEqPreset(name, gains);
    if (idx >= 0) {
        m_activePresetName = name;
        emit customPresetsChanged();
        emit activePresetChanged();
        persist();
    }
    return idx;
}

void EqController::deleteCustomPreset(int index)
{
    const Config &cfg = m_configStore->data();
    const bool wasActive = index >= 0 && index < cfg.eq_custom_count
        && m_activePresetName == QString::fromUtf8(cfg.eq_custom[index].name);
    m_configStore->deleteEqPreset(index);
    emit customPresetsChanged();
    if (wasActive) {
        m_activePresetName.clear();
        emit activePresetChanged();
    }
}

void EqController::loadFlat()
{
    loadBuiltinPreset(0); // index 0 is always "Flat" per eq.c
}
