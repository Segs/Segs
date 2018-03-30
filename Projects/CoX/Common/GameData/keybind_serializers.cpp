#include "keybind_serializers.h"
#include "keybind_definitions.h"
#include "serialization_common.h"

#include "Servers/MapServer/Events/KeybindSettings.h"
#include "DataStorage.h"

namespace {
KeyName resolveKey(const QString &name)
{
    auto iter = keyNameToEnum.find(name.toUpper());
    if(iter!=keyNameToEnum.end())
        return *iter;
    return COH_INPUT_INVALID;
}
ModKeys resolveMod(const QString &name)
{
    auto iter = modNameToEnum.find(name.toUpper());
    if(iter!=modNameToEnum.end())
        return *iter;
    return NO_MOD;
}
std::vector<Keybind> correctSecondaryBinds(std::vector<Keybind> &keybinds)
{
    QSet<QString> known_commands;
    for(Keybind & bind : keybinds)  {
        if(known_commands.contains(bind.Command)) {
            qCDebug(logKeybinds) << "Found duplicate keybind " + bind.Command + " Setting to alternate.";
            bind.IsSecondary = true;
        }
        else {
            known_commands.insert(bind.Command);
        }
    }
    return keybinds;
}
bool loadFrom(BinStore * s, Keybind & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.KeyString);
    ok &= s->read(target.Command);
    ok &= s->prepare_nested(); // will update the file size left

    QStringList combo = target.KeyString.split("+");

    if(combo.size() > 1)
    {
        target.Mods = resolveMod(combo.at(0));
        target.Key  = resolveKey(combo.at(1));
    }
    else
        target.Key = resolveKey(target.KeyString);

    qCDebug(logKeybinds) << "\tbind:" << target.KeyString << target.Key << target.Mods << target.Command;

    assert(ok && s->end_encountered());
    return ok;
}
bool loadFrom(BinStore * s, Keybind_Profiles & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.DisplayName);
    ok &= s->read(target.Name);
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;

    qCDebug(logKeybinds) << "Loading Profile:" << target.DisplayName << target.Name;

    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("KeyBind")==0) {
            target.KeybindArr.emplace_back();
            ok &= loadFrom(s,target.KeybindArr.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }

    target.KeybindArr = correctSecondaryBinds(target.KeybindArr);

    qCDebug(logKeybinds) << "Total Keybinds:" << target.KeybindArr.size();

    assert(ok);
    return ok;
}

bool loadFrom(BinStore * s, Command & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.CmdString);
    ok &= s->read(target.DisplayName);
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;

    qCDebug(logKeybinds) << target.CmdString << target.DisplayName;

    assert(ok);
    return ok;
}

bool loadFrom(BinStore * s, CommandCategory_Entry & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.DisplayName);
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Command")==0) {
            target.commands.emplace_back();
            ok &= loadFrom(s,target.commands.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

} // Anonymous namespace

bool loadFrom(BinStore * s, Parse_AllKeyProfiles & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("KeyProfile")==0) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

bool loadFrom(BinStore * s, Parse_AllCommandCategories & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("CommandCategory")==0) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

CEREAL_CLASS_VERSION(KeybindSettings, 1); // register Keybinds class version

template<class Archive>
void serialize(Archive &archive, Keybind &k)
{
    archive(cereal::make_nvp("Key",k.Key));
    archive(cereal::make_nvp("Mods",k.Mods));
    archive(cereal::make_nvp("KeyString",k.KeyString));
    archive(cereal::make_nvp("Command",k.Command));
    archive(cereal::make_nvp("IsSecondary",k.IsSecondary));
}

template<class Archive>
void serialize(Archive &archive, Keybind_Profiles &kp)
{
    archive(cereal::make_nvp("DisplayName",kp.DisplayName));
    archive(cereal::make_nvp("Name",kp.Name));
    archive(cereal::make_nvp("KeybindArr",kp.KeybindArr));
}

template<class Archive>
void serialize(Archive &archive, KeybindSettings &kbds, uint32_t const version)
{
    archive(cereal::make_nvp("AllProfiles",kbds.m_keybind_profiles));
}

template<class Archive>
void serialize(Archive &archive, CommandEntry &k, uint32_t const version)
{
    archive(cereal::make_nvp("Key",k.Key));
    archive(cereal::make_nvp("Mods",k.Mods));
}

template<class Archive>
void serialize(Archive &archive, Command &k, uint32_t const version)
{
    archive(cereal::make_nvp("CommandString",k.CmdString));
    archive(cereal::make_nvp("DisplayName",k.DisplayName));
    archive(cereal::make_nvp("CommandArr",k.CommandArr));
}

template<class Archive>
void serialize(Archive &archive, CommandCategory_Entry &k, uint32_t const version)
{
    archive(cereal::make_nvp("DisplayName",k.DisplayName));
    archive(cereal::make_nvp("Commands",k.commands));
}

template<class Archive>
void serialize(Archive &archive, Parse_AllCommandCategories &k, uint32_t const version)
{
    archive(cereal::make_nvp("AllCommandCategories",k));
}

void saveTo(const KeybindSettings &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"KeybindSettings",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &archive, KeybindSettings &bds, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &archive, KeybindSettings &bds, uint32_t const version);

void serializeToDb(const KeybindSettings &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(KeybindSettings &data,const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}
