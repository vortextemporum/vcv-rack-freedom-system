# build-automation

**Purpose:** Automated build, validation, and installation with comprehensive failure handling protocol.

---

## What It Does

Handles all build operations for JUCE plugins with intelligent error handling. Never auto-retries on failure—always presents options for investigation.

## When Invoked

**Automatically called by:**
- `plugin-workflow` (after each stage)
- `plugin-improve` (after implementing changes)
- `plugin-lifecycle` (during installation)

**Can be called manually:**
- "Build [PluginName]"
- "Compile [PluginName]"

## Build Process

### Standard Build

```bash
./scripts/build-and-install.sh [PluginName]
```

**What happens:**
1. Configures CMake (if needed)
2. Builds VST3 and AU in parallel (Release mode)
3. Extracts PRODUCT_NAME from CMakeLists.txt
4. Removes old versions from system folders
5. Installs new versions to plugin directories
6. Clears Ableton and Logic DAW caches
7. Verifies installation with timestamp checks

### Build Options

**Compile only (no install):**
```bash
./scripts/build-and-install.sh [PluginName] --no-install
```

Used in Stage 2 (Foundation) to verify compilation without deploying.

**Dry run (see what would happen):**
```bash
./scripts/build-and-install.sh [PluginName] --dry-run
```

Shows planned actions without executing them.

**Verbose output:**
```bash
./scripts/build-and-install.sh [PluginName] --verbose
```

Shows full build output inline (normally suppressed).

## Failure Protocol

### The 4 Options

When a build fails, the system STOPS and presents:

```
Build failed. What would you like to do?

1. Investigate - Run deep-research to find the root cause
2. Show me the code - Display relevant source files
3. Show me the output - Full build log
4. Wait - I'll fix it manually, then say "resume automation"
```

**Critical:** Never auto-retries. Human decides next step.

### Option 1: Investigate
Routes to `deep-research` skill with:
- Error message
- Failed compilation unit
- Context from build log

Returns structured findings with solution options.

### Option 2: Show Code
Displays source files mentioned in error:
- Line numbers from error message
- Context around the problem
- Related files if needed

### Option 3: Show Output
Full build log with:
- All compiler warnings
- Error details
- Dependency information
- CMake configuration output

### Option 4: Wait
Human makes manual fixes, then:
```
"resume automation"
```

System continues from where it stopped.

## Logging

All builds are logged to `logs/[PluginName]/`:

```
logs/[PluginName]/
├── build_YYYYMMDD_HHMMSS.log    # Build output
└── pluginval_YYYYMMDD_HHMMSS.log # Validation output
```

Logs preserved for troubleshooting.

## Parallel Builds

VST3 and AU build simultaneously:
```bash
cmake --build build --target [PluginName]_VST3 --parallel &
cmake --build build --target [PluginName]_AU --parallel &
wait
```

Faster than sequential builds.

## Installation Process

### System Folders

**VST3:**
```
~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3
```

**AU:**
```
~/Library/Audio/Plug-Ins/Components/[ProductName].component
```

### Old Version Cleanup

Before installing new version:
1. Searches for existing plugin with same name
2. Removes old VST3
3. Removes old AU
4. Ensures clean install

### Cache Clearing

After installation, clears:

**Ableton Live:**
```
~/Library/Preferences/Ableton/Live */PluginCache.db
```

**Logic Pro:**
```
~/Library/Caches/AudioUnitCache/*
```

Ensures DAWs load fresh version.

### Verification

After installation:
```bash
ls -lh ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3
ls -lh ~/Library/Audio/Plug-Ins/Components/[ProductName].component
```

Displays timestamps to confirm installation.

## Debug Mode

Enable additional logging:
```bash
export JUCE_DEBUG=1
./scripts/build-and-install.sh [PluginName]
```

Adds debug symbols and verbose output.

## WebView Plugins

Special handling for WebView-based UIs:

1. Verifies `ui/public/` directory exists
2. Confirms `index.html` present
3. Checks JUCE JavaScript module (`js/juce/index.js`)
4. Zips directory into binary data
5. Embeds zip in plugin binary

Missing files cause clear error messages.

## Product Name Extraction

Reads CMakeLists.txt:
```cmake
juce_add_plugin([PluginName]
    PRODUCT_NAME "808 Clap"
    ...
)
```

Uses PRODUCT_NAME for file naming, not plugin directory name.

**Example:**
- Directory: `plugins/ClapPlugin808/`
- Product name: "808 Clap"
- Installed as: `808 Clap.vst3` and `808 Clap.component`

## Integration

**Called by:**
- `plugin-workflow` - After each stage
- `plugin-improve` - After changes
- `plugin-lifecycle` - During install/remove
- Manual commands

**Calls:**
- `deep-research` - When build fails and user chooses "Investigate"

**Creates:**
- Build logs in `logs/[PluginName]/`
- Installed plugins in system folders

## Error Categories

### Compilation Errors
- Syntax errors
- Type mismatches
- Missing includes
- Linker errors

**Typical fix:** Option 2 (Show code) → fix → resume

### Configuration Errors
- CMake setup issues
- Missing dependencies
- Path problems

**Typical fix:** Option 3 (Show output) → diagnose → fix → resume

### WebView Errors
- Missing ui/public/ files
- JUCE module not copied
- Zip creation failure

**Typical fix:** Clear error message guides fix → resume

## Best Practices

**Trust the protocol:** Don't try to auto-fix. Present options, let human decide.

**Log everything:** Build output always saved for later review.

**Clear errors:** Error messages should point to exact problem.

**Single responsibility:** This skill builds. Research is separate. Don't blur lines.

## Success Output

```
✓ Build successful
  VST3: ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3 (34 MB)
  AU:   ~/Library/Audio/Plug-Ins/Components/[ProductName].component (34 MB)

✓ Cache cleared:
  Ableton Live: 3 cache files removed
  Logic Pro: 2 cache files removed

Plugin ready to use.
```

## Failure Output

```
✗ Build failed (compilation error in PluginProcessor.cpp:45)

Error: 'juce::AudioParameterFloat' does not name a type

What would you like to do?
1. Investigate
2. Show me the code
3. Show me the output
4. Wait
```

Human chooses next step.
