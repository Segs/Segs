#include "SEGSEventFactory.h"

#include "SEGSEvent.h"
#include "Logging.h"

#include <unordered_map>
#include <vector>
#include <cassert>
namespace
{
    struct EventDescriptorEntry
    {
        std::function<SEGSEvents::Event *()> constructor;
        const char *name;
        uint32_t type_id;
    };
    std::vector<EventDescriptorEntry> s_all_event_descriptors;
    std::unordered_map<std::string,size_t> s_name_to_event_descriptor;
    std::unordered_map<uint32_t,size_t> s_id_to_event_descriptor;
} // end of anonymous namespace
namespace SEGSEvents
{
void register_event_type(const char *name, uint32_t type_id, std::function<Event *()> constructor)
{
    assert(s_name_to_event_descriptor.find(name)==s_name_to_event_descriptor.end());
    assert(s_id_to_event_descriptor.find(type_id)==s_id_to_event_descriptor.end());
    s_all_event_descriptors.push_back({constructor,name,type_id});
    s_name_to_event_descriptor[name] = s_all_event_descriptors.size()-1;
    s_id_to_event_descriptor[type_id] = s_all_event_descriptors.size()-1;
}

Event *create_by_id(uint32_t type_id,EventSrc *src)
{
    auto iter = s_id_to_event_descriptor.find(type_id);
    if(iter==s_id_to_event_descriptor.end())
        return nullptr;
    auto res = s_all_event_descriptors[iter->second].constructor();
    res->src(src);
    return res;
}
Event *create_by_name(const char *name, EventSrc *src)
{
    auto iter = s_name_to_event_descriptor.find(name);
    if(iter==s_name_to_event_descriptor.end())
        return nullptr;
    auto res = s_all_event_descriptors[iter->second].constructor();
    res->src(src);
    return res;
}
const char *event_name(uint32_t type_id)
{
    auto iter = s_id_to_event_descriptor.find(type_id);
    if(iter==s_id_to_event_descriptor.end())
        return nullptr;
    return s_all_event_descriptors[iter->second].name;
}
Event *from_storage(std::istream &istr)
{
    Event * ev;
    uint32_t type_id;
    // we create a binary input to read the type_id;
    {
        cereal::BinaryInputArchive iarchive(istr);
        iarchive(type_id);
        ev = create_by_id(type_id);
    }
    if(!ev)
    {
        qCritical()<<"Unknown Event type" << type_id;
        return nullptr;
    }
    ev->serialize_from(istr);
    return ev;
}
void to_storage(std::ostream &ostr,Event *ev)
{
    assert(ev);
    uint32_t type_id = ev->type();
    // we create a binary input to read the type_id;
    {
        cereal::BinaryOutputArchive oarchive(ostr);
        oarchive(type_id);
        ev = create_by_id(type_id);
    }
    ev->do_serialize(ostr);
}
}
