#include <map>
#include <algorithm>
#include <string>
#include <cassert>
#include <iostream>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Node>
#include <osg/Vec3f>
#include <osg/Transform>
#include <osg/PositionAttitudeTransform>
#include <osgText/Text>
#include <osg/ShapeDrawable>
#include "mapmodel.h"
using namespace std;
typedef map<string,MapStructs::Def *> mDefDict;
MapModel::MapModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_scene.build_schema();
}

osg::ref_ptr<osg::Node> MapModel::read(QString file)
{
    bool read_ok =m_scene.read(file.toStdString());
    if(false==read_ok)
        return 0;
    osg::ref_ptr<osg::Node> res= build_osg_scene();
    reset();
    return res;
}
class OsgConverter
{
    void create_geometries()
    {
        osg::Box *bx=new osg::Box(osg::Vec3(0,0,0),5,5,5);
        osg::Geode* basicShapesGeode = new osg::Geode();
        osg::Geometry * geometry=new osg::Geometry();
        basicShapesGeode->addDrawable(geometry);
        osg::Vec3Array* pyramidVertices = new osg::Vec3Array;
        pyramidVertices->push_back( osg::Vec3( 0, 0, 0) ); // front left
        pyramidVertices->push_back( osg::Vec3(5, 0, 0) ); // front right
        pyramidVertices->push_back( osg::Vec3(5,5, 0) ); // back right
        pyramidVertices->push_back( osg::Vec3( 0,5, 0) ); // back left
        pyramidVertices->push_back( osg::Vec3( 2.5, 2.5,5) ); // peak
        osg::DrawElementsUInt* pyramidBase = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        pyramidBase->push_back(3);
        pyramidBase->push_back(2);
        pyramidBase->push_back(1);
        pyramidBase->push_back(0);
        osg::DrawElementsUInt* pyramidFaceOne =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceOne->push_back(0);
        pyramidFaceOne->push_back(1);
        pyramidFaceOne->push_back(4);

        osg::DrawElementsUInt* pyramidFaceTwo =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceTwo->push_back(1);
        pyramidFaceTwo->push_back(2);
        pyramidFaceTwo->push_back(4);

        osg::DrawElementsUInt* pyramidFaceThree =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceThree->push_back(2);
        pyramidFaceThree->push_back(3);
        pyramidFaceThree->push_back(4);

        osg::DrawElementsUInt* pyramidFaceFour =
                new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        pyramidFaceFour->push_back(3);
        pyramidFaceFour->push_back(0);
        pyramidFaceFour->push_back(4);
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) ); //index 3 white

        geometry->setVertexArray( pyramidVertices );
        geometry->addPrimitiveSet(pyramidBase);
        geometry->addPrimitiveSet(pyramidFaceOne);
        geometry->addPrimitiveSet(pyramidFaceTwo);
        geometry->addPrimitiveSet(pyramidFaceThree);
        geometry->addPrimitiveSet(pyramidFaceFour);
        geometry->setColorArray(colors);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

        scene_geometries["generic"]=   basicShapesGeode;
        basicShapesGeode = new osg::Geode();
        geometry=new osg::Geometry();
        basicShapesGeode->addDrawable(geometry);
        geometry->setVertexArray( pyramidVertices );
        geometry->addPrimitiveSet(pyramidBase);
        geometry->addPrimitiveSet(pyramidFaceOne);
        geometry->addPrimitiveSet(pyramidFaceTwo);
        geometry->addPrimitiveSet(pyramidFaceThree);
        geometry->addPrimitiveSet(pyramidFaceFour);
        colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) ); //index 3 white
        geometry->setColorArray(colors);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        scene_geometries["nature"]=   basicShapesGeode;
        basicShapesGeode = new osg::Geode();
        osg::ShapeDrawable *draw=new osg::ShapeDrawable(bx);
        //draw->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) );
        basicShapesGeode->addDrawable(draw);
        scene_geometries["street"]=   basicShapesGeode;
    }
    osg::Node *create_labelz(const std::string &lab)
    {
        osg::Geode* label = new osg::Geode();
        osgText::Text *txt=new osgText::Text();
        txt->setText(lab);
        label->addDrawable(txt);
        return label;
    }
    osg::Node *create_object(const std::string &name)
    {
        //return create_labelz(name);
        std::string geometry_group="generic";
        static bool initialized=false;
        if(initialized==false)
        {
            create_geometries();
            initialized=true;
        }
        if(name.find("NATURE")!=std::string::npos)
        {
            geometry_group="nature";
        }
        else if(name.find("STREETS")!=std::string::npos)
        {
            geometry_group="street";
        }
        else if(name.find("OMNI")!=std::string::npos)
        {
            return 0;
        }

        assert(scene_geometries[geometry_group]!=0);
        return scene_geometries[geometry_group];
    }

    map<string,osg::Node *> named_objects;
    map<string,osg::Node *> scene_geometries;
    mDefDict &m_dict;
    vector<osg::Node *> &m_nodes;
    void process_def(MapStructs::Def *df)
    {
        if(df->m_groups.size()) // grouping node
        {

            osg::Group *def_node=new osg::Group();
            def_node->setName(df->m_src_name);
            created_nodes[df]=def_node;
            for(size_t idx=0; idx<df->m_groups.size(); ++idx)
            {
                MapStructs::Group *grp=df->m_groups[idx];
                osg::Node *child=0;
                osg::PositionAttitudeTransform *pos_transform=new osg::PositionAttitudeTransform;
                Vec3 &po=grp->m_pos;
                Vec3 &rot=grp->m_rot;
                osg::Vec3 x(1,0,0);
                osg::Vec3 y(0,0,1);
                osg::Vec3 z(0,1,0);
                osg::Quat quater(rot.v[0],x,rot.v[1],y,rot.v[2],z);
                pos_transform->setPosition(osg::Vec3d(po.v[0],po.v[2],po.v[1]));
                pos_transform->setAttitude(quater);
                def_node->addChild(pos_transform);
                if(m_dict.find(grp->m_name)==m_dict.end())
                { // named an object
                    if(named_objects.find(grp->m_name)==named_objects.end())
                    {
                        // we don't have it in cache yet
                        named_objects[grp->m_name]=create_object(grp->m_name);
                    }
                    child=named_objects[grp->m_name];
                }
                else
                {
                    // let's see if we've seen this one before
                    MapStructs::Def *grouped_df=m_dict[grp->m_name];
                    if(created_nodes.find(grouped_df)==created_nodes.end())
                    {
                        // nope, we'll have to create it
                        process_def(grouped_df);
                    }
                    // at this point we are 'sure' that created nodes has proper entry for us
                    child=created_nodes[grouped_df];
                }
                pos_transform->addChild(child);
            }
            //def_node->addChild(extr);
        }
        else // not a grouping node
        {
            created_nodes[df]=create_object(df->m_src_name);
        }
        // remove processed definition
        m_dict.erase(m_dict.find(df->m_src_name));
    }

    void find_roots()
    {
        map<MapStructs::Def *,osg::Node *>::iterator iter=created_nodes.begin();
        while(iter!=created_nodes.end())
        {
            osg::Node *nd=iter->second;
            if(nd && nd->getNumParents()==0)
                m_nodes.push_back(nd);
            ++iter;
        }
    }
public:
    map<MapStructs::Def *,osg::Node *> created_nodes;
    OsgConverter(mDefDict &dict,vector<osg::Node *> &nd):m_dict(dict),m_nodes(nd)
    {}
    void process_grouping_hierarchy()
    {
        while(m_dict.size()>0)
        {
            mDefDict::iterator iter=m_dict.begin();
            MapStructs::Def *df=iter->second;
            process_def(df);
        }
        find_roots();
    }
};
osg::ref_ptr<osg::Node> MapModel::build_osg_scene()
{

    std::cerr<<"Scene file:"<<m_scene.m_scene_file<<std::endl;
    SceneStorage::vDef::iterator iter=m_scene.m_root.begin();
    SceneStorage::vDef &arr(m_scene.m_root);
    std::map<std::string,MapStructs::Def *> def_dictionary;
    while(iter!=arr.end())
    {
        MapStructs::Def *df=(*iter);
        assert(def_dictionary.find(df->m_src_name)==def_dictionary.end());
        def_dictionary[df->m_src_name]=df;
        ++iter;
    }
    iter = m_scene.m_defs.begin();
    while(iter!=m_scene.m_defs.end())
    {
        MapStructs::Def *df=(*iter);
        std::transform(df->m_src_name.begin(), df->m_src_name.end(),df->m_src_name.begin(), ::toupper);
        for(size_t idx=0; idx<df->m_groups.size(); ++idx)
        {
            MapStructs::Group *grp=df->m_groups[idx];
            std::transform(grp->m_name.begin(), grp->m_name.end(),grp->m_name.begin(), ::toupper);
        }
        assert(def_dictionary.find(df->m_src_name)==def_dictionary.end());
        def_dictionary[df->m_src_name]=df;
        ++iter;
    }
    std::cerr<<def_dictionary.size();
    std::vector<osg::Node *> osg_nodes;
    OsgConverter conv(def_dictionary,osg_nodes);
    conv.process_grouping_hierarchy();
    osg::ref_ptr<osg::Group> grp=new osg::Group;
    for(size_t idx=0; idx<osg_nodes.size(); ++idx)
    {
        grp->addChild(osg_nodes[idx]);
    }
    return grp;
}

bool MapModel::hasChildren(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return true;
    TreeStore *ptr=static_cast<TreeStore *>(parent.internalPointer());
    TreeStore *chld=ptr->nth_child(parent.row());
    if(chld==0)
        return 0;
    return chld->num_children()>0;
}
QModelIndex MapModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent==QModelIndex())
        return createIndex(row,column,(void *)&m_scene);
    else
    {
        TreeStore *ptr=static_cast<TreeStore *>(parent.internalPointer());
        if(row<ptr->num_children())
            return createIndex(row,column,ptr->nth_child(row));
        return QModelIndex();
    }
}
QModelIndex MapModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}
int MapModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    TreeStore *ptr=static_cast<TreeStore *>(parent.internalPointer());
    if(ptr==0)
        return m_scene.num_children();
    else
    {
        TreeStore *chld=ptr->nth_child(parent.row());
        return chld->num_children();
    }
    return 0;
}
int MapModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
QVariant MapModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();
    TreeStore *ptr=static_cast<TreeStore *>(index.internalPointer());
    TreeStore *chld=ptr->nth_child(index.row());
    return chld->to_string().c_str();
}
Qt::ItemFlags MapModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
