---
name: install-plugin
description: Install completed module to ~/Documents/Rack2/plugins/
argument-hint: <ModuleName>
---

# /install-plugin

<preconditions enforcement="blocking">
  <check target="PLUGINS.md" condition="status_equals" required="true">
    Module status MUST be ✅ Working (Stage 3 complete)
  </check>
  <check target="build" condition="exists" required="true">
    Module MUST have successful Release build
  </check>
  <check target="validation" condition="passed" required="true">
    make (build validation) MUST have passed
  </check>
  <check target="testing" condition="completed" required="true">
    Module MUST have been tested in VCV Rack from build folder
  </check>
  <on_failure action="block">
    Guide user to complete Stage 3 first - DO NOT proceed
  </on_failure>
</preconditions>

<routing>
  <invoke skill="plugin-lifecycle" with="$ARGUMENTS" required="true">
    Pass module name to plugin-lifecycle skill for installation
  </invoke>
</routing>

<state_contracts>
  <reads target="PLUGINS.md">
    Module entry with status ✅ Working
  </reads>
  <writes target="PLUGINS.md">
    Update status: ✅ Working → 📦 Installed
  </writes>
  <writes target="logs/[ModuleName]/build_TIMESTAMP.log">
    Build and installation log output
  </writes>
</state_contracts>
