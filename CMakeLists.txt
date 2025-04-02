cmake_minimum_required(VERSION 3.30)

project(PDrum VERSION 0.1.0)

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
# Knob Component                                                                                                       #
# --------------                                                                                                       #
########################################################################################################################

add_library(KnobComponent STATIC
        Components/Knob/src/KnobComponent.cpp
)
target_include_directories(KnobComponent PUBLIC
        Components/Knob/inc
)
target_link_libraries(KnobComponent PRIVATE
        juce::juce_audio_processors
        juce::juce_gui_basics
)
target_compile_definitions(KnobComponent PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

########################################################################################################################
# Membrane Component                                                                                                   #
# ------------------                                                                                                   #
########################################################################################################################

add_library(MembraneComponent STATIC
        Components/Membrane/src/VibratingMembraneModel.cpp
        Components/Membrane/src/VibratingMembrane.cpp
)
target_include_directories(MembraneComponent PUBLIC
        Components/Membrane/inc
)
target_link_libraries(MembraneComponent PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_basics
)
target_compile_definitions(MembraneComponent PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

########################################################################################################################
# Resonator Component                                                                                                  #
# -------------------                                                                                                  #
########################################################################################################################

add_library(ResonatorComponent STATIC
        Components/Resonator/src/ModalResonator.cpp
)
target_include_directories(ResonatorComponent PUBLIC
        Components/Resonator/inc
)
target_link_libraries(ResonatorComponent PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_basics
)
target_compile_definitions(ResonatorComponent PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

########################################################################################################################
# PDrum Plugin                                                                                                         #
# ------------                                                                                                         #
########################################################################################################################

juce_add_plugin(PDrum
        COMPANY_NAME "Scribe Now LLC"
        PRODUCT_NAME "PDrum"
        BUNDLE_ID "com.scribenow.pdrum"
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE MJPL
        FORMATS VST3 AU Standalone
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(PDrum PRIVATE
        PDrum/src/PDrum.cpp
        PDrum/src/PDrumEditor.cpp
)

target_include_directories(PDrum PRIVATE
        PDrum/inc
)

add_dependencies(PDrum
        KnobComponent
        MembraneComponent
        ResonatorComponent
)

target_compile_definitions(PDrum PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

target_link_libraries(PDrum PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_basics
        KnobComponent
        MembraneComponent
        ResonatorComponent
)
