find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Qml)

qt_add_executable(ncradio-gui
  main.cpp
  ConfigStore.cpp
  PresetListModel.cpp
  RadioController.cpp
  AudioController.cpp
  EqController.cpp
  RecordController.cpp
  ThemeController.cpp
)

# QML files live under ../qml (outside this target's source dir), so each
# needs an explicit QT_RESOURCE_ALIAS relative to the qml/ root. Globbed so
# new pages/components/dialogs are picked up automatically; CONFIGURE_DEPENDS
# means a re-run of cmake (e.g. via `cmake --build` after adding a file)
# regenerates the file list.
set(QML_ROOT ${CMAKE_SOURCE_DIR}/qml)
file(GLOB_RECURSE QML_FILES_ABS CONFIGURE_DEPENDS ${QML_ROOT}/*.qml)
foreach(abs ${QML_FILES_ABS})
  file(RELATIVE_PATH rel ${QML_ROOT} ${abs})
  set_source_files_properties(${abs} PROPERTIES QT_RESOURCE_ALIAS "${rel}")
endforeach()

qt_add_qml_module(ncradio-gui
  URI ncradiogui
  VERSION 1.0
  QML_FILES ${QML_FILES_ABS}
)

qt_add_resources(ncradio-gui "fonts"
  PREFIX "/fonts"
  BASE ${CMAKE_SOURCE_DIR}/fonts
  FILES
    ${CMAKE_SOURCE_DIR}/fonts/JetBrainsMono-Regular.ttf
    ${CMAKE_SOURCE_DIR}/fonts/JetBrainsMono-Medium.ttf
    ${CMAKE_SOURCE_DIR}/fonts/JetBrainsMono-SemiBold.ttf
    ${CMAKE_SOURCE_DIR}/fonts/JetBrainsMono-Bold.ttf
    ${CMAKE_SOURCE_DIR}/fonts/SpaceGrotesk-Variable.ttf
    ${CMAKE_SOURCE_DIR}/fonts/Orbitron-Variable.ttf
)

target_link_libraries(ncradio-gui PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::QuickControls2
  Qt6::Qml
  ncradio-backend
)
