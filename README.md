# Module Freedom System

An AI-assisted VCV Rack 2 module development system that enables conversational creation of professional modular synthesizer modules for VCV Rack. Design and build custom DSP modules through natural dialogue with Claude Code—no programming experience required.

**Originally Created by [TÂCHES](https://youtube.com/tachesteaches)**

**Forked and ported for VCV Rack Plugins by [BERK aka Princess Camel](https://berkozdemir.com/)**

**A [GLIX STUDIO](https://home.glix.studio/) experiment**

## Why This Exists

Traditional VCV Rack module development demands deep expertise in C++, DSP algorithms, SVG panel design, and the Rack API. This barrier keeps module creation restricted to experienced programmers, leaving musicians, modular enthusiasts, and sound designers dependent on existing modules.

The Module Freedom System removes that barrier entirely.

By enabling conversational module development, this system:

- **Democratizes creation**: Anyone with an idea can build it, regardless of technical background
- **Prioritizes creativity**: Focus on sonic goals and functionality, not implementation details
- **Accelerates iteration**: Ideas become working modules in hours, not weeks
- **Removes gatekeeping**: Opens modular synthesis development to the people who actually use these tools

## What You Can Build

- **Oscillators**: Analog-style VCOs, wavetable, FM, additive, granular
- **Filters**: SVF, ladder, formant, comb filters
- **Modulators**: LFOs, envelopes, sequencers, S&H, slew limiters
- **Effects**: Delays, reverbs, distortion, waveshapers, phasers
- **Utilities**: VCAs, mixers, mults, logic, quantizers, clock dividers
- **Sequencers**: Step sequencers, Euclidean patterns, generative systems
- **Experimental**: Custom DSP algorithms, hybrid processors, generative tools

All modules compile to native VCV Rack 2 format (.vcvplugin) for macOS, compatible with VCV Rack 2.6.x.

## How It Works

### 1. Dream (`/dream`)

Brainstorm your module concept through conversation:
- **Creative brief** - Vision, sonic goals, functionality
- **Parameter specification** - All controls, ports, ranges
- **Panel mockups** - Visual design and layout (SVG-based)

### 2. Plan (`/plan`)

Research and design the technical architecture:
- **DSP architecture** - Signal flow and processing strategy
- **Implementation plan** - Technical approach and complexity analysis

### 3. Implement (`/implement`)

Transform your specifications into a fully functional module through an automated workflow:

- **Build System Ready** (Stage 1): Plugin/module structure, Makefile, and all parameters implemented - validated automatically
- **DSP Engine Working** (Stage 2): Audio processing algorithms complete - validated automatically
- **Panel Integrated** (Stage 3): SVG panel with widgets and components - validated automatically
- After Stage 3 validation passes: Module complete, ready to install

### 4. Deploy & Iterate

- `/install-plugin` - Install to VCV Rack plugins folder
- `/test` - Run automated validation suite
- `/improve` - Add features or fix bugs (with regression testing)
- `/reconcile` - Reconcile state between planning and implementation

## Panel Design

Modules use SVG-based panels designed according to VCV Rack conventions:

- **Eurorack standards**: 1 HP = 5.08mm = 15px (at 75 DPI)
- **Standard heights**: 128.5mm = 380px panel height
- **Component layer**: Hidden layer with colored circles marking widget positions
- **Text-free graphics**: All text converted to paths for compatibility

### Component Markers

| Component Type | Circle Color | ID Format |
|---------------|--------------|-----------|
| Param (knob) | Red `#FF0000` | `param_NAME` |
| Input port | Green `#00FF00` | `input_NAME` |
| Output port | Blue `#0000FF` | `output_NAME` |
| Light | Magenta `#FF00FF` | `light_NAME` |

## Key Features

### Automated Build Pipeline

Build system handles SDK configuration, compilation, and installation. Uses the Rack SDK's Makefile system with proper architecture detection (Intel/Apple Silicon).

### Quality Assurance

- Automatic validation after each stage
- Build failure detection and troubleshooting
- Real-time safety verification (no allocations in process())
- Backup verification before destructive operations

### Knowledge Base

Dual-indexed troubleshooting database (`troubleshooting/`) captures solutions to build failures, runtime issues, and API misuse. The system learns from every problem encountered.

**Required Reading** (`vcv-rack-critical-patterns.md`) automatically prevents repeat mistakes by injecting proven patterns into all subagent contexts.

### Graduated Research Protocol

3-level investigation system (`/research`) for complex problems:

- **Quick**: Single-agent targeted investigation (1-2 min)
- **Moderate**: Multi-agent parallel search (3-5 min)
- **Deep**: Comprehensive multi-level analysis (5-10 min)

### Version Management

- Semantic versioning on improvements
- Git-based state tracking
- Safe rollback capabilities
- Backup verification before destructive operations

### Workflow Modes

- **Manual mode** (default): Present decision menus at each checkpoint for full control
- **Express mode**: Auto-progress through implementation stages without intermediate menus
- **Configurable**: Set preferences in `.claude/preferences.json` or use `--express`/`--manual` flags
- **Safe**: Express mode drops to manual on any error, ensuring oversight when needed

### Lifecycle Management

- `/install-plugin` - Deploy to VCV Rack plugins folder
- `/uninstall` - Remove plugin (keep source)
- `/reset-to-ideation` - Roll back to concept stage
- `/destroy` - Complete removal with verified backup
- `/clean` - Interactive cleanup menu

## System Architecture

### Contracts (Single Source of Truth)

Every module has immutable contracts in `plugins/[Name]/.ideas/`:

- `creative-brief.md` - Vision, sonic goals, functionality
- `parameter-spec.md` - Complete parameter definitions
- `architecture.md` - DSP design and signal flow
- `plan.md` - Implementation strategy
- `panel-mockups/` - SVG design references

**Zero drift**: All stages reference the same specs. No "telephone game" across workflows.

### Dispatcher Pattern

Each implementation stage runs in a fresh subagent context:

- `foundation-shell-agent` (Stage 1) - Plugin structure and parameter management
- `dsp-agent` (Stage 2) - Audio processing
- `panel-agent` (Stage 3) - SVG panel and widgets
- `validation-agent` (after each stage) - Automatic validation

**No context accumulation**: Clean separation prevents cross-contamination and keeps token usage minimal.

### Discovery Through Play

All features discoverable via:

- Slash command autocomplete (type `/` in Claude Code)
- Numbered decision menus at checkpoints
- Interactive skill prompts

**No manual required**: Learn by exploring, not reading docs.

### Checkpoint Protocol

At every completion point:

1. Auto-commit changes
2. Update state files (`.continue-here.md`, `PLUGINS.md`)
3. Present numbered decision menu
4. Wait for user response
5. Execute chosen action

**Never auto-proceeds**: You stay in control.

## Quick Start

### Prerequisites

- macOS (Catalina 10.15 or later)
- Claude Code CLI

All other dependencies (Xcode Command Line Tools, Rack SDK, etc.) can be validated and installed via `/setup`.

### First-Time Setup

```bash
# Validate and configure your system dependencies
/setup

# The setup wizard will:
# - Detect your platform and installed tools
# - Check for Rack SDK (download if missing)
# - Save configuration for build scripts
# - Generate a system report
```

### Create Your First Module

```bash
# 1. Dream the concept
/dream

# Brainstorm your module idea through conversation
# Creates: creative brief, parameter spec, panel mockups

# 2. Plan the architecture
/plan

# Research and design the technical implementation
# Creates: DSP architecture, implementation plan

# 3. Build it
/implement

# Automated workflow builds the module

# 4. Install and test
/install-plugin YourPluginName

# Module now available in VCV Rack
```

### Improve an Existing Module

```bash
# Fix bugs or add features
/improve MyPlugin

# Describe the change
# System handles versioning, testing, and rollback safety
```

### Resume Interrupted Work

```bash
# Pick up where you left off
/continue MyPlugin

# System loads checkpoint and presents next steps
```

## Complete Command Reference

### Setup

- `/setup` - Validate and configure system dependencies (first-time setup)
  - Detects platform, checks for required tools
  - Verifies Rack SDK installation
  - Saves configuration to `.claude/system-config.json`

### Development Workflow

- `/dream` - Brainstorm concept, create creative brief, parameter spec, and panel mockups
- `/plan` - Research and design DSP architecture and implementation strategy
- `/implement [Name]` - Build module through automated 3-stage workflow with continuous validation
- `/continue [Name]` - Resume paused workflow
- `/improve [Name]` - Modify completed module (with regression testing)

### Quality Assurance

- `/test [Name]` - Run automated validation suite
- `/research [topic]` - Deep investigation (3-level protocol)
- `/doc-fix` - Document solved problems for knowledge base
- `/add-critical-pattern` - Add current problem to Required Reading

### Deployment

- `/install-plugin [Name]` - Install to VCV Rack plugins folder
- `/uninstall [Name]` - Remove plugin (keep source)

### Lifecycle

- `/clean` - Interactive cleanup menu (uninstall/reset/destroy)
- `/reconcile [Name]` - Reconcile state between planning and implementation
- `/clear-cache [Name]` - Clear validation cache
- `/reset-to-ideation [Name]` - Remove implementation, keep idea/mockups
- `/destroy [Name]` - Complete removal (with verified backup)

## Project Structure

```
module-freedom-system/
├── plugins/                          # Plugin source code
│   └── [PluginName]/
│       ├── .ideas/                   # Contracts (immutable during impl)
│       │   ├── creative-brief.md
│       │   ├── parameter-spec.md
│       │   ├── architecture.md
│       │   ├── plan.md
│       │   └── panel-mockups/
│       ├── src/                      # C++ implementation
│       │   ├── plugin.cpp
│       │   ├── plugin.hpp
│       │   └── [ModuleName].cpp
│       ├── res/                      # SVG panels
│       │   └── [ModuleName].svg
│       ├── Makefile
│       └── plugin.json               # Plugin manifest
├── .claude/
│   ├── skills/                       # Specialized workflows
│   │   ├── plugin-workflow/          # Orchestrator (Build → DSP → Panel → Validation)
│   │   ├── plugin-planning/          # Research & design
│   │   ├── plugin-ideation/          # Concept brainstorming
│   │   ├── plugin-improve/           # Versioned modifications
│   │   ├── panel-mockup/             # SVG panel design system
│   │   ├── plugin-testing/           # Validation suite
│   │   ├── plugin-lifecycle/         # Install/uninstall/destroy
│   │   ├── deep-research/            # 3-level investigation
│   │   ├── troubleshooting-docs/     # Knowledge capture
│   │   └── workflow-reconciliation/  # State consistency checks
│   ├── agents/                       # Implementation subagents
│   │   ├── research-planning-agent/  # Research Complete (Stage 0)
│   │   ├── foundation-shell-agent/   # Build System Ready (Stage 1)
│   │   ├── dsp-agent/                # DSP Engine Working (Stage 2)
│   │   ├── panel-agent/              # Panel Integrated (Stage 3)
│   │   ├── validation-agent/         # Automatic validation
│   │   └── troubleshoot-agent/       # Build failures
│   ├── commands/                     # Slash command prompts
│   └── hooks/                        # Validation gates
├── scripts/
│   ├── build-and-install.sh          # Build pipeline
│   └── verify-backup.sh              # Backup integrity checks
├── troubleshooting/                  # Dual-indexed knowledge base
│   ├── build-failures/
│   ├── runtime-issues/
│   └── patterns/
│       └── vcv-rack-critical-patterns.md  # Required Reading
├── PLUGINS.md                        # Plugin registry
└── .continue-here.md                 # Active workflow state
```

## Voltage Standards

VCV Rack uses specific voltage conventions:

| Signal Type | Voltage Range | Notes |
|-------------|---------------|-------|
| **Audio** | ±5V | 10Vpp standard |
| **Unipolar CV** | 0V to +10V | LFOs, envelopes |
| **Bipolar CV** | ±5V | Modulation sources |
| **1V/octave** | Any voltage | 1V = 1 octave, 0V = C4 (261.6Hz) |
| **Gates** | 0V (off) / +10V (on) | Binary on/off |
| **Triggers** | +10V pulse, 1ms | Use `dsp::PulseGenerator` |

## Polyphony

VCV Rack supports up to **16 polyphonic channels** per cable. Modules can:
- Read channel count from inputs: `input.getChannels()`
- Set output channel count: `output.setChannels(n)`
- Use `getPolyVoltage(c)` for automatic mono-to-poly handling

## Philosophy

This system treats module development as a **creative conversation**, not a coding task.

You describe the sound, behavior, and appearance you want. The system handles the technical complexity—DSP implementation, parameter management, panel layout, build configuration, validation, deployment.

**Focus on what matters**: Creating tools that serve your music.

## Feedback Loop

The complete improvement cycle:

```
Build → Test → Find Issue → Research → Improve → Document → Validate → Deploy
    ↑                                                                      ↓
    └──────────────────────────────────────────────────────────────────────┘
```

- **deep-research** finds solutions (graduated 3-level protocol)
- **plugin-improve** applies changes (with regression testing)
- **troubleshooting-docs** captures knowledge (dual-indexed for fast lookup)
- **plugin-lifecycle** manages deployment (cache clearing, verification)
- **Required Reading** prevents repeat mistakes (auto-injected into subagents)

Every problem encountered becomes institutional knowledge. The system learns and improves over time.

## Implementation Status

- ✓ **Phase 0**: Foundation & Contracts
- ✓ **Phase 1**: Discovery System
- ✓ **Phase 2**: Workflow Engine
- ✓ **Phase 3**: Implementation Subagents
- ✓ **Phase 4**: Build & Troubleshooting System
- ✓ **Phase 5**: Validation System
- ✓ **Phase 6**: SVG Panel System
- ✓ **Phase 7**: Polish & Enhancement

**System status**: Production ready.

## Requirements

### Software

**Required:**
- macOS 10.15+ (Catalina or later)
- Claude Code CLI

**Dependencies (validated/installed via `/setup`):**
- Xcode Command Line Tools (`xcode-select --install`)
- VCV Rack SDK 2.x (from https://vcvrack.com/downloads/)
- Homebrew (for additional tools)
- Git

### Hardware

- Apple Silicon or Intel Mac
- 8GB RAM minimum (16GB recommended)
- 500MB free disk space per plugin

### Knowledge

- **Zero programming required**
- Basic understanding of modular synthesis concepts (CV, gates, audio signals)
- Ability to describe sonic goals and functionality preferences

## Contributing

This system is designed for personal use and learning. If you build something interesting with it, share your creative briefs and mockups—the process is the valuable part, not the compiled binaries.

## License

MIT - Use freely, modify as needed, share what you learn.

## Acknowledgments

Built with:

- [VCV Rack](https://vcvrack.com/) - Open-source virtual modular synthesizer
- [Claude Code](https://claude.com/claude-code) - AI-assisted development environment
- [Anthropic](https://anthropic.com/) - Claude AI models

---

**Start building**: `/dream`
