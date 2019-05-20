file(GLOB headers *.h)
set(MIGRATION_CLASSES)
set(MIGRATION_SOURCES)

# create the migrations registry file.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/migrations_gen.cpp "//This file was generated during CMake configuration phase, do not edit.\n" )

foreach(path ${headers})
        get_filename_component(class_name ${path} NAME_WE)
    message("Adding ${class_name} migration")
    list(APPEND MIGRATION_CLASSES ${class_name})
    list(APPEND MIGRATION_SOURCES ${path})
    # we include all migration headers in the generated file
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/migrations_gen.cpp "#include \"${path}\"\n" )
endforeach()

# the 'entry' functions that will fill the `target` variable with all migrations that existed at the time this file was cmake-configured
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/migrations_gen.cpp "void register_migrations(std::vector<DBMigrationStep *> &target)\n{\n" )

foreach(class ${MIGRATION_CLASSES})
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/migrations_gen.cpp "    target.push_back(new ${class});\n" )
endforeach()

file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/migrations_gen.cpp "}\n" )

set(MIGRATION_SOURCES ${MIGRATION_SOURCES} PARENT_SCOPE) # make the collected list of headers available in the parent CMakeLists
set(MIGRATION_GEN ${CMAKE_CURRENT_BINARY_DIR}/migrations_gen.cpp PARENT_SCOPE)
