# ==============================================================================
# CONFIG GENERATOR EXECUTABLE: Kub3ConfigGenerator
# ==============================================================================
qt_add_executable(Kub3ConfigGenerator
    MANUAL_FINALIZATION
    # Sources
    src/main.cpp
    src/ConfigGenerator.cpp

    # Headers
    include/ConfigGenerator.h
)

qt_finalize_target(Kub3ConfigGenerator)

target_include_directories(Kub3ConfigGenerator PRIVATE
    include
)

# Link against the main project's core library
# We only need Qt6::Core since this is a CLI utility
target_link_libraries(Kub3ConfigGenerator PRIVATE
    Kub3Core
    Qt6::Core
)

set_target_properties(Kub3ConfigGenerator PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Include binary into the "bin/" dir of the output folder
install(TARGETS Kub3ConfigGenerator RUNTIME DESTINATION bin)

# Generate deployment script for Qt dependencies
if(TARGET Qt6::Core)
    qt_generate_deploy_app_script(
        TARGET Kub3ConfigGenerator
        OUTPUT_SCRIPT deploy_script_config_gen
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script_config_gen})
endif()
