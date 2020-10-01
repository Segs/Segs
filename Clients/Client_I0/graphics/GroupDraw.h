#pragma once
#include "utils/helpers.h"
#include "renderer/RenderBonedModel.h"
#include <GL/glew.h>
#include <vector>
extern std::vector<struct DefTracker *> visibleTrays;
struct MemPool;
struct DefTracker
{
    Matrix4x3 matrix1;
    Vector3 mid;
    int size_of_ptr_arr_x28;
    int ptr_arr_x28;
    void *char_arr1_ex_grp;
    struct GroupDef *def;
    unsigned int flags;
    DefTracker *parent_group;
    DefTracker *subgroups;
    int num_subgroups;
    int def_mod_time;
    float radius;
    int def_related_id;
    struct tGrid6C *grids_in_use;
    int coll_time;
    int m_idx;
    Array *m_arr1;
    DefTracker **array_of_grps;
    DefTracker *ptr_to_strct_1;
    float size_of_arofptr1;
    int dyn_groups_max;
    struct TrickNode *instance_mods;
    GLuint RGB_VBO;
    struct Grid *light_grid;
    DefTracker **portal_groups;
    char num_portals;
    int draw_id;
    int unkn_A0;
    Handle handle;
};
static_assert (sizeof(DefTracker) == 0xA8);
struct GroupDef
{
    enum VisBlockerFlag
    {
        fFullBlocker = 1,
        fAngleBlocker = 2,
    };
    int num_subs;
    struct GroupEnt *subdefs;
    int u_8;
    char *ref_name;
    char *dir;
    char *def_type;
    Vector3 min_;
    Vector3 max_;
    Vector3 mid_;
    float vis_dist;
    float radius;
    int mod_time;

    uint32_t window : 1;
    uint32_t window_inside : 1;
    uint32_t saved : 1;
    uint32_t save_temp : 1;
    uint32_t open : 1;
    uint32_t shell : 1;
    uint32_t tray : 1;
    uint32_t region_marker : 1;// 7 - 0x80
    uint32_t volume_trigger : 1;// 8 - 0x100
    uint32_t water_volume : 1;// 9 - 0x200
    uint32_t lava_volume : 1;// 10 - 0x400
    uint32_t sewer_volume : 1;//11 - 0x800
    uint32_t door_volume : 1;//12 - 0x1000
    uint32_t obj_lib : 1;// 12 - 0x2000
    uint32_t lod_autogen : 1;// 13 - 0x4000
    uint32_t in_use : 2;// 14/15 - 0x18000
    uint32_t init : 1;// 16 - 0x20000
    uint32_t has_ambient : 1;// 18 - 0x40000
    uint32_t child_ambient : 1;// 19 - 0x80000
    uint32_t has_light : 1;
    int child_light;

    uint32_t has_sound : 1;// 0 - 1
    uint32_t sound_exclude : 1;// 1 - 2
    uint32_t lod_fromtrick : 1;// 2 - 4
    uint32_t vis_blocker : 2;// 1 VisBlocker, 2 VisAngleBlocker
    uint32_t vis_doorframe : 1;// 5 - 0x20
    uint32_t outside : 1;// 6  - 0x40
    uint32_t open_tracker : 1;// 7 - 0x80
    uint32_t has_fog : 1;// 0x100
    uint32_t has_beacon : 1;// 9 0x200
    uint32_t child_beacon : 1;// 10 - 0x400
    uint32_t has_tint_color : 1;// 11 - 0x800
    uint32_t has_tex : 2;//
    uint32_t has_fx : 1;// 0x4000
    uint32_t has_properties : 1;
    uint32_t child_properties : 1;
    uint32_t lib_ungroupable : 1;
    uint32_t parent_fade : 1;// 18 - 0x40000
    uint32_t child_parent_fade : 1;// 19 - 0x80000
    uint32_t lod_fadenode : 1;// 20 - 0x100000
    uint32_t key_light : 1;
    uint32_t breakable : 1;
    uint32_t child_breakable : 1;// 23
    int tag_id;
    int arr_entry_idx;
    float lod_near;
    float lod_far;
    float lod_near_fade;
    float lod_far_fade;
    float lod_scale;
    Model *model;
    int file_idx;
    float light_radius;
    char sound_vol;
    float sound_radius;
    float sound_ramp_feet;
    char *sound_name;
    uint32_t tint_colors[2];
    float fog_near;
    float fog_far;
    float fog_radius;
    char *Beacon_Name;
    float beacon_radius;
    float shadow_dist;
    char *tex_names[2];
    TextureBind *textures_arr[2];
    struct HashTable *groupprop_rel_map;
    int access_time;
    struct DefEntry *gf;
};
static_assert (sizeof(GroupDef) == 0xC8);
struct EditCommandState
{
    unsigned int *packet_ids;
    int sel;
    int selectall;
    int groundsnap;
    int slopesnap;
    int selcontents;
    int del;
    int libdel;
    int group;
    int ungroup;
    int unsel;
    int open;
    int editorig;
    int noerrcheck;
    int close;
    int closeinst;
    int setparent;
    int attach;
    int name;
    int setpivot;
    int hide;
    int unhide;
    int settype;
    int setfx;
    int mousewheelopenclose;
    int trayswap;
    float groundoffset;
    int setambient;
    int maxbright;
    int lightcolor;
    int lightsize;
    int colorbynumber;
    int colormemory;
    int soundsize;
    int soundvol;
    int soundname;
    int soundramp;
    int soundfind;
    int soundxclude;
    int fognear;
    int fogfar;
    uint32_t fogcolor1;
    uint32_t fogcolor2;
    int fogsize;
    int gridsize;
    int rotsize;
    int plane;
    int posrot;
    int scale;
    int zerogrid;
    int polycount;
    int canhide;
    int ignoretray;
    int objinfo;
    int cut;
    int copy;
    int paste;
    int undo;
    int showfog;
    int showvis;
    int snaptype;
    int nosnap;
    int snap3;
    int snaptovert;
    int singleaxis;
    int localrot;
    int look;
    int pan;
    int zoom;
    int lookat;
    int setview;
    int camview;
    int load;
    int import;
    int _new;
    int save;
    int savesel;
    int saveas;
    int savelibs;
    int scenefile;
    int lodscale;
    int lodnear;
    int lodfar;
    int lodnearfade;
    int lodfarfade;
    int loddisable;
    int lodenable;
    int lodfadenode;
    int showall;
    int beaconname;
    int beaconsize;
    int showconnection;
    int beaconpathstart;
    int beaconpathcontinue;
    int defineconnection;
    int beaconradii;
    int shownbeacon;
    int beaconselect;
    int togglequickplace;
    int quickplacement;
    int quickrotate;
    int setquickobject;
    int selecteditonly;
    int openonselection;
    Matrix3x3 quickPlacementMat3;
    char quickplacementobject[256];
    uint32_t tintcolor1;
    uint32_t tintcolor2;
    int tintremove;
    int replacetex1;
    int replacetex2;
    int removetex;
    int addProp;
    int removeProp;
    int showAsRadius;
    int showAsString;
    int editPropRadius;
    int ungroupall;
    int groupall;
    int checkin;
    int select_notselectable;
    DefTracker *tracker_under_mouse;
    DefTracker *last_selected_tracker;
};
static_assert (sizeof(EditCommandState) == 0x308);
struct FxMiniTracker
{
    int count;
    Handle fxIds[5];
};

struct GroupEnt
{
    const Matrix4x3 *transform=nullptr;
    GroupDef * m_def     = nullptr;
    char flags_cache=0;
    char flg2=0;
    char flg3=0;
    char flg4=0;
    float vis_dist_sqr_cache=0;
    float vist_dist_cache=0;
    Vector3 mid_cache {0,0,0};
};
static_assert (sizeof(GroupEnt) == 0x20);
struct DrawParams
{
    float scale = 0;
    DefTracker *pGrp = nullptr;
    void *tint_color_ptr = nullptr;
    TextureBind *tex_binds[2] = { nullptr,nullptr };
    Vector3 node_mid = { 0,0,0 };
};
static_assert (sizeof(DrawParams) == 0x20);
struct ModelExtraPortalRel
{
    Vector3 pos;
    float radius;
};
struct ModelExtra
{
    int portal_count;
    ModelExtraPortalRel portals[8];
};
static_assert (sizeof(ModelExtra) == 0x84);
#pragma pack(push, 1)
struct GroupInfo
{
    GroupDef **defs_arr;
    DefTracker **refs;
    int def_count;
    int def_max;
    int ref_count;
    int ref_max;
    int ref_mod_time;
    int def_access_time;
    int *ref_idxs;
    int *def_idxs;
    char filename[256];
    char scene_name[256];
    HashTable *def_ordinal_names_htab;
    MemPool *MP_128Strings;
    MemPool *matrix_pool_PP;
    MemPool *def_mem_pool;
    int loading;
    int grp_idcnt;
};
#pragma pack(pop)
extern int camera_is_inside;

extern void patch_groupdraw();
extern int segs_groupDrawRefs(const Matrix4x3 &parent_mat);
