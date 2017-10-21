#pragma once

#include <QHash>
#include <vector>
#include <Lutefisk3D/Container/Ptr.h>
#include <Lutefisk3D/Graphics/Texture.h>
#include <Lutefisk3D/Math/Vector2.h>

struct TextureModifiers;
class QString;
namespace Urho3D
{
class Texture;
class Context;
}
enum class CoHBlendMode : uint8_t;

struct TextureWrapper
{
    enum
    {
        ALPHA          = 0x1,
        RGB8           = 0x2,
        COMP4          = 0x4,
        COMP8          = 0x8,
        DUAL           = 0x10,
        CLAMP          = 0x80,
        CUBEMAPFACE    = 0x200,
        REPLACEABLE    = 0x400,
        BUMPMAP        = 0x800,
        BUMPMAP_MIRROR = 0x1000,
    };
    Urho3D::SharedPtr<Urho3D::Texture> base {nullptr};
    QString detailname;
    QString bumpmap;
    int flags {0};
    Urho3D::Vector2 scaleUV0 {0,0};
    Urho3D::Vector2 scaleUV1 {0,0};
    CoHBlendMode BlendType = CoHBlendMode(0);
    TextureModifiers *info {nullptr};
};
enum TexOpt : uint32_t
{
    FADE        = 1,
    DUAL        = 0x10,
    REPLACEABLE = 0x800,
    BUMPMAP     = 0x1000,
};
extern QHash<QString,TextureModifiers *> g_texture_path_to_mod;
TextureWrapper tryLoadTexture(Urho3D::Context *ctx, const QString &fname);
void preloadTextureNames();
std::vector<TextureWrapper> getModelTextures(Urho3D::Context *ctx,std::vector<QString> &a1);
