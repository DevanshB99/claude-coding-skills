#!/usr/bin/env bash
# Stop hook: ask the agent to run the standards review pass once code has been
# written, then let it stop.
#
# Contract: exit 0 lets the agent stop; exit 2 blocks the stop and feeds stderr
# back to the agent as its next instruction.
#
# Wire it up with install/settings.hooks.json. Verify with /hooks after adding.

set -uo pipefail

payload=$(cat)

# LOOP GUARD — mandatory. Claude Code sets stop_hook_active when this stop was
# already triggered by a hook. Without this check the hook blocks forever.
if printf '%s' "$payload" | grep -q '"stop_hook_active"[[:space:]]*:[[:space:]]*true'; then
  exit 0
fi

# Only speak up if code actually changed. Reviewing an untouched tree is noise.
if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  exit 0
fi

changed=$(git status --porcelain -- \
  '*.py' '*.cc' '*.cpp' '*.h' '*.hpp' '*.json' '*.html' '*.css' 2>/dev/null | head -40)

if [ -z "$changed" ]; then
  exit 0
fi

cat >&2 <<'MSG'
Code was written this turn but has not been through the standards review pass.

Invoke the google-modular-code skill now. Review every changed file end to end,
apply Tier 1 and Tier 2 edits, and report Tier 3 findings without editing them.
Behaviour must be identical afterwards. Do not create test files.

Then summarise what changed and stop.
MSG
exit 2
