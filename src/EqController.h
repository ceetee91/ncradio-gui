#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>

extern "C" {
#include "eq.h"
}

class ConfigStore;
class AudioController;

// Wraps eq.c/eq.h — the 11-band graphic equalizer. Owns the Eq instance;
// AudioController's Audio.eq is pointed at it once audio starts.
class EqController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QVariantList gains READ gains NOTIFY gainsChanged)
    Q_PROPERTY(QStringList bandLabels READ bandLabels CONSTANT)
    Q_PROPERTY(QStringList builtinPresetNames READ builtinPresetNames CONSTANT)
    Q_PROPERTY(QVariantList customPresetNames READ customPresetNames NOTIFY customPresetsChanged)
    Q_PROPERTY(QString activePresetName READ activePresetName NOTIFY activePresetChanged)
    Q_PROPERTY(int bandCount READ bandCount CONSTANT)

public:
    explicit EqController(ConfigStore *configStore, QObject *parent = nullptr);

    Eq *raw() { return &m_eq; }

    bool enabled() const { return m_eq.enabled != 0; }
    void setEnabled(bool on);

    QVariantList gains() const;
    QStringList bandLabels() const;
    QStringList builtinPresetNames() const;
    QVariantList customPresetNames() const;
    QString activePresetName() const { return m_activePresetName; }
    int bandCount() const { return EQ_BANDS; }

    Q_INVOKABLE void setGain(int band, double dB);
    Q_INVOKABLE void resetBand(int band);
    Q_INVOKABLE void loadBuiltinPreset(int index);
    Q_INVOKABLE void loadCustomPreset(int index);
    Q_INVOKABLE int saveCustomPreset(const QString &name);
    Q_INVOKABLE void deleteCustomPreset(int index);
    Q_INVOKABLE void loadFlat();

signals:
    void enabledChanged();
    void gainsChanged();
    void customPresetsChanged();
    void activePresetChanged();

private:
    void syncGainsFromEq();
    void persist();

    Eq m_eq{};
    ConfigStore *m_configStore;
    QString m_activePresetName;
};
