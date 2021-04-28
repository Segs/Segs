find_package(Qt5 COMPONENTS Core REQUIRED)

SET(target_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/Clue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Character.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterAttributes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterClass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CommonNetStructures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Contact.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Costume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EntityHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Friend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LFG.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VisitLocation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Movement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PlayerStatistics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PlayerProgress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Powers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StateInterpolator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StateStorage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sidekick.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Team.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Trade.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/chardata_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clientoptions_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clientoptions_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/playerdata_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/entitydata_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trays_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/anim_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bodypart_definitions.cpp
    # Resource file formats
    ${CMAKE_CURRENT_SOURCE_DIR}/anim_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/attrib_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bodypart_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scenegraph_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/charclass_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/def_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/npc_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/npc_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fx_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/keybind_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/keybind_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/particlesys_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/particle_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/costume_serializers.cpp
    #def_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/map_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/seq_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/seq_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shop_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trick_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trick_definitions.cpp
    #titles_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/power_serializers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/map_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spawn_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DataStorage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDataStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NpcStore.cpp
    #TODO: move this to Components Library
    ${CMAKE_CURRENT_SOURCE_DIR}/CoXHash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoXHash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CoHMath.h
)
SET(target_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/Clue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CommonNetStructures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Character.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterAttributes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterClass.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CharacterHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Costume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/EntityHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Friend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Contact.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LFG.h
    ${CMAKE_CURRENT_SOURCE_DIR}/VisitLocation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Movement.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PlayerStatistics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PlayerProgress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Powers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Sidekick.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StateInterpolator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StateStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Store.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Task.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Team.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Trade.h
    ${CMAKE_CURRENT_SOURCE_DIR}/chardata_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/clientoptions_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/clientoptions_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/playerdata_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/playerdata_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/entitydata_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/entitydata_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trays_serializers.h
    # Resource file formats
    ${CMAKE_CURRENT_SOURCE_DIR}/anim_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/anim_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/attrib_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bodypart_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/scenegraph_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/charclass_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/def_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gui_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gui_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/npc_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/npc_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fx_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fx_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/other_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/origin_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/keybind_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/keybind_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/particlesys_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/particle_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/costume_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/map_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/seq_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/seq_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shop_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trick_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trick_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/power_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/power_serializers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/scenegraph_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shop_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/map_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spawn_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/costume_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bodypart_definitions.h
        #titles_serializers.h
        #particle_definitions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DataStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDataStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NpcStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ClientStates.h
)
add_library(gameData STATIC ${target_CPP} ${target_INCLUDE})
set_target_properties(gameData PROPERTIES AUTOMOC ON)
add_dependencies(gameData ace_IMP cereal_IMP) # used for Singleton stuff.
target_link_libraries(gameData PUBLIC Qt5::Core)
target_link_libraries(gameData INTERFACE SEGS_Components)
target_include_directories(gameData INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
  #$<INSTALL_INTERFACE:gameData>
)
set_property(TARGET gameData APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:cereal_IMP,INTERFACE_INCLUDE_DIRECTORIES>")

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES gameData SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (gameData PROPERTIES
    FOLDER Common
)
