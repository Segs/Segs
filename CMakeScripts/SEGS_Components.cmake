MACRO(SEGS_ADD_COMPONENT component_name LINK_TYPE)
	IF(${component_name}_VISITED)

		IF("${LINK_TYPE}" STREQUAL "LIB")

			INCLUDE_DIRECTORIES(${component_name}_INCLUDE_DIR)		
			SET(target_DEPENDS ${target_DEPENDS} ${component_name})
			#MESSAGE(ERROR "Depends On ${target_DEPENDS}")

		ELSEIF("${LINK_TYPE}" STREQUAL "SOURCE")
            SET(testval "axxxx")
            SET(target_INCLUDE_DIR ${target_INCLUDE_DIR} ${${component_name}_INCLUDE_DIR})
#			SET(target_SOURCES ${target_SOURCES} ${${component_name}_SOURCES})
			SET(target_CPP ${target_CPP} ${${component_name}_CPP})
			SET(target_INCLUDE ${target_INCLUDE} ${${component_name}_INCLUDE})
			SOURCE_GROUP(${component_name}\\Source FILES ${${component_name}_CPP})
			SOURCE_GROUP(${component_name}\\Headers FILES ${${component_name}_INCLUDE})
#			MESSAGE(WARNING " Sources when adding ${component_name}:${target_SOURCES}")
#           MESSAGE(WARNING " Component includes ${component_name}:${${component_name}_INCLUDE_DIR}")
#           MESSAGE(WARNING " Includes when adding ${component_name}:${target_INCLUDE_DIR}")
		ELSE("${LINK_TYPE}" STREQUAL "LIB")

			MESSAGE(ERROR " Unknown component linking mode ${LINK_TYPE}")

		ENDIF("${LINK_TYPE}" STREQUAL "LIB")

	ELSE(${component_name}_VISITED)

		MESSAGE(ERROR " Component ${component_name} directory was not visited yet. Fix the order in which the directories are visited please.")

	ENDIF(${component_name}_VISITED)

ENDMACRO(SEGS_ADD_COMPONENT)

MACRO(SEGS_REGISTER_COMPONENT name include_dir sources includes )

	SET(${name}_VISITED TRUE CACHE INTERNAL "")
	SET(${name}_INCLUDE_DIR ${include_dir} CACHE INTERNAL "")
#   MESSAGE(WARNING " Setting Component includes ${name}:${include_dir}")
    
	SET(${name}_CPP ${sources} CACHE INTERNAL "")	
	SET(${name}_INCLUDE ${includes} CACHE INTERNAL "")
    SET(${name}_SOURCES ${${name}_CPP} ${${name}_INCLUDE} CACHE INTERNAL "")
    
ENDMACRO(SEGS_REGISTER_COMPONENT)
