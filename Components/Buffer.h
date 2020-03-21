/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>
#include <cstring>

// Warning those classes aren't thread-safe!
// NOTE: maybe this should derive from ACE_Data_Block/Message_Block ?

// Template functions are part of include file to avoid the need of explicit instantiation of templates.
class GrowingBuffer
{
static  constexpr uint32_t   DEFAULT_MAX_SIZE = 0x10000; //64K should be enough for everyone :P
public:

            //              Con/De-struction
            //////////////////////////////////////////////////////////////////////////

explicit                    GrowingBuffer(const GrowingBuffer &); // copy constructor
explicit                    GrowingBuffer(GrowingBuffer &&) noexcept; // copy constructor
                            GrowingBuffer(uint8_t *buf, uint32_t size,bool become_owner);
                            GrowingBuffer(uint32_t max_size,uint8_t safe_area, uint32_t current_size);
                            ~GrowingBuffer();

            //  Public methods
            //////////////////////////////////////////////////////////////////////////
            /* 'u' before a method denotes that this method is 'Unchecked'*/
            /* all 'Checked' methods set internal m_lasterr variable*/
            /* Unchecked functions are faster, but assume that they can read/write without violating the buffer, or their arguments*/

            void            PutString(const char *t);
            void            PutBytes(const uint8_t *t, uint32_t len);
            void            GetString(char *t);
            bool            GetBytes(uint8_t *t, uint32_t len);

inline      void            uGetString(char *t);
            void            uGetBytes(uint8_t *t, uint32_t len);
inline      void            uPutString(const char *t);
            void            uPutBytes(const uint8_t *t, uint32_t len);
                            template <typename T>
            void            Put(const T &val)
                            {
                                if(sizeof(T) > GetAvailSize())
                                    if(resize(m_write_off + sizeof(T)) == -1) // space exhausted
                                    {
                                        m_last_err = 1;
                                        return;
                                    }
                                uPut(val);
                            }

                            template <typename T>
            void            Get(T &val)
                            {
                                if(GetReadableDataSize() < sizeof(T))
                                {
                                    m_last_err = 1;
                                    return;
                                }
                                uGet(val);
                            }

                            template<typename T>
inline      void            uPut(const T &val)
                            {
                                    memcpy(m_buf+m_write_off,&val,sizeof(T)); // we copy the bits directly to avoid alignment warnings from sanitizer
                                    m_write_off+=sizeof(T);
                            }

                            template<typename T>
inline      void            uGet(T &val)
                            {
                                    memcpy(&val,m_buf+m_read_off,sizeof(T));
                                    m_read_off += sizeof(T);
                            }
                            template<typename T>
inline      T               ruGet()
                            {
                                    T val;
                                    memcpy(&val,m_buf+m_read_off,sizeof(T));
                                    m_read_off += sizeof(T);
                                    return val;
                            }

            void            PopFront(uint32_t count); //! this method removes up-to 'count' elements from the beginning of the buffer

            //              Accessors
            //////////////////////////////////////////////////////////////////////////
            uint32_t        GetMaxSize() const          { return m_max_size;}
            uint32_t        GetAvailSize() const        { return (m_size-m_write_off);}
            uint32_t        GetDataSize() const         { return m_write_off; } //writing point gives actual size of readable data
            uint32_t        GetReadableDataSize() const { return m_write_off-m_read_off; } //this much data can be read still
            uint8_t *       GetBuffer() const           { return m_buf; }
            uint32_t        getLastError() const        { return m_last_err;}
            void            setLastError(uint32_t val)  { m_last_err=val;}

            uint8_t *       read_ptr()                  { return m_buf+m_read_off;}
            const uint8_t * read_ptr() const            { return m_buf+m_read_off;}
            void            read_ptr(int off)           { m_read_off+=off;}
            uint8_t *       write_ptr()                 { return m_buf+m_write_off;}
            void            write_ptr(int off)          { m_write_off+=off;}

            void            Reset()                     {m_read_off=m_write_off=m_last_err=0;}
            void            ResetReading()              {m_read_off=0;}
            void            ResetWriting()              {m_write_off=0;} //will start at the beginning and overwrite existing contents
            void            ResetContents()             {memset(m_buf,0,m_size);}
protected:
            GrowingBuffer & operator=(const GrowingBuffer &) = delete;
            uint8_t *       m_buf;
            uint32_t        m_write_off;
            uint32_t        m_read_off;
            uint32_t        m_size;
            uint32_t        m_max_size;
            uint32_t        m_last_err;
            uint8_t         m_safe_area;
            int             resize(uint32_t accommodate_size);
};
