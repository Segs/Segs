SET(WX_INTERNAL_PATH /lib/8_0/vc_lib)
#SET(WX_INTERNAL_PATH /lib/vc_lib) # this is for single compiler version of wxwidgets
SET(wxWidgets_LIB_DIR $ENV{WXWIN}/${WX_INTERNAL_PATH})
FIND_PACKAGE(wxWidgets REQUIRED base core)

INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

SUBDIRS(src)
