# Distribution - Packaging WebView Plugins for Release

## Overview

Distributing a WebView plugin requires:
1. **Embedding web UI files** in the plugin binary
2. **Ensuring runtime dependencies** are installed on user machines
3. **Code signing and notarization** (platform-specific)
4. **Creating an installer** (optional but recommended)

This guide focuses on **WebView-specific concerns** (items 1 and 2). For general plugin distribution, consult JUCE and platform documentation.

---

## Part 1: Embedding Web UI Files

### The Problem

During development, you serve files from a dev server or file system:
```cpp
webView.goToURL("http://127.0.0.1:8080");  // Dev server
// OR
webView.goToURL("file:///path/to/ui/index.html");  // File system
```

**For production**, you need files **embedded in the plugin binary** so users don't need:
- A running web server
- Files on disk (which can be modified/deleted)
- Internet connection

### The Solution: Zip + Binary Data

1. **Zip all web UI files** using CMake
2. **Embed zip as JUCE binary data** in the binary
3. **Read from zip in resource provider** at runtime

---

## Step 1: Zip Web UI Files with CMake

**plugin/CMakeLists.txt**

```cmake
# Get absolute path to UI directory
get_filename_component(PUBLIC_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/public
    ABSOLUTE
)

# Create zip file in build directory using custom target
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
```

**This creates:** `build/webview_files.zip` containing all files from `plugin/ui/public/`

**Why custom target:**
- Ensures zip is rebuilt when source files change
- BYPRODUCTS tells CMake this file is generated
- VERBATIM ensures proper command escaping

**Verify zip contents:**
```bash
unzip -l build/webview_files.zip
# Should show: public/index.html, public/js/index.js, etc.
```

---

## Step 2: Embed Zip as Binary Data

**plugin/CMakeLists.txt** (continued)

```cmake
# Create binary data target from zip file
juce_add_binary_data(WebViewFiles
    HEADER_NAME WebViewFiles.h
    NAMESPACE webview_files
    SOURCES ${TARGET_WEBVIEW_FILES_ZIP_PATH}
)

# IMPORTANT: Ensure zip is created before binary data embedding
add_dependencies(WebViewFiles ZipWebViewFiles)

# Link binary data to plugin
target_link_libraries(${PROJECT_NAME} PRIVATE
    WebViewFiles
    # ... other libraries
)
```

**Critical:** The `add_dependencies()` line ensures the zip is created before attempting to embed it as binary data. Without this, the build may fail intermittently.

**This generates:**
- `build/plugin/juce_binarydata_WebViewFiles/WebViewFiles.h`
- `build/plugin/juce_binarydata_WebViewFiles/WebViewFiles.cpp`

**Generated header contains:**
```cpp
namespace webview_files {
    extern const char* webview_files_zip;       // Pointer to zip data
    extern const int webview_files_zipSize;     // Size in bytes
}
```

---

## Step 3: Define Zip File Prefix

Web files are stored with directory structure: `public/index.html`, `public/js/index.js`, etc.

The resource provider receives paths like `/index.html`, `/js/index.js` (no `public/` prefix).

**Solution:** Automatically derive the prefix from the path.

**plugin/CMakeLists.txt** (continued)

```cmake
# Automatically derive the prefix that will be stripped
cmake_path(GET PUBLIC_PATH FILENAME ZIPPED_FILES_PREFIX)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ZIPPED_FILES_PREFIX="${ZIPPED_FILES_PREFIX}/"
)
```

**Why automatic derivation:**
- More flexible - works regardless of directory name
- Safer - no hardcoded assumptions
- Matches the actual zip structure

**This extracts:**
- From `/path/to/plugin/ui/public` → extracts `"public"`
- Defines as `ZIPPED_FILES_PREFIX="public/"`

---

## Step 4: Implement Resource Provider

**PluginEditor.cpp**

```cpp
#include <WebViewFiles.h>  // Generated binary data header

namespace {
    // Helper: Convert InputStream to byte vector
    std::vector<std::byte> streamToVector(juce::InputStream& stream) {
        const auto sizeInBytes = static_cast<size_t>(stream.getTotalLength());
        std::vector<std::byte> result(sizeInBytes);
        stream.setPosition(0);
        [[maybe_unused]] const auto bytesRead = stream.read(result.data(), result.size());
        jassert(bytesRead == static_cast<ssize_t>(sizeInBytes));
        return result;
    }

    // Helper: Get MIME type from file extension
    const char* getMimeForExtension(const juce::String& extension) {
        static const std::unordered_map<juce::String, const char*> mimeMap = {
            {{"html"}, "text/html"},
            {{"js"}, "text/javascript"},
            {{"css"}, "text/css"},
            {{"json"}, "application/json"},
            {{"png"}, "image/png"},
            {{"jpg"}, "image/jpeg"},
            {{"svg"}, "image/svg+xml"},
            {{"woff2"}, "font/woff2"}
        };

        if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
            return it->second;

        jassertfalse;  // Unknown extension
        return "application/octet-stream";
    }

    // Read file from embedded zip
    std::vector<std::byte> getWebViewFileAsBytes(const juce::String& filepath) {
        // Create stream from binary data
        juce::MemoryInputStream zipStream{
            webview_files::webview_files_zip,
            webview_files::webview_files_zipSize,
            false  // Don't copy data
        };

        // Open zip file
        juce::ZipFile zipFile{zipStream};

        // Find entry with prefix (e.g., "public/index.html")
        if (auto* zipEntry = zipFile.getEntry(ZIPPED_FILES_PREFIX + filepath)) {
            const std::unique_ptr<juce::InputStream> entryStream{
                zipFile.createStreamForEntry(*zipEntry)
            };

            if (entryStream == nullptr) {
                jassertfalse;
                return {};
            }

            return streamToVector(*entryStream);
        }

        return {};  // File not found
    }
}

auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    // Parse requested resource
    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    // Handle dynamic resources (e.g., real-time audio data)
    if (resourceToRetrieve == "outputLevel.json") {
        juce::DynamicObject::Ptr levelData{new juce::DynamicObject{}};
        levelData->setProperty("left", processorRef.outputLevelLeft.load());

        const auto jsonString = juce::JSON::toString(levelData.get());
        juce::MemoryInputStream stream{
            jsonString.getCharPointer(),
            jsonString.getNumBytesAsUTF8(),
            false
        };

        return juce::WebBrowserComponent::Resource{
            streamToVector(stream),
            juce::String{"application/json"}
        };
    }

    // Handle static files from zip
    const auto resource = getWebViewFileAsBytes(resourceToRetrieve);
    if (!resource.empty()) {
        const auto extension = resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return Resource{std::move(resource), getMimeForExtension(extension)};
    }

    return std::nullopt;  // 404 Not Found
}
```

---

## Step 5: Switch from Dev Server to Resource Provider

**PluginEditor.cpp constructor**

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(/* ... options ... */)
{
    addAndMakeVisible(webView);

    // PRODUCTION: Serve from embedded files
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // DEVELOPMENT: Use dev server (comment out for release)
    // webView.goToURL("http://127.0.0.1:8080");

    setSize(800, 600);
}
```

---

## Part 2: Platform Runtime Dependencies

### Windows: WebView2 Runtime

**Requirement:** Users must have **Microsoft Edge WebView2 Runtime** installed.

**Options:**

1. **Evergreen Bootstrapper** (Recommended)
   - Small (~2 MB) installer that downloads latest runtime
   - Always gets latest version
   - Requires internet during first install

2. **Evergreen Standalone Installer**
   - Full runtime (~100 MB)
   - No internet needed
   - Can bundle with plugin installer

3. **Fixed Version Runtime**
   - Specific runtime version bundled with app
   - Largest size (~150 MB)
   - Complete control over version

**Implementation:**

Include installer with your plugin installer (NSIS, WiX, InnoSetup, etc.):

```nsis
; NSIS example
Section "WebView2 Runtime" SEC_WEBVIEW2
    ; Check if WebView2 already installed
    ReadRegStr $0 HKLM "SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}" "pv"
    ${If} $0 == ""
        ; Not installed - run bootstrapper
        ExecWait '"$INSTDIR\MicrosoftEdgeWebview2Setup.exe" /silent /install'
    ${EndIf}
SectionEnd
```

**Download bootstrapper:**
https://developer.microsoft.com/en-us/microsoft-edge/webview2/#download-section

**CMake linking** (already covered in setup):
```cmake
target_compile_definitions(YourPlugin PRIVATE
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
)
```

### macOS: No Extra Runtime Needed

**Built-in:** macOS 10.11+ includes WKWebView (WebKit).

**No action required** - WebView works out of the box.

### Linux: libwebkit2gtk Package

**Required:** `libwebkit2gtk-4.1-0` or `libwebkit2gtk-4.0-0`

**User installation:**
```bash
# Ubuntu/Debian
sudo apt-get install libwebkit2gtk-4.1-0

# Fedora
sudo dnf install webkit2gtk4.1

# Arch
sudo pacman -S webkit2gtk
```

**Installer approach:**
```bash
# In your .deb package post-install script
apt-get install -y libwebkit2gtk-4.1-0 || apt-get install -y libwebkit2gtk-4.0-0
```

### Android: No Extra Runtime Needed

**Built-in:** Android 5.0+ includes Android System WebView.

**No action required.**

---

## Part 3: Complete Build Configuration

### CMakeLists.txt (Complete Example)

```cmake
# plugin/CMakeLists.txt

# Define plugin
juce_add_plugin(YourPlugin
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE YRCM
    PLUGIN_CODE YPGN
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "YourPlugin"
    NEEDS_WEB_VIEW2 TRUE  # Windows only
)

# Source files
target_sources(YourPlugin PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
)

target_include_directories(YourPlugin PRIVATE include)

# Enable WebView
target_compile_definitions(YourPlugin PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    ZIPPED_FILES_PREFIX="public/"  # For reading from zip
)

# Link JUCE modules
target_link_libraries(YourPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# === WEB UI PACKAGING ===

# Get absolute path to UI directory
get_filename_component(PUBLIC_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/public
    ABSOLUTE
)

# Create zip of web UI files
set(WEBVIEW_FILES_ZIP_PATH ${CMAKE_BINARY_DIR}/webview_files.zip)

file(ARCHIVE_CREATE
    OUTPUT ${WEBVIEW_FILES_ZIP_PATH}
    PATHS ${PUBLIC_PATH}
    FORMAT zip
    VERBOSE
)

# Embed zip as binary data
juce_add_binary_data(WebViewFiles
    HEADER_NAME WebViewFiles.h
    NAMESPACE webview_files
    SOURCES ${WEBVIEW_FILES_ZIP_PATH}
)

# Link binary data to plugin
target_link_libraries(YourPlugin PRIVATE WebViewFiles)
```

---

## Part 4: Build and Test

### Build Release Version

```bash
# Configure for release
cmake --preset release  # Or: cmake -B release-build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build --preset release  # Or: cmake --build release-build --config Release
```

### Verify Embedded Files

```bash
# Extract plugin binary
cd release-build/plugin/YourPlugin_artefacts/Release/VST3/YourPlugin.vst3/Contents/MacOS/

# Check binary size (should be larger with embedded zip)
ls -lh YourPlugin

# Verify strings in binary
strings YourPlugin | grep "index.html"  # Should appear
```

### Test in DAW

1. **Install plugin** to system directory
2. **Launch DAW** (Ableton, Reaper, Logic, etc.)
3. **Load plugin** on a track
4. **Verify UI loads** - should show index.html content
5. **Check DevTools console** - no 404 errors
6. **Test all features** - parameters, visualization, etc.

---

## Part 5: Installer Creation

### macOS: Package + Installer

**Recommended tools:**
- **Packages:** http://s.sudre.free.fr/Software/Packages/about.html
- **pkgbuild/productbuild:** Command-line (built-in)

**Basic structure:**
```
YourPlugin.pkg
├── AU: ~/Library/Audio/Plug-Ins/Components/YourPlugin.component
└── VST3: ~/Library/Audio/Plug-Ins/VST3/YourPlugin.vst3
```

**Code signing:**
```bash
codesign --force --sign "Developer ID Application: Your Name" \
    YourPlugin.component

codesign --force --sign "Developer ID Application: Your Name" \
    YourPlugin.vst3
```

**Notarization:**
```bash
xcrun notarytool submit YourPlugin.pkg \
    --apple-id "your@email.com" \
    --password "app-specific-password" \
    --team-id "TEAM_ID"
```

### Windows: NSIS/InnoSetup Installer

**Example NSIS script:**
```nsis
; YourPlugin.nsi
!include "MUI2.nsh"

Name "YourPlugin"
OutFile "YourPlugin-Installer.exe"
InstallDir "$PROGRAMFILES64\Common Files\VST3"

Section "Plugin Files"
    SetOutPath "$INSTDIR"
    File /r "YourPlugin.vst3"
SectionEnd

Section "WebView2 Runtime"
    ReadRegStr $0 HKLM "SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}" "pv"
    ${If} $0 == ""
        ExecWait '"$TEMP\MicrosoftEdgeWebview2Setup.exe" /silent /install'
    ${EndIf}
SectionEnd
```

### Linux: .deb Package

**Basic structure:**
```
your-plugin_1.0.0_amd64.deb
└── usr/
    └── lib/
        └── vst3/
            └── YourPlugin.vst3/
```

**Post-install script:**
```bash
#!/bin/bash
# DEBIAN/postinst
apt-get install -y libwebkit2gtk-4.1-0 || apt-get install -y libwebkit2gtk-4.0-0
```

---

## Common Distribution Issues

### Issue: Blank WebView in Release Build

**Cause:** Forgot to switch from dev server to resource provider.

**Fix:** Ensure this line in production build:
```cpp
webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
// NOT: webView.goToURL("http://127.0.0.1:8080");
```

### Issue: 404 Errors for Static Files

**Cause:** `ZIPPED_FILES_PREFIX` mismatch, or files not in zip.

**Debug:**
1. Extract zip: `unzip -l build/webview_files.zip`
2. Verify paths: Should show `public/index.html`, not `index.html`
3. Check `ZIPPED_FILES_PREFIX` matches: `"public/"`

### Issue: WebView2 Missing Error (Windows)

**Error:** "WebView2 runtime not installed"

**Fix:** Bundle WebView2 bootstrapper with installer, or instruct users to download.

### Issue: Large Binary Size

**Cause:** Embedded zip with large assets (images, videos, libraries).

**Solutions:**
1. **Compress images** - use WebP instead of PNG
2. **Minify JavaScript** - use webpack/rollup/vite
3. **Tree-shake libraries** - only include used code
4. **External assets** - serve large files from web (requires internet)

---

## Best Practices

### 1. Test Both Dev and Production Modes

```cpp
#ifdef USE_DEV_SERVER
    webView.goToURL("http://127.0.0.1:8080");
#else
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif
```

### 2. Version Your Web UI

Embed version in HTML:
```html
<meta name="version" content="1.0.0">
```

Check in C++:
```cpp
webView.evaluateJavascript(R"(
    document.querySelector('meta[name="version"]').content
)", [](auto result) {
    if (result.getResult()) {
        DBG("Web UI version: " << result.getResult()->toString());
    }
});
```

### 3. Minimize Binary Size

- Use SVG instead of PNG/JPG where possible
- Minify/compress JavaScript with build tools
- Don't embed large libraries - load from CDN if internet available

### 4. Provide Offline Fallback

If using CDN libraries:
```html
<script src="https://cdn.jsdelivr.net/npm/plotly.js@2.0.0/dist/plotly.min.js"></script>
<script>
    // Fallback if CDN fails
    if (typeof Plotly === 'undefined') {
        document.write('<script src="js/plotly.min.js"><\/script>');
    }
</script>
```

### 5. Handle Missing Runtime Gracefully

```cpp
// Check if WebView is supported
if (!juce::WebBrowserComponent::areOptionsSupported(options)) {
    juce::AlertWindow::showMessageBox(
        juce::AlertWindow::WarningIcon,
        "WebView Not Supported",
        "Please install WebView2 runtime from microsoft.com");
    return;
}
```

---

## Summary Checklist

**Before Release:**

- [ ] Switch to resource provider in PluginEditor constructor
- [ ] Remove/comment dev server URL
- [ ] Test plugin loads in DAW (not just standalone)
- [ ] Verify all UI features work (parameters, visualization, etc.)
- [ ] Check DevTools console for errors
- [ ] Test on clean machine without dev tools installed
- [ ] Verify binary size is reasonable
- [ ] Code sign binaries (macOS/Windows)
- [ ] Create installer with runtime dependencies
- [ ] Test installer on multiple OS versions
- [ ] Write documentation for users

**Platform-Specific:**

**Windows:**
- [ ] Include WebView2 bootstrapper in installer
- [ ] Test on Windows 10 and 11
- [ ] Code sign .vst3 and .exe installer

**macOS:**
- [ ] Code sign .vst3 and .component
- [ ] Notarize installer package
- [ ] Test on Intel and Apple Silicon Macs

**Linux:**
- [ ] Include libwebkit2gtk in dependencies
- [ ] Test on Ubuntu, Fedora, Arch
- [ ] Provide install instructions for dependencies

---

## Additional Resources

- **JUCE Documentation:** https://docs.juce.com/
- **WebView2 Docs:** https://docs.microsoft.com/en-us/microsoft-edge/webview2/
- **WKWebView Docs:** https://developer.apple.com/documentation/webkit/wkwebview
- **Code Signing Guide:** https://developer.apple.com/support/code-signing/
- **Notarization Guide:** https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution

---

**You've completed the JUCE WebView documentation series!**

You now know how to:
1. Set up a JUCE WebView project
2. Communicate between C++ and JavaScript
3. Bind audio parameters to web UI controls
4. Visualize audio data in real-time
5. Use hot reloading for rapid development
6. Package and distribute your plugin

Build amazing audio plugins with modern web UIs!
