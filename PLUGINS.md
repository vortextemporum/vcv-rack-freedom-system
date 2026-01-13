# MODULE REGISTRY

## State Legend

- **💡 Ideated** - Creative brief exists, no implementation
- **💡 Ideated (Draft Params)** - Creative brief + draft parameters, ready for parallel workflow
- **🚧 Stage N** - In development (specific stage number)
- **✅ Working** - Completed all stages, not installed
- **📦 Installed** - Deployed to VCV Rack plugins folder
- **🐛 Has Issues** - Known problems (combines with other states)
- **🗑️ Archived** - Deprecated

## State Machine Rules

- If status is 🚧: ONLY plugin-workflow can modify (use `/continue` to resume)
- plugin-improve blocks if status is 🚧 (must complete workflow first)

## Build Management

- All plugin builds managed by `build-automation` skill
- Build logs: `logs/[PluginName]/build_TIMESTAMP.log`
- Installed plugins: `~/Documents/Rack2/plugins/` or `~/.Rack2/plugins/`
- Plugin format: `.vcvplugin` (zip containing plugin directory)

## Module Registry

| Plugin/Module | Status | Version | Type | Last Updated |
|---------------|--------|---------|------|--------------|
| Genesis/GenesisMono | 🚧 Stage 0 | - | Oscillator | 2026-01-13 |
| Genesis/GenesisPoly | 🚧 Stage 0 | - | Oscillator (Polyphonic) | 2026-01-13 |

**For detailed plugin information (lifecycle timeline, known issues, parameters, etc.), see:**
`plugins/[PluginName]/NOTES.md`

## Entry Template

When adding new plugins to this registry, use this format:

```markdown
| [PluginSlug]/[ModuleSlug] | [Emoji] [State] | [X.Y.Z or -] | [Type or -] | YYYY-MM-DD |
```

Create corresponding `plugins/[PluginSlug]/NOTES.md` with full details:

```markdown
# [PluginSlug] Notes

## Status
- **Current Status:** [emoji] [State Name]
- **Version:** [X.Y.Z or N/A]
- **Type:** [Module Type]

## Modules

| Module | Description | Tags |
|--------|-------------|------|
| [ModuleSlug] | [Description] | [Tags] |

## Lifecycle Timeline

- **YYYY-MM-DD:** [Event description]
- **YYYY-MM-DD (Stage N):** [Stage completion description]
- **YYYY-MM-DD (vX.Y.Z):** [Version release description]

## Known Issues

[Issue description or "None"]

## Additional Notes

[Any other relevant information - description, parameters, DSP, panel design, validation, installation locations, use cases, etc.]
```

## Module Type Tags

VCV Rack modules are categorized by tags in plugin.json:

**Sound Sources:**
- Oscillator, VCO, LFO, Clock, Random/Noise

**Sound Processors:**
- Filter, VCF, Equalizer, Distortion, Dynamics, Compressor
- Delay, Reverb, Flanger, Phaser, Chorus

**Control:**
- Envelope, VCA, Mixer, Utility, Attenuator, Sample and Hold

**Sequencing:**
- Sequencer, MIDI, Arpeggiator

**Other:**
- Polyphonic, Quad, Dual, Stereo, External, Recording, Visual
