#!/bin/bash

# Exit on error
set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}==>${NC} Starting Kub3Software Installation..."

# 1. Locate the versioned tarball
# Matches patterns like: kub8-v1.2.3-production.tar.gz
TAR_FILE=$(ls kub*-v*-production*.tar.gz 2>/dev/null | head -n 1)

if [ -z "$TAR_FILE" ]; then
    echo -e "${RED}Error:${NC} No versioned production tarball (kub*-v*-production.tar.gz) found."
    exit 1
fi

# 2. Extract Version and Model from filename
# Regex extracts the model number and the version string
if [[ $TAR_FILE =~ kub([0-9])-v([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+) ]]; then
    MODEL=${BASH_REMATCH[1]}
    VERSION=${BASH_REMATCH[2]}
else
    echo -e "${RED}Error:${NC} Could not parse version from filename $TAR_FILE"
    exit 1
fi

APP_NAME="Kub3Software_v$VERSION"
TARGET_DIR="$HOME/Desktop/.kub3wip/$APP_NAME"

echo -e "${BLUE}==>${NC} Target: ${GREEN}KUB3-$MODEL${NC} | Version: ${GREEN}$VERSION${NC}"
echo -e "${BLUE}==>${NC} Installation path: ${BLUE}$TARGET_DIR${NC}"

# 3. Handle System Dependencies (libxcb-cursor0)
if ! dpkg -s libxcb-cursor0 >/dev/null 2>&1; then
    echo -e "${BLUE}==>${NC} Installing missing system dependency: libxcb-cursor0..."
    sudo apt-get update && sudo apt-get install -y libxcb-cursor0
fi

# 4. Extract
if [ -d "$TARGET_DIR" ]; then
    echo -e "${RED}Warning:${NC} $TARGET_DIR already exists. Overwriting..."
    rm -rf "$TARGET_DIR"
fi
mkdir -p "$TARGET_DIR"

echo -e "${BLUE}==>${NC} Extracting files..."
tar -xzf "$TAR_FILE" -C "$TARGET_DIR" --strip-components=1

# 5. Done
chmod +x "$TARGET_DIR/bin/Kub3Software"

echo -e "--------------------------------------------------------"
echo -e "${GREEN}SUCCESS!${NC} Installed version $VERSION to:"
echo -e "${BLUE}$TARGET_DIR${NC}"
echo -e ""
echo -e "To run the app:"
echo -e "${BLUE}$TARGET_DIR/bin/Kub3Software${NC}"
echo -e "--------------------------------------------------------"
