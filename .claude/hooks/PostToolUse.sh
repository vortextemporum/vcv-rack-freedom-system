#!/bin/bash
# PostToolUse hook - Real-time code quality validation for VCV Rack best practices
# Layer 1 validation: Fast pattern matching (<2s), blocks on real-time safety violations
# Layer 2 validation: Silent failure detection (runs after file modifications)

INPUT=$(cat)
TOOL_NAME=$(echo "$INPUT" | jq -r '.tool_name // empty' 2>/dev/null)

# Check relevance FIRST - only validate on Write/Edit to plugin source files
if [[ ! "$TOOL_NAME" =~ ^(Write|Edit)$ ]]; then
  exit 0
fi

FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // empty' 2>/dev/null)

# Layer 0: Contract immutability enforcement (CRITICAL)
# Block modifications to contract files during Stages 1-3
if [[ "$FILE_PATH" =~ plugins/([^/]+)/.ideas/(creative-brief|parameter-spec|architecture|plan)\.md$ ]]; then
  # Extract plugin name from regex match
  PLUGIN_NAME="${BASH_REMATCH[1]}"
  PLUGIN_PATH="plugins/$PLUGIN_NAME"

  # Check if .continue-here.md exists
  if [ -f "$PLUGIN_PATH/.continue-here.md" ]; then
    # Extract current stage
    STAGE=$(grep -E '^stage:' "$PLUGIN_PATH/.continue-here.md" | head -1 | sed 's/stage: *//')

    # Block if in implementation stages (1-3)
    if [[ "$STAGE" =~ ^[1-3]$ ]]; then
      CONTRACT_FILE=$(basename "$FILE_PATH")
      echo "" >&2
      echo "❌ CONTRACT IMMUTABILITY VIOLATION" >&2
      echo "" >&2
      echo "Cannot modify $CONTRACT_FILE during Stage $STAGE (implementation)." >&2
      echo "" >&2
      echo "Contracts are immutable during Stages 1-3 to prevent drift." >&2
      echo "All implementation stages reference the same contract specifications." >&2
      echo "" >&2
      echo "If you need to modify contracts:" >&2
      echo "  1. Return to Stage 0 (planning)" >&2
      echo "  2. Update contracts" >&2
      echo "  3. Restart implementation from Stage 1" >&2
      echo "" >&2
      echo "Alternatively, use /improve after Stage 3 to make changes with versioning." >&2
      exit 1
    fi
  fi
fi

# Determine validation level based on file type
PLUGIN_SOURCE=false
PLUGIN_FILE=false

if [[ "$FILE_PATH" =~ plugins/.*/src/.*\.(cpp|hpp)$ ]]; then
  PLUGIN_SOURCE=true
  PLUGIN_FILE=true
elif [[ "$FILE_PATH" =~ plugins/.*/(Makefile|plugin\.json|res/.*\.svg)$ ]]; then
  PLUGIN_FILE=true
fi

# Skip if not a plugin file
if [ "$PLUGIN_FILE" = false ]; then
  exit 0
fi

# Extract file content based on tool type
if [[ "$TOOL_NAME" == "Write" ]]; then
  FILE_CONTENT=$(echo "$INPUT" | jq -r '.tool_input.content // empty' 2>/dev/null)
elif [[ "$TOOL_NAME" == "Edit" ]]; then
  FILE_CONTENT=$(echo "$INPUT" | jq -r '.tool_input.new_string // empty' 2>/dev/null)
fi

if [ -z "$FILE_CONTENT" ]; then
  exit 0
fi

# Layer 1: Real-time safety checks (process() function only, for C++ source files)
if [ "$PLUGIN_SOURCE" = true ]; then
  # Extract process() function for real-time safety checks
  PROCESS_FUNC=$(echo "$FILE_CONTENT" | awk '/void.*process\(const ProcessArgs/{flag=1} flag{print} /^[[:space:]]*}[[:space:]]*$/{if(flag) exit}')

  if [ -n "$PROCESS_FUNC" ]; then

    # Real-time safety violation checks
    ERRORS=""

    # Check for heap allocation
    if echo "$PROCESS_FUNC" | grep -qE '\bnew\s+|delete\s+|malloc|free\b|std::vector.*push_back'; then
      ERRORS="${ERRORS}\nERROR: Heap allocation detected in process() (new/delete/malloc/push_back)"
    fi

    # Check for blocking locks
    if echo "$PROCESS_FUNC" | grep -qE 'std::mutex|\.lock\(\)|\.try_lock\(\)|std::lock_guard|std::unique_lock'; then
      ERRORS="${ERRORS}\nERROR: Blocking mutex/lock detected in process()"
    fi

    # Check for I/O operations
    if echo "$PROCESS_FUNC" | grep -qE 'fopen|fclose|fread|fwrite|std::ifstream|std::ofstream'; then
      ERRORS="${ERRORS}\nERROR: File I/O operations detected in process()"
    fi

    # Check for console output
    if echo "$PROCESS_FUNC" | grep -qE 'std::cout|std::cerr|printf|fprintf'; then
      ERRORS="${ERRORS}\nERROR: Console output detected in process() (std::cout/printf)"
    fi

    # Report results
    if [ -n "$ERRORS" ]; then
      echo "Real-time safety violations detected in $FILE_PATH:" >&2
      echo -e "$ERRORS" >&2
      echo "" >&2
      echo "These violations can cause audio dropouts, glitches, or crashes." >&2
      echo "process() must be real-time safe (no allocations, locks, or I/O)." >&2
      exit 1  # Block workflow on ERROR
    fi
  fi

  # VCV Rack specific checks
  WARNINGS=""

  # Check for missing enum terminators
  if echo "$FILE_CONTENT" | grep -qE 'enum ParamId' && ! echo "$FILE_CONTENT" | grep -qE 'PARAMS_LEN'; then
    WARNINGS="${WARNINGS}\nWARNING: Missing PARAMS_LEN enum terminator"
  fi

  if echo "$FILE_CONTENT" | grep -qE 'enum InputId' && ! echo "$FILE_CONTENT" | grep -qE 'INPUTS_LEN'; then
    WARNINGS="${WARNINGS}\nWARNING: Missing INPUTS_LEN enum terminator"
  fi

  if echo "$FILE_CONTENT" | grep -qE 'enum OutputId' && ! echo "$FILE_CONTENT" | grep -qE 'OUTPUTS_LEN'; then
    WARNINGS="${WARNINGS}\nWARNING: Missing OUTPUTS_LEN enum terminator"
  fi

  # Check for polyphonic output without setChannels
  if echo "$FILE_CONTENT" | grep -qE 'getChannels\(\)' && ! echo "$FILE_CONTENT" | grep -qE 'setChannels'; then
    WARNINGS="${WARNINGS}\nWARNING: Reading polyphonic channels but setChannels() not found (required for poly outputs)"
  fi

  if [ -n "$WARNINGS" ]; then
    echo "Code quality recommendations for $FILE_PATH:" >&2
    echo -e "$WARNINGS" >&2
    # Don't block on warnings, just inform
  fi
fi

# Layer 2: SVG validation (for panel files)
if [[ "$FILE_PATH" =~ \.svg$ ]]; then
  # Check for text elements (not allowed in VCV Rack panels)
  if echo "$FILE_CONTENT" | grep -qE '<text[^>]*>'; then
    echo "" >&2
    echo "❌ SVG VALIDATION FAILED" >&2
    echo "" >&2
    echo "Text elements detected in SVG panel: $FILE_PATH" >&2
    echo "VCV Rack does not support text in SVG panels." >&2
    echo "" >&2
    echo "Resolution: Convert all text to paths in Inkscape:" >&2
    echo "  1. Select text" >&2
    echo "  2. Path → Object to Path" >&2
    echo "  3. Save as Plain SVG" >&2
    exit 1
  fi
fi

exit 0
