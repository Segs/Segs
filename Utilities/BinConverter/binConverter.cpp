/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup BinConverter Projects/CoX/Utilities/BinConverter
 * @{
 */

#include "GameData/DataStorage.h"
#include "GameData/CharacterAttributes.h"
#include "GameData/attrib_serializers.h"
#include "GameData/bodypart_serializers.h"
#include "GameData/bodypart_definitions.h"
#include "GameData/costume_definitions.h"
#include "GameData/costume_serializers.h"
#include "GameData/map_definitions.h"
#include "GameData/map_serializers.h"
#include "GameData/scenegraph_definitions.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/seq_definitions.h"
#include "GameData/seq_serializers.h"
#include "GameData/shop_definitions.h"
#include "GameData/shop_serializers.h"
#include "GameData/power_definitions.h"
#include "GameData/power_serializers.h"
#include "GameData/npc_definitions.h"
#include "GameData/npc_serializers.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"
#include "GameData/fx_definitions.h"
#include "GameData/fx_serializers.h"
#include "GameData/CharacterClass.h"
#include "GameData/charclass_serializers.h"
#include "GameData/def_serializers.h"
#include "GameData/other_definitions.h"
#include "GameData/origin_definitions.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include "GameData/particlesys_serializers.h"
#include "GameData/particle_definitions.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>

#include <QDirIterator>
#include <QRegularExpression>

namespace {
enum BinType {
    eInvalid,
    eLevelsDebts,
    eCombineChances,
    eBoostEffectiveness,
    eParticleSystems,
    eEntityOrigins,
    eEntityClasses,
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
    eAttribNames,
    eSceneGraph,
    eTrickDefinitions,
    ePowerDefinitions,
    eNpcDefinitions,
    eFxBehavior_Definitions,
    eFxInfo_Definitions,
    eSeq_Definitions,
};

const QHash<uint32_t,BinType> knownSerializers = {
    {levelsdebts_i0_requiredCrc         , eLevelsDebts},
    {combining_i0_requiredCrc           , eCombineChances},
    {boosteffectiveness_i0_requiredCrc  , eBoostEffectiveness},
    {particlesystems_i0_requiredCrc     , eParticleSystems},
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
    {attribnames_i0_requiredCrc         , eAttribNames},
    {scenegraph_i0_2_requiredCrc        , eSceneGraph},
    {tricks_i0_requiredCrc              , eTrickDefinitions},
    {tricks_i2_requiredCrc              , eTrickDefinitions},
    {charclass_i0_requiredCrc           , eEntityClasses},
    {origins_i0_requiredCrc             , eEntityOrigins},
    {powers_i0_requiredCrc              , ePowerDefinitions},
    {npccostumesets_i0_requiredCrc      , eNpcDefinitions},
    {fxbehaviors_i0_requiredCrc         , eFxBehavior_Definitions},
    {fxinfos_i0_requiredCrc             , eFxInfo_Definitions},
    {seqencerlist_i0_requiredCrc        , eSeq_Definitions},
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
T * doLoad(BinStore *bs) {
    T *res = new T;
    if(!loadFrom(bs, res)) {
        delete res;
        res = nullptr;
    }
    return res;
}

template<class T>
T * doLoadRef(BinStore *bs) {
    T *res = new T {};
    if(!loadFrom(bs,*res)) {
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
    qDebug()<<"   I0<"<<QString::number(levelsdebts_i0_requiredCrc,16)<<"> Experience data - 'experience.bin'";
    qDebug()<<"   I0<"<<QString::number(particlesystems_i0_requiredCrc,16)<<"> Particle system definitions - 'particles.bin'";
    qDebug()<<"   I0<"<<QString::number(shoplist_i0_requiredCrc,16)<<"> Shops data - 'stores.bin'";
    qDebug()<<"   I0<"<<QString::number(shopitems_i0_requiredCrc,16)<<"> Shops items- 'items.bin'";
    qDebug()<<"   I0<"<<QString::number(shopdepts_i0_requiredCrc,16)<<"> Shop department names data - 'depts.bin'";
    qDebug()<<"   I0<"<<QString::number(attribnames_i0_requiredCrc,16)<<"> Attrib definitions - 'attrib_names.bin'";
    qDebug()<<"   I0<"<<QString::number(tailorcosts_i0_requiredCrc,16)<<"> Tailoring cost data - 'tailorcost.bin'";
    qDebug()<<"   I0<"<<QString::number(costumesets_i0_requiredCrc,16)<<"> Costume part data - 'costume.bin'";
    qDebug()<<"   I0<"<<QString::number(bodyparts_i0_requiredCrc,16)<<"> Body part data - 'BodyParts.bin'";
    qDebug()<<"   I0<"<<QString::number(geoset_i0_requiredCrc,16)<<"> Supergroup emblem data - 'supergroupEmblems.bin'";
    qDebug()<<"   I0<"<<QString::number(palette_i0_requiredCrc,16)<<"> Color palette data - 'supergroupColors.bin'";
    qDebug()<<"   I0-2<"<<QString::number(scenegraph_i0_2_requiredCrc,16)<<"> Scene graph - 'geobin/*'";
    qDebug()<<"   I0<"<<QString::number(tricks_i0_requiredCrc,16)<<"> Trick definitions- 'tricks.bin'";
    qDebug()<<"   I2<"<<QString::number(tricks_i2_requiredCrc,16)<<"> Trick definitions- 'tricks.bin'";
    qDebug()<<"   I0<"<<QString::number(charclass_i0_requiredCrc,16)<<"> Entity class definitions- 'classes.bin' or 'villain_classes.bin'";
    qDebug()<<"   I0<"<<QString::number(origins_i0_requiredCrc,16)<<"> Entity origin definitions- 'origins.bin' or 'villain_origins.bin'";
    qDebug()<<"   I0<"<<QString::number(powers_i0_requiredCrc,16)<<"> Power definitions- 'powers.bin'";
    qDebug()<<"   I0<"<<QString::number(npccostumesets_i0_requiredCrc,16)<<"> NPC definitions - 'VillainCostume.bin'";
    qDebug()<<"   I0<"<<QString::number(fxbehaviors_i0_requiredCrc,16)<<"> FxBehavior definitions - 'behaviors.bin'";
    qDebug()<<"   I0<"<<QString::number(fxinfos_i0_requiredCrc,16)<<"> FxInfo definitions - 'fxinfo.bin'";
    qDebug()<<"   I0<"<<QString::number(seqencerlist_i0_requiredCrc,16)<<"> Sequencer definitions - 'sequencers.bin'";
    qDebug()<<"Numbers in brackets are file CRCs - bytes 8 to 13 in the bin.";
}
void convertEntitySequencerData(const QString &file_name,EntitySequencerData &tgt)
{
    QFile src_file(file_name);
    if(!src_file.open(QFile::ReadOnly))
    {
        qWarning() << "Cannot read file:"<<file_name;
    }
    QTextStream inp_str(&src_file);
    bool type_marker=false;
    while(!inp_str.atEnd())
    {
        QString line = inp_str.readLine();
        if(line.isEmpty())
            continue;
        line = line.replace(QRegularExpression("\\s+")," ");
        QStringRef lineref = line.midRef(0,line.indexOf('#')).trimmed();
        lineref = lineref.mid(0,line.indexOf("//")).trimmed();
        if(lineref.isEmpty())
            continue;
        if(!type_marker)
        {
            assert(lineref.compare(QStringLiteral("Type"))==0);
            type_marker=true;
            continue;
        }
        if(lineref.compare(QStringLiteral("End"))==0)
            break;
        QVector<QStringRef> parts=lineref.split(" ",QString::SkipEmptyParts);
        QStringRef fieldname = parts.takeFirst();
        if(QString("Sequencer").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_sequencer_name = parts[0].toLatin1();
        }
        else if(QString("SequencerType").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_seq_type = parts[0].toLatin1();
        }
        else if(QString("Graphics").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_graphics = parts[0].toLatin1();
        }
        else if(QString("LOD1_Gfx").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_lod_names[0] = parts[0].toLatin1();
        }
        else if(QString("LOD2_Gfx").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_lod_names[1] = parts[0].toLatin1();
        }
        else if(QString("LOD3_Gfx").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_lod_names[2] = parts[0].toLatin1();
        }
        else if(QString("LOD0_Dist").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_lod_dists[0] = parts[0].toFloat();
        }
        else if(QString("LOD1_Dist").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_lod_dists[1] = parts[0].toFloat();
        }
        else if(QString("LOD2_Dist").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_lod_dists[2] = parts[0].toFloat();
        }
        else if(QString("LOD3_Dist").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_lod_dists[3] = parts[0].toFloat();
        }
        else if(QString("FadeOutStart").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_fade_out_start = parts[0].toFloat();
        }
        else if(QString("FadeOutFinish").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_fade_out_finish = parts[0].toFloat();
        }
        else if(QString("ReverseFadeOutDist").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_reverse_fade_out_distance = parts[0].toFloat();
        }
        else if(QString("GeomScale").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==3);
            tgt.m_geometry_scale = {parts[0].toFloat(),parts[1].toFloat(),parts[2].toFloat()};
        }
        else if(QString("GeomScaleMax").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==3);
            tgt.m_geometry_scale_max = {parts[0].toFloat(),parts[1].toFloat(),parts[2].toFloat()};
        }
        else if(QString("MinimumAmbient").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_minimum_ambient = parts[0].toFloat();
        }
        else if(QString("VisSphereRadius").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_visibility_sphere_radius = parts[0].toFloat();
        }
        else if(QString("LightAsDoorOutside").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_light_as_door_outside = parts[0].toInt();
        }
        else if(QString("CollisionSize").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==3);
            tgt.m_collision_size = {parts[0].toFloat(),parts[1].toFloat(),parts[2].toFloat()};
        }
        else if(QString("CollisionOffset").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==3);
            tgt.m_collision_offset = {parts[0].toFloat(),parts[1].toFloat(),parts[2].toFloat()};
        }
        else if(QString("CollisionType").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_collision_type = parts[0].toLatin1();
        }
        else if(QString("Placement").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_placement = parts[0].toLatin1();
        }
        else if(QString("ShadowType").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_shadow_type = parts[0].toLatin1();
        }
        else if(QString("Shadow").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_shadow = parts[0].toLatin1();
        }
        else if(QString("ShadowSize").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==3);
            tgt.m_shadow_size = {parts[0].toFloat(),parts[1].toFloat(),parts[2].toFloat()};
        }
        else if(QString("ShadowOffset").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==3);
            tgt.m_shadow_offset = {parts[0].toFloat(),parts[1].toFloat(),parts[2].toFloat()};
        }
        else if(QString("ShadowTexture").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_shadow_texture = parts[0].toLatin1();
        }
        else if(QString("ShadowQuality").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            tgt.m_shadow_quality = parts[0].toLatin1();
        }
        else if(QString("UseShadow").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_use_shadow = parts[0].toInt();
        }
        else if(QString("ReticleHeightBias").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_reticle_mod.y = parts[0].toFloat();
        }
        else if(QString("ReticleWidthBias").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_reticle_mod.x = parts[0].toFloat();
        }
        else if(QString("FX").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_effect_names[0] = parts[0].toLatin1();
        }
        else if(QString("FX2").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_effect_names[1] = parts[0].toLatin1();
        }
        else if(QString("FX3").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_effect_names[2] = parts[0].toLatin1();
        }
        else if(QString("FX4").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_effect_names[3] = parts[0].toLatin1();
        }
        else if(QString("WorldGroup").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_world_group = parts[0].toLatin1();
        }
        else if(QString("Selection").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_selection = parts[0].toLatin1();
        }
        else if(QString("TicksToLingerAfterDeath").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_ticks_to_linger_after_death = parts[0].toInt();
        }
        else if(QString("TicksToFadeAwayAfterDeath").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_ticks_to_fade_away_after_death = parts[0].toInt();
        }
        else if(QString("NotSelectable").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_not_selectable = parts[0].toInt();
        }
        else if(QString("NoCollision").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_no_collision = parts[0].toInt();
        }
        else if(QString("HasRandomName").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_has_random_name = parts[0].toInt();
        }
        else if(QString("AnimScale").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_anim_scale = parts[0].toFloat();
        }
        else if(QString("ConstantState").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_constant_state = parts[0].toLatin1();
        }
        else if(QString("MaxAlpha").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_max_alpha = parts[0].toInt();
        }
        else if(QString("Bounciness").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_bounciness = parts[0].toFloat();
        }
        else if(QString("BoneScaleSkinny").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_bone_scale_skinny = parts[0].toLatin1();
        }
        else if(QString("BoneScaleFat").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_bone_scale_fat = parts[0].toLatin1();
        }
        else if(QString("RandomBoneScale").compare(fieldname,Qt::CaseInsensitive)==0)
        {
            assert(parts.size()==1);
            tgt.m_random_bone_scale = parts[0].toInt();
        }
        else
        {
            qDebug() << lineref.trimmed();
            exit(0);
        }
    }
}
// read all entries from ent_types, and convert them to singular ent_types.crl_json
void convertAllEntitySeqDescriptors(const QString &data_directory)
{
    SequencerTypeMap seq_data_store;
    QDirIterator iter(data_directory,QStringList {"*.txt"},QDir::Files,QDirIterator::Subdirectories);
    while(iter.hasNext())
    {
        QString next = iter.next();
        QFileInfo fi(next);
        QString base_name = QDir(data_directory).relativeFilePath(next);
        EntitySequencerData entry;
        entry.m_name = base_name.mid(0,base_name.lastIndexOf('.')).toLatin1();
        assert(seq_data_store.find(entry.m_name.toLower())==seq_data_store.end());
        convertEntitySequencerData(next,entry);
        seq_data_store[entry.m_name.toLower()] = entry;
    }
    QDir curr(QDir::current());
    curr.mkpath("data/converted");
    saveTo(seq_data_store,"data/converted/ent_types",true);
}
} // end of anonymous namespace

int main(int argc,char **argv)
{
    QCoreApplication app(argc,argv);
    if(app.arguments().size()<2) {
        showSupportedBinTypes();
        return -1;
    }
    QString src_file = app.arguments()[1];
    if(src_file.contains("ent_types"))
    {
        convertAllEntitySeqDescriptors(src_file);
        return 0;
    }
    BinType bin_type = getLoader(app.arguments()[1]);
    if(bin_type==eInvalid) {
        qCritical() << "Unhandled bin file type";
        showSupportedBinTypes();
        return -1;
    }
    QFSWrapper wrap;
    BinStore binfile;
    binfile.open(wrap,argv[1],0);
    QString target_basename=QFileInfo(argv[1]).baseName();
    bool json_output=true;

    try // handle possible cereal::RapidJSONException
    {
      if(app.arguments().size()>2)
          json_output = app.arguments()[2].toInt()!=0;

      switch(bin_type)
      {
          case eLevelsDebts:    doConvert(doLoadRef<LevelExpAndDebt>(&binfile),target_basename,json_output); break;
          case eCombineChances: doConvert(doLoadRef<Parse_Combining>(&binfile),target_basename,json_output); break;
          case eBoostEffectiveness: doConvert(doLoadRef<Parse_Effectiveness>(&binfile),target_basename,json_output); break;
          case eParticleSystems:doConvert(doLoad<Parse_AllPSystems>(&binfile),target_basename,json_output); break;
          case eShops:        doConvert(doLoadRef<AllShops_Data>(&binfile),target_basename,json_output); break;
          case eShopItems:    doConvert(doLoadRef<AllShopItems_Data>(&binfile),target_basename,json_output); break;
          case eShopDepts:    doConvert(doLoadRef<AllShopDepts_Data>(&binfile),target_basename,json_output); break;
//        case eSequencers:   doConvert(doLoad<SequencerList>(&binfile),target_basename,json_output); break;
          case eTailorCosts:  doConvert(doLoad<AllTailorCosts_Data>(&binfile),target_basename,json_output); break;
          case eCostumeSets:  doConvert(doLoad<CostumeSet_Data>(&binfile),target_basename,json_output); break;
          case eBodyParts:    doConvert(doLoadRef<BodyPartsStorage>(&binfile),target_basename,json_output); break;
          case eGroupEmblems: doConvert(doLoad<GeoSet_Data>(&binfile),target_basename,json_output); break;
          case ePaletteSets:  doConvert(doLoad<Pallette_Data>(&binfile),target_basename,json_output); break;
          case eZones:        doConvert(doLoadRef<AllMaps_Data>(&binfile),target_basename,json_output); break;
          case eAttribNames:  doConvert(doLoadRef<AttribNames_Data>(&binfile),target_basename,json_output); break;
          case eSceneGraph:   doConvert(doLoadRef<SceneGraph_Data>(&binfile),target_basename,json_output); break;
          case eTrickDefinitions: doConvert(doLoadRef<SceneModifiers>(&binfile),target_basename,json_output); break;
          case eEntityClasses: doConvert(doLoadRef<Parse_AllCharClasses>(&binfile),target_basename,json_output); break;
          case eEntityOrigins: doConvert(doLoadRef<Parse_AllOrigins>(&binfile),target_basename,json_output); break;
          case ePowerDefinitions: doConvert(doLoadRef<AllPowerCategories>(&binfile),target_basename,json_output); break;
          case eNpcDefinitions:
         {
            auto data = doLoadRef<AllNpcs_Data>(&binfile);
            if(qApp->arguments().size() > 2)
            {
                QString name_to_find = app.arguments()[2];
                std::sort(data->begin(), data->end(), [](const Parse_NPC &a, const Parse_NPC &b) -> bool {
                    return QString(a.m_Name).compare(QString(b.m_Name), Qt::CaseInsensitive) < 0;
                });
                auto iter = std::find_if(data->begin(), data->end(), [name_to_find](const Parse_NPC &n) -> bool {
                    if(n.m_Name == name_to_find)
                        return true;
                    return false;
                });
                qDebug() << iter - data->begin();
            } else
                doConvert(data, target_basename, json_output);
        }

          break;
          case eFxBehavior_Definitions: doConvert(doLoadRef<Fx_AllBehaviors>(&binfile),target_basename,json_output); break;
          case eFxInfo_Definitions: doConvert(doLoadRef<Fx_AllInfos>(&binfile),target_basename,json_output); break;
          case eSeq_Definitions: doConvert(doLoadRef<SequencerList>(&binfile),target_basename,json_output); break;
          default:
              break;
      }
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
        return -1;
    }
    catch(std::exception &e)
    {
      qCritical() << e.what();
      return -1;
    }

    return 0;
}

//! @}
