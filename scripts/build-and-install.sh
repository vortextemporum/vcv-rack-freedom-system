#!/bin/bash
set -e

# ============================================================================
# build-and-install.sh - VCV Rack Plugin Build & Installation Script
# ============================================================================
# 6-Phase Pipeline:
#   1. Pre-flight Validation
#   2. Build Plugin
#   3. Extract Plugin Info
#   4. Remove Old Versions
#   5. Install New Version
#   6. Verification
# ============================================================================

# Color output functions
success() {
    echo -e "\033[0;32m✓ $1\033[0m"
}

warning() {
    echo -e "\033[0;33m⚠ $1\033[0m"
}

error() {
    echo -e "\033[0;31m✗ $1\033[0m" >&2
}

info() {
    echo -e "\033[0;36m→ $1\033[0m"
}

# Global variables
PLUGIN_NAME=""
PLUGIN_SLUG=""
DRY_RUN=false
NO_INSTALL=false
VERBOSE=false
CLEAN_BUILD=false
LOG_FILE=""
START_TIME=$(date +%s)

# Detect architecture
ARCH=$(uname -m)
if [[ "$ARCH" == "arm64" ]]; then
    ARCH_SUFFIX="mac-arm64"
else
    ARCH_SUFFIX="mac-x64"
fi

# Parse arguments
parse_args() {
    if [ $# -eq 0 ]; then
        echo "Usage: $0 <PluginName> [--dry-run] [--no-install] [--verbose] [--clean]"
        echo ""
        echo "Arguments:"
        echo "  <PluginName>   Name of plugin directory in plugins/"
        echo "  --dry-run      Show commands without executing"
        echo "  --no-install   Build only, skip installation"
        echo "  --verbose      Show detailed output"
        echo "  --clean        Clean build (make clean first)"
        exit 1
    fi

    PLUGIN_NAME="$1"
    shift

    while [ $# -gt 0 ]; do
        case "$1" in
            --dry-run)
                DRY_RUN=true
                ;;
            --no-install)
                NO_INSTALL=true
                ;;
            --verbose)
                VERBOSE=true
                ;;
            --clean)
                CLEAN_BUILD=true
                ;;
            *)
                error "Unknown flag: $1"
                exit 1
                ;;
        esac
        shift
    done
}

# Setup logging
setup_logging() {
    local log_dir="logs/$PLUGIN_NAME"
    mkdir -p "$log_dir"
    LOG_FILE="$log_dir/build_$(date +%Y%m%d_%H%M%S).log"

    info "Log file: $LOG_FILE"
    echo "Build started at $(date)" > "$LOG_FILE"
    echo "Plugin: $PLUGIN_NAME" >> "$LOG_FILE"
    echo "Architecture: $ARCH ($ARCH_SUFFIX)" >> "$LOG_FILE"
    echo "Flags: DRY_RUN=$DRY_RUN NO_INSTALL=$NO_INSTALL VERBOSE=$VERBOSE CLEAN_BUILD=$CLEAN_BUILD" >> "$LOG_FILE"
    echo "---" >> "$LOG_FILE"
}

# Execute command (respects dry-run flag)
execute() {
    local cmd="$*"

    if [ "$VERBOSE" = true ]; then
        info "Execute: $cmd"
    fi

    echo "$ $cmd" >> "$LOG_FILE"

    if [ "$DRY_RUN" = true ]; then
        echo "[DRY-RUN] $cmd"
        return 0
    fi

    if [ "$VERBOSE" = true ]; then
        eval "$cmd" 2>&1 | tee -a "$LOG_FILE"
    else
        eval "$cmd" >> "$LOG_FILE" 2>&1
    fi

    return ${PIPESTATUS[0]}
}

# ============================================================================
# Phase 1: Pre-flight Validation
# ============================================================================
phase_1_preflight_validation() {
    echo ""
    info "Phase 1: Pre-flight Validation"
    echo "Phase 1: Pre-flight Validation" >> "$LOG_FILE"

    # Check plugin directory exists
    info "  - Checking plugin directory..."
    if [ ! -d "plugins/$PLUGIN_NAME" ]; then
        error "Plugin directory not found: plugins/$PLUGIN_NAME"
        echo "ERROR: Plugin directory not found" >> "$LOG_FILE"
        exit 1
    fi

    # Check Makefile exists
    info "  - Checking Makefile..."
    if [ ! -f "plugins/$PLUGIN_NAME/Makefile" ]; then
        error "Makefile not found in plugins/$PLUGIN_NAME/"
        echo "ERROR: Makefile not found" >> "$LOG_FILE"
        exit 1
    fi

    # Check plugin.json exists
    info "  - Checking plugin.json..."
    if [ ! -f "plugins/$PLUGIN_NAME/plugin.json" ]; then
        error "plugin.json not found in plugins/$PLUGIN_NAME/"
        echo "ERROR: plugin.json not found" >> "$LOG_FILE"
        exit 1
    fi

    # Check RACK_DIR is set or can be found
    info "  - Checking RACK_DIR..."
    if [ -z "$RACK_DIR" ]; then
        # Try common locations
        if [ -d "$HOME/Rack-SDK" ]; then
            export RACK_DIR="$HOME/Rack-SDK"
            info "  - Using RACK_DIR=$RACK_DIR"
        elif [ -d "$HOME/Developer/Rack-SDK" ]; then
            export RACK_DIR="$HOME/Developer/Rack-SDK"
            info "  - Using RACK_DIR=$RACK_DIR"
        elif [ -d "/usr/local/Rack-SDK" ]; then
            export RACK_DIR="/usr/local/Rack-SDK"
            info "  - Using RACK_DIR=$RACK_DIR"
        else
            error "RACK_DIR not set and Rack SDK not found at standard locations"
            echo "ERROR: RACK_DIR not set" >> "$LOG_FILE"
            exit 1
        fi
    else
        info "  - RACK_DIR=$RACK_DIR"
    fi
    echo "RACK_DIR: $RACK_DIR" >> "$LOG_FILE"

    # Verify Rack SDK exists
    if [ ! -f "$RACK_DIR/plugin.mk" ]; then
        error "Invalid Rack SDK: $RACK_DIR/plugin.mk not found"
        echo "ERROR: Invalid Rack SDK" >> "$LOG_FILE"
        exit 1
    fi

    # Check make available
    info "  - Checking make..."
    if ! command -v make &> /dev/null; then
        error "make not found. Install with: xcode-select --install"
        echo "ERROR: make not found" >> "$LOG_FILE"
        exit 1
    fi

    success "Pre-flight validation passed"
    echo "Pre-flight validation: PASS" >> "$LOG_FILE"
}

# ============================================================================
# Phase 2: Build Plugin
# ============================================================================
phase_2_build() {
    echo ""
    info "Phase 2: Build Plugin"
    echo "Phase 2: Build Plugin" >> "$LOG_FILE"

    local plugin_dir="plugins/$PLUGIN_NAME"

    # Handle --clean flag
    if [ "$CLEAN_BUILD" = true ]; then
        info "  - Cleaning previous build..."
        if [ "$DRY_RUN" = true ]; then
            echo "[DRY-RUN] make -C \"$plugin_dir\" clean"
        else
            make -C "$plugin_dir" clean >> "$LOG_FILE" 2>&1 || true
            echo "Cleaned previous build" >> "$LOG_FILE"
        fi
    fi

    # Build the plugin
    info "  - Building $PLUGIN_NAME..."
    local make_cmd="make -C \"$plugin_dir\" -j$(sysctl -n hw.ncpu)"

    if ! execute $make_cmd; then
        error "Build failed"
        echo "ERROR: Build failed" >> "$LOG_FILE"
        echo ""
        echo "Build log: $LOG_FILE"
        exit 1
    fi

    # Create distribution package
    info "  - Creating distribution package..."
    if ! execute "make -C \"$plugin_dir\" dist"; then
        error "Distribution package creation failed"
        echo "ERROR: dist failed" >> "$LOG_FILE"
        exit 1
    fi

    success "Build complete"
    echo "Build: SUCCESS" >> "$LOG_FILE"
}

# ============================================================================
# Phase 3: Extract Plugin Info
# ============================================================================
phase_3_extract_plugin_info() {
    echo ""
    info "Phase 3: Extract Plugin Info"
    echo "Phase 3: Extract Plugin Info" >> "$LOG_FILE"

    local plugin_json="plugins/$PLUGIN_NAME/plugin.json"

    # Extract slug from plugin.json
    info "  - Parsing plugin.json..."
    PLUGIN_SLUG=$(jq -r '.slug' "$plugin_json")

    if [ -z "$PLUGIN_SLUG" ] || [ "$PLUGIN_SLUG" = "null" ]; then
        warning "Could not extract slug, using directory name as fallback"
        PLUGIN_SLUG="$PLUGIN_NAME"
        echo "WARNING: Using fallback PLUGIN_SLUG=$PLUGIN_SLUG" >> "$LOG_FILE"
    else
        info "  - Plugin slug: $PLUGIN_SLUG"
        echo "PLUGIN_SLUG: $PLUGIN_SLUG" >> "$LOG_FILE"
    fi

    # Extract version
    local version=$(jq -r '.version' "$plugin_json")
    info "  - Plugin version: $version"
    echo "VERSION: $version" >> "$LOG_FILE"

    success "Plugin info extracted: $PLUGIN_SLUG v$version"
}

# ============================================================================
# Phase 4: Remove Old Versions
# ============================================================================
phase_4_remove_old_versions() {
    echo ""
    info "Phase 4: Remove Old Versions"
    echo "Phase 4: Remove Old Versions" >> "$LOG_FILE"

    # VCV Rack 2 plugins directory
    local rack_plugins_dir

    # Check for Rack 2 plugins directory
    if [ -d "$HOME/Documents/Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/Documents/Rack2/plugins"
    elif [ -d "$HOME/.Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/.Rack2/plugins"
    elif [ -d "$HOME/Library/Application Support/Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/Library/Application Support/Rack2/plugins"
    else
        warning "VCV Rack 2 plugins directory not found"
        echo "WARNING: Rack plugins directory not found" >> "$LOG_FILE"
        return 0
    fi

    info "  - Plugins directory: $rack_plugins_dir"
    echo "PLUGINS_DIR: $rack_plugins_dir" >> "$LOG_FILE"

    # Search for existing plugin
    info "  - Searching for old plugin..."
    if [ -d "$rack_plugins_dir/$PLUGIN_SLUG" ]; then
        if [ "$DRY_RUN" = true ]; then
            echo "[DRY-RUN] Would remove: $rack_plugins_dir/$PLUGIN_SLUG"
        else
            info "  - Removing old plugin: $rack_plugins_dir/$PLUGIN_SLUG"
            rm -rf "$rack_plugins_dir/$PLUGIN_SLUG"
            echo "Removed old plugin: $rack_plugins_dir/$PLUGIN_SLUG" >> "$LOG_FILE"
        fi
    else
        info "  - No old plugin found"
        echo "No old plugin found" >> "$LOG_FILE"
    fi

    success "Old versions removed"
}

# ============================================================================
# Phase 5: Install New Version
# ============================================================================
phase_5_install_new_version() {
    echo ""
    info "Phase 5: Install New Version"
    echo "Phase 5: Install New Version" >> "$LOG_FILE"

    # VCV Rack 2 plugins directory
    local rack_plugins_dir

    # Check for Rack 2 plugins directory
    if [ -d "$HOME/Documents/Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/Documents/Rack2/plugins"
    elif [ -d "$HOME/.Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/.Rack2/plugins"
    elif [ -d "$HOME/Library/Application Support/Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/Library/Application Support/Rack2/plugins"
    else
        # Create default directory
        rack_plugins_dir="$HOME/Documents/Rack2/plugins"
        mkdir -p "$rack_plugins_dir"
        info "  - Created plugins directory: $rack_plugins_dir"
    fi

    # Find the built .vcvplugin file
    local plugin_dir="plugins/$PLUGIN_NAME"
    local dist_dir="$plugin_dir/dist"

    # Look for the plugin package
    local plugin_file=""
    if [ -f "$dist_dir/$PLUGIN_SLUG-*.vcvplugin" ]; then
        plugin_file=$(ls -t "$dist_dir"/$PLUGIN_SLUG-*.vcvplugin 2>/dev/null | head -1)
    fi

    # Fallback: look for any .vcvplugin
    if [ -z "$plugin_file" ]; then
        plugin_file=$(ls -t "$dist_dir"/*.vcvplugin 2>/dev/null | head -1)
    fi

    if [ -z "$plugin_file" ] || [ ! -f "$plugin_file" ]; then
        # Alternative: check if plugin directory exists (unzipped format)
        if [ -d "$plugin_dir/plugin" ]; then
            info "  - Installing from unzipped plugin directory..."
            if [ "$DRY_RUN" = true ]; then
                echo "[DRY-RUN] cp -R \"$plugin_dir/plugin\" \"$rack_plugins_dir/$PLUGIN_SLUG\""
            else
                cp -R "$plugin_dir/plugin" "$rack_plugins_dir/$PLUGIN_SLUG"
                echo "Installed from directory: $plugin_dir/plugin" >> "$LOG_FILE"
            fi
        else
            error "Plugin package not found in $dist_dir/"
            echo "ERROR: Plugin package not found" >> "$LOG_FILE"
            exit 1
        fi
    else
        # Install from .vcvplugin file (it's a zip)
        info "  - Installing from: $plugin_file"
        if [ "$DRY_RUN" = true ]; then
            echo "[DRY-RUN] unzip -o \"$plugin_file\" -d \"$rack_plugins_dir/\""
        else
            unzip -o "$plugin_file" -d "$rack_plugins_dir/" >> "$LOG_FILE" 2>&1
            echo "Installed: $plugin_file" >> "$LOG_FILE"
        fi
    fi

    success "New version installed"
}

# ============================================================================
# Phase 6: Verification
# ============================================================================
phase_6_verification() {
    echo ""
    info "Phase 6: Verification"
    echo "Phase 6: Verification" >> "$LOG_FILE"

    # VCV Rack 2 plugins directory
    local rack_plugins_dir

    if [ -d "$HOME/Documents/Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/Documents/Rack2/plugins"
    elif [ -d "$HOME/.Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/.Rack2/plugins"
    elif [ -d "$HOME/Library/Application Support/Rack2/plugins" ]; then
        rack_plugins_dir="$HOME/Library/Application Support/Rack2/plugins"
    fi

    local plugin_path="$rack_plugins_dir/$PLUGIN_SLUG"

    # Check plugin exists (skip in dry-run)
    info "  - Checking plugin exists..."
    if [ "$DRY_RUN" = false ] && [ ! -d "$plugin_path" ]; then
        error "Plugin not found at: $plugin_path"
        echo "ERROR: Plugin verification failed" >> "$LOG_FILE"
        exit 1
    fi

    if [ "$DRY_RUN" = true ]; then
        # In dry-run mode, just show what would be verified
        info "  - Would check plugin files..."
        echo ""
        success "Verification complete (dry-run)"
        echo ""
        info "Would verify:"
        echo "  Plugin: $plugin_path"
    else
        # Check required files
        info "  - Checking plugin files..."

        local has_plugin_json=false
        local has_dylib=false

        if [ -f "$plugin_path/plugin.json" ]; then
            has_plugin_json=true
        fi

        if ls "$plugin_path"/*.dylib >/dev/null 2>&1; then
            has_dylib=true
        fi

        if [ "$has_plugin_json" = false ]; then
            warning "plugin.json not found in installed plugin"
        fi

        if [ "$has_dylib" = false ]; then
            warning "No .dylib found in installed plugin"
        fi

        # Get plugin size
        local plugin_size=$(du -sh "$plugin_path" | cut -f1)

        echo ""
        success "Verification complete"
        echo ""
        info "Installed plugin:"
        echo "  Path: $plugin_path"
        echo "  Size: $plugin_size"

        echo "" >> "$LOG_FILE"
        echo "Verification: PASS" >> "$LOG_FILE"
        echo "Plugin: $plugin_path ($plugin_size)" >> "$LOG_FILE"
    fi
}

# ============================================================================
# Main execution
# ============================================================================
main() {
    parse_args "$@"
    setup_logging

    echo "============================================================================"
    echo "VCV Rack Plugin Build & Installation: $PLUGIN_NAME"
    echo "============================================================================"

    # Phase 1: Pre-flight Validation
    phase_1_preflight_validation

    # Phase 2: Build
    phase_2_build

    # If --no-install, exit early
    if [ "$NO_INSTALL" = true ]; then
        echo ""
        success "Build complete (--no-install flag set, skipping installation)"

        local end_time=$(date +%s)
        local duration=$((end_time - START_TIME))
        info "Build time: ${duration}s"
        info "Log: $LOG_FILE"

        echo "" >> "$LOG_FILE"
        echo "Build completed at $(date)" >> "$LOG_FILE"
        echo "Duration: ${duration}s" >> "$LOG_FILE"
        exit 0
    fi

    # Phase 3: Extract Plugin Info
    phase_3_extract_plugin_info

    # Phase 4: Remove Old Versions
    phase_4_remove_old_versions

    # Phase 5: Install New Version
    phase_5_install_new_version

    # Phase 6: Verification
    phase_6_verification

    # Final success message
    echo ""
    echo "============================================================================"
    success "Build and installation complete!"
    echo "============================================================================"

    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    info "Total time: ${duration}s"
    info "Log: $LOG_FILE"

    echo ""
    info "To test: Open VCV Rack and add the module from the browser"

    echo "" >> "$LOG_FILE"
    echo "Build completed at $(date)" >> "$LOG_FILE"
    echo "Duration: ${duration}s" >> "$LOG_FILE"
}

# Run main
main "$@"
