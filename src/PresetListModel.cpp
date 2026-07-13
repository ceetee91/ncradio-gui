#include "PresetListModel.h"
#include "ConfigStore.h"

#include <QLocale>

PresetListModel::PresetListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void PresetListModel::setConfigStore(ConfigStore *store)
{
    if (m_configStore == store)
        return;

    if (m_configStore)
        disconnect(m_configStore, nullptr, this, nullptr);

    beginResetModel();
    m_configStore = store;
    endResetModel();

    if (m_configStore) {
        connect(m_configStore, &ConfigStore::presetsChanged, this, [this] {
            beginResetModel();
            endResetModel();
        });
    }

    emit configStoreChanged();
}

int PresetListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_configStore)
        return 0;
    return m_configStore->presetCount();
}

QVariant PresetListModel::data(const QModelIndex &index, int role) const
{
    if (!m_configStore || !index.isValid())
        return {};

    const int row = index.row();
    if (row < 0 || row >= m_configStore->presetCount())
        return {};

    switch (role) {
    case FreqHzRole:
        return m_configStore->presetFreqHz(row);
    case FreqMhzRole:
        return m_configStore->presetFreqHz(row) / 1000000.0;
    case FreqLabelRole:
        return QLocale::c().toString(m_configStore->presetFreqHz(row) / 1000000.0, 'f', 2);
    case NameRole:
        return m_configStore->presetName(row);
    case PresetIndexRole:
        return row;
    default:
        return {};
    }
}

QHash<int, QByteArray> PresetListModel::roleNames() const
{
    return {
        {FreqHzRole, "freqHz"},
        {FreqMhzRole, "freqMhz"},
        {FreqLabelRole, "freqLabel"},
        {NameRole, "name"},
        {PresetIndexRole, "presetIndex"},
    };
}
