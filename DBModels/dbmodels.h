#ifndef DBMODELS_H
#define DBMODELS_H

#include <QSqlQuery>
#include <QDebug>
#include "dbmodelbase.h"

#if 0
class ModelTemplate : public DBModelBase
{
public:
    ModelTemplate(QObject *parent = nullptr) : DBModelBase(parent){initModel();}
    void initModel() override
    {
        m_data_fields = QStringList({});
        m_roles = m_data_fields;
        m_sqlAndWarns = QMap<QString, QString>({
                        {"select", ""},
                        {"insert", ""},
                        {"update", ""},
                        {"remove", ""},
                        {"insert_warn", ""},
                        {"update_warn", ""},
                        {"remove_warn", ""},
                        {"remove_confirm", ""}
                        });
    }
};
#endif

class UserManageModel : public DBModelBase
{
public:
    UserManageModel(QObject *parent = nullptr) : DBModelBase(parent){initModel();}
    void initModel() override
    {
        m_data_fields = QStringList({"userid", "password", "name", "role", "music", "volume", "screen_lock"});
        m_roles = m_data_fields;
        m_sqlAndWarns = QMap<QString, QString>({
                        {"select", "select * from user_info"},
                        {"find", "select * from user_info where %1 ='%2'"},
                        {"insert", "insert into user_info(userid, password, name, role, music, volume, screen_lock) values(?, ?, ?, ?, ?, ?, ?)"},
                        {"update", "update user_info set "},
                        {"remove", "delete from user_info where userid = '%1'"},
                        {"insert_warn", "账号id已存在，添加失败"},
                        {"update_warn", "修改失败，请刷新列表后重试"},
                        {"remove_warn", "删除失败，请刷新列表后重试"},
                        {"remove_confirm", "确定删除该账号吗？"}
                        });
    }
    void cussort(int col, Qt::SortOrder order) override
    {
        if(col == 1)//密码不排序
            return;
        DBModelBase::cussort(col, order);
    }
};

class CureRecordModel : public DBModelBase
{
public:
    CureRecordModel(QObject *parent = nullptr) : DBModelBase(parent){initModel();}
    void initModel() override
    {
        m_data_fields = QStringList({"serial_num","phone","patient_name","operator_id","meridians","start_time","end_time","times","device_id","dur_time"});
        m_roles = m_data_fields;
        m_sqlAndWarns = QMap<QString, QString>({
                        {"select", "select * from cure_record"},
                        {"insert", "insert into cure_record(serial_num, phone, patient_name, operator_id, meridians, start_time, end_time, times, device_id, dur_time) values(?, ?, ?, ?, ?, ?, ?, (select count(*) from cure_record where phone = ?) + 1, ?, ?)"},
                        {"insert_warn", "保存调理记录失败，请联系运维处理"}
                        });
    }
};

class FormulaManageModel : public DBModelBase
{
public:
    FormulaManageModel(QObject *parent = nullptr) : DBModelBase(parent){initModel();}
    void initModel() override
    {
        m_data_fields = QStringList({"serial_num", "patient_id", "diagnosis", "meridians", "cure_mode", "cure_force", "cure_times", "note", "creator", "create_time", "formula_mod"});
        m_roles = QStringList({"patient_name", "patient_id", "create_time", "serial_num", "creator", "diagnosis", "meridians", "cure_mode", "cure_force", "cure_times", "note", "formula_mod", "mode_wtf"});
        m_sqlAndWarns = QMap<QString, QString>({
                        {"select", "select * from formula"},
                        {"find", "select * from formula where %1 ='%2' order by serial_num desc"},
                        {"insert", "insert into formula(serial_num, patient_id, diagnosis, meridians, cure_mode, cure_force, cure_times, note, creator, create_time, formula_mod) values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"},
                        {"update", "update formula set "},
                        {"remove", "delete from formula where serial_num = '%1'"},
                        {"insert_warn", "新建处方失败，请联系运维处理"},
                        {"update_warn", "修改处方失败，请联系运维处理"},
                        {"remove_warn", "删除处方失败，请联系运维处理"},
                        {"remove_confirm", "确定删除该处方吗？"}
                        });
    }
protected:
    QMap<QString, QVariant> getViewRow(QMap<QString, QVariant> datRow) override
    {
        auto ret = datRow;
        QSqlQuery q;
        q.exec(QString("select name from patient_info where idcard = '%1'").arg(datRow["patient_id"].toString()));
        if(q.next())
            ret.insert("patient_name", q.value("name"));
        else
            ret.insert("patient_name", "");
        q.exec(QString("select name_wtf from formula_tpl where name = '%1'").arg(datRow["formula_mod"].toString()));
        if(q.next())
            ret.insert("mode_wtf", q.value("name_wtf"));
        else
            ret.insert("mode_wtf", "");
        return ret;
    }
};

class FormulaTplManageModel : public DBModelBase
{
public:
    FormulaTplManageModel(QObject *parent = nullptr) : DBModelBase(parent){initModel();}
    void initModel() override
    {
        m_data_fields = QStringList({"name", "meridians", "name_wtf", "detail"});
        m_roles = m_data_fields;
        m_sqlAndWarns = QMap<QString, QString>({
                        {"select", "select * from formula_tpl order by detail asc"},
                        {"find", "select * from formula_tpl where %1 ='%2'"},
                        {"insert", "insert into formula_tpl(name, meridians, name_wtf, detail) values(?, ?, ?, ?)"},
                        {"update", "update formula_tpl set "},
                        {"remove", "delete from formula_tpl where name = '%1'"},
                        {"insert_warn", "该调理方案已存在"},
                        {"update_warn", "修改调理方案失败，请联系运维处理"},
                        {"remove_warn", "删除调理方案失败，请联系运维处理"},
                        {"remove_confirm", "确定删除该调理方案吗？"}
                        });
    }
};

class PatientManageModel : public DBModelBase
{
public:
    PatientManageModel(QObject *parent = nullptr) : DBModelBase(parent){initModel();}
    void initModel() override
    {
        m_data_fields = QStringList({"idcard", "name", "gender", "age", "phone", "creator", "create_time", "note"});
        m_roles = QStringList({"idcard", "name", "gender", "age", "phone", "creator", "create_time", "note", "formula_mod", "meridians", "idcard_wtf"});
        m_sqlAndWarns = QMap<QString, QString>({
                        {"select", "select * from patient_info"},
                        {"find", "select * from patient_info where %1 ='%2'"},
                        {"insert", "insert into patient_info(name, idcard, gender, age, phone, creator, create_time, note) values(?, ?, ?, ?, ?, ?, ?, ?)"},
                        {"update", "update patient_info set "},
                        {"remove", "delete from patient_info where idcard = '%1'"},
                        {"insert_warn", "该联系方式已存在"},
                        {"update_warn", "修改患者信息失败，请联系运维处理"},
                        {"remove_warn", "删除患者信息失败，请联系运维处理"},
                        {"remove_confirm", "确定删除该患者信息吗？"}
                        });
    }
protected:
    QMap<QString, QVariant> getViewRow(QMap<QString, QVariant> datRow) override
    {
        auto ret = datRow;
        QSqlQuery q;
        q.exec(QString("select * from formula where patient_id = '%1' order by serial_num desc").arg(datRow["idcard"].toString()));
        if(q.next())
        {
            ret.insert("formula_mod", q.value("formula_mod"));
            ret.insert("meridians", q.value("meridians"));
        }
        else
            ret.insert("formula_mod", "");
        QString idcard = ret["idcard"].toString();
        ret["idcard_wtf"] = idcard;
        ret["idcard"] = idcard.replace(idcard.size() - 8, 8, "********");
        return ret;
    }
};

class AppointmentListModel : public DBModelBase
{
public:
    AppointmentListModel(QObject *parent = nullptr) : DBModelBase(parent){initModel();}
    void initModel() override
    {
        m_roles = QStringList({"serial_num", "patient_id", "meridians", "name", "phone", "diagnosis", "time"});
    }
    void setModelDat(QJsonArray jsonarr) override
    {
        beginResetModel();
        m_viewdata.clear();
        for(int i = 0; i < jsonarr.size(); i++)
        {
            auto jsonobj = jsonarr[i].toObject();
            QMap<QString, QVariant> tmp;
            for(int j = 0; j < m_roles.size(); j++)
                tmp.insert(m_roles[j], jsonobj.value(m_roles[j]).toString());
            m_viewdata.append(tmp);
        }
        endResetModel();
    }
};

#endif // DBMODELS_H
