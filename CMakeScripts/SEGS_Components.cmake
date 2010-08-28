MACRO(GET_GLOBAL_AND_APPEND append_to global_name)
	UNSET(tmp)
	GET_PROPERTY(tmp GLOBAL PROPERTY ${global_name})
	LIST(APPEND ${append_to} ${tmp})
	LIST(REMOVE_DUPLICATES ${append_to})
ENDMACRO(GET_GLOBAL_AND_APPEND)
MACRO(SEGS_ADD_COMPONENT component_name LINK_TYPE)
	GET_PROPERTY(visited GLOBAL PROPERTY ${component_name}_VISITED SET)
	IF(visited)

		IF("${LINK_TYPE}" STREQUAL "LIB")

			SET(target_DEPENDS ${target_DEPENDS} ${component_name})
			GET_GLOBAL_AND_APPEND(target_INCLUDE_DIR ${component_name}_INCLUDE_DIR)
			GET_GLOBAL_AND_APPEND(target_DEPENDS ${component_name}_DEPENDS)

		ELSEIF("${LINK_TYPE}" STREQUAL "SOURCE")
			GET_GLOBAL_AND_APPEND(target_INCLUDE_DIR ${component_name}_INCLUDE_DIR)
			GET_GLOBAL_AND_APPEND(target_CPP ${component_name}_CPP)
			GET_GLOBAL_AND_APPEND(target_INCLUDE ${component_name}_INCLUDE)
			SOURCE_GROUP(${component_name}\\Source FILES ${${component_name}_CPP})
			SOURCE_GROUP(${component_name}\\Headers FILES ${${component_name}_INCLUDE})

		ELSE("${LINK_TYPE}" STREQUAL "LIB")

			MESSAGE(ERROR " Unknown component linking mode ${LINK_TYPE}")

		ENDIF("${LINK_TYPE}" STREQUAL "LIB")

	ELSE(visited)

		MESSAGE(WARNING " Component ${component_name} directory was not visited yet. Fix the order in which the directories are visited please.")

	ENDIF(visited)

ENDMACRO(SEGS_ADD_COMPONENT)
MACRO(SEGS_REGISTER_COMPONENT name include_dir sources includes )
	GET_PROPERTY(visited GLOBAL PROPERTY ${name}_VISITED SET)
	IF(visited)
		#MESSAGE(ERROR " Component ${component_name} already registered.")
	ELSE(visited)
		SET_PROPERTY(GLOBAL PROPERTY ${name}_VISITED TRUE)
		SET_PROPERTY(GLOBAL PROPERTY ${name}_INCLUDE_DIR ${include_dir})
		SET_PROPERTY(GLOBAL PROPERTY ${name}_DEPENDS ${target_DEPENDS})
		SET_PROPERTY(GLOBAL PROPERTY ${name}_CPP ${sources})	
		SET_PROPERTY(GLOBAL PROPERTY ${name}_INCLUDE ${includes})
		SET_PROPERTY(GLOBAL PROPERTY ${name}_SOURCES ${${name}_CPP} ${${name}_INCLUDE})
	ENDIF(visited)
    
ENDMACRO(SEGS_REGISTER_COMPONENT)
