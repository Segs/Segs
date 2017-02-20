#include "Common/GameData/DataStorage.h"
#include "Common/GameData/shop_definitions.h"
#include "Common/GameData/costume_serializers.h"
#include "Common/GameData/costume_definitions.h"

#include "Common/GameData/scenegraph_serializers.h"
#include "Common/GameData/scenegraph_definitions.h"
#include "Common/GameData/bodypart_serializers.h"
#include "Common/GameData/shop_serializers.h"
//#include "Common/GameData/seq_serializers.h"
//#include "Common/GameData/def_serializers.h"
//#include "Common/GameData/particlesys_serializers.h"
#include "Common/GameData/map_serializers.h"
#include "Common/GameData/map_definitions.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QFileInfo>

namespace {
enum BinType {
    eInvalid,
    eLevelsDebts,
    eCombineChances,
    eBoostEffectiveness,
    eParticleSystems,
    eShops,
    eShopItems,
    eShopDepts,
    eSequencers,
    eTailorCosts,
    eCostumeSets,
    eBodyParts,
    ePaletteSets,
    eGroupEmblems,
    eZones,
    eSceneGraph,
};
static const QHash<uint32_t,BinType> knownSerializers = {
//    {levelsdebts_i0_requiredCrc         , eLevelsDebts},
//    {combining_i0_requiredCrc           , eCombineChances},
//    {boosteffectiveness_i0_requiredCrc  , eBoostEffectiveness},
//    {particlesystems_i0_requiredCrc     , eParticleSystems},
    {shoplist_i0_requiredCrc            , eShops},
    {shopitems_i0_requiredCrc           , eShopItems},
    {shopdepts_i0_requiredCrc           , eShopDepts},
//    {seqencerlist_i0_requiredCrc        , eSequencers},
    {tailorcosts_i0_requiredCrc         , eTailorCosts},
    {costumesets_i0_requiredCrc         , eCostumeSets},
    {bodyparts_i0_requiredCrc           , eBodyParts},
    {palette_i0_requiredCrc             , ePaletteSets},
    {geoset_i0_requiredCrc              , eGroupEmblems },
    {zones_i0_requiredCrc               , eZones},
    {scenegraph_i0_requiredCrc              , eSceneGraph},
};
BinType getLoader(const QString &fname)
{
    QFile fl(fname);
    if(!fl.exists() || !fl.open(QFile::ReadOnly)) {
        qDebug() << "Cannot open file:" << fname;
        return eInvalid;
    }
    if(fl.size()<12 || fl.read(8) != "CrypticS") {
        qDebug() << "File "<<fname<<"is missing magic 'CrypticS' string";
        return eInvalid;
    }
    uint32_t template_hash;
    fl.read((char *)&template_hash,sizeof(template_hash));
    return knownSerializers.value(template_hash,eInvalid);
}
template<class T>
T * doLoad(BinStore &bs) {
    T *res = new T;
    if(!loadFrom(&bs,res)) {
        delete res;
        res = nullptr;
    }
    return res;
}
template<class T>
T * doLoadRef(BinStore &bs) {
    T *res = new T;
    if(!loadFrom(&bs,*res)) {
        delete res;
        res = nullptr;
    }
    return res;
}
template<class T>
bool doConvert(T *src_struct,const QString &fname,bool text_format=false)
{
    saveTo(*src_struct,fname,text_format);
    delete src_struct;
    return false;
}
void showSupportedBinTypes()
{
    qDebug()<<"Currently supported file types ";
    qDebug()<<"   I0<"<<QString::number(shoplist_i0_requiredCrc,16)<<"> Shops data - 'stores.bin'";
    qDebug()<<"   I0<"<<QString::number(shopitems_i0_requiredCrc,16)<<"> Shops items- 'items.bin'";
    qDebug()<<"   I0<"<<QString::number(shopdepts_i0_requiredCrc,16)<<"> Shop department names data - 'depts.bin'";
    qDebug()<<"   I0<"<<QString::number(tailorcosts_i0_requiredCrc,16)<<"> Tailoring cost data - 'tailorcost.bin'";
    qDebug()<<"   I0<"<<QString::number(costumesets_i0_requiredCrc,16)<<"> Costume part data - 'costume.bin'";
    qDebug()<<"   I0<"<<QString::number(bodyparts_i0_requiredCrc,16)<<"> Body part data - 'BodyParts.bin'";
    qDebug()<<"   I0<"<<QString::number(geoset_i0_requiredCrc,16)<<"> Supergroup emblem data - 'supergroupEmblems.bin'";
    qDebug()<<"   I0<"<<QString::number(palette_i0_requiredCrc,16)<<"> Color palette data - 'supergroupColors.bin'";
    qDebug()<<"   I0<"<<QString::number(scenegraph_i0_requiredCrc,16)<<"> Scene graph - 'geobin/*'";
    qDebug()<<"I0 - issue 0 ";
    qDebug()<<"Numbers in brackets are file CRCs - bytes 8 to 13 in the bin.";
}
}
int main(int argc,char **argv)
{
    QCoreApplication app(argc,argv);
    if(app.arguments().size()<2) {
        showSupportedBinTypes();
        return -1;
    }
    BinType bin_type = getLoader(app.arguments()[1]);
    if(bin_type==eInvalid) {
        qCritical() << "Unhandled bin file type";
        showSupportedBinTypes();
        return -1;
    }
    BinStore binfile;
    binfile.open(argv[1],0);
    QString target_basename=QFileInfo(argv[1]).baseName();
    bool json_output=true;
    if(app.arguments().size()>2)
        json_output = app.arguments()[2].toInt()!=0;
    switch(bin_type) {
//        case eLevelsDebts:    doConvert(doLoad<LevelExpAndDebt>(binfile),target_basename,json_output); break;
//        case eCombineChances: doConvert(doLoad<Parse_Combining>(binfile),target_basename,json_output); break;
//        case eBoostEffectiveness: doConvert(doLoad<Parse_Effectiveness>(binfile),target_basename,json_output); break;
//        case eParticleSystems:doConvert(doLoad<Parse_AllPSystems>(binfile),target_basename,json_output); break;
        case eShops:        doConvert(doLoadRef<AllShops_Data>(binfile),target_basename,json_output); break;
        case eShopItems:    doConvert(doLoad<AllShopItems_Data>(binfile),target_basename,json_output); break;
        case eShopDepts:    doConvert(doLoad<AllShopDepts_Data>(binfile),target_basename,json_output); break;
//        case eSequencers:   doConvert(doLoad<SequencerList>(binfile),target_basename,json_output); break;
        case eTailorCosts:  doConvert(doLoad<AllTailorCosts_Data>(binfile),target_basename,json_output); break;
        case eCostumeSets:  doConvert(doLoad<CostumeSet_Data>(binfile),target_basename,json_output); break;
        case eBodyParts:    doConvert(doLoad<AllBodyParts_Data>(binfile),target_basename,json_output); break;
        case eGroupEmblems: doConvert(doLoad<GeoSet_Data>(binfile),target_basename,json_output); break;
        case ePaletteSets:  doConvert(doLoad<Pallette_Data>(binfile),target_basename,json_output); break;
        case eZones:        doConvert(doLoadRef<AllMaps_Data>(binfile),target_basename,json_output); break;
        case eSceneGraph:   doConvert(doLoadRef<SceneGraph_Data>(binfile),target_basename,json_output); break;
        default:
            break;
    }
    return 0;
}

