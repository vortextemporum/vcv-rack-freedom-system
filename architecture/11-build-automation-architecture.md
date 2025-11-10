## Build Automation Architecture

### build-and-install.sh Script

**Location:** `scripts/build-and-install.sh`

**Purpose:** Centralized, single-source-of-truth script for complete build-and-install workflow

**Architecture:** 7-phase pipeline with pre-flight validation and comprehensive verification

**Pipeline:**

```
1. Pre-flight Validation
   ├─> Plugin directory exists?
   ├─> CMakeLists.txt exists?
   └─> PRODUCT_NAME extractable?

2. Build (parallel VST3 + AU)
   └─> cmake --build --config Release --target VST3 --target AU

3. Extract PRODUCT_NAME
   └─> Parse CMakeLists.txt (handles space-separated names)

4. Remove Old Versions
   ├─> ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3
   └─> ~/Library/Audio/Plug-Ins/Components/[ProductName].component

5. Install New Versions
   ├─> Copy VST3 to system directory
   └─> Copy AU to system directory

6. Clear DAW Caches (critical for detecting updates)
   ├─> Ableton Plugin Scanner cache
   ├─> AudioUnitCache
   └─> Restart AudioComponentRegistrar

7. Verification
   ├─> Files exist in system directories?
   ├─> File sizes reasonable?
   └─> Timestamps recent (< 60 seconds)?

8. Logging
   └─> logs/[PluginName]/build_YYYYMMDD_HHMMSS.log
```

**Flags:**

**`--dry-run`** - Show what would happen without making changes
- Use for: Testing, verifying paths, checking removals
- Output: All commands displayed, none executed

**`--no-install`** - Build only, skip installation and cache clearing
- Use for: Stage 2 (Foundation) verification, build testing
- Stops after build verification, doesn't touch system folders

**Usage Examples:**

```bash
# Standard workflow (Stages 4-6)
./scripts/build-and-install.sh MicroGlitch

# Stage 2 (Foundation) - verify compilation only
./scripts/build-and-install.sh TestPlugin --no-install

# Test before installing
./scripts/build-and-install.sh MicroGlitch --dry-run
```

**Invoked By:**

Skills:
- build-automation (Stages 2-6 of plugin-workflow)
- plugin-improve (after changes)
- plugin-lifecycle (installation workflow)
- plugin-testing (build validation tests)

Commands:
- `/test [Plugin] build`
- `/install-plugin [Plugin]`

**Implementation Details:** See `procedures/scripts/build-and-install.md` for complete script specification and `procedures/skills/build-automation.md` for failure protocol and integration patterns.

**Why Centralized:**

v1 pain points (before script):
- Manual cmake commands prone to errors
- Forgot to clear caches → tested stale version
- VST3/AU out of sync
- PRODUCT_NAME mismatches caused confusion

v2 solution:
- Single source of truth for build workflow
- Consistent behavior across all skills
- Automatic cache clearing eliminates "testing old version" bugs
- Handles PRODUCT_NAME extraction automatically
- Ensures both formats always in sync

**Output Format:**

Success:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Build & Install: MicroGlitch
  Product Name: Micro Glitch
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🔨 Building MicroGlitch (VST3 + AU)...
✓ Build successful

📦 Verifying build artifacts...
✓ VST3 found
✓ AU found

🗑️  Removing old versions...
✓ Removed

📥 Installing new versions...
✓ VST3 installed
✓ AU installed

🧹 Clearing DAW caches...
✓ Ableton cache cleared
✓ AU cache cleared
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

🎹 Next step: Rescan plugins in your DAW
```

**Error Handling:**

```bash
# Plugin not found
❌ Error: Plugin directory not found: plugins/InvalidName
Available plugins: [list]

# Build fails
❌ Build failed. Check log: logs/[Plugin]/build_TIMESTAMP.log
[Exits immediately, full output in log]

# Build artifacts not found
❌ Error: VST3 not found at expected path
[Indicates PRODUCT_NAME mismatch]

# Installation warnings
⚠️  Warning: VST3 timestamp is 120 seconds old (expected < 60)
[Possible stale file]
```

**Technical Implementation:**

**Build system:** Uses **Ninja** generator (recommended for CLI automation)
- Faster than Make for incremental builds
- Better parallelization than Xcode for command-line use
- CMake configuration: `cmake -G Ninja -B build`
- Build command: `cmake --build build --config Release` (cmake detects Ninja automatically)

**Build parallelization:** Uses cmake parallel targets (VST3 + AU build simultaneously)
```bash
cmake --build build --config Release --parallel --target PluginName_VST3 --target PluginName_AU
```

**PRODUCT_NAME extraction:**
```bash
PRODUCT_NAME=$(grep "PRODUCT_NAME" CMakeLists.txt | sed 's/.*PRODUCT_NAME "\(.*\)".*/\1/')
```
Handles space-separated names (e.g., "808 Clap" vs directory "ClapPlugin808")

**Timestamp verification:** Files must be < 60 seconds old to ensure fresh installation

**Exit on error:** Uses `set -e` - any command failure stops immediately

**Build system fallback:** If Ninja not installed, falls back to Make (cmake default on macOS)

**Color-coded output:**
- 🔵 Blue: Section headers
- 🟢 Green: Success
- 🟡 Yellow: Warnings
- 🔴 Red: Errors

**Cache Clearing Rationale:**

DAWs cache plugin lists for faster scanning. Without cache clearing:
- DAWs don't detect new plugins
- Updated plugins appear as old version
- "Why isn't my change showing up?" debugging waste

What gets cleared:
- `~/Library/Caches/Ableton Plugin Scanner/*`
- `~/Library/Caches/AudioUnitCache/*`
- AudioComponentRegistrar process (forces macOS AU registry rebuild)

---

## Related Procedures

This architecture is implemented through:

- `procedures/scripts/build-and-install.md` - Complete build-and-install.sh script specification
- `procedures/skills/build-automation.md` - Build failure protocol (4-option system) and workflow integration
