#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include "CRUD_Events.h"
#include "MapLink.h"

class BitStream;
class MapRef;
typedef CRUDLink_Event MapLinkEvent; //<MapLink>
class SceneEvent : public MapLinkEvent
{
    void dependent_dump(void);
public:
            SceneEvent();

    void    serializefrom(BitStream &src);
    void    serializeto(BitStream &tgt) const;

    std::string m_map_desc;
    uint32_t ref_count;
    int ref_crc;
    int var_4;
    bool m_outdoor_map;
    bool current_map_flags;
    size_t num_base_elems;
    int undos_PP;
    bool var_14;
    std::vector<std::string> m_trays;
    std::vector<u32> m_crc;
    std::vector<MapRef> m_refs;
    int unkn1;
    int m_map_number;
    bool unkn2;
protected:
    void getGrpElem(BitStream &src,int idx);
    void reqWorldUpdateIfPak(BitStream &src);
    void groupnetrecv_5(BitStream &src,int a,int b);
};

