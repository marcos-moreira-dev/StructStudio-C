cmake_minimum_required(VERSION 3.26)

project(StructStudioC VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(CTest)
include(ExternalProject)

if(WIN32)
    enable_language(RC)
endif()

find_program(PYTHON_RUNTIME NAMES python3 python REQUIRED)
find_program(NINJA_EXECUTABLE ninja REQUIRED)
find_program(MAKENSIS_EXECUTABLE makensis)

set(LIBUI_SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/libui-ng")
set(LIBUI_BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/libui-ng-build")
set(LIBUI_LIBRARY "${LIBUI_BINARY_DIR}/meson-out/libui.a")

if(NOT EXISTS "${LIBUI_SOURCE_DIR}/meson.build")
    message(FATAL_ERROR "No se encontro libui-ng en external/libui-ng. El proyecto requiere esa dependencia oficial.")
endif()

ExternalProject_Add(libui_ng_project
    SOURCE_DIR "${LIBUI_SOURCE_DIR}"
    BINARY_DIR "${LIBUI_BINARY_DIR}"
    # We build the official libui-ng sources with Meson/Ninja to keep the
    # dependency reproducible inside the repository.
    CONFIGURE_COMMAND "${PYTHON_RUNTIME}" -m mesonbuild.mesonmain setup "${LIBUI_BINARY_DIR}" "${LIBUI_SOURCE_DIR}" --buildtype=debug --default-library=static -Dtests=false -Dexamples=false
    BUILD_COMMAND "${NINJA_EXECUTABLE}" -C "${LIBUI_BINARY_DIR}"
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${LIBUI_LIBRARY}"
)

add_library(libui_ng STATIC IMPORTED GLOBAL)
set_target_properties(libui_ng PROPERTIES
    IMPORTED_LOCATION "${LIBUI_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LIBUI_SOURCE_DIR}"
    INTERFACE_COMPILE_DEFINITIONS "_UI_STATIC"
)
add_dependencies(libui_ng libui_ng_project)

if(WIN32)
    set(LIBUI_SYSTEM_LIBS
        user32 kernel32 gdi32 comctl32 uxtheme msimg32 comdlg32
        d2d1 dwrite ole32 oleaut32 oleacc uuid windowscodecs
    )
elseif(UNIX AND NOT APPLE)
    # The Unix backend of libui-ng is implemented over GTK3.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0>=3.10.0)
    set(LIBUI_SYSTEM_LIBS PkgConfig::GTK3 ${CMAKE_DL_LIBS} m)
else()
    message(FATAL_ERROR "La configuracion actual solo prepara Windows y Linux.")
endif()

target_link_libraries(libui_ng INTERFACE ${LIBUI_SYSTEM_LIBS})
if(WIN32 AND NOT MSVC)
    target_link_libraries(libui_ng INTERFACE stdc++)
endif()

add_library(cjson STATIC external/cjson/cJSON.c)
target_include_directories(cjson PUBLIC external/cjson)

add_library(structstudio_common STATIC
    src/common/error.c
    src/common/util.c
)
target_include_directories(structstudio_common PUBLIC include)

add_library(structstudio_core STATIC
    src/core/model.c
    src/core/api_base.c
    src/core/api_analysis.c
    src/core/api_analysis_playback.c
    src/core/api_theory.c
    src/core/api_transform.c
    src/core/api_examples.c
    src/core/api_analysis_graph_advanced.c
    src/core/api_tree.c
    src/core/api_primary.c
    src/core/api_secondary.c
    src/core/api_tertiary.c
    src/core/api_mutation.c
    src/core/api_layout.c
    src/core/api_validation.c
)
target_include_directories(structstudio_core PUBLIC include)
target_link_libraries(structstudio_core PUBLIC structstudio_common)

add_library(structstudio_persistence STATIC
    src/persistence/document_io.c
)
target_include_directories(structstudio_persistence PUBLIC include external/cjson)
target_link_libraries(structstudio_persistence PUBLIC structstudio_core cjson)

add_library(structstudio_render STATIC
    src/render/render.c
)
target_include_directories(structstudio_render PUBLIC include external/stb)
target_link_libraries(structstudio_render PUBLIC structstudio_core libui_ng)
if(NOT MSVC)
    target_link_libraries(structstudio_render PUBLIC m)
endif()

add_library(structstudio_editor STATIC
    src/editor/animation.c
    src/editor/editor.c
)
target_include_directories(structstudio_editor PUBLIC include)
target_link_libraries(structstudio_editor PUBLIC structstudio_core structstudio_persistence structstudio_render)

add_library(structstudio_ui STATIC
    src/ui/main_window.c
    src/ui/system_dialogs.c
)
target_include_directories(structstudio_ui PUBLIC include)
target_link_libraries(structstudio_ui PUBLIC structstudio_editor structstudio_render libui_ng)

set(STRUCTSTUDIO_APP_SOURCES
    src/app/main.c
    src/app/windows_bootstrap.c
)
if(WIN32)
    # libui-ng static builds on Windows require the application executable to
    # embed the Common Controls v6 manifest itself.
    list(APPEND STRUCTSTUDIO_APP_SOURCES src/app/windows_resources.rc)
endif()

add_executable(StructStudioC ${STRUCTSTUDIO_APP_SOURCES})
target_include_directories(StructStudioC PRIVATE include)
target_link_libraries(StructStudioC PRIVATE structstudio_ui)

install(TARGETS StructStudioC
    RUNTIME DESTINATION .
    BUNDLE DESTINATION .
)

install(FILES
    README.md
    DESTINATION .
)

install(DIRECTORY
    samples/
    DESTINATION samples
    FILES_MATCHING
        PATTERN "*.json"
)

install(DIRECTORY
    docs/
    DESTINATION docs
    FILES_MATCHING
        PATTERN "*.md"
)

install(FILES
    assets/icons/structstudio_classic.ico
    DESTINATION assets/icons
)

add_executable(structstudio_core_smoke tests/core_smoke.c)
target_include_directories(structstudio_core_smoke PRIVATE include)
target_link_libraries(structstudio_core_smoke PRIVATE structstudio_editor)
add_test(NAME core_smoke COMMAND structstudio_core_smoke)

add_executable(structstudio_core_variants_smoke tests/core_variants_smoke.c)
target_include_directories(structstudio_core_variants_smoke PRIVATE include)
target_link_libraries(structstudio_core_variants_smoke PRIVATE structstudio_editor)
add_test(NAME core_variants_smoke COMMAND structstudio_core_variants_smoke)

add_executable(structstudio_analysis_smoke tests/analysis_smoke.c)
target_include_directories(structstudio_analysis_smoke PRIVATE include)
target_link_libraries(structstudio_analysis_smoke PRIVATE structstudio_editor)
add_test(NAME analysis_smoke COMMAND structstudio_analysis_smoke)

add_executable(structstudio_graph_analysis_advanced_smoke tests/graph_analysis_advanced_smoke.c)
target_include_directories(structstudio_graph_analysis_advanced_smoke PRIVATE include)
target_link_libraries(structstudio_graph_analysis_advanced_smoke PRIVATE structstudio_editor)
add_test(NAME graph_analysis_advanced_smoke COMMAND structstudio_graph_analysis_advanced_smoke)

add_executable(structstudio_analysis_playback_smoke tests/analysis_playback_smoke.c)
target_include_directories(structstudio_analysis_playback_smoke PRIVATE include)
target_link_libraries(structstudio_analysis_playback_smoke PRIVATE structstudio_editor)
add_test(NAME analysis_playback_smoke COMMAND structstudio_analysis_playback_smoke)

add_executable(structstudio_theory_smoke tests/theory_smoke.c)
target_include_directories(structstudio_theory_smoke PRIVATE include)
target_link_libraries(structstudio_theory_smoke PRIVATE structstudio_editor)
add_test(NAME theory_smoke COMMAND structstudio_theory_smoke)

add_executable(structstudio_examples_smoke tests/examples_smoke.c)
target_include_directories(structstudio_examples_smoke PRIVATE include)
target_link_libraries(structstudio_examples_smoke PRIVATE structstudio_editor)
add_test(NAME examples_smoke COMMAND structstudio_examples_smoke)

add_executable(structstudio_animation_smoke tests/animation_smoke.c)
target_include_directories(structstudio_animation_smoke PRIVATE include)
target_link_libraries(structstudio_animation_smoke PRIVATE structstudio_editor)
add_test(NAME animation_smoke COMMAND structstudio_animation_smoke)

add_executable(structstudio_transform_smoke tests/transform_smoke.c)
target_include_directories(structstudio_transform_smoke PRIVATE include)
target_link_libraries(structstudio_transform_smoke PRIVATE structstudio_editor)
add_test(NAME transform_smoke COMMAND structstudio_transform_smoke)

function(enable_project_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

enable_project_warnings(structstudio_common)
enable_project_warnings(structstudio_core)
enable_project_warnings(structstudio_persistence)
enable_project_warnings(structstudio_render)
enable_project_warnings(structstudio_editor)
enable_project_warnings(structstudio_ui)
enable_project_warnings(StructStudioC)
enable_project_warnings(structstudio_core_smoke)
enable_project_warnings(structstudio_core_variants_smoke)
enable_project_warnings(structstudio_analysis_smoke)
enable_project_warnings(structstudio_graph_analysis_advanced_smoke)
enable_project_warnings(structstudio_analysis_playback_smoke)
enable_project_warnings(structstudio_theory_smoke)
enable_project_warnings(structstudio_examples_smoke)
enable_project_warnings(structstudio_animation_smoke)
enable_project_warnings(structstudio_transform_smoke)

set(CPACK_PACKAGE_NAME "StructStudio C")
set(CPACK_PACKAGE_VENDOR "StructStudio C")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Editor desktop en C para crear, analizar y estudiar estructuras de datos.")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "StructStudio C")
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_PACKAGE_EXECUTABLES "StructStudioC" "StructStudio C")
set(CPACK_CREATE_DESKTOP_LINKS "StructStudioC")

if(WIN32)
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/images/Struct Studio C.png")
    set(CPACK_NSIS_DISPLAY_NAME "StructStudio C")
    set(CPACK_NSIS_PACKAGE_NAME "StructStudio C")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/assets/icons/structstudio_classic.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/assets/icons/structstudio_classic.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "StructStudioC.exe")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_CONTACT "StructStudio C")
    if(MAKENSIS_EXECUTABLE)
        set(CPACK_GENERATOR "NSIS;ZIP")
    else()
        set(CPACK_GENERATOR "ZIP")
    endif()
elseif(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
