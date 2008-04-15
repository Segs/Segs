# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $
ADD_SUBDIRECTORY(Globals) # includes each project should really have version numbers, types etc.
ADD_SUBDIRECTORY(ConfigExtensions) # contains extensions to ACE's config file processing
ADD_SUBDIRECTORY(GrowingBuffer) 
ADD_SUBDIRECTORY(Bitstream)
ADD_SUBDIRECTORY(CRUDP_Protocol)

