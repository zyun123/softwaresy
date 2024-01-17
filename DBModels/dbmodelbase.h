#ifndef DBMODELBASE_H
#define DBMODELBASE_H

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

class DBModelBase : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DBModelBase(QObject *parent = nullptr);

    virtual void initModel() = 0;

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void resetData();
    Q_INVOKABLE virtual void cussort(int col, Qt::SortOrder order);
    Q_INVOKABLE void filter(QString keyword, QStringList props, QString from = "", QString to = "", QString btwProp = "");
    Q_INVOKABLE bool insertData(QVariantList vars);
    Q_INVOKABLE bool modifyData(QStringList props, QStringList vars);
    Q_INVOKABLE bool removeData(int row);
    Q_INVOKABLE QVariantList find(QString field, QString value);

    Q_INVOKABLE virtual void setModelDat(QJsonArray jsonarr){};
    Q_INVOKABLE QJsonObject getRowDat(int row);

protected:
    QStringList m_data_fields;//数据字段
    QList<QMap<QString, QVariant>> m_data;//数据
    QStringList m_roles;//视图字段
    QList<QMap<QString, QVariant>> m_viewdata;//视图数据
    QMap<QString, QString> m_sqlAndWarns;

    virtual QMap<QString, QVariant> getViewRow(QMap<QString, QVariant> datRow){return datRow;}//将一行数据转为一行视图
};

#endif // DBMODELBASE_H
