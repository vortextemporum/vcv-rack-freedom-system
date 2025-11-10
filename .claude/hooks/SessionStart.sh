#!/bin/bash
# SessionStart hook - Validate development environment
# Runs once at session start, non-blocking warnings only

echo "Validating development environment..."

# Check Python 3 availability (needed for validators)
if ! command -v python3 &> /dev/null; then
  echo "WARNING: python3 not found - validation scripts won't work" >&2
else
  PYTHON_VERSION=$(python3 --version 2>&1)
  echo "✓ $PYTHON_VERSION"
fi

# Check jq availability (needed for JSON parsing in hooks)
if ! command -v jq &> /dev/null; then
  echo "WARNING: jq not found - hooks may fail" >&2
else
  JQ_VERSION=$(jq --version 2>&1)
  echo "✓ jq $JQ_VERSION"
fi

# Check for JUCE (optional, informational only)
if [ -d "/Applications/JUCE" ]; then
  echo "✓ JUCE found at /Applications/JUCE"
elif [ -d "$HOME/JUCE" ]; then
  echo "✓ JUCE found at $HOME/JUCE"
else
  echo "INFO: JUCE not found at standard locations (may be installed elsewhere)"
fi

# Check for CMake
if command -v cmake &> /dev/null; then
  CMAKE_VERSION=$(cmake --version | head -1)
  echo "✓ $CMAKE_VERSION"
else
  echo "WARNING: cmake not found - builds will fail" >&2
fi

# Check for Ninja (optional but recommended)
if command -v ninja &> /dev/null; then
  NINJA_VERSION=$(ninja --version 2>&1)
  echo "✓ ninja $NINJA_VERSION (fast builds enabled)"
else
  echo "INFO: ninja not found (builds will use default generator)"
fi

# Check for Xcode (macOS only)
if [[ "$OSTYPE" == "darwin"* ]]; then
  if command -v xcodebuild &> /dev/null; then
    XCODE_VERSION=$(xcodebuild -version 2>&1 | head -1)
    echo "✓ $XCODE_VERSION"
  else
    echo "WARNING: Xcode not found - macOS builds will fail" >&2
  fi
fi

# Check git availability
if command -v git &> /dev/null; then
  GIT_VERSION=$(git --version 2>&1)
  echo "✓ $GIT_VERSION"
else
  echo "WARNING: git not found - version control disabled" >&2
fi

echo "Environment validation complete"
exit 0  # Never block session start
