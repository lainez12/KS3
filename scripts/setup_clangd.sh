#!/bin/sh

DEFAULT_QT_PATH="$HOME/Qt/6.8.3/gcc_64"

# Ask the user for the path
printf "Enter your Qt installation path [%s]: " "$DEFAULT_QT_PATH"
read -r QT_PATH

# Use default if user just pressed Enter
QT_PATH="${QT_PATH:-$DEFAULT_QT_PATH}"

# Safely expand '~' if the user typed it manually
case "$QT_PATH" in
    "~"/*) QT_PATH="$HOME/${QT_PATH#\~/}" ;;
    "~")   QT_PATH="$HOME" ;;
esac

# Validate directory existence
if [ ! -d "$QT_PATH" ]; then
    echo "Error: Directory '$QT_PATH' does not exist."
    exit 1
fi

echo "Using Qt path: $QT_PATH"

# Execute CMake with the chosen path
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH="$QT_PATH"

# Create .clangd configuration file
echo "CompileFlags:
  CompilationDatabase: \".\"" > .clangd

echo "Configuration finished successfully."
