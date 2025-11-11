<objective>
Create a conversational, intelligent setup wizard for the Plugin Freedom System that guides Mac users through installing prerequisites and configuring paths for JUCE audio plugin development. This wizard runs once after Claude Code plugin installation and creates a `.plugin-freedom-config.json` configuration file.

The setup must be friendly, interactive, and handle both fresh installations and users with existing tools. It should detect what's already installed, offer to install missing tools, and configure paths for non-standard installations.
</objective>

<context>
This is for packaging the Plugin Freedom System as a Claude Code plugin for distribution. Users will install it via:
```bash
/plugin marketplace add lexchristopherson/plugin-freedom-system
/plugin install plugin-freedom-system
/setup  # This command you're creating
```

Target platform: macOS (initial release)
Users may be audio developers with varying technical backgrounds.
The system requires: JUCE framework, CMake, Xcode Command Line Tools, and optionally pluginval.

Current system references:
- Root CMakeLists.txt at `./CMakeLists.txt` (line 9 hardcodes `/Applications/JUCE`)
- Build script at `./scripts/build-and-install.sh`
</context>

<requirements>

## Core Functionality

1. **Detection Phase** - Check what's already installed:
   - JUCE framework (common locations: `/Applications/JUCE`, `~/JUCE`, `~/Developer/JUCE`)
   - CMake (`which cmake`)
   - Xcode Command Line Tools (`xcode-select -p`)
   - pluginval (optional, `which pluginval`)

2. **Conversational Installation** - For missing tools:
   - Explain what each tool does and why it's needed
   - Ask user if they want to install it
   - Provide installation commands or guidance
   - Verify installation succeeded before proceeding

3. **Path Configuration** - For existing or newly installed tools:
   - Detect standard locations automatically
   - If not found, ask user for custom path
   - Validate paths (check directories exist, binaries work)
   - Support non-standard JUCE installations

4. **Config File Creation**:
   - Write `.plugin-freedom-config.json` at repository root with:
     ```json
     {
       "juce_path": "/path/to/JUCE",
       "cmake_path": "/usr/local/bin/cmake",
       "pluginval_path": "/usr/local/bin/pluginval",
       "platform": "macos",
       "setup_date": "2025-01-11",
       "version": "1.0.0"
     }
     ```

5. **CMakeLists.txt Update**:
   - Modify root CMakeLists.txt to read from config file instead of hardcoded path
   - Keep backup of original file

</requirements>

<conversational_design>

## Interaction Pattern

The setup should feel like talking to a helpful colleague, not running a script.

**Good examples:**
- "I found CMake installed at `/usr/local/bin/cmake`. ✓"
- "JUCE isn't in the usual spots. Where did you install it? (Or I can guide you through installation)"
- "pluginval is optional but recommended for validating your plugins. Install it now? (y/n)"

**Avoid:**
- Robotic: "CMake detected: /usr/local/bin/cmake"
- Assuming knowledge: "Install via homebrew"
- No explanation: "Enter JUCE path:"

## Progressive Disclosure

Don't overwhelm with all questions at once. Flow:
1. Welcome message explaining what setup will do
2. Check one tool at a time
3. Handle issues before moving to next tool
4. Summary at the end showing what's configured

</conversational_design>

<implementation>

## Structure

Create `/setup` command at `.claude/commands/setup.md` that expands to this comprehensive setup prompt.

## Detection Logic

```bash
# JUCE detection priority
1. Check /Applications/JUCE
2. Check ~/JUCE
3. Check ~/Developer/JUCE
4. Search common development directories
5. Ask user for path
6. Offer installation guidance

# Validate JUCE path
- Directory must exist
- Must contain CMakeLists.txt or modules/ subdirectory
- Ideally check JUCE version compatibility (JUCE 7+)
```

## Installation Guidance

For each tool, provide platform-appropriate commands:

**CMake:**
```bash
brew install cmake
```

**Xcode Command Line Tools:**
```bash
xcode-select --install
```

**JUCE:**
- Download from https://juce.com/get-juce/download
- Or clone: `git clone https://github.com/juce-framework/JUCE.git ~/JUCE`

**pluginval:**
```bash
brew install pluginval
```
Or download from GitHub releases.

## Error Handling

- If installation fails, explain the error and suggest manual installation
- If path validation fails, re-prompt with helpful message
- If user cancels, save partial config and explain how to resume
- Never fail silently - always explain what went wrong and how to fix it

## CMakeLists.txt Update

Transform line 9 from:
```cmake
add_subdirectory(/Applications/JUCE JUCE)
```

To:
```cmake
# Read JUCE path from Plugin Freedom System config
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.plugin-freedom-config.json")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.plugin-freedom-config.json" PFS_CONFIG)
    string(JSON JUCE_PATH_FROM_CONFIG GET ${PFS_CONFIG} juce_path)
    if(EXISTS "${JUCE_PATH_FROM_CONFIG}")
        set(JUCE_PATH "${JUCE_PATH_FROM_CONFIG}")
    endif()
endif()

# Fallback to environment variable
if(NOT JUCE_PATH)
    set(JUCE_PATH $ENV{JUCE_PATH})
endif()

# Final validation
if(NOT JUCE_PATH OR NOT EXISTS "${JUCE_PATH}")
    message(FATAL_ERROR
        "JUCE framework not found. Run '/setup' command to configure paths.\n"
        "Or set JUCE_PATH environment variable.")
endif()

add_subdirectory(${JUCE_PATH} JUCE)
```

</implementation>

<tools_and_commands>

Use these tools during setup:

1. **Bash** - For detection and installation:
   - `which cmake`
   - `xcode-select -p`
   - `brew install ...`
   - Directory existence checks

2. **AskUserQuestion** - For interactive decisions:
   - "Install CMake via homebrew?"
   - "Enter custom JUCE path"
   - "Install optional pluginval?"

3. **Write** - For creating config file:
   - `.plugin-freedom-config.json`

4. **Edit** - For updating CMakeLists.txt:
   - Modify line 9 to read from config

5. **Read** - To verify existing files and validate paths

Invoke detection commands in parallel when possible for speed:
- Check cmake, xcode-select, and JUCE paths simultaneously
- Only install sequentially (user must confirm each)

</tools_and_commands>

<output>

After successful setup, the repository should have:

1. **`.plugin-freedom-config.json`** - Configuration file with validated paths
2. **`CMakeLists.txt`** - Updated to read from config (backup saved as `CMakeLists.txt.backup`)
3. **Terminal output** - Clear summary of what was configured

Final message should look like:
```
✓ Setup Complete!

Configured tools:
  • JUCE Framework: /Applications/JUCE (v7.0.9)
  • CMake: /usr/local/bin/cmake (v3.28.1)
  • Xcode Tools: /Applications/Xcode.app/Contents/Developer
  • pluginval: /usr/local/bin/pluginval

Configuration saved to .plugin-freedom-config.json

Ready to create plugins! Try:
  /dream              # Start ideating
  /implement MyPlugin # Build a plugin
```

</output>

<success_criteria>

Setup is successful when:

1. All required tools are installed or paths configured
2. `.plugin-freedom-config.json` exists with valid paths
3. CMakeLists.txt updated to read from config
4. Test command succeeds: `cmake --version` (proves CMake works)
5. JUCE path validation succeeds: directory exists and contains JUCE files
6. User receives clear confirmation message with next steps

</success_criteria>

<edge_cases>

Handle these scenarios gracefully:

1. **Homebrew not installed** - Offer to install it or provide manual installation instructions
2. **JUCE in unexpected location** - Accept custom path and validate it
3. **Multiple JUCE versions** - Let user choose which to use
4. **Partial previous setup** - Detect existing config and offer to update specific tools
5. **Permission issues** - Guide user to fix permissions or install in user directory
6. **Setup interruption** - Save progress so user can resume with `/setup --resume`
7. **Xcode already installed but CLI tools missing** - Detect and install just CLI tools

</edge_cases>

<verification>

Before declaring setup complete:

1. Run test commands to verify each tool works:
   ```bash
   cmake --version
   xcode-select -p
   ls "$JUCE_PATH/CMakeLists.txt"
   ```

2. Validate config file is valid JSON:
   ```bash
   cat .plugin-freedom-config.json | python3 -m json.tool
   ```

3. Test CMakeLists.txt change by running:
   ```bash
   cd /tmp && cmake /path/to/plugin-freedom-system -B build_test
   ```
   Should not error about missing JUCE path.

4. Confirm user sees success message and knows what to do next

</verification>

<why_this_matters>

**Why conversational?** Audio developers range from coding experts to musicians learning programming. Friendly guidance reduces friction and builds trust.

**Why validate paths?** A misconfigured path means the entire build system fails. Better to catch issues during setup than during plugin development.

**Why config file?** Environment variables require shell configuration knowledge. A config file is discoverable, version-controllable, and portable.

**Why optional pluginval?** It's valuable but not required for development. Users can add it later without breaking their workflow.

</why_this_matters>
