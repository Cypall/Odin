#!/bin/sh
# Oding server launcher script for Unix.

dir=`dirname "$0"`
cd "$dir"
set -e

# Compile Odin if not compiled already
if ! [ -f Odin-Account ] || ! [ -f Odin-Character ] || ! [ -f Odin-Zone ]; then
	make -C src
fi

# Start servers
nice ./Odin-Account &
nice ./Odin-Character &
exec ./Odin-Zone
