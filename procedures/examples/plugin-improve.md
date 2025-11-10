# plugin-improve Examples

## Example 1: Bug Fix (Patch Version)

**User:** "fix the filters in MicroGlitch"

**Detection:**
1. Read PLUGINS.md → MicroGlitch v1.0.0, Status 📦
2. Status is 📦 → Invoke plugin-improve
3. "fix" = bug fix → Patch version (1.0.1)

**Execution:**
1. Backup v1.0.0
2. Fix filter code
3. Build + validate
4. Install to system
5. Update CMakeLists.txt: VERSION 1.0.1
6. Update CHANGELOG.md:
   ```markdown
   ## [1.0.1] - 2025-11-06
   ### Fixed
   - HP/LP filter parameters now respond during playback
   ```
7. Update PLUGINS.md: Version 1.0.1
8. Commit: "improve: MicroGlitch v1.0.1 - fix filter parameters"
9. Tag: v1.0.1

## Example 2: New Feature (Minor Version)

**User:** "add a distortion parameter to MicroGlitch"

**Detection:**
1. Read PLUGINS.md → MicroGlitch v1.0.1, Status 📦
2. "add" = new feature → Minor version (1.1.0)

**Execution:**
1. Backup v1.0.1
2. Add distortion parameter to APVTS
3. Add distortion DSP to processBlock
4. Add distortion slider to GUI
5. Build + validate
6. Install to system
7. Update CMakeLists.txt: VERSION 1.1.0
8. Update CHANGELOG.md:
   ```markdown
   ## [1.1.0] - 2025-11-06
   ### Added
   - Distortion parameter (0-100%) for saturation control
   ```
9. Update PLUGINS.md: Version 1.1.0, add distortion to parameter list
10. Commit: "improve: MicroGlitch v1.1.0 - add distortion parameter"
11. Tag: v1.1.0

## Example 3: Iterative Changes

**User:** "improve the GUI in ReverbPlugin"

**Detection:**
1. Read PLUGINS.md → ReverbPlugin, Status 📦 (no version listed)
2. Assume v1.0.0, propose v1.1.0 (GUI improvement = minor)

**Execution:**
1. Backup v1.0.0
2. User: "make the knobs bigger"
3. I increase knob size
4. Build + validate
5. Show: "Ready to finalize? Or keep going?"
6. User: "also change the background color to blue"
7. I change background
8. Build + validate
9. Show: "Ready to finalize? Or keep going?"
10. User: "yes, finalize it"
11. Install to system
12. Update all version locations
13. Commit: "improve: ReverbPlugin v1.1.0 - GUI improvements"
14. Tag: v1.1.0
