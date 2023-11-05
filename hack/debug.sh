#!/bin/bash

set -euo pipefail

usage() {
    echo >&2 "usage: $0 TEST [TEST ...]"
    echo >&2
    echo >&2 "example:"
    echo >&2 "  $0 ./test/misc/nbody.tcf"
}

if [ $# -lt 1 ]; then
    echo "Missing test argument(s)"
    usage
    exit 1
fi

BUILD_DIR="${BUILD_DIR:-build}"

ninja -C build talvos-cmd
TAVLOS_CMD="$PWD/build/tools/talvos-cmd/talvos-cmd"

while [[ $# -gt 0 ]]; do
    TEST="$1"; shift
    (
        cd "$(dirname "$TEST")"
        # gdb hates reading commands from pipes, and bash >5.1 optimize small herestrings/heredocs into pipes
        # instead, force bash to use a temp file for the herestring
        SUFFIX=''
        if (( BASH_VERSINFO[0] >= 5 )); then
            if (( BASH_VERSINFO[0] == 5 && BASH_VERSINFO[1] == 1)); then
                SUFFIX=$'\n'"$(head -c "$(</proc/sys/fs/pipe-max-size)" /dev/zero | tr "\0" ' ')"
            else
                # this turns off the pipes behavior, but only in bash >=5.2
                # see: http://git.savannah.gnu.org/cgit/bash.git/tree/redir.c?h=bash-5.2&id=74091dd4e8086db518b30df7f222691524469998#n463
                BASH_COMPAT=50
            fi
        fi
        TALVOS_WORKER_THREADS=1 \
        gdb --return-child-result -ex 'set confirm off' -ex r \
            --command /dev/fd/3 3<<<"if ! \$_isvoid (\$_exitcode)"$'\n'quit$'\n'end"$SUFFIX" \
            --args "$TAVLOS_CMD" "$(basename "$TEST")"
    )
done

