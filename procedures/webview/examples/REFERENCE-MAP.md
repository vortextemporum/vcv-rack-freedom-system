# Code to Documentation Reference Map

This document maps specific code in the examples to their corresponding documentation sections.

## How to Use This Map

**Format:** `[Code File:Line]` → `[Doc Section]`

**Example:**
```
PluginEditor.cpp:108 → 02-project-setup.md # WebView Options
```

This means line 108 in PluginEditor.cpp implements a pattern explained in section "WebView Options" of 02-project-setup.md.

---

## Complete Example Code Map

### CMakeLists.txt

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 1-20 | Plugin definition | 02-project-setup.md # Step 3: Plugin CMakeLists.txt |
| 7-15 | juce_add_plugin with WebView flags | 02-project-setup.md # Plugin Definition |
| 18-19 | NEEDS_WEB_BROWSER, NEEDS_WEBVIEW2 | 02-project-setup.md # Windows WebView2 Setup |
| 22-27 | Compile-time product info | 03-communication-patterns.md # Method 4: Initialization Data |
| 54-78 | Zip web UI files | 07-distribution.md # Step 1: Zip Web UI Files |
| 80-90 | Embed as binary data | 07-distribution.md # Step 2: Embed Zip as Binary Data |
| 82 | ZIPPED_FILES_PREFIX | 07-distribution.md # Step 3: Define Zip File Prefix |
| 94-103 | Link libraries | 02-project-setup.md # Link JUCE Modules |
| 105-113 | JUCE_WEB_BROWSER, WebView2 flags | 02-project-setup.md # Enable WebView |

### ParameterIDs.hpp

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 6-8 | Parameter ID definitions | 04-parameter-binding.md # Step 1: Define Parameter ID |
| 6 | GAIN (float param) | 04-parameter-binding.md # Float Parameter |
| 7 | BYPASS (bool param) | 04-parameter-binding.md # Bool Parameter |
| 8 | DISTORTION_TYPE (choice param) | 04-parameter-binding.md # Choice Parameter |

### PluginProcessor.h

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 39-41 | Expose state getter | 02-project-setup.md # Public State Accessor |
| 43-46 | Expose parameter getter | 04-parameter-binding.md # Choice Parameter Setup |
| 48 | Atomic output level | 05-audio-visualization.md # Step 1: Measure Audio Level |
| 51-55 | Parameters struct | 04-parameter-binding.md # Parameter Management |
| 57-58 | createParameterLayout | 02-project-setup.md # Parameter Creation |
| 62-63 | Envelope follower | 05-audio-visualization.md # Envelope Following |

### PluginProcessor.cpp

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 20 | Initialize APVTS | 02-project-setup.md # AudioProcessorValueTreeState |
| 81-96 | prepareToPlay setup | 05-audio-visualization.md # Initialize Envelope Follower |
| 85-92 | Envelope follower config | 05-audio-visualization.md # Ballistics Filter Setup |
| 127-178 | processBlock | 05-audio-visualization.md # Measure Audio Data |
| 144-146 | Bypass check | 04-parameter-binding.md # Use Bool Parameter |
| 148-164 | Distortion processing | 04-parameter-binding.md # Use Choice Parameter |
| 149-156 | TanH distortion | Audio processing example |
| 157-163 | Sigmoid distortion | Audio processing example |
| 166 | Apply gain | 04-parameter-binding.md # Use Float Parameter |
| 168-177 | Envelope follower process | 05-audio-visualization.md # Process Envelope |
| 204-234 | createParameterLayout | 04-parameter-binding.md # All Parameter Types |
| 210-215 | Float parameter creation | 04-parameter-binding.md # Step 2: Create AudioParameterFloat |
| 217-223 | Bool parameter creation | 04-parameter-binding.md # Step 2: Create AudioParameterBool |
| 225-231 | Choice parameter creation | 04-parameter-binding.md # Step 2: Create AudioParameterChoice |

### PluginEditor.h

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 10-11 | Inherit from Timer | 05-audio-visualization.md # Step 2: Emit Periodic Events |
| 18 | timerCallback declaration | 05-audio-visualization.md # Timer Callback |
| 22 | getResource declaration | 03-communication-patterns.md # Resource Provider Pattern |
| 23-25 | nativeFunction declaration | 03-communication-patterns.md # Native Functions |
| 27-30 | C++ GUI components | Hybrid UI example |
| 34-42 | C++ parameter components | 04-parameter-binding.md # Traditional JUCE Pattern |
| 44-46 | WebView relays | 04-parameter-binding.md # Step 3: Create Relay |
| 48 | WebBrowserComponent | 02-project-setup.md # WebView Component |
| 50-52 | WebView parameter attachments | 04-parameter-binding.md # Step 4: Create Attachment |

### PluginEditor.cpp

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 15-24 | streamToVector helper | 07-distribution.md # Helper Functions |
| 26-48 | getMimeForExtension helper | 07-distribution.md # MIME Type Lookup |
| 50-53 | Event ID helper | 03-communication-patterns.md # Event Management |
| 55-58 | ZIPPED_FILES_PREFIX check | 07-distribution.md # Zip Prefix Validation |
| 67-86 | getWebViewFileAsBytes | 07-distribution.md # Step 4: Read from Zip |
| 88 | LOCAL_DEV_SERVER_ADDRESS | 06-development-workflow.md # Dev Server Setup |
| 91-156 | Constructor | 02-project-setup.md # Plugin Editor Implementation |
| 95-103 | C++ parameter attachments | 04-parameter-binding.md # Traditional Attachments |
| 104-106 | WebView relays init | 04-parameter-binding.md # Relay Initialization |
| 107-147 | WebView options | 02-project-setup.md # WebView Configuration |
| 108-116 | WebView2 backend config | 02-project-setup.md # Windows WebView2 Options |
| 117 | withNativeIntegrationEnabled | 03-communication-patterns.md # Native Integration |
| 118-123 | Resource provider + CORS | 06-development-workflow.md # Dev Server CORS |
| 124-126 | Initialization data | 03-communication-patterns.md # Method 4 |
| 127-128 | User script | 03-communication-patterns.md # Method 3 |
| 129-137 | Event listener (JS → C++) | 03-communication-patterns.md # Method 2 (JS→C++) |
| 138-144 | Native function | 03-communication-patterns.md # Method 1 (JS→C++) |
| 145-147 | withOptionsFrom relays | 04-parameter-binding.md # Pass Relay Options |
| 148-156 | WebView attachments init | 04-parameter-binding.md # Attachment Initialization |
| 163 | goToURL (resource provider) | 07-distribution.md # Production Mode |
| 166 | goToURL (dev server) commented | 06-development-workflow.md # Development Mode |
| 168-181 | evaluateJavascript example | 03-communication-patterns.md # Method 1 (C++→JS) |
| 184-188 | emitEvent example | 03-communication-patterns.md # Method 2 (C++→JS) |
| 192-204 | C++ GUI setup | Hybrid UI example |
| 209 | startTimer | 05-audio-visualization.md # Start Timer |
| 226-228 | timerCallback | 05-audio-visualization.md # Emit Audio Events |
| 230-255 | getResource | 03-communication-patterns.md # Resource Provider |
| 237-245 | Serve dynamic JSON | 05-audio-visualization.md # Step 3: Serve Audio Data |
| 247-252 | Serve static files from zip | 07-distribution.md # Serve Embedded Files |
| 257-269 | nativeFunction implementation | 03-communication-patterns.md # Native Function Handler |

### index.html

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 1-8 | Basic HTML structure | 02-project-setup.md # Web UI Files |
| 5 | Import Plotly CDN | 05-audio-visualization.md # Import Plotly |
| 6 | Import module script | 02-project-setup.md # JavaScript Import |
| 12-28 | Product info table | 03-communication-patterns.md # Display Init Data |
| 30-31 | Communication test buttons | 03-communication-patterns.md # UI Controls |
| 34 | Range input (slider) | 04-parameter-binding.md # HTML Slider |
| 36-37 | Checkbox | 04-parameter-binding.md # HTML Checkbox |
| 40-41 | Select element | 04-parameter-binding.md # HTML Select |
| 44 | Plotly plot div | 05-audio-visualization.md # Plotly Container |

### index.js

| Lines | Pattern | Documentation |
|-------|---------|---------------|
| 1 | Import JUCE library | 02-project-setup.md # JavaScript Import |
| 3 | Verify import | 06-development-workflow.md # Debugging |
| 5-10 | Listen for C++ event | 03-communication-patterns.md # C++→JS Event |
| 12-16 | Access initialization data | 03-communication-patterns.md # Initialization Data |
| 18 | Get native function | 03-communication-patterns.md # Get Native Function |
| 20-26 | Call native function | 03-communication-patterns.md # Call Native Function |
| 28-35 | Emit event to C++ | 03-communication-patterns.md # JS→C++ Event |
| 37-47 | Float parameter binding | 04-parameter-binding.md # JavaScript Slider Binding |
| 38 | getSliderState | 04-parameter-binding.md # Get Slider State |
| 39-41 | Slider → Parameter | 04-parameter-binding.md # User Input → Parameter |
| 43 | Set step from properties | 04-parameter-binding.md # Slider Properties |
| 45-47 | Parameter → Slider | 04-parameter-binding.md # Parameter → UI Update |
| 49-56 | Bool parameter binding | 04-parameter-binding.md # JavaScript Checkbox Binding |
| 50 | getToggleState | 04-parameter-binding.md # Get Toggle State |
| 51-53 | Checkbox → Parameter | 04-parameter-binding.md # Checkbox Input |
| 54-56 | Parameter → Checkbox | 04-parameter-binding.md # Toggle Update |
| 58-76 | Choice parameter binding | 04-parameter-binding.md # JavaScript ComboBox Binding |
| 61-63 | getComboBoxState | 04-parameter-binding.md # Get ComboBox State |
| 64-69 | Populate options | 04-parameter-binding.md # Populate Dropdown |
| 70-73 | Parameter → ComboBox | 04-parameter-binding.md # ComboBox Update |
| 74-76 | ComboBox → Parameter | 04-parameter-binding.md # ComboBox Input |
| 78-90 | Initialize Plotly plot | 05-audio-visualization.md # Create Plotly Chart |
| 92-119 | Audio visualization | 05-audio-visualization.md # Complete Pattern |
| 92 | Listen for audio update | 05-audio-visualization.md # Step 4: Listen for Events |
| 93 | Fetch audio data | 05-audio-visualization.md # Step 5: Fetch Data |
| 97-117 | Animate plot | 05-audio-visualization.md # Step 6: Update Visualization |

---

## Pattern Index

Quick lookup: "I want to do X" → "Look at this code"

### Communication Patterns

| Pattern | Code Location | Doc Reference |
|---------|---------------|---------------|
| **C++→JS: Evaluate JavaScript** | PluginEditor.cpp:168-181 | 03-communication-patterns.md # Method 1 |
| **C++→JS: Emit Event** | PluginEditor.cpp:184-188 | 03-communication-patterns.md # Method 2 |
| **C++→JS: User Script** | PluginEditor.cpp:127-128 | 03-communication-patterns.md # Method 3 |
| **C++→JS: Init Data** | PluginEditor.cpp:124-126, index.js:12-16 | 03-communication-patterns.md # Method 4 |
| **C++→JS: Resource Provider** | PluginEditor.cpp:230-255 | 03-communication-patterns.md # Resource Provider |
| **JS→C++: Native Function** | PluginEditor.cpp:138-144, index.js:18-26 | 03-communication-patterns.md # Method 1 |
| **JS→C++: Emit Event** | PluginEditor.cpp:129-137, index.js:28-35 | 03-communication-patterns.md # Method 2 |

### Parameter Binding

| Pattern | Code Location | Doc Reference |
|---------|---------------|---------------|
| **Float Parameter (Complete)** | ParameterIDs.hpp:6, PluginProcessor.cpp:210-215, PluginEditor.h:44+50, PluginEditor.cpp:104+148, index.js:37-47 | 04-parameter-binding.md # Float Parameter |
| **Bool Parameter (Complete)** | ParameterIDs.hpp:7, PluginProcessor.cpp:217-223, PluginEditor.h:45+51, PluginEditor.cpp:105+151, index.js:49-56 | 04-parameter-binding.md # Bool Parameter |
| **Choice Parameter (Complete)** | ParameterIDs.hpp:8, PluginProcessor.cpp:225-231, PluginEditor.h:46+52, PluginEditor.cpp:106+154, index.js:58-76 | 04-parameter-binding.md # Choice Parameter |
| **Relay Initialization** | PluginEditor.cpp:104-106 | 04-parameter-binding.md # Relay Pattern |
| **Attachment Initialization** | PluginEditor.cpp:148-156 | 04-parameter-binding.md # Attachment Pattern |

### Audio Visualization

| Pattern | Code Location | Doc Reference |
|---------|---------------|---------------|
| **Envelope Follower Setup** | PluginProcessor.h:62-63, PluginProcessor.cpp:85-92 | 05-audio-visualization.md # Envelope Following |
| **Measure Audio Level** | PluginProcessor.cpp:168-177 | 05-audio-visualization.md # Process Audio |
| **Emit Periodic Events** | PluginEditor.cpp:209, 226-228 | 05-audio-visualization.md # Timer Pattern |
| **Serve Dynamic JSON** | PluginEditor.cpp:237-245 | 05-audio-visualization.md # Serve Data |
| **Fetch and Visualize** | index.js:92-119 | 05-audio-visualization.md # JavaScript Visualization |

### Distribution

| Pattern | Code Location | Doc Reference |
|---------|---------------|---------------|
| **Zip Web Files** | CMakeLists.txt:60-78 | 07-distribution.md # Step 1 |
| **Embed as Binary Data** | CMakeLists.txt:84-90 | 07-distribution.md # Step 2 |
| **Read from Zip** | PluginEditor.cpp:67-86 | 07-distribution.md # Step 4 |
| **Serve Embedded Files** | PluginEditor.cpp:247-252 | 07-distribution.md # Serve Files |

### Development Workflow

| Pattern | Code Location | Doc Reference |
|---------|---------------|---------------|
| **Dev Server URL** | PluginEditor.cpp:88, 166 | 06-development-workflow.md # Dev Server |
| **CORS Allowance** | PluginEditor.cpp:123 | 06-development-workflow.md # CORS Fix |
| **Production URL** | PluginEditor.cpp:163 | 06-development-workflow.md # Production Mode |

---

## Cross-Pattern Examples

### Complete Float Parameter Flow

**Track parameter from C++ to JavaScript and back:**

1. **Define ID:** ParameterIDs.hpp:6
2. **Create parameter:** PluginProcessor.cpp:210-215
3. **Declare relay:** PluginEditor.h:44
4. **Declare attachment:** PluginEditor.h:50
5. **Initialize relay:** PluginEditor.cpp:104
6. **Pass relay to WebView:** PluginEditor.cpp:145
7. **Initialize attachment:** PluginEditor.cpp:148-150
8. **HTML element:** index.html:34
9. **Get state in JS:** index.js:38
10. **Bind UI→param:** index.js:39-41
11. **Bind param→UI:** index.js:45-47
12. **Use in processing:** PluginProcessor.cpp:166

### Complete Audio Visualization Flow

**Track audio data from processBlock to chart:**

1. **Declare atomic:** PluginProcessor.h:48
2. **Setup follower:** PluginProcessor.cpp:85-92
3. **Measure level:** PluginProcessor.cpp:168-177
4. **Store to atomic:** PluginProcessor.cpp:176-177
5. **Start timer:** PluginEditor.cpp:209
6. **Emit event:** PluginEditor.cpp:226-228
7. **Serve JSON:** PluginEditor.cpp:237-245
8. **Listen in JS:** index.js:92
9. **Fetch data:** index.js:93-96
10. **Animate chart:** index.js:97-117

---

## File-by-File Complete Reference

### All Documentation References by File

**ParameterIDs.hpp:**
- 04-parameter-binding.md (Step 1: Define Parameter ID)

**PluginProcessor.h:**
- 02-project-setup.md (Basic Plugin Structure)
- 04-parameter-binding.md (Parameters Struct Pattern)
- 05-audio-visualization.md (Atomic Variables, Envelope Follower)

**PluginProcessor.cpp:**
- 02-project-setup.md (APVTS Initialization)
- 04-parameter-binding.md (createParameterLayout, All Three Parameter Types)
- 05-audio-visualization.md (prepareToPlay, processBlock, Envelope Following)

**PluginEditor.h:**
- 02-project-setup.md (Editor Structure)
- 03-communication-patterns.md (Function Declarations)
- 04-parameter-binding.md (Relays, Attachments)
- 05-audio-visualization.md (Timer Inheritance)

**PluginEditor.cpp:**
- 02-project-setup.md (Constructor, Basic Setup)
- 03-communication-patterns.md (All 6 Communication Methods)
- 04-parameter-binding.md (Initialization Order, Relay/Attachment Setup)
- 05-audio-visualization.md (Timer, Resource Provider for JSON)
- 06-development-workflow.md (Dev Server vs Production)
- 07-distribution.md (Read from Zip, Serve Files)

**index.html:**
- 02-project-setup.md (Basic HTML Structure)
- 03-communication-patterns.md (Display Init Data)
- 04-parameter-binding.md (All Three Control Types)
- 05-audio-visualization.md (Plotly Container)

**index.js:**
- 02-project-setup.md (Import JUCE Library)
- 03-communication-patterns.md (All JavaScript Patterns)
- 04-parameter-binding.md (All Three Parameter Bindings)
- 05-audio-visualization.md (Fetch and Visualize)

**CMakeLists.txt:**
- 02-project-setup.md (Complete CMake Setup)
- 03-communication-patterns.md (Compile Definitions for Init Data)
- 07-distribution.md (Zip, Binary Data, Deployment)

---

## Usage Examples

### "I want to add a slider parameter"

**Code to copy:**
1. **ID:** ParameterIDs.hpp:6
2. **Create:** PluginProcessor.cpp:210-215
3. **C++ setup:** PluginEditor.h:44+50, PluginEditor.cpp:104+148
4. **HTML:** index.html:34
5. **JS:** index.js:37-47

**Docs to read:** 04-parameter-binding.md # Float Parameter

### "I want to display audio levels"

**Code to copy:**
1. **Measure:** PluginProcessor.h:48+62-63, PluginProcessor.cpp:85-92+168-177
2. **Emit:** PluginEditor.cpp:209+226-228
3. **Serve:** PluginEditor.cpp:237-245
4. **Visualize:** index.html:5+44, index.js:78-119

**Docs to read:** 05-audio-visualization.md

### "I want to package for release"

**Code to copy:**
1. **CMake:** CMakeLists.txt:60-90
2. **Read zip:** PluginEditor.cpp:67-86
3. **Serve files:** PluginEditor.cpp:247-252
4. **Switch URL:** PluginEditor.cpp:163 (uncomment)

**Docs to read:** 07-distribution.md

---

## Next Steps

- **Browse examples:** See `examples/complete-example/` and `examples/starter-template/`
- **Quick reference:** See `../QUICK-REFERENCE.md` for code snippets
- **Full docs:** See `../README.md` for complete documentation index
