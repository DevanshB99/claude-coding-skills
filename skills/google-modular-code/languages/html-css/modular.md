# HTML/CSS — modular structure

Load when starting a page, organising stylesheets, or building a reusable component.

## Layout

```
site/
├── index.html
├── styles/
│   ├── main.css           # imports/orders the rest; no rules of its own
│   ├── tokens.css         # custom properties: colour, spacing, type scale
│   ├── base.css           # element defaults, reset
│   ├── layout.css         # page-level structure
│   └── components/
│       ├── card.css
│       └── nav.css
└── scripts/
    └── main.js
```

One file per component, named for the component. No `misc.css`.

## Components

A component owns its own class namespace and does not reach outside it. Everything a card needs lives
under `.card`:

```css
.card { }
.card-title { }
.card-body { }
.card--compact { }   /* variant */
```

- **Never style by tag inside a component** (`.card h2`) — it breaks the moment the markup changes. Give
  the element a class.
- **No positional selectors** (`.card > div:nth-child(2)`) — they encode markup order as a dependency.
- **A component does not set its own outer margin or position.** The parent layout decides where it
  sits; the component decides what it looks like. That is what makes it reusable.

## Design tokens instead of magic values

Declare values once as custom properties and reference them. This is the CSS equivalent of naming a
constant.

```css
:root {
  --colour-text: #222;
  --space-md: 1rem;
  --radius: 4px;
}

.card {
  border-radius: var(--radius);
  color: var(--colour-text);
  padding: var(--space-md);
}
```

Changing the scale then happens in one place. Never scatter `#222` across twenty files.

## Keeping specificity flat

Specificity wars are the CSS version of tight coupling. Avoid them structurally:

- One class, one purpose. Prefer a flat `.card-title` to a nested `.card .header .title`.
- Nest at most one level deep.
- No IDs in selectors.
- No `!important`.
- Order files from general to specific in `main.css` so later rules win naturally, without escalation.

## Extension without modification

Add a variant class rather than editing the base rule or overriding it from elsewhere:

```css
.button { }
.button--primary { }     /* extends, does not override from a distance */
.button--disabled { }
```

The base stays untouched, and the variant is visible in the markup.

## HTML structure

- **Semantic landmarks first**: `<header>`, `<nav>`, `<main>`, `<footer>`. Screen readers and your future
  self both navigate by them.
- **One `<h1>` per page**, headings in order with no skipped levels — the heading structure is the
  document outline, not a font-size picker.
- **Extract repeated markup** into a template/partial/component rather than copying it. Repeated markup
  drifts exactly like repeated code.
- Keep the DOM shallow. Deep wrappers usually mean a layout being solved by nesting instead of by CSS.

## Robustness

- Progressive enhancement: the page works with HTML alone, looks right with CSS, gains behaviour with JS.
- Fallback declaration before a modern one, so an unsupported value is simply ignored.
- Relative units and flexible layout so an unexpected viewport degrades rather than breaks.
- Validate rather than patch.

## Testing shape

**Only when the user asked for tests.** General policy is in `core/testing.md`. For HTML/CSS the checks are mostly static:

- `html-validate` (or the W3C validator) in CI over every page — invalid markup is a bug you can catch
  for free.
- `stylelint` for CSS, with the config committed.
- **Accessibility is testable**: `axe-core` or `pa11y` catches missing `alt`, unlabelled controls,
  insufficient contrast, and broken heading order. Run it in CI, not by hand.
- Keyboard-only pass on anything interactive: every control reachable by Tab, visible focus ring.
- Check the page with CSS disabled — reading order should still make sense. That is the real test of
  whether the markup is semantic.
- Screenshot/visual-regression tests only for components that genuinely regress; they are expensive and
  brittle, so keep them few.

---
*Adapted from the QA of Code guidance (https://best-practice-and-impact.github.io/qa-of-code-guidance/),
OGL v3.0, and the Google HTML/CSS Style Guide. See `NOTICE.md`.*
