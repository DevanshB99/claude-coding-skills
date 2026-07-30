# Installing the review pass

The skill runs **after** code is written, not while. Two ways to trigger it — wire both; they cover
different cases.

## 1. The skill itself

```bash
mkdir -p .claude/skills
cp -r skills/google-modular-code .claude/skills/     # this project only
# or: cp -r skills/google-modular-code ~/.claude/skills/   # every project
```

## 2. Manual trigger — `/review-standards`

```bash
mkdir -p .claude/commands
cp install/commands/review-standards.md .claude/commands/
```

Type `/review-standards` whenever you want a pass. Predictable, and the right choice if you dislike
automatic edits.

## 3. Automatic trigger — `Stop` hook

Fires when the agent finishes a turn, checks whether code changed, and if so instructs it to run the
review before stopping.

```bash
cp install/review-on-stop.sh .claude/review-on-stop.sh
chmod +x .claude/review-on-stop.sh
```

Then merge `install/settings.hooks.json` into `.claude/settings.json`:

```json
{
  "hooks": {
    "Stop": [
      { "hooks": [ { "type": "command", "command": "$CLAUDE_PROJECT_DIR/.claude/review-on-stop.sh" } ] }
    ]
  }
}
```

Confirm it registered with `/hooks`.

### How the hook behaves

| Situation | Result |
|---|---|
| Code files changed (`.py .cc .cpp .h .hpp .json .html .css`) | exit 2 — blocks the stop, tells the agent to review |
| Nothing changed, or only non-code files | exit 0 — silent |
| Not a git repository | exit 0 — silent |
| **Stop already triggered by this hook** (`stop_hook_active`) | exit 0 — **loop guard** |

**The loop guard is not optional.** A `Stop` hook that always blocks will block forever. The script
checks `stop_hook_active` in the hook payload and stands down when it is set. If you modify the script,
keep that check.

Verify the paths yourself before trusting it:

```bash
echo '{"stop_hook_active": true}'  | .claude/review-on-stop.sh; echo "expect 0, got $?"
echo '{"stop_hook_active": false}' | .claude/review-on-stop.sh; echo "expect 2 if code changed, got $?"
```

Hook payload fields and exit-code semantics can change between Claude Code versions. If the hook stops
firing after an upgrade, check `/hooks` and the current hook documentation before assuming the script is
broken.

## Isolation: what this does and does not guarantee

The point of triggering after generation is that the first pass writes code **unconstrained** — no style
rules in context, so nothing is suppressed for the sake of conformance. That matters: an earlier A/B
found rules applied *during* generation made front ends measurably worse, because "don't add anything
speculative" read as "don't add polish". See `../test/README.md`.

What you get:

- **Generation is not shaped by the rules.** The first pass optimises for the user's request alone.
- **The reviewer reads finished code**, so it judges what exists rather than steering what is being written.

What is not achievable:

- **An installed skill's one-line `description` is visible from session start** — that is how the model
  knows the skill exists. The generating pass sees that line. It does not see any rule.

To eliminate even that, have the hook launch the review in a **subagent** — a fresh context that reads the
rules and returns edits, so the generating context never holds them. The prompt in `review-on-stop.sh`
already reads naturally as a subagent instruction; add "Use a subagent to do this" to the message if you
want it enforced.

## Turning it off

Delete the `Stop` block from `.claude/settings.json`, or remove `.claude/review-on-stop.sh`. The skill
still works via `/review-standards`.
