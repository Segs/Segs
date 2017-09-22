#ifndef COHMODELCONVERTER_H
#define COHMODELCONVERTER_H
#include <Lutefisk3D/Graphics/StaticModel.h>

struct ConvertedModel;
struct ConvertedNode;
struct DefChild;
Urho3D::StaticModel *convertToLutefiskModel(Urho3D::Context *ctx, Urho3D::Node *tgtnode, ConvertedNode *def);
#endif // COHMODELCONVERTER_H
