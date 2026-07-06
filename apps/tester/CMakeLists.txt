# ==============================================================================
# MOTOR TESTER EXECUTABLE: Kub3Tester
# ==============================================================================

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "gui;gui/Views;gui/Components")

qt_add_executable(Kub3Tester
    MANUAL_FINALIZATION

    # Sources
    src/main.cpp
    src/Logger.h
    src/Controllers/ProcedureTestController.cpp
    src/Controllers/MotorTestController.cpp
    src/Controllers/CameraTestController.cpp
    src/ApplicationBuilder.cpp
    src/Services/MotorTestService.cpp
    src/Services/FocalTestService.cpp
    src/Services/LightingTestService.cpp
    src/Views/ViewBase.cpp
    src/Views/MotorTestView.cpp
    src/Views/ProcedureTestView.cpp
    src/Views/CamerasTestView.cpp
    src/Views/Components/RealTimeCurveWidget.cpp
    src/Views/Components/FocalTestModule.cpp
    src/Views/Components/CamLightTestModule.cpp
    src/Views/Components/CameraTestModule.cpp
    src/TesterMainWindow.cpp
    src/ViewModels/ProcedureTestViewModel.cpp
    src/ViewModels/MotorTestViewModel.cpp
    src/ViewModels/CamerasTestViewModel.cpp

    # Headers
    include/Controllers/MotorTestController.h
    include/Controllers/ProcedureTestController.h
    include/Controllers/CameraTestController.h
    include/TesterMainWindow.h
    include/ApplicationBuilder.h
    include/Services/MotorTestService.h
    include/Services/FocalTestService.h
    include/Services/LightingTestService.h
    include/Views/ViewBase.h
    include/Views/MotorTestView.h
    include/Views/ProcedureTestView.h
    include/Views/CamerasTestView.h
    include/Views/Components/RealTimeCurveWidget.h
    include/Views/Components/FocalTestModule.h
    include/Views/Components/CamLightTestModule.h
    include/Views/Components/CameraTestModule.h
    include/ViewModels/ProcedureTestViewModel.h
    include/ViewModels/MotorTestViewModel.h
    include/ViewModels/CamerasTestViewModel.h

    # UI
    gui/resources.qrc
    gui/TesterMainWindow.ui
    gui/Views/ProcedureTestView.ui
    gui/Views/MotorTestView.ui
)

qt_finalize_target(Kub3Tester)

target_include_directories(Kub3Tester PRIVATE
    include
)

# Link against the main project's core library
target_link_libraries(Kub3Tester PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(Kub3Tester PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Include binary into the "bin/" dir of the output folder
install(TARGETS Kub3Tester RUNTIME DESTINATION bin)

if(TARGET Qt6::Core)
    qt_generate_deploy_app_script(
        TARGET Kub3Tester
        OUTPUT_SCRIPT deploy_script_config
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script_config})
endif()
