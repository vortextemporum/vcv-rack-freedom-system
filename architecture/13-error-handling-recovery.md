## Error Handling & Recovery

### Error Taxonomy

Complete classification of failure modes and recovery procedures:

#### 1. Subagent Invocation Errors

**Cause:** Task tool failure, subagent file missing, context limit exceeded

**Symptoms:**
- Task tool returns error
- Subagent file not found
- "Context limit exceeded" message

**Recovery:**
```typescript
1. **Verify subagent exists** - Check .claude/agents/[name].md
2. **Check Task tool** - Verify Claude Code version supports Task tool
3. **Reduce context** - Truncate older messages, prioritize contracts
4. **Retry** - Temporary failures may succeed on retry
```

**Prevention:**
- Validate subagent existence before invocation
- Monitor context usage throughout workflow
- Design subagents for incremental context growth

#### 2. Subagent Report Parsing Errors

**Cause:** Malformed JSON, extra text, markdown formatting, partial output

**Symptoms:**
- JSON parse failure
- Missing required fields
- Invalid enum values

**Recovery:**
```typescript
1. **Extract from markdown** - Look for ```json blocks
2. **Brace matching** - Find first '{' and last '}'
3. **Partial data** - Use what's parseable, flag missing fields
4. **Retry subagent** - May have been interrupted mid-output
5. **Manual inspection** - Check what subagent actually did
```

**Prevention:**
- Subagents must wrap JSON in markdown code blocks
- Explicit schema validation in subagents before output
- Timeout handling to prevent partial outputs

#### 3. Build Failures

**Cause:** Syntax errors, missing headers, linker errors, CMake issues

**Symptoms:**
- Compile errors in build log
- Missing juce::juce_* modules
- Undefined symbols during linking

**Recovery:**
```bash
# 1. Check build log
cat logs/[Plugin]/build_[timestamp].log | grep "error:"

# 2. Common fixes
- Syntax error → Fix code at indicated line
- Missing header → Add JUCE module to CMakeLists.txt
- Linker error → Implement missing function or add source file

# 3. Retry build
./scripts/build-and-install.sh [Plugin] --no-install
```

**Prevention:**
- JUCE foundation rules enforced via hooks
- Syntax validation before build attempt
- CMake template ensures correct module linking

**PRODUCT_NAME Extraction Failure:**

**Scenario:** CMakeLists.txt has PRODUCT_NAME on multiple lines, commented out, or missing entirely.

**Fallback strategy:**
1. Try single-line regex extraction
2. Try multi-line extraction (PRODUCT_NAME after juce_add_plugin)
3. Use plugin directory name with warning

**Implementation:** `build-and-install.sh` extract_product_name() function handles all cases gracefully.

**Warning logged when fallback used:**
```
⚠️  Warning: Could not extract PRODUCT_NAME from CMakeLists.txt
⚠️  Using plugin directory name: PluginName
⚠️  This may cause issues if directory name contains spaces/special chars
```

#### 4. Validation Failures

**Cause:** Contract violations, incomplete implementation, code quality issues

**Symptoms:**
- Validator returns status="FAIL"
- Specific issues listed in validation report
- Mismatch between claimed and actual outputs

**Recovery:**
```typescript
User options:
1. **Fix issues** - Address validation findings
2. **Re-run stage** - Try implementation again
3. **Override** - Log justification, continue (for false positives)
4. **Report false positive** - Improve validator rules

Override logging format:
{
  "stage": "4-DSP",
  "validator_issue": "Flagged valid denormal protection pattern",
  "user_justification": "Using juce::FloatVectorOperations::disableDenormalisedNumberSupport()",
  "timestamp": "2025-11-09T14:30:25Z"
}
```

**Prevention:**
- Clear contract specifications
- Validator uses Sonnet for semantic understanding
- Regular validator calibration against false positives

#### 5. Git Commit Failures

**Cause:** Merge conflicts, uncommitted changes, detached HEAD

**Symptoms:**
- Git commit returns non-zero exit code
- "Cannot commit: files conflict" message
- Changes not appearing in git log

**Recovery:**
```bash
# 1. Check git status
git status

# 2. Resolve conflicts
git diff  # See conflicts
# Manually resolve in files
git add .

# 3. Retry commit
git commit -m "Stage N complete"
```

**Prevention:**
- Single-user workflow (v2.0)
- Atomic commits per stage
- No concurrent modifications

#### 6. Hook Script Failures

**Cause:** Python errors, timeout, missing dependencies, incorrect exit codes

**Symptoms:**
- Hook blocks tool execution
- Timeout after configured seconds
- Python traceback in hook output

**Recovery:**
```bash
# 1. Disable hooks temporarily
export CLAUDE_DISABLE_HOOKS=1

# 2. Test hook script directly
python .claude/hooks/scripts/juce-validator.py [args]

# 3. Fix script, re-enable
unset CLAUDE_DISABLE_HOOKS
```

**Prevention:**
- Hook timeout configuration per script
- Comprehensive error handling in hooks
- Dry-run mode for hook development

### Hook-Based Validation Strategy

The system uses hooks at critical points to enforce contracts and maintain workflow integrity. Six hook events provide layered validation:

#### 1. UserPromptSubmit - Auto-Context Injection

**Purpose:** Automatically inject relevant plugin context when user mentions a plugin, enabling seamless workflow resumption.

**Implementation:**
```bash
#!/bin/bash
# .claude/hooks/inject-plugin-context.sh

INPUT=$(cat)

# Extract plugin name from user prompt
PLUGIN_NAME=$(echo "$INPUT" | grep -oP 'continue\s+\K\w+Plugin')

if [ -z "$PLUGIN_NAME" ]; then
  exit 0  # No plugin mentioned, skip
fi

# Check if plugin exists
if [ ! -d "plugins/$PLUGIN_NAME" ]; then
  exit 0  # Plugin doesn't exist
fi

# Inject handoff and status into context
if [ -f "plugins/$PLUGIN_NAME/.ideas/handoff.md" ]; then
  echo "AUTO-INJECTED CONTEXT:"
  cat "plugins/$PLUGIN_NAME/.ideas/handoff.md"

  # Also inject current status line from PLUGINS.md
  grep "$PLUGIN_NAME" PLUGINS.md
fi

exit 0  # Never block, only augment
```

**Use case:** User says "continue DelayPlugin" → Hook automatically loads handoff.md showing current stage, recent commits, next steps → Claude receives full context without manual file loading.

#### 2. Stop - Stage Completion Enforcement

**Purpose:** Enforce "zero drift" guarantee by verifying stage commits actually happened before allowing workflow to continue.

**Implementation:**
```bash
#!/bin/bash
# .claude/hooks/verify-stage-commit.sh

INPUT=$(cat)

# Check if hook is relevant
if [ ! -f ".claude/handoff.md" ]; then
  exit 0  # No active workflow
fi

# Check if we're in implementation stages
CURRENT_STAGE=$(grep "current_stage:" .claude/handoff.md | cut -d: -f2 | xargs)
if [[ ! "$CURRENT_STAGE" =~ ^[2-6]$ ]]; then
  exit 0  # Not in implementation stages
fi

# Verify git commit exists
LAST_COMMIT=$(git log -1 --oneline --since="5 minutes ago")
if [[ "$LAST_COMMIT" =~ "Stage $CURRENT_STAGE" ]]; then
  exit 0  # Valid commit found
else
  echo "ERROR: Stage $CURRENT_STAGE incomplete - no git commit found" >&2
  echo "Required: git commit -m 'Stage $CURRENT_STAGE complete: [description]'" >&2
  exit 2  # Block Claude from continuing
fi
```

**Use case:** Stage 3 (shell-agent subagent) claims completion → Stop hook checks git log → If no commit with "Stage 3", blocks and forces commit → Guarantees state consistency.

#### 3. SubagentStop - Validate Against Task AND Contract

**Purpose:** Validate subagent output against both its task prompt AND relevant contracts before allowing workflow to continue.

**Implementation:**
```bash
#!/bin/bash
# .claude/hooks/validate-subagent.sh

INPUT=$(cat)

# Parse subagent name from input
SUBAGENT=$(echo "$INPUT" | jq -r '.subagent_name')

if [ -z "$SUBAGENT" ]; then
  exit 0  # Not a subagent completion
fi

# Stage-specific validation
case "$SUBAGENT" in
  foundation-agent)
    # Stage 2: Build system must exist and build
    [ -f "CMakeLists.txt" ] || { echo "Missing CMakeLists.txt" >&2; exit 2; }
    cmake --build build --config Release || { echo "Build failed" >&2; exit 2; }
    ;;

  shell-agent)
    # Stage 3: Parameters must match parameter-spec.md
    python3 .claude/hooks/validators/validate-parameters.py || exit 2
    ;;

  dsp-agent)
    # Stage 4: DSP components from architecture.md must be present
    python3 .claude/hooks/validators/validate-dsp-components.py || exit 2
    ;;

  gui-agent)
    # Stage 5: UI bindings must match parameter IDs
    python3 .claude/hooks/validators/validate-gui-bindings.py || exit 2
    ;;

  *)
    exit 0  # Not a workflow subagent
    ;;
esac

exit 0  # All validations passed
```

**Use case:** foundation-agent subagent finishes Stage 2 → SubagentStop hook runs → Checks CMakeLists.txt exists → Attempts build → If build fails, blocks with error (exit 2) → Subagent must fix and retry.

#### 4. PreCompact - Preserve Contracts During Compaction

**Purpose:** Preserve contracts verbatim during context compaction in long sessions (40-80 min workflows).

**Implementation:**
```bash
#!/bin/bash
# .claude/hooks/preserve-contracts.sh

INPUT=$(cat)

# List critical files to preserve during compaction
cat <<EOF
PRESERVE_EXACT:
- plugins/*/\.ideas/creative-brief\.md
- plugins/*/\.ideas/parameter-spec\.md
- plugins/*/\.ideas/architecture\.md
- \.claude/handoff\.md
- PLUGINS\.md

REASON: These are contractual documents that must remain byte-for-byte identical.
Summarizing them would cause drift between design and implementation.
EOF

exit 0  # Always succeeds, provides guidance to compaction
```

**Use case:** Context window fills after 60 minutes of Stage 4 work → PreCompact hook tells compaction system "keep parameter-spec.md verbatim" → Compaction summarizes conversation but preserves all contracts → No drift in long sessions.

#### 5. PostToolUse - JUCE Best Practices Validation

**Purpose:** Validate code edits against JUCE best practices after Edit/Write operations.

**Implementation:**
```python
#!/usr/bin/env python3
# .claude/hooks/scripts/juce-validator.py

import json
import sys
import re

def validate_juce_code(file_path, content):
    violations = []

    # Check for allocations in processBlock
    if 'processBlock' in content:
        if re.search(r'(new|malloc|std::vector|std::string)\s*\(', content):
            violations.append("Memory allocation detected in processBlock()")

    # Check for blocking operations
    if re.search(r'(sleep|mutex\.lock|std::this_thread::sleep)', content):
        violations.append("Blocking operation in real-time context")

    # Check for denormal protection
    if 'processBlock' in content and 'FloatVectorOperations' not in content:
        violations.append("Missing denormal protection pattern")

    return violations

# Read hook input
input_data = json.load(sys.stdin)
tool_input = input_data.get('tool_input', {})

if tool_input.get('tool_name') not in ['Edit', 'Write']:
    sys.exit(0)  # Only validate edits/writes

file_path = tool_input.get('file_path', '')
if not file_path.endswith('.cpp'):
    sys.exit(0)  # Only validate C++ files

content = tool_input.get('content', '')
violations = validate_juce_code(file_path, content)

if violations:
    print(f"JUCE best practices violations in {file_path}:", file=sys.stderr)
    for v in violations:
        print(f"  - {v}", file=sys.stderr)
    sys.exit(2)  # Block

sys.exit(0)  # Pass
```

**Use case:** dsp-agent subagent writes processBlock() code with `new float[]` → PostToolUse hook catches allocation → Blocks with error → Subagent must use pre-allocated buffers instead.

#### 6. SessionStart - Environment Validation

**Purpose:** Verify development environment is properly configured before workflow begins.

**Implementation:**
```bash
#!/bin/bash
# .claude/hooks/verify-environment.sh

# Check JUCE installation
if [ ! -d "$HOME/JUCE" ]; then
  echo "Warning: JUCE not found at $HOME/JUCE" >&2
  echo "Some features may not work correctly" >&2
fi

# Check CMake available
if ! command -v cmake &> /dev/null; then
  echo "ERROR: cmake not found in PATH" >&2
  exit 2  # Block - CMake is required
fi

# Check git configured
if ! git config user.name &> /dev/null; then
  echo "ERROR: git not configured (run: git config --global user.name 'Your Name')" >&2
  exit 2  # Block - git required for stage commits
fi

exit 0  # Environment valid
```

**Use case:** User starts new session → SessionStart hook validates environment → If CMake missing, blocks with error before any work begins → Prevents late-stage build failures.

### Conditional Hook Execution

**Critical pattern:** Hooks must only fire when relevant. Don't validate stage commits when user just asks "what's in this file?"

**Template pattern for all hooks:**

```bash
#!/bin/bash

INPUT=$(cat)

# 1. Parse input to understand context
# ...

# 2. Check if hook is relevant
if [ ! <condition indicating hook should run> ]; then
  exit 0  # Gracefully skip
fi

# 3. Do validation work
# ...

# 4. Exit appropriately
exit 0  # Pass - allow continuation
exit 2  # Block - show error to Claude, force fix
```

**Example scenarios:**

- **Stop hook** skips when no .claude/handoff.md exists (no active workflow)
- **SubagentStop hook** skips when subagent name isn't workflow-related
- **PostToolUse hook** skips when edited file isn't C++ code
- **UserPromptSubmit hook** skips when no plugin name mentioned

This prevents hooks from inappropriately blocking normal operations while maintaining strict enforcement during workflows.

#### 7. Context Limit Exceeded

**Cause:** Large contracts, verbose code, extensive message history

**Symptoms:**
- "Context limit exceeded" from Claude API
- Subagent truncates mid-response
- Incomplete JSON reports

**Recovery:**
```typescript
1. **Truncate history** - Keep only recent N messages
2. **Summarize contracts** - Extract key sections only
3. **Split stage** - Break into smaller phases
4. **Use Opus** - Higher context limit (200k vs 100k)
```

**Prevention:**
- Incremental context management
- Contract size limits (architecture.md < 50KB)
- Subagent context budgeting

#### 8. Timeout Errors

**Cause:** Long-running subagents, complex DSP, large codebase analysis

**Symptoms:**
- Subagent timeout after configured minutes
- Partial work completed
- No report returned

**Recovery:**
```typescript
User options:
1. **Extend timeout** - Add 30 minutes, retry
2. **Show progress** - Inspect partial work in filesystem
3. **Cancel** - Investigate why so slow
4. **Split work** - Break stage into smaller phases
```

**Prevention:**
- Configurable timeouts per subagent
- Progress indicators (subagents log phases completed)
- Complexity-based timeout adjustment

### Hook Timeout Configuration

**Problem:** Hardcoded 30-second timeout insufficient for complex validation

**Solution:**

`.claude/settings.json`:
```json
{
  "hooks": {
    "defaultTimeout": 30,
    "timeoutOverrides": {
      "juce-validator.py": 60,
      "parameter-consistency-check.py": 45,
      "code-quality-scan.py": 90
    },
    "onTimeout": "warn"
  }
}
```

**Per-hook configuration:**

`.claude/hooks/hooks.json`:
```json
{
  "PostToolUse": [
    {
      "name": "juce-validator",
      "command": "python ${HOOK_DIR}/scripts/juce-validator.py",
      "timeout": 60,
      "async": false,
      "onTimeout": {
        "action": "warn",
        "message": "JUCE validation taking longer than expected..."
      }
    }
  ]
}
```

**Rationale:**
- Parameter checks on 50+ parameters need > 30s
- Complex codebase scans need > 60s
- Build artifact verification is fast (< 10s)

### Performance Budgets

**Note:** With Claude Code Max Plan, model selection optimized for **quality over speed**. All timings assume Sonnet/Opus usage with extended thinking where beneficial.

Expected timing for each workflow stage:

| Stage | Complexity 1-2 | Complexity 3 | Complexity 4-5 | Notes |
|-------|----------------|--------------|----------------|-------|
| **Stage 0: Research** | 3-5 min | 5-10 min | 10-15 min | JUCE docs via Context7 + extended thinking for algorithm exploration |
| **Stage 1: Planning** | 2-3 min | 3-5 min | 5-10 min | Complexity assessment + extended thinking for phase breakdown |
| **Stage 2: Foundation** | 2-4 min | 3-7 min | 5-10 min | CMake + skeleton build |
| **Stage 3: Shell** | 2-3 min | 3-5 min | 5-8 min | Parameter boilerplate |
| **Stage 4: DSP** | 5-10 min | 10-30 min | 30-60 min | Opus for complex algorithms, extended thinking enabled |
| **Stage 5: GUI** | 5-8 min | 8-15 min | 15-25 min | WebView binding + layout |
| **Stage 6: Validation** | 3-5 min | 5-10 min | 10-15 min | Pluginval + preset creation |
| **Total** | 22-38 min | 40-82 min | 80-143 min | End-to-end single session |

**Validator overhead:** Add 1-2 minutes per stage (Sonnet with extended thinking for semantic analysis)

**Extended thinking impact:**
- Research: +2-5 minutes (algorithm comparison, deep JUCE API exploration)
- Planning: +1-3 minutes (phase breakdown for complexity 4-5)
- DSP: +3-8 minutes (complex algorithm design)
- Validation: +1-2 minutes (deep contract analysis)

**Trade-off:** Extended thinking time investment pays off by catching issues early and improving first-attempt success rate

**If exceeding budget by 2x:** Investigate for inefficiencies or consider splitting stage

### Log Rotation Strategy

**Problem:** Logs accumulate indefinitely - 50 plugins × 10 builds each = 500+ log files consuming disk space

**Solution:** Automatic rotation with configurable retention policies

**Implementation:**

`.claude/settings.json`:
```json
{
  "logs": {
    "retention": {
      "days": 30,
      "maxFilesPerPlugin": 10,
      "maxTotalSize": "500MB"
    },
    "rotation": {
      "enabled": true,
      "schedule": "daily",
      "compress": true
    }
  }
}
```

**Rotation logic (in build-automation skill):**

```bash
#!/bin/bash
# Log rotation function
rotate_logs() {
  local PLUGIN_NAME=$1
  local LOG_DIR="logs/${PLUGIN_NAME}"
  local MAX_FILES=10

  # Create log directory if needed
  mkdir -p "${LOG_DIR}"

  # Count existing build logs
  local log_count=$(find "${LOG_DIR}" -name "build_*.log" | wc -l)

  if [[ $log_count -ge $MAX_FILES ]]; then
    # Delete oldest logs to maintain limit
    local to_delete=$((log_count - MAX_FILES + 1))
    find "${LOG_DIR}" -name "build_*.log" -type f | \
      sort | \
      head -n $to_delete | \
      xargs rm -f
  fi
}

# Call at START of build-and-install.sh (before creating new log)
rotate_logs "${PLUGIN_NAME}"
```

**Invocation:** Called at the beginning of `scripts/build-and-install.sh` before creating new build log.

**Retention policies:**

| Log Type | Keep Recent | Keep by Age | Compress After |
|----------|-------------|-------------|----------------|
| Build logs | Last 10 | 30 days | 7 days |
| Validation logs | Last 10 | 30 days | 7 days |
| Install logs | Last 5 | 30 days | Never (small) |
| System logs | N/A | 90 days | 30 days |

**Manual cleanup:**

```bash
# Clean all logs older than 30 days
find logs/ -name "*.log" -mtime +30 -delete

# Clean all logs for archived plugins
for plugin in $(grep "🗑️ Archived" PLUGINS.md | awk '{print $2}'); do
  rm -rf "logs/${plugin}"
done

# Compress all uncompressed logs older than 7 days
find logs/ -name "*.log" -mtime +7 ! -name "*.gz" -exec gzip {} \;
```

**Storage estimates:**

- Uncompressed log: ~50-200 KB
- Compressed log: ~10-40 KB (5:1 ratio)
- 50 plugins × 10 logs × 100 KB = ~50 MB uncompressed
- With compression: ~10 MB

**Monitoring:**

```bash
# Check total log size
du -sh logs/

# Check logs per plugin
du -sh logs/*/ | sort -h

# Largest log files
find logs/ -type f -exec du -h {} + | sort -hr | head -20
```

### Security Considerations

#### Hook Script Execution

**Risk:** Hooks run with full filesystem access, same privileges as Claude Code

**Mitigation (v2.0):**
- User responsible for auditing hook scripts before enabling
- Hooks stored in `.claude/hooks/scripts/` (version controlled)
- Clear warning in documentation: "Only enable hooks you trust"

**Future (v2.1+):**
- Sandboxed execution (Docker, VM, WASM)
- Permission system (hooks declare required capabilities)
- Code signing for official hooks

#### MCP Server Security

**Risk:** API keys exposed in environment variables, potential logging

**Mitigation (v2.0):**
```bash
# Store in environment (not committed)
export CONTEXT7_API_KEY="..."

# Or use OS keychain
security add-generic-password -a $USER -s "context7-api" -w "..."
```

**Future:**
- Encrypted credential storage
- OAuth token rotation
- Rate limiting per MCP server

#### MCP Server Failure Handling

**Scenario:** Context7 is down, rate-limited, or returns errors when fetching JUCE documentation.

**Strategy:** Graceful degradation with logging, never block workflow.

**Implementation:**

1. **Primary:** Attempt Context7 lookup (5s timeout)
2. **Fallback 1:** Check local cache (`.cache/juce-docs/[version]/[class].md`)
3. **Fallback 2:** Continue without docs, log warning with manual lookup URL

**Logging:**
```yaml
timestamp: "2025-11-08T14:30:25Z"
service: "context7"
error: "Connection timeout after 5s"
fallback_used: "local_cache"
impact: "none"  # none | degraded | blocked
```

**Decision:** Never block workflow on external service failures. Context7 is enhancement, not requirement.

#### WebView Content Security

**Risk:** User-authored HTML/CSS/JS in plugin UI could have vulnerabilities

**Mitigation:**
- No external resource loading (Content Security Policy)
- All resources served from embedded zip
- No eval() or inline scripts recommended

**Generated UI includes:**
```html
<meta http-equiv="Content-Security-Policy"
      content="default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline';">
```

---

## Related Procedures

This error handling architecture is implemented through:

- `procedures/skills/build-automation.md` - Build failure protocol (4-option system: Investigate/Show/Wait/Other)
- `procedures/agents/troubleshooter.md` - Multi-level investigation protocol (Levels 1-4)
- `procedures/skills/deep-research.md` - Tier 3 graduated research protocol

---
