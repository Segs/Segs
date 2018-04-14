#include "GameDBSyncContext.h"

#include "GameDBSyncEvents.h"
#include "Settings.h"

#include <ace/Thread.h>

#include <QDebug>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

namespace {
bool prepQuery(QSqlQuery &qr,const QString &txt) {
    if(!qr.prepare(txt)) {
        qDebug() << "SQL_ERROR:"<<qr.lastError();
        return false;
    }
    return true;
}
bool doIt(QSqlQuery &qr) {
    if(!qr.exec()) {
        qDebug() << "SQL_ERROR:"<<qr.lastError();
        return false;
    }
    return true;
}
}

GameDbSyncContext::GameDbSyncContext()
{
}
GameDbSyncContext::~GameDbSyncContext()
{
}
int64_t GameDbSyncContext::getDbVersion(QSqlDatabase &db)
{
    QSqlQuery version_query(
                QStringLiteral("SELECT version FROM table_versions WHERE table_name='db_version' ORDER BY id DESC LIMIT 1"),
                db);
    if(!version_query.exec())
    {
        qDebug() << version_query.lastError();
        return -1;
    }
    if(!version_query.next())
        return -1;
    return version_query.value(0).toInt();
}
// Maybe one day we'll need to read different db configs per-thread, for now all just read the same file.
bool GameDbSyncContext::loadAndConfigure()
{
    char thread_name_buf[16];
    ACE_Thread_ID our_id;
    if(m_setup_complete)
    {
        qCritical()<< "This DbSyncContext has already been configured";
        return false;
    }

    qInfo() << "Loading GameDbSync settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup(QStringLiteral("AdminServer"));
    QStringList driver_list {"QSQLITE","QPSQL"};
    our_id.to_string(thread_name_buf); // Ace is using template specialization to acquire the lenght of passed buffer

    config.beginGroup(QStringLiteral("CharacterDatabase"));
    // this indent is here to mark the nesting of config block
        QString dbdriver = config.value(QStringLiteral("db_driver"),"QSQLITE").toString();
        QString dbhost = config.value(QStringLiteral("db_host"),"127.0.0.1").toString();
        int dbport = config.value(QStringLiteral("db_port"),"5432").toInt();
        QString dbname = config.value(QStringLiteral("db_name"),"segs_game").toString();
        QString dbuser = config.value(QStringLiteral("db_user"),"segsadmin").toString();
        QString dbpass = config.value(QStringLiteral("db_pass"),"segs123").toString();
    config.endGroup(); // CharacterDatabase
    config.endGroup(); // AdminServer

    QSqlDatabase *db2;
    if(!driver_list.contains(dbdriver.toUpper())) {
        qCritical() << "Database driver" << dbdriver << " not supported";
        return false;
    }
    db2 = new QSqlDatabase(QSqlDatabase::addDatabase(dbdriver,QStringLiteral("CharacterDatabase_")+thread_name_buf));
    db2->setHostName(dbhost);
    db2->setPort(dbport);
    db2->setDatabaseName(dbname);
    db2->setUserName(dbuser);
    db2->setPassword(dbpass);
    m_db.reset(db2); // at this point we become owner of the db
    if(!m_db->open())
    {
        qCritical().noquote() << "Failed to open database:" <<dbname;
        return false;
    }
    int db_version=getDbVersion(*m_db);
    if(db_version!=required_db_version)
    {
        qCritical() << "Wrong db version:"<<db_version<<"this GameDatabase service requires:"<<required_db_version;
        return false;
    }
    m_prepared_fill.reset(new QSqlQuery(*m_db));
    m_prepared_account_insert.reset(new QSqlQuery(*m_db));
    m_prepared_account_select.reset(new QSqlQuery(*m_db));
    m_prepared_entity_select.reset(new QSqlQuery(*m_db));
    m_prepared_char_select.reset(new QSqlQuery(*m_db));
    m_prepared_char_exists.reset(new QSqlQuery(*m_db));
    m_prepared_char_insert.reset(new QSqlQuery(*m_db));
    m_prepared_char_delete.reset(new QSqlQuery(*m_db));
    m_prepared_char_update.reset(new QSqlQuery(*m_db));
    m_prepared_costume_insert.reset(new QSqlQuery(*m_db));
    m_prepared_costume_update.reset(new QSqlQuery(*m_db));

    prepQuery(*m_prepared_char_update,
                "UPDATE characters SET "
                "char_name=:char_name, chardata=:chardata, entitydata=:entitydata, bodytype=:bodytype, "
                ""
                "supergroup_id=:supergroup_id, options=:options "
                "WHERE id=:id ");
    prepQuery(*m_prepared_costume_update,
                "UPDATE costume SET "
                "costume_index=:costume_index, skin_color=:skin_color, parts=:parts "
                "WHERE character_id=:id ");
    prepQuery(*m_prepared_fill,"SELECT * FROM costume WHERE character_id=? AND costume_index=?");
    prepQuery(*m_prepared_account_select,"SELECT * FROM accounts WHERE account_id=?");
    prepQuery(*m_prepared_account_insert,"INSERT INTO accounts  (account_id,max_slots) VALUES (?,?)");
    prepQuery(*m_prepared_char_insert,
                "INSERT INTO characters  ("
                "slot_index, account_id, char_name, chardata, entitydata, "
                "bodytype, height, physique, "
                "hitpoints, endurance, "
                "supergroup_id, options"
                ") VALUES ("
                ":slot_index, :account_id, :char_name, :chardata, :entitydata, "
                ":bodytype, :height, :physique, "
                ":hitpoints, :endurance, "
                ":supergroup_id, :options"
                ")");
    prepQuery(*m_prepared_costume_insert,
                "INSERT INTO costume (character_id,costume_index,skin_color,parts) VALUES "
                "(:id,:costume_index,:skin_color,:parts)");
    prepQuery(*m_prepared_entity_select,"SELECT * FROM characters WHERE id=:id");
    prepQuery(*m_prepared_char_select,"SELECT * FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(*m_prepared_char_exists,"SELECT exists (SELECT 1 FROM characters WHERE char_name = $1 LIMIT 1)");
    prepQuery(*m_prepared_char_delete,"DELETE FROM characters WHERE account_id=? AND slot_index=?");

    return true;
}

bool GameDbSyncContext::performUpdate(const CharacterUpdateData &data)
{
    m_prepared_char_update->bindValue(QStringLiteral(":id"), uint32_t(data.m_id)); // for WHERE statement only
    m_prepared_char_update->bindValue(QStringLiteral(":char_name"), data.m_char_name);
    m_prepared_char_update->bindValue(QStringLiteral(":chardata"), data.m_char_data);
    m_prepared_char_update->bindValue(QStringLiteral(":entitydata"), data.m_entitydata);
    m_prepared_char_update->bindValue(QStringLiteral(":bodytype"), data.m_bodytype);
    m_prepared_char_update->bindValue(QStringLiteral(":height"), data.m_height);
    m_prepared_char_update->bindValue(QStringLiteral(":physique"), data.m_physique);
    m_prepared_char_update->bindValue(QStringLiteral(":supergroup_id"), data.m_supergroup_id);
    m_prepared_char_update->bindValue(QStringLiteral(":options"), data.m_options);
    return doIt(*m_prepared_char_update);
}

bool GameDbSyncContext::performUpdate(const CostumeUpdateData &data)
{
    m_prepared_costume_update->bindValue(QStringLiteral(":id"), uint32_t(data.m_db_id)); // for WHERE statement only
    m_prepared_costume_update->bindValue(QStringLiteral(":costume_index"), data.m_costume_index);
    m_prepared_costume_update->bindValue(QStringLiteral(":skin_color"), data.m_skin_color);
    m_prepared_costume_update->bindValue(QStringLiteral(":parts"), data.m_parts);
    return doIt(*m_prepared_costume_update);
}

bool GameDbSyncContext::getAccount(const GameAccountRequestData &data,GameAccountResponseData &result)
{
    // Try to find the acount in db
    m_prepared_account_select->bindValue(0,quint64(data.m_auth_account_id));
    if(!doIt(*m_prepared_account_select))
        return false;
    if(!m_prepared_account_select->next() && !data.create_if_does_not_exist)
        return false;

    if(data.create_if_does_not_exist)
    {
        m_prepared_account_insert->bindValue(0,quint64(data.m_auth_account_id));
        m_prepared_account_insert->bindValue(1,data.max_character_slots);
        if(!doIt(*m_prepared_account_select))
            return false;
        m_prepared_account_select->bindValue(0,quint64(data.m_auth_account_id));
        if(!doIt(*m_prepared_account_select))
            return false;
        if(!m_prepared_account_select->next() && !data.create_if_does_not_exist)
            return false;
    }
    result.m_game_server_acc_id = m_prepared_account_select->value("id").toULongLong();
    result.m_max_slots = m_prepared_account_select->value("max_slots").toULongLong();
    result.m_characters.resize(result.m_max_slots);
    int idx=0;
    m_prepared_char_select->bindValue(0,quint64(result.m_game_server_acc_id));
    for(auto &character : result.m_characters)
    {
        m_prepared_char_select->bindValue(1,uint16_t(idx));
        character.index = idx++;
        if(!doIt(*m_prepared_char_select))
            return false;
        if(!m_prepared_char_select->next())
        {
            character.reset(); // empty slot
            continue;
        }
        character.m_db_id = (m_prepared_char_select->value("id").toUInt());
        character.m_account_id = (m_prepared_char_select->value("account_id").toUInt());
        QString name=m_prepared_char_select->value("char_name").toString();
        character.m_name =  name.isEmpty() ? "EMPTY" : name;
        character.m_HitPoints = (m_prepared_char_select->value("hitpoints").toUInt());
        character.m_Endurance = (m_prepared_char_select->value("endurance").toUInt());
        character.m_serialized_chardata = m_prepared_char_select->value("chardata").toString();
        character.m_serialized_options  = m_prepared_char_select->value("options").toString();
        character.m_serialized_gui      = m_prepared_char_select->value("gui").toString();
        character.m_serialized_keybinds = m_prepared_char_select->value("keybinds").toString();

        GameAccountResponseCostumeData costume;
        // appearance related.

        costume.m_body_type = m_prepared_char_select->value("bodytype").toUInt();
        costume.m_height = m_prepared_char_select->value("height").toFloat();
        costume.m_physique = m_prepared_char_select->value("physique").toFloat();
        costume.m_slot_index = 0;
        costume.m_character_id = character.m_db_id;

        m_prepared_fill->bindValue(0,quint64(character.m_db_id));
        m_prepared_fill->bindValue(1,(uint16_t)costume.m_slot_index);
        if(!doIt(*m_prepared_fill))
            continue;

        if(!m_prepared_fill->next()) // retry with the first one
        {
            m_prepared_fill->bindValue(1,0); // get first costume

            if(!doIt(*m_prepared_fill))
                continue;
            if(!m_prepared_fill->next()) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) GameDbSyncContext::getAccount no costumes.\n")),false);
                continue;
            }
        }
        costume.skin_color = m_prepared_fill->value("skin_color").toUInt();
        costume.m_serialized_data = m_prepared_fill->value("parts").toString();
        character.m_costumes.emplace_back(costume);

    }
    return true;
}
bool GameDbSyncContext::removeCharacter(const RemoveCharacterRequestData &data)
{
    m_prepared_char_delete->bindValue(0,quint64(data.account_id));
    m_prepared_char_delete->bindValue(1,(uint32_t)data.slot_idx);
    if(!doIt(*m_prepared_char_delete))
        return false;
    return true;
}

bool GameDbSyncContext::checkNameClash(const WouldNameDuplicateRequestData &data, WouldNameDuplicateResponseData &result)
{
    m_prepared_char_exists->bindValue(0,data.m_name);
    if(!doIt(*m_prepared_char_exists))
        return false;
    if(!m_prepared_char_exists->next())
        return false;
    // TODO: handle case of multiple accounts with same name ?
    result.m_would_duplicate = m_prepared_char_exists->value(0).toBool();
    return true;
}
/*
#define STR_OR_EMPTY(c) ((!c.isEmpty()) ? c:"EMPTY")
#define STR_OR_VERY_EMPTY(c) ((c!=0) ? c:"")
bool CharacterDatabase::fill( Entity *e)
{
    assert(e);
    EntityData *ed = &e->m_entity_data;
#ifdef DEBUG_DB
    qDebug().noquote() << e->m_db_id;
#endif

    m_prepared_entity_select.bindValue(":id",quint64(e->m_char->m_db_id));
    if(!doIt(m_prepared_entity_select))
        return false;

    if(!m_prepared_entity_select.next()) // retry with the first one
    {
        m_prepared_entity_select.bindValue(":id",quint64(e->m_char->m_db_id));
        if(!doIt(m_prepared_entity_select))
            return false;
        if(!m_prepared_entity_select.next()) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill no matching entity.\n")),false);
            return false;
        }
    }

    e->m_supergroup.m_SG_id = (m_prepared_entity_select.value("supergroup_id").toUInt());

    QString entity_data;
    entity_data = (m_prepared_entity_select.value("entitydata").toString());
    serializeFromDb(*ed,entity_data);

    // Can't pass direction through cereal, so let's update it here.
    e->m_direction = fromCoHYpr(ed->m_orientation_pyr);

#ifdef DEBUG_DB
    qDebug().noquote() << entity_data;
    qDebug().noquote() << glm::to_string(ed->m_orientation_pyr).c_str();
    qDebug().noquote() << glm::to_string(e->m_direction).c_str();
#endif
    return true;
}
bool CharacterDatabase::fill( Character *c)
{
    CharacterData *cd = &c->m_char_data;
    assert(c&&c->getAccountId());

    serializeFromDb(*cd,char_data);

#ifdef DEBUG_DB
    qDebug() << "m_db_id:" << c->m_db_id;
    //qDebug().noquote() << char_data;
#endif

    CharacterCostume *main_costume = new CharacterCostume;
    c->m_costumes.push_back(main_costume);
    main_costume->serializeFromDb(serialized_parts);
    main_costume->m_non_default_costme_p = false;
    return true;
}

//TODO: Initialize this function with Entity instead of Character
bool CharacterDatabase::create(uint64_t gid, uint8_t slot, Entity *e)
{
    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));

    assert(e && e->m_char);

    EntityData *ed = &e->m_entity_data;
    Character &c(*e->m_char);

    assert(gid>0);
    assert(slot<8);

    CharacterData *cd = &c.m_char_data;
    cd->m_last_online = QDateTime::currentDateTimeUtc().toString();

    Costume *cst = c.getCurrentCostume();
    if(!cst) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CharacterDatabase::create cannot insert char without costume.\n"))
                         ,false);
    }

    //":slot_index, :account_id, :char_name, :chardata, :bodytype, :hitpoints, :endurance, "
    //":posx, :posy, :posz, :orientp, :orienty, :orientr, :supergroup_id, :options "

    m_prepared_char_insert.bindValue(":slot_index", uint32_t(slot));
    m_prepared_char_insert.bindValue(":account_id", quint64(gid));
    m_prepared_char_insert.bindValue(":char_name", c.m_name);
    m_prepared_char_insert.bindValue(":bodytype", c.getCurrentCostume()->m_body_type);
    m_prepared_char_insert.bindValue(":hitpoints", c.m_current_attribs.m_HitPoints);
    m_prepared_char_insert.bindValue(":endurance", c.m_current_attribs.m_Endurance);
    m_prepared_char_insert.bindValue(":supergroup_id", 0);
    m_prepared_char_insert.bindValue(":options", 0);

    QString entity_data;
    serializeToDb(*ed,entity_data);
    m_prepared_char_insert.bindValue(":entitydata", entity_data);

    QString char_data;
    serializeToDb(*cd,char_data);
    m_prepared_char_insert.bindValue(":chardata", char_data);

#ifdef DEBUG_DB
    qDebug().noquote() << entity_data;
    qDebug().noquote() << char_data;
#endif

    if(!doIt(m_prepared_char_insert))
        return false;
    int64_t char_id = m_prepared_char_insert.lastInsertId().toLongLong();
    c.m_db_id = char_id;

#ifdef DEBUG_DB
    qDebug() << "char_id: " << char_id << ":" << c->m_db_id;
#endif

    // create costume
    QString costume_parts;
    cst->serializeToDb(costume_parts);
    m_prepared_costume_insert.bindValue(":id",quint64(char_id));
    m_prepared_costume_insert.bindValue(":costume_index",uint32_t(0));
    m_prepared_costume_insert.bindValue(":skin_color",uint32_t(cst->skin_color));
    m_prepared_costume_insert.bindValue(":parts",costume_parts);

    if(!doIt(m_prepared_costume_insert))
        return false;
    return true;
}
bool sendEntityStoreRequest(Entity *e)
{
    assert(e);
    if(! HandlerLocator::getDBSync_Handler())
    {
        qCritical() << "Database sync service is not running";
    }
    EntityData &entity_data(e->m_entity_data);
    assert(e->m_char);
    Character &character(*e->m_char);
    Costume *cst = character.getCurrentCostume();
    if(!cst)
    {
        qCritical() << "Cannot store Entities without costumes.";
        return false;
    }

    CharacterUpdateData cud;
    cud.m_char_name = character.getName();
    serializeToDb(character.m_char_data,cud.m_char_data);
    serializeToDb(entity_data,cud.m_entitydata);
#ifdef DEBUG_DB
    qDebug().noquote() << cud.m_entitydata;
    qDebug().noquote() << cud.m_char_data;
#endif

    cud.m_options.clear(); //TODO: character.m_options, need to be serialized here
    cud.m_bodytype = character.getCurrentCostume()->m_body_type;
    cud.m_id = character.m_db_id;
    cud.m_supergroup_id = uint32_t(e->m_supergroup.m_SG_id);
    HandlerLocator::getDBSync_Handler()->putq(new CharacterUpdateMessage(std::move(cud)));

    // Update costume
    CostumeUpdateData costume_update_data;
    cst->serializeToDb(costume_update_data.m_parts);
    costume_update_data.m_db_id = character.m_db_id;
    costume_update_data.m_skin_color = uint32_t(cst->skin_color);
    costume_update_data.m_costume_index = 0;
    HandlerLocator::getDBSync_Handler()->putq(new CostumeUpdateMessage(std::move(costume_update_data)));
    return true;
}
bool CharacterDatabase::update( Entity *e )
{
    assert(e);
    assert(e->m_char);
    Character &c(*e->m_char);
    c.m_char_data.m_last_online = QDateTime::currentDateTimeUtc().toString();
    return sendEntityStoreRequest(e);
}

*/
