#!/bin/bash

# Exit immediately if a command fails
set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

usage() {
    echo -e "${BLUE}Usage:${NC} $0 [4|6|8] [--debug]"
    exit 1
}

if [ -z "$1" ]; then
    echo -e "${RED}Error:${NC} No machine model specified."
    usage
fi

HW_VERSION=$1
MODEL=$2
shift
shift # Double shift to check for additional arguments

# Default to production; change if --debug is provided
DEBUG_SUFFIX=""
DEPLOY_TYPE="production"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -d|--debug)
            DEBUG_SUFFIX="-dbg"
            DEPLOY_TYPE="debug"
            ;;
        *)
            echo -e "${RED}Error:${NC} Unknown parameter passed: $1"
            usage
            ;;
    esac
    shift
done

# 1. Read and sanitize version from .version file
if [ ! -f ".version" ]; then
    echo -e "${RED}Error:${NC} .version file not found in root directory."
    exit 1
fi
VERSION=$(cat .version | tr -d '\r' | xargs)

echo -e "${BLUE}==>${NC} Starting ${DEPLOY_TYPE} deployment for ${GREEN}KUB$HW_VERSION-$MODEL (Version: $VERSION)${NC}..."

# Setup dynamic names based on deployment type
PRESET="kub$HW_VERSION-$MODEL-production$DEBUG_SUFFIX"
BUILD_DIR="build/kub$HW_VERSION-$MODEL-production$DEBUG_SUFFIX"
DIST_FOLDER="kub$HW_VERSION-$MODEL-production$DEBUG_SUFFIX"
TAR_NAME="kub$HW_VERSION-$MODEL-v$VERSION-production$DEBUG_SUFFIX.tar.gz"

# 2. Configure & Build
NPROC=$(( $(nproc 2>/dev/null || echo 1) - 1 ))
[ "$NPROC" -lt 1 ] && NPROC=1

echo -e "${BLUE}==>${NC} Configuring and building preset: ${GREEN}$PRESET${NC}"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET" -j $NPROC

# 3. Install into dist
echo -e "${BLUE}==>${NC} Installing from build directory: ${GREEN}$BUILD_DIR${NC}"
cmake --install "$BUILD_DIR"

# 4. Create Versioned Tarball
echo -e "${BLUE}==>${NC} Archiving bundle to ${GREEN}$TAR_NAME${NC}..."

cd dist
# Zip the targeted folder using the dynamically generated names
tar -czf "$TAR_NAME" "$DIST_FOLDER"
cp ../scripts/setup.sh .
cd ..

echo -e "--------------------------------------------------------"
echo -e "${GREEN}SUCCESS!${NC} $DEPLOY_TYPE deployment package created:"
echo -e "${BLUE}dist/$TAR_NAME${NC}"
echo -e "--------------------------------------------------------"
