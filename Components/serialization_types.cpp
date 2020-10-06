#include "Components/serialization_types.h"

#include "Components/serialization_common.h"
template <class Archive>
void CEREAL_SAVE_FUNCTION_NAME( Archive & ar, const BitStream &bs )
{
    ar( cereal::make_size_tag( bs.m_size ) ); // number of elements
    for(size_t i=0; i<bs.m_size; ++i)
        ar(bs.m_buf[i]);
    ar(bs.m_write_off,bs.m_read_off,bs.m_max_size,bs.m_last_err,bs.m_safe_area);
    ar( bs.m_byteAligned,bs.m_read_bit_off,bs.m_write_bit_off );
}

//! Serialization for non-arithmetic vector types
template <class Archive>
void CEREAL_LOAD_FUNCTION_NAME( Archive & ar,BitStream &bs )
{
    cereal::size_type vectorSize;
    ar( cereal::make_size_tag( vectorSize ) );
    bs.m_size = vectorSize;
    bs.m_buf = new uint8_t [bs.m_size];
    for(size_t i=0; i<bs.m_size; ++i)
        ar(bs.m_buf[i]);
    ar(bs.m_write_off,bs.m_read_off,bs.m_max_size,bs.m_last_err,bs.m_safe_area);
    ar( bs.m_byteAligned,bs.m_read_bit_off,bs.m_write_bit_off );
}
SPECIALIZE_SPLIT_SERIALIZATIONS(BitStream)
