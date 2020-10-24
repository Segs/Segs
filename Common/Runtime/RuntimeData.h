#pragma once
#include "Common/Runtime/HandleBasedStorage.h"

#include <QHash>

struct SceneModifiers;
struct FSWrapper;
namespace SEGS
{
    struct RuntimeData;
}
extern SEGS::RuntimeData& getRuntimeData();
namespace SEGS
{
using HTexture = SingularStoreHandleT<20,12,struct TextureWrapper>;
struct PrefabStore;

struct RuntimeData
{
    //! Here we store handles to loaded texture headers
    QHash<QByteArray, HTexture> m_loaded_textures;
    //! map from texture name to full file path
    QHash<QByteArray, QByteArray> m_texture_paths;
    PrefabStore *           m_prefab_mapping = nullptr; //!< maps directories and model names to geosets
    SceneModifiers *        m_modifiers      = nullptr;
    FSWrapper *             m_wrapper        = nullptr;
    bool                    m_ready          = false; //!< set to true if runtime data was read.
    bool prepare(FSWrapper *fs,const QByteArray &directory_path);


    bool read_prefab_definitions(const QByteArray &directory_path);
    bool read_model_modifiers(const QByteArray &directory_path);
    // This is a non-copyable type
    RuntimeData(const RuntimeData &) = delete;
    RuntimeData &operator=(const RuntimeData&) = delete;
private:
    friend RuntimeData& ::getRuntimeData();
    RuntimeData() = default;
};
} //end of SEGS namespace

