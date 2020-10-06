
file(GLOB source_files "*.cpp" )
file(GLOB header_files "*.h" )

set_engine_plugin_options(
    NAME coh_model
    SHARED ON
    CLASSPROP ResourceImporterCoHModel
    TYPE resource
    SOURCES ${source_files} ${header_files} ${thirdparty_sources}
    LIBS common_runtime
)


 