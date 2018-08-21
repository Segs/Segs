find_package(Qt5 COMPONENTS Core REQUIRED)
set(GAMEDATA_SRC
    chardata_definitions.h
    chardata_serializers.cpp
    chardata_serializers.h
    clientoptions_definitions.h
    clientoptions_definitions.cpp
    clientoptions_serializers.cpp
    clientoptions_serializers.h
    playerdata_serializers.cpp
    playerdata_serializers.h
    playerdata_definitions.h
    entitydata_definitions.h
    entitydata_serializers.cpp
    entitydata_serializers.h
# Resource file formats
    anim_serializers.cpp
    anim_serializers.h
    anim_definitions.h
    attrib_serializers.cpp
    attrib_serializers.h
    attrib_definitions.h
    bodypart_serializers.cpp
    bodypart_serializers.h
    scenegraph_serializers.h
    scenegraph_serializers.cpp
    charclass_definitions.h
    charclass_serializers.cpp
    charclass_serializers.h
    def_serializers.cpp
    def_serializers.h
    gui_definitions.h
    gui_serializers.cpp
    gui_serializers.h
    npc_serializers.cpp
    npc_serializers.h
    npc_definitions.h
    fx_serializers.cpp
    fx_serializers.h
    fx_definitions.h
    other_definitions.h
    origin_definitions.h
    keybind_serializers.cpp
    keybind_serializers.h
    keybind_definitions.h
    keybind_definitions.cpp
    #particlesys_serializers.cpp
    #particlesys_serializers.h
    costume_serializers.cpp
    costume_serializers.h
    #def_serializers.cpp
    map_serializers.cpp
    map_serializers.h
#    seq_serializers.cpp
#    seq_serializers.h
    seq_definitions.h
    shop_serializers.cpp
    shop_serializers.h
    trick_serializers.cpp
    trick_serializers.h
    #titles_serializers.cpp
    #titles_serializers.h
    #particle_definitions.h
    power_definitions.h
    power_serializers.cpp
    power_serializers.h
    scenegraph_definitions.h
    shop_definitions.h
    map_definitions.h
    map_definitions.cpp
    costume_definitions.h
    bodypart_definitions.h

    DataStorage.cpp
    DataStorage.h
    #TODO: move this to Components Library
    CoXHash.cpp
    CoXHash.h
    CoHMath.h
)
add_library(gameData STATIC ${GAMEDATA_SRC})
add_dependencies(gameData ace_IMP cereal_IMP) # used for Singleton stuff.
target_link_libraries(gameData PUBLIC Qt5::Core)
target_link_libraries(gameData INTERFACE SEGS_Components)
set_property(TARGET gameData APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:cereal_IMP,INTERFACE_INCLUDE_DIRECTORIES>")
