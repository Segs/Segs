OPTION(Example01_QtClient "Build Qt based PC client ?" OFF)
OPTION(Example01_ConsoleClient "Build commandline based PC client ?" OFF)

#ADD_SUBDIRECTORY(Auth)
IF(Example01_ConsoleClient)
  ADD_SUBDIRECTORY(ConsoleBased)
ENDIF()

IF(Example01_QtClient)
  ADD_SUBDIRECTORY(QtBased)
ENDIF()
