#!/usr/bin/env bash
# Verifies the review pass preserved behaviour: builds the pre-review sources
# (../runs-cpp/without/src) and the reviewed ones, then diffs their output.
set -uo pipefail
here=$(cd "$(dirname "$0")" && pwd)
orig="$here/../runs-cpp/without/src"
work=$(mktemp -d); trap 'rm -rf "$work"' EXIT

g++ -std=c++20 -O0 -o "$work/orig" "$orig"/*.cpp        || { echo "original failed to build"; exit 1; }
g++ -std=c++20 -O0 -o "$work/rev"  "$here"/reviewed/*.cpp || { echo "reviewed failed to build"; exit 1; }

fail=0 n=0
while IFS= read -r args; do
  n=$((n+1))
  a=$("$work/orig" --cli $args 2>&1; echo "rc=$?")
  b=$("$work/rev"  --cli $args 2>&1; echo "rc=$?")
  [ "$a" = "$b" ] || { fail=$((fail+1)); echo "DIFF: $args"; }
done <<'CASES'
1 -3 2
1 -2 1
1 0 1
0 2 -4
0 0 5
0 0 0
2.5 -7.5 3
1e300 1 1
-1 0 4
0.0001 -1 1
1 1e-12 -1
abc 1 2
CASES
echo "$n cases, $fail differences"
[ "$fail" -eq 0 ]
