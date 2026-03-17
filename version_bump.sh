#!/bin/bash

# Ensure .version exists
if [ ! -f .version ]; then
    echo "1.0.0.0" > .version
fi

# Read current version
IFS='.' read -r major minor patch build < .version
echo "Current version: $major.$minor.$patch.$build"

echo "Select bump type:"
echo "1) Major (X.0.0.0)"
echo "2) Minor (X.Y.0.0)"
echo "3) Patch (X.Y.Z.0)"
echo "4) Build (X.Y.Z.W+1) [Default]"
read -p "Selection [4]: " choice
choice=${choice:-4}

case $choice in
    1)
        major=$((major + 1))
        minor=0
        patch=0
        build=0
        ;;
    2)
        minor=$((minor + 1))
        patch=0
        build=0
        ;;
    3)
        patch=$((patch + 1))
        build=0
        ;;
    *)
        build=$((build + 1))
        ;;
esac

new_version="$major.$minor.$patch.$build"
echo "$new_version" > .version
echo "Version updated to: $new_version"

# Optional: Automatically commit the version change
# git add .version
# git commit -m "chore: bump version to $new_version"
