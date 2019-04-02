#pragma once
#include "Common/Runtime/HandleBasedStorage.h"

#include <QHash>

struct SceneModifiers;

namespace SEGS
{
using HTexture = SingularStoreHandleT<struct TextureWrapper>;
struct PrefabStore;

struct RuntimeData
{
    //! Here we store handles to loaded texture headers
    QHash<QString, HTexture> m_loaded_textures;
    //! map from texture name to full file path
    QHash<QString, QString> m_texture_paths;
    PrefabStore *           m_prefab_mapping = nullptr; //!< maps directories and model names to geosets
    SceneModifiers *        m_modifiers      = nullptr;

    bool prepare(const QString &directory_path);

    bool read_prefab_definitions(const QString &directory_path);
    bool read_model_modifiers(const QString &directory_path);
};
void preloadTextureNames(const QString &basepath);
} //end of SEGS namespace

extern SEGS::RuntimeData& getRuntimeData();
