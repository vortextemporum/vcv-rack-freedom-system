# /install-plugin

**Purpose:** Build completed plugin in Release mode and install to system folders for DAW use.

---

## Syntax

```bash
/install-plugin [PluginName]
```

## What It Does

Compiles the plugin in optimized Release mode and installs it to macOS system plugin directories. Makes the plugin available in Logic Pro, Ableton Live, and other DAWs.

**This should ONLY be run after Stage 6 is complete.**

## Preconditions

Before running this command:
- ✅ Stage 6 (Validation & Presets) is complete
- ✅ Final commit has been approved
- ✅ pluginval validation passed
- ✅ Plugin tested in DAW from build folder

If any are false, complete Stage 6 first.

## How It Works

### Uses Centralized Build Script

Single command handles everything:

```bash
./scripts/build-and-install.sh [PluginName]
```

**This script automatically:**
1. Builds VST3 + AU in Release mode (parallel)
2. Extracts PRODUCT_NAME from CMakeLists.txt
3. Removes old versions from system folders
4. Installs new versions to:
   - VST3: `~/Library/Audio/Plug-Ins/VST3/`
   - AU: `~/Library/Audio/Plug-Ins/Components/`
5. Clears ALL DAW caches (Ableton + Logic)
6. Restarts AudioComponentRegistrar
7. Verifies installation with timestamp checks
8. Saves logs to `logs/[PluginName]/build_TIMESTAMP.log`

### Updates PLUGINS.md

After installation, updates the plugin entry:

```markdown
### ReverbPlugin

**Status:** 📦 Installed
**Installed:** 2025-11-07
**Formats:** VST3, AU
**Locations:**
- VST3: `~/Library/Audio/Plug-Ins/VST3/Reverb.vst3`
- AU: `~/Library/Audio/Plug-Ins/Components/Reverb.component`
```

## Success Output

```
✓ ReverbPlugin Installed Successfully

Build Mode: Release (optimized)

Installed Formats:
- ✓ VST3: ~/Library/Audio/Plug-Ins/VST3/Reverb.vst3 (2.4 MB)
- ✓ AU: ~/Library/Audio/Plug-Ins/Components/Reverb.component (2.4 MB)

Cache Status: Cleared (AudioComponentRegistrar killed, AU cache cleared)

Next Steps:
1. Open your DAW (Logic Pro, Ableton Live, etc.)
2. Rescan plugins (or restart DAW if it doesn't auto-detect)
3. Load Reverb in a project
4. Test audio processing and presets
5. Enjoy!

Logs:
- Build & Installation: logs/ReverbPlugin/build_2025-11-07_14-30-25.log
```

## Error Handling

**If build fails:**
```
Build failed at [stage]

Error: [exact error message]

Full log: logs/[PluginName]/build_TIMESTAMP.log

Should I investigate this error or wait for your input?
```

**If installation fails:**
```
Installation failed: [reason]

Check:
- File paths correct?
- Permissions OK?
- Disk space available?

Full log: logs/[PluginName]/build_TIMESTAMP.log
```

**If cache clear fails:**
```
Warning: Cache clear failed (not critical)

Plugin installed successfully, but DAW caches may be stale.
Manual fix: Restart your DAW to pick up new plugin.
```

## Why This Matters

**Release vs Debug builds:**
- Release: Optimized, smaller, faster (production use)
- Debug: Includes symbols, slower (development only)

**System folder installation:**
- DAWs scan system folders for plugins
- Without installation, plugin only works in build folder
- Clearing caches ensures DAW recognizes new version

**Overwrites old versions:**
- If plugin was already installed, this updates it
- Safe to run multiple times
- Old versions replaced completely

## Routes To

`plugin-lifecycle` skill handles the installation workflow.

## Example

```
User: /install-plugin ReverbPlugin
Assistant: Installing ReverbPlugin. Is this correct?
User: Yes
Assistant: [Builds Release mode]
          [Installs to system folders]
          [Clears caches]
          [Updates PLUGINS.md]

          ✓ ReverbPlugin installed successfully

          Installed formats:
          - VST3: ~/Library/Audio/Plug-Ins/VST3/Reverb.vst3
          - AU: ~/Library/Audio/Plug-Ins/Components/Reverb.component

          Open your DAW and test!
```

## Related Commands

- `/test [Plugin] build` - Build and validate (but don't install)
- `/improve [Plugin]` - Auto-installs after making changes
- `/show-standalone [Plugin]` - Open UI for quick visual check

## When To Use

**Use after:**
- Stage 6 completion
- Final validation passes
- Ready to use plugin in real projects

**Auto-invoked by:**
- `plugin-workflow` (after Stage 6)
- `plugin-improve` (after changes)

## Verification

Before declaring success:
- ✅ Both VST3 and AU files exist in system folders
- ✅ File sizes are reasonable (>100KB typically)
- ✅ PLUGINS.md updated with 📦 Installed status
- ✅ Logs saved
- ✅ No errors during any step

## Tips

**Test before installing:** Use Debug build in DAW from `build/` folder first.

**Cache clearing is essential:** DAWs cache plugin lists—clearing ensures they see new version.

**Check logs if issues:** Full build output saved to logs/ directory.

**Restart DAW if not detected:** Some DAWs need restart to recognize new plugins.

**Uninstall later:** Delete files from `~/Library/Audio/Plug-Ins/` to remove.

## Important Notes

- Release builds are optimized (smaller, faster, no debug symbols)
- Clearing caches makes DAWs recognize the plugin
- Overwrites any existing installation
- Some DAWs cache plugin lists—may need to rescan or restart
- Can uninstall by deleting files from `~/Library/Audio/Plug-Ins/`
