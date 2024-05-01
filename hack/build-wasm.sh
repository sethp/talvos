#!/bin/bash

set -euo pipefail

[[ $# -gt 0 ]] || {
    echo >&2 "usage: $0 PATH"
    echo >&2 "missing required PATH to copy artifacts into once they're built"
    echo >&2 "e.g. $0 ../learn-gpgpu/wasm"
    exit 2
}

uniqifier() {
	# we use the inode of the tmpfile as part of the uniqifier
	# `mktemp` effectively reserves a name for our use, but
	# leaves a dangling file around unless we clean it up. using
	# the inode lets us do the "unlink an open file" trick, while
	# still reserving a unique name.
	#
	# see also: https://unix.stackexchange.com/a/181938
	#
	# why do this tortured, cursed thing? it handles:
	# * concurrent runs of this script[0]
	# * unexpected exits (even `kill -9`, after the rm)
	# * temp file "fixation" races
	#
	# remove when: docker provides a way to get the unique content sha
	# back out of a `docker build` other than `-q` which hides the output.
	#
	# [0] or indeed any script using the same docker repo prefix,
	#     as long as they share the same tmp directory or filesystem
	! [[ -e /proc/$$/fd/45 ]] || {
		# since bash 4.1 there's also a way to get the shell to allocate a fd for us,
		# but the default bash on _some_ platforms (macOS) is still version 3.
		# cf. https://stackoverflow.com/a/41620630
		echo >&2 "cannot generate uniqifier: file descriptor 45 already in use; did you call it twice?"
		echo >&2 "(if you need two unique tags, try making a \`uniqifier2\` copy and changing the file descriptor number)"
		exit 1
	}
	tmpfile=$(mktemp)
	exec 45<"$tmpfile"
	inode=$(stat -c'%i' "$tmpfile")
	rm "$tmpfile"

	echo "$(basename "$tmpfile")-ino-$inode"
}
TAG="talvos-build:$(uniqifier)"

set -x

cd "$(dirname "${BASH_SOURCE[0]}")/.."

docker build -f Dockerfile.emscripten . --target=talvos -t "$TAG"

[ -d ./build/emscripten-docker ] || {
	docker run -it --rm -v "$(pwd)":/usr/src/talvos -w /usr/src/talvos \
    "$TAG" -- \
    cmake -B build/emscripten-docker
}

docker run -it --rm -v "$(pwd)":/usr/src/talvos -w /usr/src/talvos \
    "$TAG" -- \
    cmake --build build/emscripten-docker --target talvos-wasm

cp --verbose --update=older build/emscripten-docker/tools/talvos-cmd/talvos-wasm.* "$1"
