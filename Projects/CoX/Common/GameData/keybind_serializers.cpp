#include "keybind_serializers.h"
#include "keybind_definitions.h"
#include "serialization_common.h"

#include "DataStorage.h"
namespace {
static const QMap<QString,KeyName> keyNameToEnum = {
    {"ESCAPE",COH_INPUT_ESCAPE},
    {"1",COH_INPUT_1},
    {"2",COH_INPUT_2},
    {"3",COH_INPUT_3},
    {"4",COH_INPUT_4},
    {"5",COH_INPUT_5},
    {"6",COH_INPUT_6},
    {"7",COH_INPUT_7},
    {"8",COH_INPUT_8},
    {"9",COH_INPUT_9},
    {"0",COH_INPUT_0},
    {"MINUS",COH_INPUT_MINUS},
    {"EQUALS",COH_INPUT_EQUALS},
    {"TAB",COH_INPUT_TAB},
    {"Q",COH_INPUT_Q},
    {"W",COH_INPUT_W},
    {"E",COH_INPUT_E},
    {"R",COH_INPUT_R},
    {"T",COH_INPUT_T},
    {"Y",COH_INPUT_Y},
    {"U",COH_INPUT_U},
    {"I",COH_INPUT_I},
    {"O",COH_INPUT_O},
    {"P",COH_INPUT_P},
    {"LBRACKET",COH_INPUT_LBRACKET},
    {"RBRACKET",COH_INPUT_RBRACKET},
    {"RETURN",COH_INPUT_RETURN},
    {"LCONTROL",COH_INPUT_LCONTROL},
    {"A",COH_INPUT_A},
    {"S",COH_INPUT_S},
    {"D",COH_INPUT_D},
    {"F",COH_INPUT_F},
    {"G",COH_INPUT_G},
    {"H",COH_INPUT_H},
    {"J",COH_INPUT_J},
    {"K",COH_INPUT_K},
    {"L",COH_INPUT_L},
    {"SEMICOLON",COH_INPUT_SEMICOLON},
    {"APOSTROPHE",COH_INPUT_APOSTROPHE},
    {"TILDE",COH_INPUT_TILDE},
    {"LSHIFT",COH_INPUT_LSHIFT},
    {"BACKSLASH",COH_INPUT_BACKSLASH},
    {"Z",COH_INPUT_Z},
    {"X",COH_INPUT_X},
    {"C",COH_INPUT_C},
    {"V",COH_INPUT_V},
    {"B",COH_INPUT_B},
    {"N",COH_INPUT_N},
    {"M",COH_INPUT_M},
    {"COMMA",COH_INPUT_COMMA},
    {"PERIOD",COH_INPUT_PERIOD},
    {"SLASH",COH_INPUT_SLASH},
    {"RSHIFT",COH_INPUT_RSHIFT},
    {"MULTIPLY",COH_INPUT_MULTIPLY},
    {"SPACE",COH_INPUT_SPACE},
    {"F1",COH_INPUT_F1},
    {"F2",COH_INPUT_F2},
    {"F3",COH_INPUT_F3},
    {"F4",COH_INPUT_F4},
    {"F5",COH_INPUT_F5},
    {"F6",COH_INPUT_F6},
    {"F7",COH_INPUT_F7},
    {"F8",COH_INPUT_F8},
    {"F9",COH_INPUT_F9},
    {"F10",COH_INPUT_F10},
    {"NUMLOCK",COH_INPUT_NUMLOCK},
    {"SCROLL",COH_INPUT_SCROLL},
    {"NUMPAD7",COH_INPUT_NUMPAD7},
    {"NUMPAD8",COH_INPUT_NUMPAD8},
    {"NUMPAD9",COH_INPUT_NUMPAD9},
    {"NUMPAD4",COH_INPUT_NUMPAD4},
    {"NUMPAD5",COH_INPUT_NUMPAD5},
    {"NUMPAD6",COH_INPUT_NUMPAD6},
    {"NUMPAD1",COH_INPUT_NUMPAD1},
    {"NUMPAD2",COH_INPUT_NUMPAD2},
    {"NUMPAD3",COH_INPUT_NUMPAD3},
    {"NUMPAD0",COH_INPUT_NUMPAD0},
    {"F11",COH_INPUT_F11},
    {"F12",COH_INPUT_F12},
    {"F13",COH_INPUT_F13},
    {"F14",COH_INPUT_F14},
    {"F15",COH_INPUT_F15},
    {"NUMPADEQUALS",COH_INPUT_NUMPADEQUALS},
    {"ABNT_C1",COH_INPUT_ABNT_C1},
    {"ABNT_C2",COH_INPUT_ABNT_C2},
    {"COLON",COH_INPUT_COLON},
    {"UNDERLINE",COH_INPUT_UNDERLINE},
    {"NEXTTRACK",COH_INPUT_NEXTTRACK},
    {"NUMPADENTER",COH_INPUT_NUMPADENTER},
    {"RCONTROL",COH_INPUT_RCONTROL},
    {"MUTE",COH_INPUT_MUTE},
    {"CALCULATOR",COH_INPUT_CALCULATOR},
    {"PLAYPAUSE",COH_INPUT_PLAYPAUSE},
    {"MEDIASTOP",COH_INPUT_MEDIASTOP},
    {"VOLUMEDOWN",COH_INPUT_VOLUMEDOWN},
    {"VOLUMEUP",COH_INPUT_VOLUMEUP},
    {"WEBHOME",COH_INPUT_WEBHOME},
    {"NUMPADCOMMA",COH_INPUT_NUMPADCOMMA},
    {"SYSRQ",COH_INPUT_SYSRQ},
    {"PAUSE",COH_INPUT_PAUSE},
    {"HOME",COH_INPUT_HOME},
    {"END",COH_INPUT_END},
    {"INSERT",COH_INPUT_INSERT},
    {"DELETE",COH_INPUT_DELETE},
    {"WEBSEARCH",COH_INPUT_WEBSEARCH},
    {"WEBFAVORITES",COH_INPUT_WEBFAVORITES},
    {"WEBREFRESH",COH_INPUT_WEBREFRESH},
    {"WEBSTOP",COH_INPUT_WEBSTOP},
    {"WEBFORWARD",COH_INPUT_WEBFORWARD},
    {"WEBBACK",COH_INPUT_WEBBACK},
    {"MYCOMPUTER",COH_INPUT_MYCOMPUTER},
    {"MAIL",COH_INPUT_MAIL},
    {"MEDIASELECT",COH_INPUT_MEDIASELECT},
    {"BACKSPACE",COH_INPUT_BACKSPACE},
    {"NUMPADSTAR",COH_INPUT_NUMPADSTAR},
    {"LALT",COH_INPUT_LALT},
    {"CAPSLOCK",COH_INPUT_CAPSLOCK},
    {"NUMPADMINUS",COH_INPUT_NUMPADMINUS},
    {"NUMPADPLUS",COH_INPUT_NUMPADPLUS},
    {"NUMPADPERIOD",COH_INPUT_NUMPADPERIOD},
    {"NUMPADSLASH",COH_INPUT_NUMPADSLASH},
    {"RALT",COH_INPUT_RALT},
    {"UPARROW",COH_INPUT_UPARROW},
    {"PGUP",COH_INPUT_PGUP},
    {"LEFTARROW",COH_INPUT_LEFTARROW},
    {"RIGHTARROW",COH_INPUT_RIGHTARROW},
    {"DOWNARROW",COH_INPUT_DOWNARROW},
    {"PGDN",COH_INPUT_PGDN},
    {"LBUTTON",COH_INPUT_LBUTTON},
    {"MBUTTON",COH_INPUT_MBUTTON},
    {"RBUTTON",COH_INPUT_RBUTTON},
    {"BUTTON4",COH_INPUT_BUTTON4},
    {"BUTTON5",COH_INPUT_BUTTON5},
    {"BUTTON6",COH_INPUT_BUTTON6},
    {"BUTTON7",COH_INPUT_BUTTON7},
    {"BUTTON8",COH_INPUT_BUTTON8},
    {"MOUSE_SCROLL",COH_INPUT_MOUSE_SCROLL},
    {"JOY1",COH_INPUT_JOY1},
    {"JOY2",COH_INPUT_JOY2},
    {"JOY3",COH_INPUT_JOY3},
    {"JOY4",COH_INPUT_JOY4},
    {"JOY5",COH_INPUT_JOY5},
    {"JOY6",COH_INPUT_JOY6},
    {"JOY7",COH_INPUT_JOY7},
    {"JOY8",COH_INPUT_JOY8},
    {"JOY9",COH_INPUT_JOY9},
    {"JOY10",COH_INPUT_JOY10},
    {"JOY11",COH_INPUT_JOY11},
    {"JOY12",COH_INPUT_JOY12},
    {"JOY13",COH_INPUT_JOY13},
    {"JOY14",COH_INPUT_JOY14},
    {"JOY15",COH_INPUT_JOY15},
    {"JOY16",COH_INPUT_JOY16},
    {"JOY17",COH_INPUT_JOY17},
    {"JOY18",COH_INPUT_JOY18},
    {"JOY19",COH_INPUT_JOY19},
    {"JOY20",COH_INPUT_JOY20},
    {"JOY21",COH_INPUT_JOY21},
    {"JOY22",COH_INPUT_JOY22},
    {"JOY23",COH_INPUT_JOY23},
    {"JOY24",COH_INPUT_JOY24},
    {"JOY25",COH_INPUT_JOY25},
    {"JOYPAD_UP",COH_INPUT_JOYPAD_UP},
    {"JOYPAD_DOWN",COH_INPUT_JOYPAD_DOWN},
    {"JOYPAD_LEFT",COH_INPUT_JOYPAD_LEFT},
    {"JOYPAD_RIGHT",COH_INPUT_JOYPAD_RIGHT},
    {"POV1_UP",COH_INPUT_POV1_UP},
    {"POV1_DOWN",COH_INPUT_POV1_DOWN},
    {"POV1_LEFT",COH_INPUT_POV1_LEFT},
    {"POV1_RIGHT",COH_INPUT_POV1_RIGHT},
    {"POV2_UP",COH_INPUT_POV2_UP},
    {"POV2_DOWN",COH_INPUT_POV2_DOWN},
    {"POV2_LEFT",COH_INPUT_POV2_LEFT},
    {"POV2_RIGHT",COH_INPUT_POV2_RIGHT},
    {"POV3_UP",COH_INPUT_POV3_UP},
    {"POV3_DOWN",COH_INPUT_POV3_DOWN},
    {"POV3_LEFT",COH_INPUT_POV3_LEFT},
    {"POV3_RIGHT",COH_INPUT_POV3_RIGHT},
    {"JOYSTICK1_UP",COH_INPUT_JOYSTICK1_UP},
    {"JOYSTICK1_DOWN",COH_INPUT_JOYSTICK1_DOWN},
    {"JOYSTICK1_LEFT",COH_INPUT_JOYSTICK1_LEFT},
    {"JOYSTICK1_RIGHT",COH_INPUT_JOYSTICK1_RIGHT},
    {"JOYSTICK2_UP",COH_INPUT_JOYSTICK2_UP},
    {"JOYSTICK2_DOWN",COH_INPUT_JOYSTICK2_DOWN},
    {"JOYSTICK2_LEFT",COH_INPUT_JOYSTICK2_LEFT},
    {"JOYSTICK2_RIGHT",COH_INPUT_JOYSTICK2_RIGHT},
    {"JOYSTICK3_UP",COH_INPUT_JOYSTICK3_UP},
    {"JOYSTICK3_DOWN",COH_INPUT_JOYSTICK3_DOWN},
    {"JOYSTICK3_LEFT",COH_INPUT_JOYSTICK3_LEFT},
    {"JOYSTICK3_RIGHT",COH_INPUT_JOYSTICK3_RIGHT},
};
KeyName resolveKey(const QString &name)
{
    auto iter = keyNameToEnum.find(name.toUpper());
    if(iter!=keyNameToEnum.end())
        return *iter;
    return COH_INPUT_INVALID;
}
bool loadFrom(BinStore * s, Keybind & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.Key);
    ok &= s->read(target.Command);
    ok &= s->prepare_nested(); // will update the file size left
    target.actualKey = resolveKey(target.Key);
    assert(ok && s->end_encountered());
    return ok;
}
bool loadFrom(BinStore * s, KeyProfiles_Entry & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.DisplayName);
    ok &= s->read(target.Name);
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
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
    assert(ok && s->end_encountered());
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

}

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

CEREAL_CLASS_VERSION(Keybinds, 1); // register Keybinds class version

template<class Archive>
void serialize(Archive &archive, Keybind &k, uint32_t const version)
{
    archive(cereal::make_nvp("ActualKey",k.actualKey));
    archive(cereal::make_nvp("Key",k.Key));
    archive(cereal::make_nvp("Command",k.Command));
}

template<class Archive>
void serialize(Archive &archive, KeybindData &k, uint32_t const version)
{
    archive(cereal::make_nvp("Key",k.key));
    archive(cereal::make_nvp("Mods",k.mods));
    archive(cereal::make_nvp("Command",k.command));
}

template<class Archive>
void serialize(Archive &archive, KeyProfiles_Entry &k, uint32_t const version)
{
    archive(cereal::make_nvp("DisplayName",k.DisplayName));
    archive(cereal::make_nvp("Name",k.Name));
    archive(cereal::make_nvp("KeybindArr",k.KeybindArr));
}

template<class Archive>
void serialize(Archive &archive, CommandEntry &k, uint32_t const version)
{
    archive(cereal::make_nvp("KeyName",k.keyname));
    archive(cereal::make_nvp("ModKeys",k.key_mods));
}

template<class Archive>
void serialize(Archive &archive, Command &k, uint32_t const version)
{
    archive(cereal::make_nvp("CommandString",k.CmdString));
    archive(cereal::make_nvp("DisplayName",k.DisplayName));
    archive(cereal::make_nvp("Binding",k.bound_to));
}

template<class Archive>
void serialize(Archive &archive, CommandCategory_Entry &k, uint32_t const version)
{
    archive(cereal::make_nvp("DisplayName",k.DisplayName));
    archive(cereal::make_nvp("Commands",k.commands));
}

template<class Archive>
void serialize(Archive &archive, Parse_AllKeyProfiles &k, uint32_t const version)
{
    archive(cereal::make_nvp("KeyProfiles",k));
}

template<class Archive>
void serialize(Archive &archive, Parse_AllCommandCategories &k, uint32_t const version)
{
    archive(cereal::make_nvp("AllCommandCategories",k));
}

// TODO: Which of these do we really need?
template<class Archive>
void serialize(Archive &archive, Keybinds &kbds, uint32_t const version)
{
    archive(cereal::make_nvp("Profile",kbds.m_cur_keybind_profile));
    archive(cereal::make_nvp("KeyBinds",kbds.binds));
}

void saveTo(const Keybinds &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"Keybinds",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &archive, Keybinds &bds, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &archive, Keybinds &bds, uint32_t const version);

void serializeToDb(const Keybinds &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(Keybinds &data,const QString &src)
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
