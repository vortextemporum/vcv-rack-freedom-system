#!/bin/bash
# PostToolUse hook - Real-time code quality validation for JUCE best practices
# Layer 1 validation: Fast pattern matching (<2s), blocks on real-time safety violations

INPUT=$(cat)
TOOL_NAME=$(echo "$INPUT" | jq -r '.tool_name // empty' 2>/dev/null)

# Check relevance FIRST - only validate on Write/Edit to plugin source files
if [[ ! "$TOOL_NAME" =~ ^(Write|Edit)$ ]]; then
  exit 0
fi

FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // empty' 2>/dev/null)
if [[ ! "$FILE_PATH" =~ plugins/.*/Source/.*\.(cpp|h)$ ]]; then
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

# Extract processBlock function for real-time safety checks
PROCESS_BLOCK=$(echo "$FILE_CONTENT" | awk '/void.*processBlock.*AudioBuffer/{flag=1} flag{print} /^}$/{if(flag) exit}')

if [ -z "$PROCESS_BLOCK" ]; then
  # No processBlock found, skip validation
  exit 0
fi

# Real-time safety violation checks
ERRORS=""

# Check for heap allocation
if echo "$PROCESS_BLOCK" | grep -qE '\bnew\s+|delete\s+|malloc|free\b'; then
  ERRORS="${ERRORS}\nERROR: Heap allocation detected in processBlock (new/delete/malloc/free)"
fi

# Check for blocking locks
if echo "$PROCESS_BLOCK" | grep -qE 'std::mutex|\.lock\(\)|\.try_lock\(\)|std::lock_guard|std::unique_lock'; then
  ERRORS="${ERRORS}\nERROR: Blocking mutex/lock detected in processBlock"
fi

# Check for I/O operations
if echo "$PROCESS_BLOCK" | grep -qE 'File::|URL::|FileOutputStream|FileInputStream'; then
  ERRORS="${ERRORS}\nERROR: File I/O operations detected in processBlock"
fi

# Check for console output
if echo "$PROCESS_BLOCK" | grep -qE 'std::cout|std::cerr|printf|fprintf'; then
  ERRORS="${ERRORS}\nERROR: Console output detected in processBlock (std::cout/printf)"
fi

# Best practice warnings (advisory only)
WARNINGS=""

# Check for ScopedNoDenormals
if ! echo "$PROCESS_BLOCK" | grep -q 'ScopedNoDenormals'; then
  WARNINGS="${WARNINGS}\nWARNING: Missing ScopedNoDenormals in processBlock"
fi

# Report results
if [ -n "$ERRORS" ]; then
  echo "Real-time safety violations detected in $FILE_PATH:" >&2
  echo -e "$ERRORS" >&2
  echo "" >&2
  echo "These violations can cause audio dropouts, glitches, or crashes." >&2
  echo "processBlock must be real-time safe (no allocations, locks, or I/O)." >&2
  exit 1  # Block workflow on ERROR
fi

if [ -n "$WARNINGS" ]; then
  echo "Code quality recommendations for $FILE_PATH:" >&2
  echo -e "$WARNINGS" >&2
  # Don't block on warnings, just inform
fi

exit 0
