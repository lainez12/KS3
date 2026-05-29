#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

PORT=$1
BIN_FILE=$2
INSTALL_DIR="/usr/local/bin"
BOSSAC_BIN="$INSTALL_DIR/bossac"

# 1. Validate inputs
if [ -z "$PORT" ] || [ -z "$BIN_FILE" ]; then
    echo "Usage: $0 <serial_port> <firmware.bin>"
    echo "Example: $0 /dev/ttyACM0 firmware.bin"
    exit 1
fi

# 2. Convert firmware path to an absolute path to prevent 'No such file' errors
if command -v realpath &> /dev/null; then
    BIN_FILE=$(realpath "$BIN_FILE")
else
    # Fallback if realpath is not installed
    BIN_FILE="$(cd "$(dirname "$BIN_FILE")" && pwd)/$(basename "$BIN_FILE")"
fi

if [ ! -f "$BIN_FILE" ]; then
    echo "Error: Firmware file '$BIN_FILE' does not exist."
    exit 1
fi

# 3. Check if bossac is installed, build it if not
if [ ! -f "$BOSSAC_BIN" ]; then
    echo "================================================="
    echo " bossac not found in $INSTALL_DIR"
    echo " Cloning from GitHub and building from source..."
    echo "================================================="
    
    # Create a secure temporary directory
    TMP_DIR=$(mktemp -d -t bossa-XXXXXX)
    
    echo " -> Cloning BOSSA repository to $TMP_DIR..."
    git clone https://github.com/shumatech/BOSSA.git "$TMP_DIR"
    
    echo " -> Building bossac..."
    # Build in a subshell so we don't mess up our current directory
    (
        cd "$TMP_DIR"
        make bin/bossac
    )
    
    echo " -> Installing bossac to $INSTALL_DIR (requires super user privileges)..."
    sudo cp "$TMP_DIR/bin/bossac" "$BOSSAC_BIN"
    sudo chmod +x "$BOSSAC_BIN"
    
    echo " -> Cleaning up temporary files..."
    rm -rf "$TMP_DIR"
    echo " Build and installation complete!"
    echo ""
fi

# 4. Execute the Flash sequence
echo "================================================="
echo " Flashing Arduino Due (Native Port)"
echo "================================================="
echo "Tool:     $BOSSAC_BIN"
echo "Port:     $PORT"
echo "Firmware: $BIN_FILE"
echo "================================================="

# Bash array of optimized arguments
FLASH_ARGS=(
    "--port=$PORT"
    "--usb-port=1"
    "--arduino-erase"
    "--erase"
    "--write"
    "--verify"
    "--boot=1"
    "--reset"
    "$BIN_FILE"
)

# Check if the user has write permissions to the serial port.
# If not, automatically elevate using sudo.
if [ -c "$PORT" ] && [ ! -w "$PORT" ]; then
    echo "Warning: You do not have write permissions for $PORT."
    echo "Elevating to sudo to perform the flash..."
    sudo "$BOSSAC_BIN" "${FLASH_ARGS[@]}"
else
    "$BOSSAC_BIN" "${FLASH_ARGS[@]}"
fi

echo "================================================="
echo " Flash complete! Arduino is rebooting."
echo "================================================="
