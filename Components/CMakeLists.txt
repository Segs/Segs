ADD_SUBDIRECTORY(Common)
 # this must be first here, as both Client, and Server Components can depend on Common

# Only build example client if set to ON during configuration
IF(BUILD_Example01)
  ADD_SUBDIRECTORY(Client)
ENDIF(BUILD_Example01)

ADD_SUBDIRECTORY(Server)
