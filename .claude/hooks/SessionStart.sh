#!/bin/bash
# SessionStart hook - Validate VCV Rack development environment
# Runs once at session start, provides warnings and actionable fixes

echo "━━━ Module Freedom System - Environment Validation ━━━"
echo ""

# Clean expired cache entries
if [ -f ".claude/utils/validation-cache.sh" ]; then
  source .claude/utils/validation-cache.sh
  clean_cache 2>/dev/null
fi

ERRORS=0
WARNINGS=0

# CRITICAL: Python 3 (required for helper scripts)
if ! command -v python3 &> /dev/null; then
  echo "❌ CRITICAL: python3 not found" >&2
  echo "   VCV Rack SDK helper scripts require Python" >&2
  echo "   FIX: brew install python3" >&2
  ERRORS=$((ERRORS + 1))
else
  PYTHON_VERSION=$(python3 --version 2>&1)
  echo "✓ $PYTHON_VERSION"
fi

# CRITICAL: jq (required for JSON parsing in hooks)
if ! command -v jq &> /dev/null; then
  echo "❌ CRITICAL: jq not found" >&2
  echo "   Hooks will fail - state management broken" >&2
  echo "   FIX: brew install jq" >&2
  ERRORS=$((ERRORS + 1))
else
  JQ_VERSION=$(jq --version 2>&1)
  echo "✓ $JQ_VERSION"
fi

# CRITICAL: Build tools (platform-specific)
if [[ "$OSTYPE" == "darwin"* ]]; then
  # macOS: Xcode required
  if command -v xcodebuild &> /dev/null; then
    XCODE_VERSION=$(xcodebuild -version 2>&1 | head -1)
    echo "✓ $XCODE_VERSION"
  else
    echo "❌ CRITICAL: Xcode not found" >&2
    echo "   macOS builds will fail" >&2
    echo "   FIX: xcode-select --install" >&2
    ERRORS=$((ERRORS + 1))
  fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  # Linux: gcc/g++ required
  if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -1)
    echo "✓ $GCC_VERSION"
  elif command -v clang++ &> /dev/null; then
    CLANG_VERSION=$(clang++ --version | head -1)
    echo "✓ $CLANG_VERSION"
  else
    echo "❌ CRITICAL: No C++ compiler found (g++ or clang++)" >&2
    echo "   Builds will fail" >&2
    echo "   FIX: sudo apt install build-essential" >&2
    ERRORS=$((ERRORS + 1))
  fi
fi

# CRITICAL: VCV Rack SDK (required for plugin builds)
RACK_SDK_FOUND=0
RACK_SDK_PATH=""

# Check standard locations for Rack SDK
if [ -n "$RACK_DIR" ] && [ -d "$RACK_DIR" ]; then
  # Environment variable set
  RACK_SDK_PATH="$RACK_DIR"
  RACK_SDK_FOUND=1
  echo "✓ Rack SDK found at $RACK_DIR (via RACK_DIR env)"
elif [ -d "$HOME/Rack-SDK" ]; then
  RACK_SDK_PATH="$HOME/Rack-SDK"
  RACK_SDK_FOUND=1
  echo "✓ Rack SDK found at ~/Rack-SDK"
elif [ -d "$HOME/Developer/Rack-SDK" ]; then
  RACK_SDK_PATH="$HOME/Developer/Rack-SDK"
  RACK_SDK_FOUND=1
  echo "✓ Rack SDK found at ~/Developer/Rack-SDK"
elif [ -d "/usr/local/Rack-SDK" ]; then
  RACK_SDK_PATH="/usr/local/Rack-SDK"
  RACK_SDK_FOUND=1
  echo "✓ Rack SDK found at /usr/local/Rack-SDK"
fi

if [ $RACK_SDK_FOUND -eq 0 ]; then
  echo "❌ CRITICAL: VCV Rack SDK not found" >&2
  echo "   All module builds will fail" >&2
  echo "   FIX: Download from https://vcvrack.com/downloads/" >&2
  echo "        Extract to ~/Rack-SDK" >&2
  echo "        OR set RACK_DIR environment variable" >&2
  ERRORS=$((ERRORS + 1))
else
  # Check SDK version by looking at helper.py or version file
  if [ -f "$RACK_SDK_PATH/helper.py" ]; then
    echo "  → SDK helper.py available"
  fi
fi

# HIGH PRIORITY: VCV Rack application (for testing)
VCV_RACK_FOUND=0
if [ -d "/Applications/VCV Rack 2 Free.app" ]; then
  echo "✓ VCV Rack 2 Free found in /Applications"
  VCV_RACK_FOUND=1
elif [ -d "/Applications/VCV Rack 2 Pro.app" ]; then
  echo "✓ VCV Rack 2 Pro found in /Applications"
  VCV_RACK_FOUND=1
elif [ -d "$HOME/Applications/VCV Rack 2 Free.app" ]; then
  echo "✓ VCV Rack 2 Free found in ~/Applications"
  VCV_RACK_FOUND=1
elif [ -d "$HOME/Applications/VCV Rack 2 Pro.app" ]; then
  echo "✓ VCV Rack 2 Pro found in ~/Applications"
  VCV_RACK_FOUND=1
fi

if [ $VCV_RACK_FOUND -eq 0 ]; then
  echo "⚠️  WARNING: VCV Rack 2 not found" >&2
  echo "   You won't be able to test modules without it" >&2
  echo "   FIX: brew install --cask vcv-rack" >&2
  echo "        OR download from https://vcvrack.com/Rack" >&2
  WARNINGS=$((WARNINGS + 1))
fi

# HIGH PRIORITY: Git (required for version control)
if command -v git &> /dev/null; then
  GIT_VERSION=$(git --version 2>&1)
  echo "✓ $GIT_VERSION"
else
  echo "⚠️  WARNING: git not found" >&2
  echo "   Version control disabled - workflow state won't persist" >&2
  echo "   FIX: brew install git" >&2
  WARNINGS=$((WARNINGS + 1))
fi

# MEDIUM PRIORITY: Make (should be included with Xcode CLT)
if command -v make &> /dev/null; then
  MAKE_VERSION=$(make --version 2>&1 | head -1)
  echo "✓ $MAKE_VERSION"
else
  echo "❌ CRITICAL: make not found" >&2
  echo "   VCV Rack SDK uses Makefile build system" >&2
  echo "   FIX: xcode-select --install" >&2
  ERRORS=$((ERRORS + 1))
fi

# LOW PRIORITY: Inkscape (optional for SVG panel editing)
if command -v inkscape &> /dev/null; then
  echo "✓ Inkscape available (SVG editing enabled)"
elif [ -d "/Applications/Inkscape.app" ]; then
  echo "✓ Inkscape.app found in /Applications"
else
  echo "ℹ️  INFO: Inkscape not found (SVG editing limited)" >&2
  echo "   FIX (optional): brew install --cask inkscape" >&2
fi

echo ""
echo "━━━ Validation Summary ━━━"
if [ $ERRORS -gt 0 ]; then
  echo "❌ $ERRORS critical error(s) found"
  echo "   Workflow will fail - fix critical issues before running /plan or /implement"
  echo ""
  echo "Quick fix command:"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "  brew install python3 jq && xcode-select --install"
    echo ""
    echo "  Download Rack SDK from: https://vcvrack.com/downloads/"
    echo "  Extract to: ~/Rack-SDK"
  else
    echo "  sudo apt install python3 jq build-essential"
  fi
  echo ""
  echo "Session will continue, but workflows will fail until dependencies are installed"
fi

if [ $WARNINGS -gt 0 ]; then
  echo "⚠️  $WARNINGS warning(s) - system usable but degraded functionality"
fi

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
  echo "✅ All dependencies validated - system ready"
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Never block session start (allow user to see errors and fix)
exit 0
