#include <ace/Task.h>
#include <ace/Thread_Mutex.h>
#include <ace/Future.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include <cstdio>
#include <memory>
#include <iostream>
#include <msgpack.hpp>

class ActiveObj : public ACE_Task<ACE_MT_SYNCH>
{
public:
    int open(void *);
    int close(u_long flags);
    int svc()
    {
        while(true)
        {
            auto_ptr<ACE_Method_Request> method_obj(m_activation_queue.dequeue());
            if(method_obj->call()==-1)
            {
                //log the call error
                break;
            }
        }
        return 0;
    }
    ACE_Future<int> ping();
private:
    ACE_Activation_Queue m_activation_queue;
};
static void write_buffer(const msgpack::sbuffer &buf)
{
    FILE *fp;
    fp = fopen("Test","wb");
    fwrite(buf.data(),1,buf.size(),fp);
    fclose(fp);
}
void on_message(msgpack::object &obj,std::auto_ptr<msgpack::zone> z)
{
    std::cerr<<obj.type<<"\n";
}
int main(int argc,char **argv)
{
    msgpack::sbuffer buf;
    std::string xx="packme";
    msgpack::packer<msgpack::sbuffer> packme(buf);
    msgpack::unpacker unp;
    packme << xx <<11<<-11;
    write_buffer(buf);

    msgpack::unpacked msg;
    msgpack::unpacker pac;

    pac.reserve_buffer(buf.size());
    memcpy(pac.buffer(),buf.data(),buf.size());
    pac.buffer_consumed(buf.size());

    while(pac.next(&msg))
    {
        // do some with the object with the zone.
        msgpack::object obj = msg.get();
        std::auto_ptr<msgpack::zone> z = msg.zone();
        on_message(obj, z);

        //// boost::shared_ptr is also usable:
        // boost::shared_ptr<msgpack::zone> life(z.release());
        // on_message(result.get(), life);
    }
    return 0;
}
