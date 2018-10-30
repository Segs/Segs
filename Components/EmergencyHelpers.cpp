#include "EmergencyHelpers.h"

#include "CircularBlockBuffer.h"

#ifdef _MSCVER
#include <io.h>
#else
#include <unistd.h>
#endif

/// Store the circular buffer state to a provided FD
/// this function is used by crash handler to save the state to help with post-mortem debugging
void emergency_store(CircularBlockBuffer &buf,int fd)
{
    write(fd,&buf.m_max_size,sizeof(buf.m_max_size));
    write(fd,&buf.m_wr_offset,sizeof(buf.m_wr_offset));
    write(fd,&buf.m_rd_offset,sizeof(buf.m_rd_offset));
    write(fd,buf.m_memory.get(),buf.m_max_size);
}
