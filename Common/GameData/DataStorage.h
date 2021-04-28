/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/serialization_common.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <map>
#include <type_traits>

struct RGBA;

typedef glm::vec3 Vec3;
typedef glm::vec2 Vec2;
class BinStore // binary storage
{
    struct FileEntry {
        QString name;
        uint32_t date=0;
    };
    QIODevice *m_str = nullptr;
    size_t bytes_read=0;
    uint32_t bytes_to_read=0;
    std::vector<uint32_t> m_file_sizes; // implicit stack
    std::vector<FileEntry> m_entries;

    template<class V>
    size_t read_internal(V &res)
    {
        if(!m_file_sizes.empty() && current_fsize()<sizeof(V))
            return 0;
        m_str->read((char *)&res,sizeof(V));
        if(!m_file_sizes.empty())
        {
            bytes_read+=sizeof(V);
            (*m_file_sizes.rbegin())-=sizeof(V);
        }
        return sizeof(V);
    }
    const QByteArray &read_pstr(size_t maxlen);
    void        skip_pstr();
    bool        read_data_blocks(bool file_data_blocks);
    bool        check_bin_version_and_crc(uint32_t req_crc);
    uint32_t    current_fsize() {return *m_file_sizes.rbegin();}
    uint32_t    read_header(QByteArray &name, size_t maxlen);
    void        fixup();
public:

    QString     source_name() {
        return read_str(12000);
    }
    bool        read_bytes(char *tgt,size_t sz);
    const QByteArray &read_str(size_t maxlen);
    bool        read(uint32_t &v);
    bool        read(int32_t &v);
    bool        read(float &v);
    bool        read(uint16_t &v);
    bool        read(uint8_t &v);
    bool        readU(uint8_t &v);
    bool        read(Vec2 &v);
    bool        read(Vec3 &v);
    bool        read(RGBA &v);
    bool        read(std::vector<uint32_t> &v);
    bool        read(std::vector<int32_t> &res);
    bool        read(std::vector<float> &res);
    bool        read(std::vector<QByteArray> &res);
    bool        read(std::vector<std::vector<QString>> &res);
    bool        read(uint8_t *&val, uint32_t length);
    bool        read(QByteArray &val);
    bool        read(std::pair<uint8_t,uint8_t> &v) {
                    uint8_t skipped, skipped2;
                    return read_internal(v.first)!=0 &&
                    read_internal(v.second)!=0 && read_internal(skipped) != 0 && read_internal(skipped2) != 0;
                }
                template<class Enum>
    bool        readEnum(Enum &val)
                {
                    typename std::underlying_type<Enum>::type true_val;
                    if(!read(true_val))
                        return false;
                    val = Enum(true_val);
                    return true;
                }
                template<class Enum>
    bool        readEnum(std::vector<Enum> &val)
                {
                    std::vector<typename std::underlying_type<Enum>::type> true_val;
                    if(!read(true_val))
                        return false;
                    // this is bad, and I feel bad for writing it :/
                    val = std::move(*(std::vector<Enum> *)(&true_val));
                    return true;
                }
    void        prepare();
    bool        prepare_nested();
    bool        nesting_name(QByteArray &name);
    void        nest_in() {  }
    void        nest_out() { m_file_sizes.pop_back(); }
    bool        end_encountered() const;
    bool        open(FSWrapper &fs,const QString & name, uint32_t required_crc);
                ~BinStore();

};
