cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

# DOC: 02-project-setup.md # Root CMakeLists.txt
project(WebViewStarter VERSION 1.0.0)

# PATTERN: Include CPM for JUCE dependency management
include(cmake/CPM.cmake)  # Or use git submodule: add_subdirectory(libs/juce)

# DOC: 02-project-setup.md # JUCE via CPM
CPMAddPackage("gh:juce-framework/JUCE#8.0.6")

# DOC: 02-project-setup.md # Plugin Definition
juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "YourCompany"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE YRCM  # Change: 4 uppercase letters
    PLUGIN_CODE WVST                # Change: 4 characters
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "WebView Starter"
    # CRITICAL: Enable WebView support
    NEEDS_WEB_BROWSER TRUE
    NEEDS_WEBVIEW2 TRUE  # Windows only
)

# Get product info for passing to C++
get_target_property(PRODUCT_NAME ${PROJECT_NAME} JUCE_PRODUCT_NAME)
get_target_property(COMPANY_NAME ${PROJECT_NAME} JUCE_COMPANY_NAME)

# Pass compile-time constants to C++
target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_PRODUCT_NAME="${PRODUCT_NAME}"
    JUCE_COMPANY_NAME="${COMPANY_NAME}"
    JUCE_PRODUCT_VERSION="${PROJECT_VERSION}"
)

# Source files
target_sources(${PROJECT_NAME} PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    include/PluginProcessor.h
    include/PluginEditor.h
    include/ParameterIDs.h
)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# DOC: 07-distribution.md # Zip Web UI Files
set(WEBVIEW_FILES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui/public")

# Copy JUCE frontend library
file(COPY
    "${JUCE_MODULES_DIR}/juce_gui_extra/native/javascript/"
    DESTINATION "${WEBVIEW_FILES_SOURCE_DIR}/js/juce/"
)

# Create zip of web UI files
set(WEBVIEW_FILES_ZIP_NAME "webview_files.zip")
set(TARGET_WEBVIEW_FILES_ZIP_PATH "${CMAKE_BINARY_DIR}/${WEBVIEW_FILES_ZIP_NAME}")
cmake_path(ABSOLUTE_PATH WEBVIEW_FILES_SOURCE_DIR NORMALIZE OUTPUT_VARIABLE PUBLIC_PATH)
cmake_path(GET PUBLIC_PATH PARENT_PATH WORKING_DIRECTORY)

add_custom_target(ZipWebViewFiles
    COMMAND ${CMAKE_COMMAND} -E tar cvf
        "${TARGET_WEBVIEW_FILES_ZIP_PATH}"
        --format=zip
        "${PUBLIC_PATH}"
    BYPRODUCTS "${TARGET_WEBVIEW_FILES_ZIP_PATH}"
    WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    COMMENT "Zipping WebView files..."
    VERBATIM
)

# DOC: 07-distribution.md # Define Zip Prefix
cmake_path(GET PUBLIC_PATH FILENAME ZIPPED_FILES_PREFIX)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ZIPPED_FILES_PREFIX="${ZIPPED_FILES_PREFIX}/"
)

# DOC: 07-distribution.md # Embed as Binary Data
juce_add_binary_data(WebViewFiles
    HEADER_NAME WebViewFiles.h
    NAMESPACE webview_files
    SOURCES ${TARGET_WEBVIEW_FILES_ZIP_PATH}
)
add_dependencies(WebViewFiles ZipWebViewFiles)

# DOC: 02-project-setup.md # Link Libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra  # Required for WebBrowserComponent
        WebViewFiles
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# DOC: 02-project-setup.md # Enable WebView
target_compile_definitions(${PROJECT_NAME} PUBLIC
    JUCE_WEB_BROWSER=1                                # Enable WebView
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1      # Windows: static link
)
