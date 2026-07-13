#pragma once

#include <QAbstractListModel>

class ConfigStore;

// Read-through QAbstractListModel over ConfigStore's presets, for the
// sidebar station list. Mutations (add/rename/delete) go through
// ConfigStore directly; this model just re-syncs on presetsChanged().
class PresetListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ConfigStore *configStore READ configStore WRITE setConfigStore NOTIFY configStoreChanged)

public:
    enum Roles {
        FreqHzRole = Qt::UserRole + 1,
        FreqMhzRole,
        FreqLabelRole,
        NameRole,
        PresetIndexRole,
    };
    Q_ENUM(Roles)

    explicit PresetListModel(QObject *parent = nullptr);

    ConfigStore *configStore() const { return m_configStore; }
    void setConfigStore(ConfigStore *store);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void configStoreChanged();

private:
    ConfigStore *m_configStore = nullptr;
};
