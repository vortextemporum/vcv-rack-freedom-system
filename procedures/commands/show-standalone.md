# /show-standalone

**Purpose:** Open the plugin UI in Standalone mode for quick visual inspection during development.

---

## Syntax

```bash
/show-standalone [PluginName]
```

## What It Does

Opens the plugin's UI as a standalone macOS application. Provides immediate visual feedback without requiring a DAW. Perfect for rapid UI iteration during development.

## How It Works

### 1. Identifies Plugin

Uses plugin name from argument or infers from conversation context.

### 2. Checks for Standalone Build

Looks for:
```
build/plugins/[PluginName]/[PluginName]_artefacts/Debug/Standalone/[PluginName].app
```

If exists → launches it

If not exists → builds it first

### 3. Builds Standalone (if needed)

```bash
cmake --build build --target [PluginName]_Standalone
```

**Note:** Uses manual cmake (not build-and-install.sh). Standalone is a separate target from VST3/AU plugins.

### 4. Launches Application

```bash
open "build/plugins/[PluginName]/[PluginName]_artefacts/Debug/Standalone/[PluginName].app"
```

Window appears showing full plugin UI with working controls.

### 5. Reports Success

```
✓ MicroGlitch UI opened successfully

The plugin window should now be visible on your screen.
- Test all controls for proper layout and behavior
- Check parameter ranges and default values
- Verify any graphics or custom components render correctly
```

## Why Standalone?

**Advantages over loading in DAW:**
- Launches directly as macOS application
- No host application needed
- Immediate visual feedback
- Faster iteration cycle
- Built-in audio I/O for testing
- MIDI input/output for testing MIDI-controlled parameters

**Use cases:**
- UI development and iteration
- Visual design verification
- Quick parameter testing
- Layout debugging
- Component rendering checks

## Fast Iteration Workflow

For rapid UI development:

1. Keep Standalone app open
2. Make changes to PluginEditor.cpp
3. Run `/show-standalone [Plugin]` again
4. New build replaces running app
5. Close old window, new version opens

This gives you: **edit → build → test** in seconds.

## Troubleshooting

**App won't launch:**
```
Check build completed:
ls -la build/plugins/[Plugin]/[Plugin]_artefacts/Debug/Standalone/

Check Console.app for crash logs

Try clean rebuild:
cmake --build build --target [Plugin]_Standalone --clean-first
```

**Audio device errors:**
Normal on first launch—app is setting up audio I/O. Can be configured in app settings if needed.

**UI appears but is blank:**
```
Check paint() method in PluginEditor.cpp
Verify component initialization in constructor
Look for assertion failures in Xcode debugger
```

**Can't find .app file:**
```
Verify CMakeLists.txt includes Standalone format:
FORMATS VST3 AU Standalone
```

## Example

```
User: /show-standalone MicroGlitch
Assistant: [Checks for build - exists]
          [Launches application]

          ✓ MicroGlitch UI opened successfully

          The plugin window should be visible.
          Test controls and verify layout.
```

**With build required:**
```
User: /show-standalone NewPlugin
Assistant: [Checks for build - not found]

          Building Standalone target...
          [Runs cmake --build]
          ✓ Build complete

          [Launches application]
          ✓ NewPlugin UI opened successfully
```

## Fallback Options

**If Standalone fails, alternatives:**

### Option 1: AudioPluginHost
```bash
open "/Applications/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app"
```

Then manually:
1. Options → Edit the list of available plug-ins
2. Scan for plugins
3. Add plugin to audio graph

### Option 2: Build AudioPluginHost
```bash
cd /path/to/JUCE/extras/AudioPluginHost/Builds/MacOSX
xcodebuild -configuration Debug
```

### Option 3: Use a DAW
Load VST3 or AU from build folder:
- VST3: `build/plugins/[Plugin]/[Plugin]_artefacts/Debug/VST3/`
- AU: `build/plugins/[Plugin]/[Plugin]_artefacts/Debug/AU/`

## Related Commands

- `/test [Plugin]` - Full validation with pluginval
- `/install-plugin [Plugin]` - Install to system folders for DAW use
- `/improve [Plugin]` - Make changes after visual inspection

## When To Use

**Use /show-standalone when:**
- Developing UI (Stage 5)
- Quick visual checks
- Testing parameter layout
- Verifying component rendering
- Iterating on design

**Don't use when:**
- Need full DAW integration testing (use actual DAW)
- Testing audio routing (use DAW or AudioPluginHost)
- Need plugin format validation (use `/test`)

## Tips

**Keep it open:** Launch once, rebuild as needed—window closes and reopens with new version.

**Quick checks:** Fastest way to see UI changes without DAW overhead.

**Audio testing:** Standalone has built-in audio I/O—works for basic audio testing.

**MIDI testing:** Connect MIDI controller to test MIDI-learned parameters.

**Not a replacement:** Still test in actual DAW before considering done.

## Output

After successful launch:
- Plugin window visible on screen
- All controls functional
- Audio I/O available (can test processing)
- MIDI input available (can test MIDI control)
- Ready for visual inspection and interaction testing
