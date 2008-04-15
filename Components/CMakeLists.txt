ADD_SUBDIRECTORY(Common)
 # this must be first here, as both Client, and Server Components can depend on Common
ADD_SUBDIRECTORY(Client)

ADD_SUBDIRECTORY(Server)