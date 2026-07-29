# HTML/CSS — Google style

Distilled from the Google HTML/CSS Style Guide.

## General

- 2-space indent, never tabs.
- **Lowercase everything**: element names, attributes, attribute values (where case-insensitive),
  selectors, properties, hex colours.
- No trailing whitespace.
- UTF-8, no BOM (`<meta charset="utf-8">`).
- Use HTTPS for embedded resources (`https://`), not protocol-relative `//`.
- Mark action items `TODO:` with enough context to act on. Never leave a bare `TODO`.

## HTML

**Structure**
- `<!doctype html>` — lowercase, first line.
- **`lang` on `<html>`**, always: `<html lang="en">`. Screen readers pick pronunciation from it and
  translation tools depend on it. Mark an inline language change with `lang` on that element too.
- Valid HTML; nest correctly and close what must be closed.
- **Semantics over presentation.** Use the element that means what you intend: `<button>` for a button,
  `<a>` for navigation, `<h1>`–`<h6>` in order, `<nav>`/`<main>`/`<article>`/`<section>` for structure.
  A `<div>` with a click handler is wrong when a `<button>` exists.
- **Separate concerns.** Structure in HTML, presentation in CSS, behaviour in JS. No `style` attributes,
  no presentational elements (`<center>`, `<font>`), no inline `onclick`. Link stylesheets and scripts.
- **Multimedia fallbacks.** `alt` on every `<img>` (empty `alt=""` for purely decorative images),
  captions/transcripts for video and audio.
- **No entity references** (`&mdash;`, `&rdquo;`) — the file is UTF-8, so type the character. Keep only
  the characters with syntactic meaning: `&amp;`, `&lt;`, `&gt;`, `&nbsp;`.
- Omit `type` on `<link rel="stylesheet">` and `<script>`.

**Formatting**
- A new line for every block, list, or table element; indent every child.
- Attribute values in **double quotes**.
- Break long lines at a point that keeps readability; each continuation indented.

```html
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>Quarterly report</title>
    <link rel="stylesheet" href="styles.css">
  </head>
  <body>
    <main>
      <h1>Quarterly report</h1>
      <ul class="report-list">
        <li>First</li>
        <li>Second</li>
      </ul>
    </main>
  </body>
</html>
```

## CSS

**Selectors and names**
- Valid CSS.
- Class names are **meaningful or generic**, describing purpose rather than appearance: `.author`,
  `.nav-primary` — not `.blue-text`, `.col-left`.
- As short as possible, as long as necessary. `.nav` not `.navigation`; `.author` not `.atr`.
- **Hyphen-delimited**: `.report-list`. Never `.reportList` or `.report_list`.
- **Avoid ID selectors** — not reusable, and their specificity is a trap. Use classes.
- **Don't qualify a class with a type selector** — `.error`, not `div.error`.
- Optionally namespace with a project prefix (`.rep-header`) to avoid collisions.

**Declarations**
- Use **shorthand properties** where they apply: `padding: 0 1em 2em;` not four declarations.
- **Omit units on `0`**: `margin: 0;` not `margin: 0px;`.
- **Keep leading zeros**: `opacity: 0.5;` not `.5`.
- **3-character hex where possible**: `#ebc;` not `#eebbcc;`.
- **Avoid `!important`.** It signals a specificity problem — fix the selector instead.
- **No hacks and no user-agent sniffing.** Try a different approach first.

**Formatting**
- One declaration per line, each ending in a semicolon — including the last.
- Space after the property colon: `color: red;`.
- Space between the last selector and the opening brace: `.a {`.
- Each selector in a comma-separated group on its own line.
- Indent all block content.
- Blank line between rules.
- **Single quotes** for attribute selectors and property values: `content: '';`,
  `input[type='text']`. (Note this differs from HTML, which uses double quotes.)
- Alphabetise declarations within a block — optional, but pick one and be consistent.

```css
.report-list,
.report-summary {
  border: 1px solid #ccc;
  margin: 0;
  padding: 0 1em;
}

.report-list > li {
  list-style: none;
}
```

## Comments

Explain only what the markup cannot say — a non-obvious layout constraint, a browser workaround, why an
override exists. Group large stylesheets with a short section comment when it genuinely aids navigation.
Default depth is `core/comments.md` L0: no narration of what a rule does.

```css
/* Fallback for browsers without container query support. */
```

## Robustness

There is nothing to catch here; resilience is structural:

- Semantic markup and `alt` text degrade gracefully when CSS, JS, or an image fails.
- Provide a fallback value before a modern one rather than assuming support:
  `color: #eee; color: color-mix(in srgb, white 90%, black);`
- Use relative units and flexible layouts so content survives an unexpected viewport.
- Validate markup and CSS rather than fixing symptoms with `!important`.

---
*Distilled from https://google.github.io/styleguide/htmlcssguide.html (CC BY 3.0). See `NOTICE.md`.*
