#!/usr/bin/env bash
# Golden-output tests for the REPL.
# Each caseNN.in is fed via --script; stdout is compared to caseNN.out.
# Normalization: strip prompts, replace file paths with <FILE>.

set -e
export LC_ALL=C
REPL=./build/repl
DIR=tests/repl_golden
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

normalize() {
    sed -e 's/^> //' \
        -e 's|/tmp/[^ ]*|<FILE>|g' \
        -e 's|tests/repl_golden/[^ ]*\.repl|<FILE>|g'
}

fail=0
for in_file in "$DIR"/*.in; do
    base=$(basename "$in_file" .in)
    out_file="$DIR/$base.out"
    got="$TMP/$base.got"
    "$REPL" --script "$in_file" > "$got" 2>&1 || true
    if ! diff -u <(normalize < "$out_file") <(normalize < "$got") > "$TMP/$base.diff"; then
        echo "FAIL: $base"
        cat "$TMP/$base.diff"
        fail=1
    else
        echo "ok:   $base"
    fi
done

if [ "$fail" -ne 0 ]; then
    echo "golden tests FAILED"
    exit 1
fi
echo "all golden tests passed"
