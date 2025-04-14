# The minimum required version of CMake
cmake_minimum_required(VERSION 3.30)

# Set the project name and version
project(PDrum VERSION 1.0.0)

# Set some information about the JUCE project
set(JUCE_COMPANY_NAME "Scribe Now LLC")
set(JUCE_PRODUCT_NAME "PDrum")
set(JUCE_BUNDLE_ID "com.scribenow.pdrum")
set(TARGET_NAME "pdrum")

# Enable/Disable Address Sanitizer
set(ASAN_ON OFF)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Make the C++ standard a required property
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent to grab JUCE
include(FetchContent)

# Pull JUCE from GitHub
FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG master
)
FetchContent_MakeAvailable(juce)

# Debug/Release flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # If building for Debug, use clang-tidy and AddressSanitizer
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" REQUIRED)
    set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "-checks=-*,modernize-*")
    if (NOT MSVC AND ASAN_ON)
        set(TARGET_COMPILE_OPTIONS -fsanitize=address -fno-omit-frame-pointer -Wall)
        set(TARGET_LINK_OPTIONS -fsanitize=address)
    else ()
        # Note that AddressSanitizer is not supported on MSVC
        set(TARGET_COMPILE_OPTIONS -Wall)
        set(TARGET_LINK_OPTIONS "")
    endif ()
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (NOT MSVC)
        set(TARGET_COMPILE_OPTIONS -O3 -DNDEBUG -Wall)
    else ()
        set(TARGET_COMPILE_OPTIONS /O2 -DNDEBUG -Wall)
    endif ()
    set(TARGET_LINK_OPTIONS "")
else ()
    message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}")
endif ()

################################################################################
# Plugin                                                                       #
# ------------                                                                 #
################################################################################

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # If we're building for macOS, build the VST3, AU, and Standalone formats
    set(PLUGIN_FORMATS VST3 AU Standalone)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # If we're building for Windows, build the VST3 and Standalone formats
    set(PLUGIN_FORMATS VST3 Standalone)
else ()
    # If we're building for Linux, build the VST3 and Standalone formats
    set(PLUGIN_FORMATS VST3 Standalone)
    # Also disable Webkit
    add_definitions(-DJUCE_WEB_BROWSER=0)
endif ()

# Add a JUCE plugin and define the module settings
juce_add_plugin(${TARGET_NAME}
        COMPANY_NAME ${JUCE_COMPANY_NAME}
        PRODUCT_NAME ${JUCE_PRODUCT_NAME}
        BUNDLE_ID ${JUCE_BUNDLE_ID}
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE MJPL
        FORMATS ${PLUGIN_FORMATS}
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        MICROPHONE_PERMISSION_ENABLED TRUE
        MICROPHONE_PERMISSION_TEXT "Need access to your audio interface"
        ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/dist/Resources/dsp-ark-logo.png"
        COPY_PLUGIN_AFTER_BUILD TRUE
)

# Define the plugin's source files
target_sources(${TARGET_NAME} PRIVATE
        Components/Knob/src/KnobComponent.cpp
        Components/Membrane/src/VibratingMembraneModel.cpp
        Components/Membrane/src/VibratingMembrane.cpp
        Components/Resonator/src/ModalResonatorModel.cpp
        Components/Resonator/src/ModalResonator.cpp
        PDrum/src/PDrum.cpp
        PDrum/src/PDrumEditor.cpp
)

# Ensure the inc folder is included in the search path for included files
target_include_directories(${TARGET_NAME} PRIVATE
        Components/Knob/inc
        Components/Membrane/inc
        Components/Resonator/inc
        PDrum/inc
)

# Prevent JUCE from including its own module settings since they're defined here
target_compile_definitions(${TARGET_NAME} PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

# Link the JUCE libraries
target_link_libraries(${TARGET_NAME} PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_opengl
)

# If we're building for Debug, make sure clang-tidy is used
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${TARGET_NAME} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
    )
endif ()

# Use the Debug/Release flags defined above
# These need to be individually attached to each executable target
foreach(format IN LISTS PLUGIN_FORMATS)
    string(TOUPPER ${format} UPPER_FORMAT)
    set(component_name "${TARGET_NAME}_${format}")
    if (TARGET ${component_name})
        target_compile_options(${component_name} PRIVATE ${TARGET_COMPILE_OPTIONS})
        target_link_options(${component_name} PRIVATE ${TARGET_LINK_OPTIONS})
    endif ()
endforeach()

# Link libcurl on Linux only
if (UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE CURL::libcurl)
endif ()