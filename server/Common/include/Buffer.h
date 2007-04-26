/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Buffer.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include "Base.h"
#include <ace/Message_Block.h>

// Warning those classes aren't thread-safe!
// maybe this should derive from ACE_Data_Block/Message_Block ?

// Template functions are part of include file to avoid the need of explicit instantiation of templates.
class GrowingBuffer
{
	static const size_t DEFAULT_MAX_SIZE = 0x10000; //64K should be enough for everyone :P
public:
		//			Con/De-struction
		//////////////////////////////////////////////////////////////////////////
	
explicit			GrowingBuffer(const GrowingBuffer &); // copy constructor
					GrowingBuffer(u8 *buf, size_t size,bool become_owner);
					//! this constructs an empty growing buffer, and optionally pre-allocates storage space
					GrowingBuffer(size_t max_size,u8 safe_area,size_t current_size); 
					~GrowingBuffer();

		//			Public methods
		//////////////////////////////////////////////////////////////////////////
					/* 'u' before a method denotes that this method is 'Unchecked'*/
					/* all 'Checked' methods set internal m_lasterr variable*/
					/* Unchecked functions are faster, but assume that they can read/write without violating the buffer, or their arguments*/
		
		void		PutString(const char *t);
		void		PutBytes(const u8 *t, size_t len);
		void		GetString(char *t); //! Warning: this function argument must be a valid character array which size must be at least the same as retrieved string's
		bool		GetBytes(u8 *t, size_t len);

inline	void		uGetString(char *t);
		void		uGetBytes(u8 *t,size_t len);
inline  void		uPutString(const char *t);
	void			uPutBytes(const u8 *t,size_t len);
template<typename T> 
		void		Put(const T &val)
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
		void		Get(T &val)
		{
			if(GetReadableDataSize()<sizeof(T))
			{
				m_last_err = 1;
				return;
			}
			uGet(val);
		}

template<typename T> 
inline	void		uPut(const T &val)
		{
			*((T *)&m_buf[m_write_off]) = val; //let the compiler decide how to put type T into array
			m_write_off+=sizeof(T);
		}

template<typename T> 
inline	void		uGet(T &val)
		{
			val = *((T *)&m_buf[m_read_off]);
			m_read_off += sizeof(T);
		}
		void		PopFront(size_t count); //! this method removes up-to 'count' elements from the beginning of the buffer

		//			Accessors
		//////////////////////////////////////////////////////////////////////////	
		size_t		GetMaxSize() const		{ return m_max_size;}
		size_t		GetAvailSize() const	{ return (m_size-m_write_off);}
		size_t		GetDataSize() const		{ return m_write_off; }; //writing point gives actual size of readable data
		size_t		GetReadableDataSize() const { return m_write_off-m_read_off; }; //this much data can be read still
		u8 *		GetBuffer() const		{ return m_buf; };
		u32			getLastError() const	{ return m_last_err;}
		void		setLastError(u32 val)	{ m_last_err=val;}

		u8 *		read_ptr()				{ return m_buf+m_read_off;}
		void		read_ptr(int off)		{ m_read_off+=off;}
		u8 *		write_ptr()				{ return m_buf+m_write_off;}
		void		write_ptr(int off)		{ m_write_off+=off;}
	
		void		Reset(void)				{m_read_off=m_write_off=m_last_err=0;}
		void		ResetReading(void)		{m_read_off=0;}
		void		ResetWriting(void)		{m_write_off=0;} //will start at the beginning and overwrite existing contents
		void		ResetContents(void)		{memset(m_buf,0,m_size);}
protected:
		GrowingBuffer & operator=(const GrowingBuffer &);
		u8			m_safe_area;
		size_t		m_write_off;
		size_t		m_read_off;
		size_t		m_size;
		size_t		m_max_size;
		u8 *		m_buf;
		u32			m_last_err;
		int			resize(size_t accommodate_size); //! this method will try to resize GrowingBuffer to accommodate_size elements (in reality it preallocates a 'few' more )
	// if the new size is 0, then internal buffer object is deleted, freeing all memory
	// Warning: when buffer is growing, only it's part that contains any valid data is copied (i.e. from start, to write_off )
	// returns -2 if there were problems allocating new block of memory for the internal storage
	// returns -1 if new size exceeds maximum size allowed for this buffer
	// returns 0 if everything went ok

};
