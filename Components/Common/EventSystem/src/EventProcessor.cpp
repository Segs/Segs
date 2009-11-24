#include "EventProcessor.h"

int EventProcessor::open( void *args /* = 0 */ )
{
	return super::open(args);
}

int EventProcessor::handle_timeout( const ACE_Time_Value &current_time, const void *act /* = 0 */ )
{
	SEGSEvent *mb=new SEGSEvent(1);
	return putq(mb);
}

int EventProcessor::svc( void )
{
	SEGSEvent *mb;
	ACE_Time_Value prev_val(ACE_OS::gettimeofday());
	while(getq(mb,0)!=-1)
	{
		if(mb->type()==0)
		{
			mb->release();
			return 0;
		}
		dispatch(mb);
		mb->release();
	}
	return 0;
}