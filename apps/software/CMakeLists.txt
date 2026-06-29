set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "gui;gui/Components;gui/Views;gui/Views/Settings;gui/Views/Exposure;gui/Views/Alignment")

qt_add_executable(Kub3Software
    MANUAL_FINALIZATION
    
    # Sources
    src/main.cpp
    src/Logger.h
    src/ApplicationBuilder.cpp
    src/ViewModels/HomeViewModel.cpp
    src/ViewModels/SettingsViewModel.cpp
    src/ViewModels/ExposureMenuViewModel.cpp
    src/ViewModels/Exposure/ExposureSettingsViewModel.cpp
    src/ViewModels/Exposure/FavoriteExposureSettingsViewModel.cpp
    src/ViewModels/Exposure/RecapExposureSettingsViewModel.cpp
    src/ViewModels/Exposure/CompleteExposureViewModel.cpp
    src/ViewModels/Exposure/ProgressExposureViewModel.cpp
    src/ViewModels/Exposure/SaveExposureSettingsViewModel.cpp
    src/ViewModels/Settings/MachineStatusViewModel.cpp
    src/ViewModels/Settings/VersionViewModel.cpp
    src/ViewModels/Settings/TemperatureViewModel.cpp
    src/ViewModels/Settings/UpdateSoftwareViewModel.cpp
    src/ViewModels/Settings/ScreenshotExportViewModel.cpp
    src/ViewModels/Settings/LedTestViewModel.cpp
    src/ViewModels/Settings/OperatingTimesViewModel.cpp
    src/ViewModels/Alignment/LoadParametersViewModel.cpp
    src/ViewModels/Alignment/SaveParametersViewModel.cpp
    src/ViewModels/Alignment/VisualisationViewModel.cpp
    src/ViewModels/Alignment/DistanceViewModel.cpp
    src/ViewModels/Alignment/ContactSelectionViewModel.cpp
    src/Views/Exposure/ExposureSettingsView.cpp
    src/Views/Exposure/FavoriteExposureSettingsView.cpp
    src/Views/Exposure/RecapExposureSettingsView.cpp
    src/Views/Exposure/CompleteExposureView.cpp
    src/Views/Exposure/ProgressExposureView.cpp
    src/Views/Exposure/SaveExposureSettingsView.cpp
    src/Views/MainWindow.cpp
    src/Views/ViewBase.cpp
    src/Views/KeyboardConnections.cpp
    src/Views/AlignmentViewBase.cpp
    src/Views/SettingsViewBase.cpp
    src/Views/ExposureViewBase.cpp
    src/Views/HomeView.cpp
    src/Views/HomeEightView.cpp
    src/Views/ExposureMenuView.cpp
    src/Views/SettingsView.cpp
    src/Views/Settings/MachineStatusView.cpp
    src/Views/Settings/VersionView.cpp
    src/Views/Settings/TemperatureView.cpp
    src/Views/Settings/UpdateSoftwareView.cpp
    src/Views/Settings/ScreenshotExportView.cpp
    src/Views/Settings/LedTestView.cpp
    src/Views/Settings/OperatingTimesView.cpp
    src/Views/Alignment/LoadParametersView.cpp
    src/Views/Alignment/SaveParametersView.cpp
    src/Views/Alignment/VisualisationView.cpp
    src/Views/Alignment/DistanceView.cpp
    src/Views/Alignment/ContactSelectionView.cpp
    src/Views/Components/FavoriteExposureSettingButton.cpp
    src/Views/Components/HardForceContactForm.cpp
    src/Views/Components/RealPositionCameras.cpp
    src/Views/Components/MessageOverlay.cpp
    src/Views/Components/NavButtonManager.cpp
    src/Views/Components/NavButton.cpp
    src/Views/Components/UpBar.cpp
    src/Views/Components/ActionBox.cpp
    src/Views/Components/BottomCroppedCircle.cpp
    src/Views/Components/ScreenshotThumbnailWidget.cpp
    src/Views/Modules/CamerasTestView.cpp

    # Headers
    include/ApplicationBuilder.h
    include/ViewModels/HomeViewModel.h
    include/ViewModels/SettingsViewModel.h
    include/ViewModels/ExposureMenuViewModel.h
    include/ViewModels/Exposure/ExposureSettingsViewModel.h
    include/ViewModels/Exposure/FavoriteExposureSettingsViewModel.h
    include/ViewModels/Exposure/RecapExposureSettingsViewModel.h
    include/ViewModels/Exposure/CompleteExposureViewModel.h
    include/ViewModels/Exposure/ProgressExposureViewModel.h
    include/ViewModels/Exposure/SaveExposureSettingsViewModel.h
    include/ViewModels/Settings/MachineStatusViewModel.h
    include/ViewModels/Settings/VersionViewModel.h
    include/ViewModels/Settings/TemperatureViewModel.h
    include/ViewModels/Settings/UpdateSoftwareViewModel.h
    include/ViewModels/Settings/ScreenshotExportViewModel.h
    include/ViewModels/Settings/LedTestViewModel.h
    include/ViewModels/Settings/OperatingTimesViewModel.h
    include/ViewModels/Alignment/LoadParametersViewModel.h
    include/ViewModels/Alignment/SaveParametersViewModel.h
    include/ViewModels/Alignment/VisualisationViewModel.h
    include/ViewModels/Alignment/DistanceViewModel.h
    include/ViewModels/Alignment/ContactSelectionViewModel.h
    include/Views/Modules/CamerasTestView.h
    include/Views/Exposure/ExposureSettingsView.h
    include/Views/Exposure/FavoriteExposureSettingsView.h
    include/Views/Exposure/RecapExposureSettingsView.h
    include/Views/Exposure/CompleteExposureView.h
    include/Views/Exposure/ProgressExposureView.h
    include/Views/Exposure/SaveExposureSettingsView.h
    include/Views/MainWindow.h
    include/Views/ViewBase.h
    include/Views/KeyboardConnections.h
    include/Views/AlignmentViewBase.h
    include/Views/SettingsViewBase.h
    include/Views/ExposureViewBase.h
    include/Views/HomeView.h
    include/Views/HomeEightView.h
    include/Views/ExposureMenuView.h
    include/Views/SettingsView.h
    include/Views/Settings/MachineStatusView.h
    include/Views/Settings/TemperatureView.h
    include/Views/Settings/VersionView.h
    include/Views/Settings/UpdateSoftwareView.h
    include/Views/Settings/ScreenshotExportView.h
    include/Views/Settings/LedTestView.h
    include/Views/Settings/OperatingTimesView.h
    include/Views/Alignment/LoadParametersView.h
    include/Views/Alignment/SaveParametersView.h
    include/Views/Alignment/VisualisationView.h
    include/Views/Alignment/DistanceView.h
    include/Views/Alignment/ContactSelectionView.h
    include/Views/Components/FavoriteExposureSettingButton.h
    include/Views/Components/NavButtonManager.h
    include/Views/Components/HardForceContactForm.h
    include/Views/Components/RealPositionCameras.h
    include/Views/Components/MessageOverlay.h
    include/Views/Components/NavButton.h
    include/Views/Components/UpBar.h
    include/Views/Components/ActionBox.h
    include/Views/Components/BottomCroppedCircle.h
    include/Views/Components/ScreenshotThumbnailWidget.h

    # UI
    gui/resources.qrc
    gui/MainWindow.ui
    gui/Views/Exposure/ExposureSettingsView.ui
    gui/Views/Exposure/FavoriteExposureSettingsView.ui
    gui/Views/Exposure/RecapExposureSettingsView.ui
    gui/Views/Exposure/CompleteExposureView.ui
    gui/Views/Exposure/ProgressExposureView.ui
    gui/Views/HomeView.ui
    gui/Views/HomeEightView.ui
    gui/Views/ExposureMenuView.ui
    gui/Views/SettingsView.ui
    gui/Views/Settings/MachineStatusView.ui
    gui/Views/Settings/VersionView.ui
    gui/Views/Settings/TemperatureView.ui
    gui/Views/Settings/UpdateSoftwareView.ui
    gui/Views/Settings/ScreenshotExportView.ui
    gui/Views/Settings/LedTestView.ui
    gui/Views/Settings/OperatingTimesView.ui
    gui/Views/Alignment/LoadParametersView.ui
    gui/Views/Alignment/SaveParametersView.ui
    gui/Views/Alignment/VisualisationView.ui
    gui/Views/Alignment/DistanceView.ui
    gui/Views/Alignment/ContactSelectionView.ui
    gui/Components/HardForceContactForm.ui
    gui/Components/RealPositionCameras.ui
    gui/Components/NavButton.ui
)

qt_finalize_target(Kub3Software)

target_include_directories(Kub3Software PRIVATE include)
target_link_libraries(Kub3Software PRIVATE Kub3Core Qt6::Gui Qt6::Widgets Qt6::Svg)

set_target_properties(Kub3Software PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Install App
install(TARGETS Kub3Software RUNTIME DESTINATION bin)

# Explicitly deploy the MVS Camera libraries
if(MVS_LIBRARY)
    get_filename_component(MVS_LIB_DIR "${MVS_LIBRARY}" DIRECTORY)
    install(DIRECTORY "${MVS_LIB_DIR}/"
        DESTINATION lib
        FILES_MATCHING PATTERN "*.so*"
    )
endif()

qt_generate_deploy_app_script(TARGET Kub3Software OUTPUT_SCRIPT deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
install(SCRIPT ${deploy_script})
