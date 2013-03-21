/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <ace/Message_Block.h>

// Warning those classes aren't thread-safe!
// maybe this should derive from ACE_Data_Block/Message_Block ?

// Template functions are part of include file to avoid the need of explicit instantiation of templates.
class GrowingBuffer
{
static const size_t DEFAULT_MAX_SIZE = 0x10000; //64K should be enough for everyone :P
public:
            //              Con/De-struction
            //////////////////////////////////////////////////////////////////////////

explicit                    GrowingBuffer(const GrowingBuffer &); // copy constructor
                            GrowingBuffer(uint8_t *buf, size_t size,bool become_owner);
                            GrowingBuffer(size_t max_size,uint8_t safe_area,size_t current_size);
                            ~GrowingBuffer();

            //  Public methods
            //////////////////////////////////////////////////////////////////////////
            /* 'u' before a method denotes that this method is 'Unchecked'*/
            /* all 'Checked' methods set internal m_lasterr variable*/
            /* Unchecked functions are faster, but assume that they can read/write without violating the buffer, or their arguments*/

            void            PutString(const char *t);
            void            PutBytes(const uint8_t *t, size_t len);
            void            GetString(char *t);
            std::string     GetString();
            bool            GetBytes(uint8_t *t, size_t len);

inline      void            uGetString(char *t);
            void            uGetBytes(uint8_t *t,size_t len);
inline      void            uPutString(const char *t);
            void            uPutBytes(const uint8_t *t,size_t len);
                            template<typename T>
            void            Put(const T &val)
                            {
                                    if(sizeof(T)>GetAvailSize())
                                            if(resize(m_write_off+sizeof(T))==-1) // space exhausted
                                            {
                                                    m_last_err = 1;
                                                    return;
                                            }
                                    uPut(val);
                            }

                            template<typename T>
            void            Get(T &val)
                            {
                                    if(GetReadableDataSize()<sizeof(T))
                                    {
                                            m_last_err = 1;
                                            return;
                                    }
                                    uGet(val);
                            }

                            template<typename T>
inline      void            uPut(const T &val)
                            {
                                    *((T *)&m_buf[m_write_off]) = val; //let the compiler decide how to put type T into array
                                    m_write_off+=sizeof(T);
                            }

                            template<typename T>
inline      void            uGet(T &val)
                            {
                                    val = *((T *)&m_buf[m_read_off]);
                                    m_read_off += sizeof(T);
                            }
                            template<typename T>
inline      T               ruGet(void)
                            {
                                    T val = *((T *)&m_buf[m_read_off]);
                                    m_read_off += sizeof(T);
                                    return val;
                            }

            void            PopFront(size_t count); //! this method removes up-to 'count' elements from the beginning of the buffer

            //              Accessors
            //////////////////////////////////////////////////////////////////////////
            size_t          GetMaxSize() const          { return m_max_size;}
            size_t          GetAvailSize() const        { return (m_size-m_write_off);}
            size_t          GetDataSize() const         { return m_write_off; } //writing point gives actual size of readable data
            size_t          GetReadableDataSize() const { return m_write_off-m_read_off; } //this much data can be read still
            uint8_t *       GetBuffer() const           { return m_buf; }
            uint32_t        getLastError() const        { return m_last_err;}
            void            setLastError(uint32_t val)  { m_last_err=val;}

            uint8_t *       read_ptr()                  { return m_buf+m_read_off;}
            void            read_ptr(int off)           { m_read_off+=off;}
            uint8_t *       write_ptr()                 { return m_buf+m_write_off;}
            void            write_ptr(int off)          { m_write_off+=off;}

            void            Reset(void)                 {m_read_off=m_write_off=m_last_err=0;}
            void            ResetReading(void)          {m_read_off=0;}
            void            ResetWriting(void)          {m_write_off=0;} //will start at the beginning and overwrite existing contents
            void            ResetContents(void)         {memset(m_buf,0,m_size);}
protected:
            GrowingBuffer & operator=(const GrowingBuffer &);
            uint8_t         m_safe_area;
            size_t          m_write_off;
            size_t          m_read_off;
            size_t          m_size;
            size_t          m_max_size;
            uint8_t *       m_buf;
            uint32_t        m_last_err;
            int             resize(size_t accommodate_size);
};
