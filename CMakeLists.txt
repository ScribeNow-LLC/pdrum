cmake_minimum_required(VERSION 3.30)

project(PDrum VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent to grab JUCE
include(FetchContent)
FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG master
)
FetchContent_MakeAvailable(juce)

########################################################################################################################
# PDrum Plugin                                                                                                         #
# ------------                                                                                                         #
########################################################################################################################

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PDRUM_FORMATS VST3 AU Standalone)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PDRUM_FORMATS VST3 Standalone)
else ()
    # Linux, Raspberry Pi, etc.
    set(PDRUM_FORMATS VST3 Standalone)
endif ()

if (UNIX AND NOT APPLE)
    add_definitions(-DJUCE_WEB_BROWSER=0)
endif ()

juce_add_plugin(PDrum
        COMPANY_NAME "Scribe Now LLC"
        PRODUCT_NAME "PDrum"
        BUNDLE_ID "com.scribenow.pdrum"
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE MJPL
        FORMATS ${PDRUM_FORMATS}
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(PDrum PRIVATE
        Components/Knob/src/KnobComponent.cpp
        Components/Membrane/src/VibratingMembraneModel.cpp
        Components/Membrane/src/VibratingMembrane.cpp
        Components/Resonator/src/ModalResonatorModel.cpp
        Components/Resonator/src/ModalResonator.cpp
        PDrum/src/PDrum.cpp
        PDrum/src/PDrumEditor.cpp
)

target_include_directories(PDrum PRIVATE
        Components/Knob/inc
        Components/Membrane/inc
        Components/Resonator/inc
        PDrum/inc
)

target_compile_definitions(PDrum PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

target_link_libraries(PDrum PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_opengl
)

# Link libcurl on Linux only
if (UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_link_libraries(PDrum PRIVATE CURL::libcurl)
endif ()
