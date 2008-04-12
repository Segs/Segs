# I found simple solution to setting correct values for finding wxwidgets wxWidgets_LIB_DIR 
# should be set by hand to proper directory i.e. ..wxWidgets-2.8.4/lib/7_1/vc_lib or wxWidgets-2.8.4/lib/8_0/vc_lib
FIND_PACKAGE(wxWidgets REQUIRED base core)

IF (wxWidgets_FOUND)
    IF(wxWidgets_INCLUDE_DIRS)
      INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS})
    ENDIF(wxWidgets_INCLUDE_DIRS)
    IF (wxWidgets_CXX_FLAGS)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")
    ENDIF(wxWidgets_CXX_FLAGS)
    IF(wxWidgets_DEFINITIONS)
      ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
    ENDIF(wxWidgets_DEFINITIONS)
ENDIF (wxWidgets_FOUND)


INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

SUBDIRS(src)
