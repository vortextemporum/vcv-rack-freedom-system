## File System Design

### Complete Directory Structure

```
taches-claude-juce-system/
├── .claude/                          # Claude Code configuration
│   ├── CLAUDE.md                     # System navigation index
│   ├── settings.json                 # Hooks configuration
│   ├── agents/                       # Subagents
│   │   ├── foundation-agent.md
│   │   ├── shell-agent.md
│   │   ├── dsp-agent.md
│   │   ├── gui-agent.md
│   │   ├── verifier-agent.md
│   │   └── troubleshooter.md
│   ├── commands/                     # Slash commands
│   │   ├── dream.md
│   │   ├── implement.md
│   │   ├── improve.md
│   │   ├── test.md
│   │   ├── continue.md
│   │   ├── install-plugin.md
│   │   └── show-standalone.md
│   ├── ui-templates/                 # Reusable UI design templates
│   │   ├── vintage-hardware.yaml
│   │   ├── modern-minimal.yaml
│   │   ├── analog-modular.yaml
│   │   └── [user-custom-templates].yaml
│   ├── skills/                       # Workflow agents
│   │   ├── plugin-workflow/
│   │   │   ├── SKILL.md
│   │   │   └── stages/
│   │   │       ├── stage-0-research.md
│   │   │       ├── stage-1-planning.md
│   │   │       └── ...
│   │   ├── plugin-improve/
│   │   │   ├── SKILL.md
│   │   │   └── reference/
│   │   ├── plugin-ideation/
│   │   │   └── SKILL.md
│   │   ├── ui-mockup/
│   │   │   └── SKILL.md
│   │   ├── build-automation/
│   │   ├── plugin-lifecycle/
│   │   ├── plugin-testing/
│   │   ├── context-resume/
│   │   ├── design-sync/
│   │   ├── deep-research/
│   │   ├── juce-foundation/
│   │   └── troubleshooting-docs/
│   └── hooks/                        # Quality gates
│       ├── scripts/
│       │   ├── juce-validator.py
│       │   ├── parameter-consistency-check.py
│       │   ├── load-juce-context.sh
│       │   └── validators/
│       │       ├── foundation.py
│       │       ├── shell.py
│       │       ├── dsp.py
│       │       └── gui.py
│       └── hooks.json                # Hook configuration
│
├── plugins/                          # Per-plugin workspaces
│   └── [PluginName]/
│       ├── .ideas/                   # Pre-implementation (never deleted)
│       │   ├── creative-brief.md     # Vision (from plugin-ideation)
│       │   ├── parameter-spec.md     # Parameter contract (from ui-mockup finalization)
│       │   ├── architecture.md       # DSP contract (from Stage 0)
│       │   ├── improvements/         # Enhancement proposals
│       │   └── mockups/              # UI designs
│       │       ├── v1-ui.yaml        # Iteration 1 spec
│       │       ├── v1-ui-test.html   # Iteration 1 browser preview
│       │       ├── v2-ui.yaml        # Iteration 2 spec
│       │       ├── v2-ui-test.html   # Iteration 2 browser preview
│       │       ├── v3-ui.yaml        # FINALIZED spec
│       │       ├── v3-ui-test.html   # FINALIZED browser preview
│       │       ├── v3-ui.html        # Production UI (JUCE integrated)
│       │       ├── v3-editor.h       # C++ header template
│       │       ├── v3-editor.cpp     # C++ implementation template
│       │       ├── v3-cmake.txt      # Build config snippets
│       │       └── v3-implementation-steps.md  # Integration guide
│       ├── Source/                   # C++ implementation
│       │   ├── PluginProcessor.h
│       │   ├── PluginProcessor.cpp
│       │   ├── PluginEditor.h
│       │   └── PluginEditor.cpp
│       ├── ui/                       # WebView UI assets
│       │   └── public/
│       │       ├── index.html        # Copied from v3-ui.html
│       │       └── js/
│       │           └── juce/         # JUCE frontend library (auto-copied)
│       │               └── index.js
│       ├── CMakeLists.txt            # Build configuration
│       ├── plan.md                   # Stage-by-stage breakdown (from Stage 1)
│       ├── research.md               # Technical investigation (from Stage 0)
│       ├── CHANGELOG.md              # Version history (from Stage 6 or plugin-improve)
│       └── .continue-here.md         # Session state (temporary)
│
├── logs/                             # Build and validation logs (not git-tracked)
│   ├── [PluginName]/
│   │   ├── build_YYYYMMDD_HHMMSS.log
│   │   └── pluginval_YYYYMMDD_HHMMSS.log
│   └── system/
│       └── juce_version.txt
│
├── backups/                          # Version snapshots (not git-tracked)
│   └── [PluginName]/
│       └── v[X.Y.Z]/                 # Complete snapshot before improvement
│
├── scripts/                          # Automation scripts
│   ├── build-and-install.sh          # Build + install + verify (see Build Automation below)
│   └── ...
│
├── docs/                             # System documentation
│   ├── architecture.md               # THIS FILE
│   ├── ui-design-rules.md            # WebView UI constraints (global rules)
│   ├── skills/                       # Skill usage guides
│   ├── agents/                       # Agent documentation
│   ├── hooks/                        # Hook documentation
│   ├── troubleshooting/              # Problem solutions database
│   └── webview-implementation-guide.md  # WebView patterns
│
├── PLUGINS.md                        # Plugin registry (lifecycle state)
└── README.md                         # System overview
```

### Naming Conventions

**Directories:**
- Lowercase with hyphens: `plugin-workflow/`
- Plugin names: PascalCase: `ReverbPlugin/`

**Files:**
- Markdown: lowercase-with-hyphens: `creative-brief.md`
- Code: PascalCase: `PluginProcessor.cpp`
- Scripts: lowercase-with-hyphens: `build-and-install.sh`
- Mockups: versioned: `v1-ui.yaml`, `v2-ui-test.html`

**Special files:**
- `.continue-here.md` - Session state (dot prefix, temporary)
- `.ideas/` - Pre-implementation (dot prefix, never deleted)
- `PLUGINS.md` - Registry (uppercase, important)
- `CHANGELOG.md` - Version history (uppercase, standard)

### Version Control Strategy

**Git-tracked:**
```
plugins/[Name]/.ideas/              # Creative vision, contracts, mockups
plugins/[Name]/Source/              # Implementation
plugins/[Name]/ui/public/           # UI assets
plugins/[Name]/CMakeLists.txt       # Build config
plugins/[Name]/plan.md              # Implementation plan
plugins/[Name]/research.md          # Technical research
plugins/[Name]/CHANGELOG.md         # Version history
PLUGINS.md                          # Registry
.claude/                            # System config
scripts/                            # Automation
docs/                               # Documentation
```

**NOT git-tracked:**
```
plugins/[Name]/.continue-here.md    # Session state (temporary)
logs/                               # Build logs
backups/                            # Version snapshots
plugins/[Name]/build/               # CMake build directory
```

**.gitignore patterns:**
```gitignore
# Session state
.continue-here.md

# Build artifacts
build/
logs/
backups/

# OS
.DS_Store
```

### File Discovery Mechanism

**Skills discovered via:**
```bash
find .claude/skills -name "SKILL.md" | while read file; do
  parse_frontmatter "$file"
  register_skill
done
```

**Agents discovered via:**
```bash
find .claude/agents -name "*.md" | while read file; do
  parse_frontmatter "$file"
  register_agent
done
```

**Commands discovered via:**
```bash
find .claude/commands -name "*.md" | while read file; do
  parse_frontmatter "$file"
  register_command "/${name}"
done
```

**Hooks loaded via:**
```bash
if [ -f .claude/hooks/hooks.json ]; then
  load_hooks .claude/hooks/hooks.json
elif [ -f .claude/settings.json ]; then
  load_hooks .claude/settings.json
fi
```

---
