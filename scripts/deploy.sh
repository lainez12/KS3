#!/bin/bash

# Exit immediately if a command fails
set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

usage() {
    echo -e "${BLUE}Usage:${NC} $0 [4|6|8]"
    exit 1
}

if [ -z "$1" ]; then
    echo -e "${RED}Error:${NC} No machine model specified."
    usage
fi

MODEL=$1

# 1. Read and sanitize version from .version file
if [ ! -f ".version" ]; then
    echo -e "${RED}Error:${NC} .version file not found in root directory."
    exit 1
fi
VERSION=$(cat .version | tr -d '\r' | xargs)

echo -e "${BLUE}==>${NC} Starting production deployment for ${GREEN}KUB3-$MODEL (Version: $VERSION)${NC}..."

# 2. Configure & Build
cmake --preset "kub3-$MODEL-production"
cmake --build --preset "kub3-$MODEL-production"

# 3. Install into dist
cmake --install "build/kub$MODEL-production"

# 4. Create Versioned Tarball
# We name it kubX-vX.X.X-production.tar.gz
TAR_NAME="kub$MODEL-v$VERSION-production.tar.gz"
echo -e "${BLUE}==>${NC} Archiving bundle to ${GREEN}$TAR_NAME${NC}..."

cd dist
# We zip the folder but name the archive with the version
tar -czf "$TAR_NAME" "kub$MODEL-production"
cp ../scripts/setup.sh .
cd ..

echo -e "--------------------------------------------------------"
echo -e "${GREEN}SUCCESS!${NC} Deployment package created:"
echo -e "${BLUE}dist/$TAR_NAME${NC}"
echo -e "--------------------------------------------------------"
