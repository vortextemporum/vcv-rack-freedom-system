# plugin-lifecycle

**Purpose:** Manage plugin installation to macOS system folders and clean removal workflows.

---

## What It Does

Handles installation of compiled plugins to system plugin directories and clean removal of deprecated plugins. Manages file operations that require specific macOS paths.

## When Invoked

**Automatically called by:**
- `plugin-workflow` Stage 6 (after validation complete)
- `plugin-improve` (after successful build)
- `build-automation` (as part of build-and-install)

**Manually triggered:**
- "Install [PluginName]"
- "Remove [PluginName]"
- "Uninstall [PluginName]"

## Installation Process

### Step 1: Build Verification

**Checks that plugin binaries exist:**
```
build/plugins/[PluginName]/[PluginName]_artefacts/Release/VST3/[ProductName].vst3
build/plugins/[PluginName]/[PluginName]_artefacts/Release/AU/[ProductName].component
```

**If missing:**
```
Plugin not built yet. Build first?
```

Offers to invoke `build-automation`

### Step 2: Product Name Extraction

**Reads CMakeLists.txt:**
```cmake
juce_add_plugin([PluginName]
    PRODUCT_NAME "808 Clap"  # ← Extracts this
    ...
)
```

**Uses product name** for installation filename, not directory name

**Example:**
- Directory: `ClapPlugin808`
- Product name: `"808 Clap"`
- Installs as: `808 Clap.vst3` and `808 Clap.component`

### Step 3: Old Version Removal

**Searches system folders:**
```bash
find ~/Library/Audio/Plug-Ins/VST3/ -name "[ProductName].vst3"
find ~/Library/Audio/Plug-Ins/Components/ -name "[ProductName].component"
```

**If found, removes:**
```
Found existing version: 808 Clap.vst3 (modified 2025-11-05)
Removing old version...
✓ Removed: ~/Library/Audio/Plug-Ins/VST3/808 Clap.vst3
```

**Purpose:** Clean install without old files

### Step 4: Copy to System Folders

**VST3 installation:**
```bash
cp -R build/.../[ProductName].vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**AU installation:**
```bash
cp -R build/.../[ProductName].component ~/Library/Audio/Plug-Ins/Components/
```

**Preserves bundles** (uses `cp -R` for .vst3 and .component bundles)

### Step 5: Permissions Verification

**Sets correct permissions:**
```bash
chmod -R 755 ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3
chmod -R 755 ~/Library/Audio/Plug-Ins/Components/[ProductName].component
```

**Ensures DAWs can load** the plugins

### Step 6: Cache Clearing

**Clears DAW caches:**

**Ableton Live:**
```bash
rm -f ~/Library/Preferences/Ableton/Live\ */PluginCache.db
```

**Logic Pro:**
```bash
rm -rf ~/Library/Caches/AudioUnitCache/*
```

**Purpose:** Forces DAWs to rescan and load fresh plugin version

### Step 7: Verification

**Confirms installation:**
```bash
ls -lh ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3
ls -lh ~/Library/Audio/Plug-Ins/Components/[ProductName].component
```

**Shows timestamps:**
```
✓ Installed VST3:
  ~/Library/Audio/Plug-Ins/VST3/808 Clap.vst3
  Modified: 2025-11-08 15:30:45
  Size: 34 MB

✓ Installed AU:
  ~/Library/Audio/Plug-Ins/Components/808 Clap.component
  Modified: 2025-11-08 15:30:45
  Size: 34 MB
```

**Confirms fresh installation** (timestamp matches current time)

## Removal Process

### Clean Uninstall

**Requested by:**
- "Remove [PluginName]"
- "Uninstall [PluginName]"

**Process:**

1. **Locate plugin files:**
   ```bash
   find ~/Library/Audio/Plug-Ins -name "[ProductName].*"
   ```

2. **Confirm removal:**
   ```
   Found:
   - ~/Library/Audio/Plug-Ins/VST3/DelayPlugin.vst3
   - ~/Library/Audio/Plug-Ins/Components/DelayPlugin.component

   Remove both? [y/N]
   ```

3. **Remove files:**
   ```bash
   rm -rf ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3
   rm -rf ~/Library/Audio/Plug-Ins/Components/[ProductName].component
   ```

4. **Clear caches:**
   Same cache clearing as installation

5. **Verify removal:**
   ```
   ✓ Removed DelayPlugin.vst3
   ✓ Removed DelayPlugin.component
   ✓ Caches cleared

   Plugin uninstalled.
   ```

**Does NOT remove:**
- Source files in `plugins/` directory
- Build artifacts
- Documentation

**Only removes** system-installed binaries

## System Folder Structure

**macOS plugin locations:**

```
~/Library/Audio/Plug-Ins/
├── VST3/
│   └── [ProductName].vst3           # VST3 bundle
├── Components/
│   └── [ProductName].component      # Audio Unit bundle
└── Presets/
    └── [Manufacturer]/[ProductName]/ # (optional) Factory presets
```

**Created automatically** by macOS if they don't exist

## Product Name Handling

### Why Product Name Matters

**CMakeLists.txt defines two names:**

```cmake
juce_add_plugin(ClapPlugin808        # ← Directory/target name
    PRODUCT_NAME "808 Clap"           # ← Display name in DAW
    COMPANY_NAME "Taches"
    ...
)
```

**Directory name:** Used for build system, file organization
**Product name:** Used for DAW display, installation filename

**Installation uses product name** so DAW shows friendly name

### Extraction Logic

**Reads CMakeLists.txt:**
```bash
grep "PRODUCT_NAME" plugins/[PluginName]/CMakeLists.txt | sed 's/.*PRODUCT_NAME "\(.*\)".*/\1/'
```

**Falls back to directory name** if PRODUCT_NAME not found

**Example outputs:**
- `PRODUCT_NAME "Vintage Delay"` → Installs as `Vintage Delay.vst3`
- No PRODUCT_NAME → Installs as `DelayPlugin.vst3`

## Error Handling

### Build Files Missing

```
✗ Error: Plugin binaries not found

Expected:
- build/plugins/DelayPlugin/DelayPlugin_artefacts/Release/VST3/DelayPlugin.vst3
- build/plugins/DelayPlugin/DelayPlugin_artefacts/Release/AU/DelayPlugin.component

Build the plugin first:
./scripts/build-and-install.sh DelayPlugin
```

### Permission Denied

```
✗ Error: Cannot copy to ~/Library/Audio/Plug-Ins/

This usually indicates permission issues.

Try:
sudo chmod -R u+w ~/Library/Audio/Plug-Ins/
```

(Rare on macOS user directories)

### Disk Space

```
✗ Error: Insufficient disk space

Required: 70 MB (VST3 + AU)
Available: 12 MB

Free up space and try again.
```

## Integration

**Called by:**
- `plugin-workflow` (final stage)
- `plugin-improve` (after changes)
- `build-automation` (part of build-and-install)
- Manual commands

**Calls:**
- File system operations (cp, rm, chmod)
- Cache clearing scripts

**Creates:**
- Plugin files in system folders
- (Removes files during uninstall)

## Platform Notes

### macOS Specifics

**Gatekeeper:**
Plugins must be signed for distribution. Development plugins work unsigned.

**Notarization:**
Not required for personal use. Required for distribution outside App Store.

**Hardened Runtime:**
Not required for audio plugins during development.

**User vs System:**
This skill installs to user folders (`~/Library`), not system folders (`/Library`). No sudo required.

### Bundle Structure

**VST3:**
```
PluginName.vst3/
└── Contents/
    └── MacOS/
        └── PluginName  # Actual binary
```

**AU:**
```
PluginName.component/
└── Contents/
    ├── MacOS/
    │   └── PluginName  # Actual binary
    └── Resources/
```

**Must preserve bundle structure** when copying

## Testing After Installation

**Verification steps:**

1. **File timestamps:**
   Check that installed files have current timestamp

2. **DAW rescan:**
   Open DAW, rescan plugins

3. **Load test:**
   Create new track, load plugin

4. **Parameter test:**
   Move controls, verify audio changes

5. **Save/load test:**
   Save project, reload, verify state

**If plugin doesn't appear in DAW:**
- Check cache was cleared
- Verify file permissions (755)
- Check DAW plugin paths
- Rescan plugins in DAW preferences

## Best Practices

**Always clear caches:** DAWs cache plugin info aggressively

**Verify timestamps:** Ensure you're testing new version, not cached old version

**Use product names:** Makes DAW plugin list more user-friendly

**Test both formats:** Some DAWs prefer VST3, others prefer AU

**Keep source separate:** Never delete source when uninstalling binaries

## Success Output

```
✓ Installation complete

VST3: ~/Library/Audio/Plug-Ins/VST3/808 Clap.vst3 (34 MB)
  Modified: 2025-11-08 15:30:45

AU: ~/Library/Audio/Plug-Ins/Components/808 Clap.component (34 MB)
  Modified: 2025-11-08 15:30:45

Cache cleared:
  Ableton Live: 3 cache files removed
  Logic Pro: 2 cache files removed

Plugin ready to use in DAWs.

Restart any open DAWs and rescan plugins.
```

Clear confirmation that installation succeeded.
