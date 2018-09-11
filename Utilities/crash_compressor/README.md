== Crash State Compressor  ==

This utility application will read the specific SEGS dump file, and try to reduce it's size, so that the dump contains only the crashing EventProcessor's states and events.
When all of 

Psuedo code:

```
void crash_handler()
{
    save_crashing_ep(currently_running_ep);
    save_unprocesessed_eps();
}
void process_all() {

    for(auto ep : all_event_processors)
    {
	currently_running_ep = &ep;
        while(ep.process_single_event();
    }
}
int main(int argc, char **argv)
{
  if(dump_file.size()>0)
  {
    all_event_processors = deserialize(dump_file);
    process_all();
  }
  else
  {
    zlib_compress_all_crashing_eps()
  }
}
```    