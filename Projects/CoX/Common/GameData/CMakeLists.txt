find_package(Qt5Core REQUIRED)
set(GAMEDATA_SRC
    Colors.h
    serialization_common.h
    bodypart_serializers.cpp
    bodypart_serializers.h
    #particlesys_serializers.cpp
    #particlesys_serializers.h
    costume_serializers.cpp
    costume_serializers.h
    #def_serializers.cpp
    #seq_serializers.cpp
    #seq_serializers.h
    #shop_serializers.cpp
    #shop_serializers.h
    #titles_serializers.cpp
    #titles_serializers.h
    #particle_definitions.h
    costume_definitions.h
    bodypart_definitions.h
)
add_library(gameData OBJECT ${GAMEDATA_SRC})
set_property(TARGET gameData APPEND PROPERTY COMPILE_DEFINITIONS "$<TARGET_PROPERTY:Qt5::Core,INTERFACE_COMPILE_DEFINITIONS>")
set_property(TARGET gameData APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>")
set_property(TARGET gameData APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:cereal_IMP,INTERFACE_INCLUDE_DIRECTORIES>")
set_property(TARGET gameData APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:static_storage,INTERFACE_INCLUDE_DIRECTORIES>")
