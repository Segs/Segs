
SET(target_CPP
    SlashCommand.cpp
    SlashCommand_Contacts.cpp
    SlashCommand_Debug.cpp
    SlashCommand_Email.cpp
    SlashCommand_Friends.cpp
    SlashCommand_Misc.cpp
    SlashCommand_Movement.cpp
    SlashCommand_Powers.cpp
    SlashCommand_Scripts.cpp
    SlashCommand_Settings.cpp
    SlashCommand_Stats.cpp
    SlashCommand_SuperGroup.cpp
    SlashCommand_Teams.cpp
    SlashCommand_Trade.cpp
)
SET(target_INCLUDE
    SlashCommand.h
    SlashCommand_Contacts.h
    SlashCommand_Debug.h
    SlashCommand_Email.h
    SlashCommand_Friends.h
    SlashCommand_Misc.h
    SlashCommand_Movement.h
    SlashCommand_Powers.h
    SlashCommand_Scripts.h
    SlashCommand_Settings.h
    SlashCommand_Stats.h
    SlashCommand_SuperGroup.h
    SlashCommand_Teams.h
    SlashCommand_Trade.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

add_library(SlashCommands STATIC ${target_SOURCES})
target_link_libraries(SlashCommands PUBLIC Qt5::Core gameData ScriptingEngine)

get_target_property(TARGET_SOURCES SlashCommands SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (SlashCommands PROPERTIES
    FOLDER Servers
)
