
file(GLOB source_files "*.cpp" )
file(GLOB header_files "*.h" )

set_engine_plugin_options(
    NAME coh_texture
    SHARED ON
    CLASSPROP ResourceLoaderCoHTexture
    TYPE resource
    SOURCES ${source_files} ${header_files} ${thirdparty_sources}
)


