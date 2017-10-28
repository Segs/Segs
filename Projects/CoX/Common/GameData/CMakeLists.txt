find_package(Qt5Core REQUIRED)
set(GAMEDATA_SRC
    Colors.h
    serialization_common.h
    anim_serializers.cpp
    anim_serializers.h
    attrib_serializers.cpp
    attrib_serializers.h
    bodypart_serializers.cpp
    bodypart_serializers.h
    scenegraph_serializers.h
    scenegraph_serializers.cpp
    charclass_definitions.h
    charclass_serializers.cpp
    charclass_serializers.h
    def_serializers.cpp
    def_serializers.h
    other_definitions.h
    origin_definitions.h
    #particlesys_serializers.cpp
    #particlesys_serializers.h
    costume_serializers.cpp
    costume_serializers.h
    #def_serializers.cpp
    map_serializers.cpp
    map_serializers.h
    #seq_serializers.cpp
    #seq_serializers.h
    shop_serializers.cpp
    shop_serializers.h
    trick_serializers.cpp
    trick_serializers.h
    #titles_serializers.cpp
    #titles_serializers.h
    #particle_definitions.h
    scenegraph_definitions.h
    shop_definitions.h
    map_definitions.h
    costume_definitions.h
    bodypart_definitions.h

    WorldData.cpp
    WorldData.h
    DataStorage.cpp
    DataStorage.h
    #TODO: move this to Components Library
    CoXHash.cpp
    CoXHash.h
)
add_library(gameData STATIC ${GAMEDATA_SRC})
add_dependencies(gameData ace_IMP) # used for Singleton stuff.
qt5_use_modules(gameData Core)
set_property(TARGET gameData APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:cereal_IMP,INTERFACE_INCLUDE_DIRECTORIES>")
