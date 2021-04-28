#include "Sidekick.h"

#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

template<class Archive>
void Sidekick::serialize(Archive &archive, uint32_t const version)
{
    if(version != Sidekick::class_version)
    {
        qCritical() << "Failed to serialize Sidekick, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("HasSidekick",m_has_sidekick));
    archive(cereal::make_nvp("SidekickDbId",m_db_id));
    archive(cereal::make_nvp("SidekickType",m_type));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Sidekick)
CEREAL_CLASS_VERSION(Sidekick, Sidekick::class_version) // register PowerTrayGroup class version
