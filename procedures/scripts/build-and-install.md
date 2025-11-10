# build-and-install.sh

**Location:** `scripts/build-and-install.sh`

**Purpose:** Centralized script for building JUCE plugins in Release mode and installing them to system plugin directories.

---

## What It Does

Single script that handles the complete build-and-install workflow:

1. **Builds** VST3 + AU in Release mode (parallel)
2. **Extracts** PRODUCT_NAME from CMakeLists.txt
3. **Removes** old versions from system folders
4. **Installs** new versions to system directories
5. **Clears** all DAW caches (Ableton + Logic)
6. **Restarts** AudioComponentRegistrar
7. **Verifies** installation with timestamp checks
8. **Logs** everything to `logs/[PluginName]/build_TIMESTAMP.log`

## Usage

```bash
./scripts/build-and-install.sh <PluginName> [flags]
```

## Flags

**`--dry-run`** - Show what would be done without making changes

Use when:
- Testing the script
- Verifying paths before actual installation
- Checking what would be removed/installed

Output shows all commands that would run without executing them.

**`--no-install`** - Build only, skip installation and cache clearing

Use when:
- Stage 2 (Foundation) - just need to verify it compiles
- Testing builds without touching system folders
- Debugging build issues

Stops after build verification, doesn't install or clear caches.

## Examples

**Standard workflow (Stages 4-6):**
```bash
./scripts/build-and-install.sh MicroGlitch
```

Builds, installs, clears caches, verifies—ready to test in DAW.

**Stage 2 (Foundation) - compile only:**
```bash
./scripts/build-and-install.sh TestPlugin --no-install
```

Verifies plugin compiles without installing anywhere.

**Test before installing:**
```bash
./scripts/build-and-install.sh MicroGlitch --dry-run
```

Shows exactly what would happen without making changes.

## What It Checks

**Pre-flight validation:**
- Plugin directory exists (`plugins/[PluginName]/`)
- CMakeLists.txt exists
- PRODUCT_NAME extractable from CMakeLists.txt

**Build artifacts:**
- VST3 exists at expected path
- AU exists at expected path

**Installation verification:**
- Files copied to system directories
- File sizes reasonable
- Timestamps are recent (< 60 seconds old)

## Paths

**Build artifacts:**
```
build/plugins/[PluginName]/[PluginName]_artefacts/Release/VST3/[ProductName].vst3
build/plugins/[PluginName]/[PluginName]_artefacts/Release/AU/[ProductName].component
```

**Installation targets:**
```
~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3
~/Library/Audio/Plug-Ins/Components/[ProductName].component
```

**Logs:**
```
logs/[PluginName]/build_YYYYMMDD_HHMMSS.log
```

## Cache Clearing

**Why this matters:**
DAWs cache plugin lists to speed up scanning. Without clearing caches, they may not detect new/updated plugins.

**What gets cleared:**
- `~/Library/Caches/Ableton Plugin Scanner/*` - Ableton's plugin database
- `~/Library/Caches/AudioUnitCache/*` - macOS AU cache
- AudioComponentRegistrar process killed - Forces macOS to rebuild AU registry

## Output

**Successful build and install:**
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Build & Install: MicroGlitch
  Product Name: Micro Glitch
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🔨 Building MicroGlitch (VST3 + AU)...
✓ Build successful

📦 Verifying build artifacts...
✓ VST3 found: build/plugins/MicroGlitch/...
✓ AU found: build/plugins/MicroGlitch/...

🗑️  Removing old versions...
  Removed old VST3
  Removed old AU

📥 Installing new versions...
✓ VST3 installed
✓ AU installed

🧹 Clearing DAW caches...
✓ Ableton Plugin Scanner cache cleared
✓ AudioUnitCache cleared
✓ AudioComponentRegistrar restarted

🔍 Verifying installation...
✓ VST3: 2.4M, modified 2025-11-07 14:30:25
✓ AU: 2.4M, modified 2025-11-07 14:30:25

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ MicroGlitch built and installed successfully
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Installation locations:
  VST3: ~/Library/Audio/Plug-Ins/VST3/Micro Glitch.vst3
  AU:   ~/Library/Audio/Plug-Ins/Components/Micro Glitch.component

Build log: logs/MicroGlitch/build_20251107_143025.log

🎹 Next step: Rescan plugins in your DAW (Logic/Ableton)
```

**With --dry-run:**
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Build & Install: MicroGlitch
  Product Name: Micro Glitch
  Mode: DRY RUN (no actual changes)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🔨 Building MicroGlitch (VST3 + AU)...
  [DRY RUN] Would run: cmake --build build --config Release --target MicroGlitch_VST3 --target MicroGlitch_AU

📦 Verifying build artifacts...
  [DRY RUN] Would verify:
    VST3: build/plugins/MicroGlitch/...
    AU: build/plugins/MicroGlitch/...

🗑️  Removing old versions...
  [DRY RUN] Would remove (if exists):
    VST3: ~/Library/Audio/Plug-Ins/VST3/Micro Glitch.vst3
    AU: ~/Library/Audio/Plug-Ins/Components/Micro Glitch.component

[...continues with dry run output...]

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Dry run complete - no changes made
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Run without --dry-run to actually build and install.
```

**With --no-install:**
```
[...build output...]

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Build complete (--no-install flag set)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Build artifacts:
  VST3: build/plugins/TestPlugin/TestPlugin_artefacts/Release/VST3/Test Plugin.vst3
  AU:   build/plugins/TestPlugin/TestPlugin_artefacts/Release/AU/Test Plugin.component

Build log: logs/TestPlugin/build_20251107_143025.log

Run without --no-install to install to system plugin directories.
```

## Error Handling

**Plugin not found:**
```
❌ Error: Plugin directory not found: plugins/InvalidName

Available plugins:
  MicroGlitch
  ClapPlugin808
  SimpleDelay
```

**Build fails:**
```
❌ Build failed. Check log: logs/[PluginName]/build_TIMESTAMP.log
```

Script exits immediately. Full build output saved to log file.

**Build artifacts not found:**
```
❌ Error: VST3 not found at expected path:
  build/plugins/[Plugin]/[Plugin]_artefacts/Release/VST3/[ProductName].vst3
```

Indicates build succeeded but artifacts in unexpected location—check CMakeLists.txt PRODUCT_NAME.

**Installation warnings:**
```
⚠️  Warning: VST3 timestamp is 120 seconds old (expected < 60)
```

File installed but timestamp suggests it may not be the fresh build—possible stale file.

## When Auto-Invoked

**By skills:**
- `build-automation` - Stages 2-6 of plugin-workflow
- `plugin-improve` - After making changes
- `plugin-lifecycle` - Installation workflow
- `plugin-testing` - Build validation tests

**By commands:**
- `/test [Plugin] build` - Build + pluginval validation
- `/install-plugin [Plugin]` - Final installation after Stage 6

## Why Centralized

**Before this script:**
- Manual cmake commands
- Forgot to clear caches → tested stale version
- VST3/AU out of sync
- PRODUCT_NAME mismatches caused confusion

**With this script:**
- Single source of truth for build workflow
- Consistent behavior across all skills
- No "testing old version" bugs
- Handles PRODUCT_NAME extraction automatically
- Ensures both formats always in sync

## Maintenance

**When to update:**
- New plugin formats added (VST, AAX)
- New DAWs requiring cache clearing
- Different macOS system paths
- Additional build validation needed

**Testing changes:**
Use `--dry-run` flag to verify script logic without making changes.

## Related

- `build-automation` skill - Invokes this script
- `/test [Plugin] build` command - Uses this for validation
- `/install-plugin` command - Uses this for installation
- `plugin-improve` skill - Auto-builds after changes

## Tips

**Always check logs:** Full build output saved even on success—useful for debugging.

**Use --dry-run first:** See exactly what would happen before making changes.

**Stage 2 uses --no-install:** Only verify compilation, don't install yet.

**Release mode is optimized:** Smaller files, faster performance, no debug symbols.

**Cache clearing is critical:** Without it, DAWs may not see updated plugin.

## Technical Details

**Build parallelization:**
Uses cmake parallel targets for VST3 + AU—both build simultaneously.

**PRODUCT_NAME extraction:**
```bash
PRODUCT_NAME=$(grep "PRODUCT_NAME" "plugins/$PLUGIN_NAME/CMakeLists.txt" | sed 's/.*PRODUCT_NAME "\(.*\)".*/\1/')
```

Handles cases where PRODUCT_NAME differs from directory name (e.g., "808 Clap" vs "ClapPlugin808").

**Timestamp verification:**
Checks files are < 60 seconds old to ensure fresh installation, not stale files.

**Exit on error:**
Script uses `set -e`—any command failure stops execution immediately.

**Color-coded output:**
- 🔵 Blue: Section headers
- 🟢 Green: Success
- 🟡 Yellow: Warnings/info
- 🔴 Red: Errors

## Output

After successful execution:
- ✅ Plugin built in Release mode
- ✅ Installed to system directories
- ✅ DAW caches cleared
- ✅ Ready to test in Logic/Ableton
- ✅ Full log saved for troubleshooting
