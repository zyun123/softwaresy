#include "dbmodelbase.h"
#include <Other/Logger.h>
#include <QtSql>

DBModelBase::DBModelBase(QObject *parent) : QAbstractTableModel(parent)
{

}

int DBModelBase::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_viewdata.size();
}

int DBModelBase::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(m_viewdata.size() > 0)
        return m_viewdata.at(0).keys().size();
    else
        return 0;
}

QVariant DBModelBase::data(const QModelIndex &index, int role) const
{
    if(m_roles[role - Qt::UserRole - 1] == "password")
        return "******";
    else
        return m_viewdata[index.row()].value(m_roles[role - Qt::UserRole - 1]);
}

QHash<int, QByteArray> DBModelBase::roleNames() const
{
    QHash<int, QByteArray> tab;
    for(int i = 0; i < m_roles.size(); i++)
        tab.insert(Qt::UserRole + 1 + i, m_roles[i].toLocal8Bit());
    return tab;
}

void DBModelBase::resetData()
{
    beginResetModel();
    m_viewdata.clear();
    m_data.clear();
    QSqlQuery q;
    q.exec(m_sqlAndWarns["select"]);
    while(q.next())
    {
        QMap<QString, QVariant> tmp;
        for(int i = 0; i < m_data_fields.size(); i++)
            tmp.insert(m_data_fields[i], q.value(m_data_fields[i]));
        m_data.append(tmp);
    }
    for(int i = 0; i < m_data.size(); i++)
        m_viewdata.append(getViewRow(m_data[i]));
    endResetModel();
}

void DBModelBase::cussort(int col, Qt::SortOrder order)
{
    beginResetModel();
    std::sort(m_viewdata.begin(), m_viewdata.end(), [=](QMap<QString, QVariant> &a, QMap<QString, QVariant> &b)->bool{
        if(a[m_roles[col]] < b[m_roles[col]] && order == Qt::AscendingOrder ||
                a[m_roles[col]] > b[m_roles[col]] && order == Qt::DescendingOrder)
            return true;
        else
            return false;
    });
    endResetModel();
}

void DBModelBase::filter(QString keyword, QStringList props, QString from, QString to, QString btwProp)
{
    beginResetModel();
    m_viewdata.clear();
    QVector<bool> row_flag(m_data.size(), true);
    if(btwProp != "")//区间筛选
    {
        for(int i = 0; i < m_data.size(); i++)
        {
            auto low = from.toULongLong();
            auto high = to.toULongLong();
            auto val = m_data[i][btwProp].toULongLong();
            if(!(val >= low && val <= high))
                row_flag[i] = false;
        }
    }
    if(keyword != "")//字段搜索
    {
        for(int i = 0; i < m_data.size(); i++)
        {
            if(row_flag[i] == true)
            {
                bool flag = false;
                for(int j = 0; j < props.size(); j++)
                {
                    if(m_data[i].contains(props[j]) && m_data[i][props[j]].toString().contains(keyword))
                    {
                        flag = true;
                        break;
                    }
                }
                row_flag[i] = flag;
            }
        }
    }
    for(int i = 0; i < m_data.size(); i++)
        if(row_flag[i] == true)
            m_viewdata.append(getViewRow(m_data[i]));
    endResetModel();
}

bool DBModelBase::insertData(QVariantList vars)
{
    if(!m_sqlAndWarns.contains("insert"))
        return false;
    QSqlQuery q;
    q.prepare(m_sqlAndWarns["insert"]);
    for(int i = 0; i < m_data_fields.size(); i++)
        q.addBindValue(vars[i]);
    if(q.exec())
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        QMap<QString, QVariant> tmp;
        for(int i = 0; i < m_data_fields.size(); i++)
            tmp.insert(m_data_fields[i], vars[i]);
        m_data.append(tmp);
        m_viewdata.append(getViewRow(tmp));
        endInsertRows();
    }
    else
        return false;
    LOG_INFO << q.lastQuery();
    return true;
}

bool DBModelBase::modifyData(QStringList props, QStringList vars)
{
    if(!m_sqlAndWarns.contains("update") || props.size() != vars.size())
        return false;
    QSqlQuery q;
    QString exestr = m_sqlAndWarns["update"];
    for(int i = 0; i < props.size() - 1; i++)
    {
        exestr += props[i] + "='" + vars[i] + "'";
        if(i < props.size() - 2)
            exestr += ",";
    }
    exestr += " where " + props[props.size() - 1] + "='" + vars[vars.size() - 1] + "'";
    q.prepare(exestr);
    if(q.exec())
    {
        beginResetModel();
        QMap<QString, QVariant> tmpdat;
        for(int i = 0; i < m_data.size(); i++)
        {
            if(m_data[i][m_data_fields[0]].toString() == vars[vars.size() - 1])
            {
                for(int j = 1; j < m_data_fields.size(); j++)
                    m_data[i][m_data_fields[j]] = vars[j];
                tmpdat = m_data[i];
                break;
            }
        }
        for(int i = 0; i < m_viewdata.size(); i++)
        {
            if(m_viewdata[i][m_data_fields[0]].toString() == vars[vars.size() - 1])
            {
                m_viewdata[i] = getViewRow(tmpdat);
                break;
            }
        }
        endResetModel();
    }
    else
        return false;
    LOG_INFO << q.lastQuery();
    return true;
}

bool DBModelBase::removeData(int row)
{
    if(!m_sqlAndWarns.contains("remove"))
        return false;
    auto pk = m_viewdata[row][m_data_fields[0]].toString();//TODO 主键暂都默认为字符串
    QSqlQuery q;
    q.prepare(m_sqlAndWarns["remove"].arg(pk));
    if(q.exec())
    {
        beginRemoveRows(QModelIndex(), row, row);
        m_viewdata.removeAt(row);
        for(int i = 0; i < m_data.size(); i++)
        {
            if(m_data[i][m_data_fields[0]].toString() == pk)
            {
                m_data.removeAt(i);
                break;
            }
        }
        endRemoveRows();
    }
    else
        return false;
    LOG_INFO << q.lastQuery();
    return true;
}

QJsonObject DBModelBase::getRowDat(int row)
{
    QJsonObject obj;
    for(int i = 0; i < m_roles.size(); i++)
        obj.insert(m_roles[i], m_viewdata[row][m_roles[i]].toString());
    return obj;
}

QVariantList DBModelBase::find(QString field, QString value)
{
    if(!m_sqlAndWarns.contains("find"))
        return {};
    QVariantList rst;
    QSqlQuery q;
    q.exec(m_sqlAndWarns["find"].arg(field).arg(value));
    while(q.next())
    {
        QVariantMap tmp;
        for(int i = 0; i < m_data_fields.size(); i++)
            tmp.insert(m_data_fields[i], q.value(m_data_fields[i]).toString());
        rst.append(tmp);
    }
    return rst;
}
