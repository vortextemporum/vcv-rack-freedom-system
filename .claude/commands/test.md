---
name: test
description: Validate modules through automated tests
---

# /test

When user runs `/test [ModuleName?] [mode?]`, invoke the plugin-testing skill or build-automation skill.

## Preconditions

<preconditions enforcement="blocking">
  <check target="PLUGINS.md" condition="module_exists">
    Module entry MUST exist in PLUGINS.md
  </check>
  <check target="PLUGINS.md::status" condition="not_equals(💡 Ideated)">
    Status MUST NOT be 💡 Ideated (requires implementation to test)
  </check>
  <rejection_message status="💡 Ideated">
[ModuleName] is not implemented yet (Status: 💡 Ideated).
Use /implement [ModuleName] to build it first.
  </rejection_message>
</preconditions>

<state_interactions>
  <reads>
    - PLUGINS.md (module status, test capabilities)
  </reads>
  <writes>
    - None (testing results logged to console, skills handle any state updates)
  </writes>
</state_interactions>

## Three Test Methods

**Mode: automated (plugin-testing skill)**
- **Requirement:** plugins/{ModuleName}/Tests/ directory exists
- **Invocation:** Invoke the plugin-testing skill via Skill tool
- **Duration:** ~2 minutes
- **Tests:** Parameter stability (all combinations, edge cases), state save/restore (preset corruption), processing stability (buffer sizes, sample rates), thread safety (concurrent access), edge case handling (silence, extremes, denormals)

**Mode: build (build-automation skill)**
- **Requirement:** Always available (no dependencies)
- **Invocation:** Invoke the build-automation skill via Skill tool
- **Duration:** 5-10 minutes
- **Steps:** Run make to build .vcvplugin, install to ~/Documents/Rack2/plugins/, verify module loads in VCV Rack

**Mode: manual (checklist only)**
- **Requirement:** Always available
- **Invocation:** Display checklist directly (no skill invocation)
- **Checklist items:** Load & initialize, audio processing, parameter testing, state management, performance, compatibility, stress testing

## Behavior

<behavior>
  <case arguments="none">
    List available modules with test status from PLUGINS.md
  </case>
  <case arguments="module_only">
    Present test method decision menu (adapt based on Tests/ directory existence)
  </case>
  <case arguments="plugin_and_mode">
    Execute test directly via appropriate skill
  </case>
</behavior>

## Integration

This command is auto-invoked by plugin-workflow after Stages 2 (DSP) and 3 (GUI) for automatic validation.

## Error Handling

<error_handling>
  <on_failure type="automated_tests|build|make (build validation)">
    Present standard failure menu:
    1. Investigate (trigger deep-research skill)
    2. Show me the code
    3. Show full output
    4. I'll fix it manually (or continue anyway for make (build validation))
  </on_failure>
</error_handling>

## Output

After successful testing, display:
- Success message with test method
- Next step suggestions (manual DAW testing, /improve if issues found)
