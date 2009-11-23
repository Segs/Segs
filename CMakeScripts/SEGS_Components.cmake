MACRO(SEGS_ADD_COMPONENT component_name LINK_TYPE)
	IF(${component_name}_VISITED)

		IF("${LINK_TYPE}" STREQUAL "LIB")

            SET(target_INCLUDE_DIR ${target_INCLUDE_DIR} ${${component_name}_INCLUDE_DIR})
			SET(target_DEPENDS ${target_DEPENDS} ${component_name} ${${component_name}_DEPENDS})

		ELSEIF("${LINK_TYPE}" STREQUAL "SOURCE")

            SET(target_INCLUDE_DIR ${target_INCLUDE_DIR} ${${component_name}_INCLUDE_DIR})
			SET(target_CPP ${target_CPP} ${${component_name}_CPP})
			SET(target_INCLUDE ${target_INCLUDE} ${${component_name}_INCLUDE})
			SOURCE_GROUP(${component_name}\\Source FILES ${${component_name}_CPP})
			SOURCE_GROUP(${component_name}\\Headers FILES ${${component_name}_INCLUDE})

		ELSE("${LINK_TYPE}" STREQUAL "LIB")

			MESSAGE(ERROR " Unknown component linking mode ${LINK_TYPE}")

		ENDIF("${LINK_TYPE}" STREQUAL "LIB")

	ELSE(${component_name}_VISITED)

		MESSAGE(ERROR " Component ${component_name} directory was not visited yet. Fix the order in which the directories are visited please.")

	ENDIF(${component_name}_VISITED)

ENDMACRO(SEGS_ADD_COMPONENT)

MACRO(SEGS_REGISTER_COMPONENT name include_dir sources includes )

    IF(${name}_VISITED)
		#MESSAGE(ERROR " Component ${component_name} already registered.")
    ELSE(${name}_VISITED)

        SET(${name}_VISITED TRUE CACHE INTERNAL "")
        SET(${name}_INCLUDE_DIR ${include_dir} CACHE INTERNAL "")
        SET(${name}_DEPENDS ${target_DEPENDS} CACHE INTERNAL "") # warning! will pull in all current dependencies 
        SET(${name}_CPP ${sources} CACHE INTERNAL "")	
        SET(${name}_INCLUDE ${includes} CACHE INTERNAL "")
        SET(${name}_SOURCES ${${name}_CPP} ${${name}_INCLUDE} CACHE INTERNAL "")
    ENDIF(${name}_VISITED)
    
ENDMACRO(SEGS_REGISTER_COMPONENT)
