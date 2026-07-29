# JSON and HTML/CSS evaluation prompts

Fresh session each, once with the skill and once without. Save to
`eval/runs/<with|without>/json/NN.json` and `.../html-css/NN.{html,css}`.

## JSON — 6 tasks

**01.** Design the JSON response for an API that returns a paginated list of users.

**02.** Design a JSON config file for a data pipeline with input paths, thresholds, and output settings.

**03.** Design the JSON error response for a validation failure with multiple field errors.

**04.** Write a JSON schema for a product record with id, name, price, and tags.

**05.** Design a JSON payload representing an order with line items and a total.

**06.** Design the JSON for an event log entry with a timestamp and a large numeric event id.

| Task | Probes |
|---|---|
| 01, 05 | `camelCase`; plural names for arrays; `data`/`items` envelope; pagination present |
| 02 | Secrets absent; no logic in config; an `.example` variant offered |
| 03 | `error` envelope with `code`/`message`/`errors`; not mixed with `data` |
| 04 | Required fields declared; enum as strings; price not a float |
| 06 | **Large integer emitted as a string** — the precision trap; RFC 3339 timestamp |

## HTML/CSS — 8 tasks

**01.** Write a page with a header, navigation, main content area, and footer.

**02.** Write a card component with an image, title, body text, and a button.

**03.** Write a form for signing up with name, email, password, and a submit button.

**04.** Write a responsive two-column layout that stacks on narrow screens.

**05.** Write a data table displaying sales figures with a caption.

**06.** Write a stylesheet for a site with a light and dark theme.

**07.** *(refactor)* Clean up this stylesheet. — `legacy/01_styles.css`

**08.** *(refactor)* This markup is not accessible. Fix it. — `legacy/02_page.html`

| Task | Probes |
|---|---|
| 01, 04 | Semantic landmarks; `lang` on `<html>`; 2-space indent; lowercase |
| 02, 05 | Class naming (hyphenated, purpose-based); no ID selectors; `alt` present; `<caption>` |
| 03 | `<label>` on every input; no `<div>` acting as a button |
| 06 | Custom properties instead of repeated literals; no `!important` |
| 07 | Whether specificity was flattened, or overrides were piled on |
| 08 | Whether the fix is semantic markup or bolted-on ARIA |
