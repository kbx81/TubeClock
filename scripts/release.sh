#!/bin/bash
# Builds all firmware profiles and commits the .bin files to releases/<version>/.
#
# Usage: Run from the project root directory.
#   ./scripts/release.sh
#
# Prerequisites:
#   - arm-none-eabi-gcc toolchain in PATH
#   - Working tree must be clean (no uncommitted changes)
#   - Version in src/v1/version.h must not already have a git tag

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VERSION_H="$PROJECT_ROOT/src/v1/version.h"
BUILD_DIR="$PROJECT_ROOT/build/v1"
SRC_DIR="$PROJECT_ROOT/src/v1"

# Read version from version.h
MAJOR=$(sed -n 's/^#define FIRMWARE_VERSION_MAJOR[[:space:]]*//p' "$VERSION_H")
MINOR=$(sed -n 's/^#define FIRMWARE_VERSION_MINOR[[:space:]]*//p' "$VERSION_H")
PATCH=$(sed -n 's/^#define FIRMWARE_VERSION_PATCH[[:space:]]*//p' "$VERSION_H")
VERSION="$(printf '%02d' "$MAJOR").$(printf '%02d' "$MINOR").$(printf '%02d' "$PATCH")"

echo "Release version: $VERSION"

# Guard: clean working tree
if ! git -C "$PROJECT_ROOT" diff --quiet || ! git -C "$PROJECT_ROOT" diff --cached --quiet; then
    echo "Error: working tree has uncommitted changes. Commit or stash them first." >&2
    exit 1
fi

# Guard: tag must not already exist
if git -C "$PROJECT_ROOT" tag | grep -qx "$VERSION"; then
    echo "Error: tag '$VERSION' already exists." >&2
    exit 1
fi

RELEASE_DIR="$PROJECT_ROOT/releases/$VERSION"
mkdir -p "$RELEASE_DIR"

profile_name() {
    case $1 in
        0) echo "us" ;;
        1) echo "eu" ;;
        2) echo "minimal" ;;
        3) echo "kbx" ;;
    esac
}

for PROFILE in 0 1 2 3; do
    NAME="$(profile_name "$PROFILE")"
    echo ""
    echo "=== Building profile $PROFILE ($NAME) ==="
    (cd "$SRC_DIR" && make clean && make PROFILE="$PROFILE")
    cp "$BUILD_DIR/TubeClock.bin" "$RELEASE_DIR/TubeClock-$VERSION-$NAME.bin"
    echo "  -> $RELEASE_DIR/TubeClock-$VERSION-$NAME.bin"
done

echo ""
echo "=== Committing release ==="
git -C "$PROJECT_ROOT" add "$RELEASE_DIR"
git -C "$PROJECT_ROOT" commit -m "Release $VERSION"
git -C "$PROJECT_ROOT" tag "$VERSION"

echo ""
echo "Done. To publish:"
echo "  git push && git push --tags"
