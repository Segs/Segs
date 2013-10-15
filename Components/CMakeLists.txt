if(ENABLE_SCRIPTING_ENGINE)
    ADD_SUBDIRECTORY(Scripting)
endif()
ADD_SUBDIRECTORY(ConfigExtensions) # contains extensions to ACE's config file processing
ADD_SUBDIRECTORY(EventSystem)
ADD_SUBDIRECTORY(GrowingBuffer)
ADD_SUBDIRECTORY(Bitstream)




