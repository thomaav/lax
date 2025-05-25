#!/bin/bash

set -e

cd "$(dirname "$0")"
IFS=$'\n' shaders=$(git ls-files --modified --others --exclude-standard)
for shader in ${shaders[@]}; do
	case $shader in
		*.vert | *.vert | *.comp)
			glslangValidator -V "${shader}" -o "${shader}.spv"
			;;
	esac
done
