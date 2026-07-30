const OPERATOR_SYMBOLS = { "+": "+", "-": "−", "*": "×", "/": "÷" };

const displayEl = document.getElementById("display");
const historyEl = document.getElementById("history");
const keysEl = document.querySelector(".keys");

const state = {
  entry: "0",
  left: null,
  operator: null,
  freshEntry: true,
  errored: false,
};

function render() {
  displayEl.textContent = state.entry;
  displayEl.classList.toggle("error", state.errored);
  historyEl.textContent =
    state.left === null ? "" : `${state.left} ${OPERATOR_SYMBOLS[state.operator]}`;

  document.querySelectorAll(".key.op").forEach((key) => {
    key.classList.toggle("active", key.dataset.operator === state.operator && state.freshEntry);
  });
}

function reset() {
  state.entry = "0";
  state.left = null;
  state.operator = null;
  state.freshEntry = true;
  state.errored = false;
}

function showError(message) {
  state.entry = message;
  state.left = null;
  state.operator = null;
  state.freshEntry = true;
  state.errored = true;
}

async function computeOnServer(left, operator, right) {
  const response = await fetch("/api/calculate", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ left, operator, right }),
  });
  const payload = await response.json().catch(() => ({ error: "bad server response" }));
  return response.ok ? { value: payload.result } : { error: payload.error || "calculation failed" };
}

async function resolvePending() {
  if (state.left === null || state.operator === null) {
    return true;
  }
  const outcome = await computeOnServer(state.left, state.operator, state.entry);
  if (outcome.error) {
    showError(outcome.error);
    render();
    return false;
  }
  state.entry = outcome.value;
  state.left = null;
  state.operator = null;
  state.freshEntry = true;
  return true;
}

function pushDigit(digit) {
  if (state.errored) {
    reset();
  }
  if (state.freshEntry) {
    state.entry = digit;
    state.freshEntry = false;
    return;
  }
  if (state.entry === "0") {
    state.entry = digit;
    return;
  }
  if (state.entry.replace("-", "").replace(".", "").length < 15) {
    state.entry += digit;
  }
}

function pushDecimal() {
  if (state.errored) {
    reset();
  }
  if (state.freshEntry) {
    state.entry = "0.";
    state.freshEntry = false;
    return;
  }
  if (!state.entry.includes(".")) {
    state.entry += ".";
  }
}

function negate() {
  if (state.errored || state.entry === "0") {
    return;
  }
  state.entry = state.entry.startsWith("-") ? state.entry.slice(1) : `-${state.entry}`;
}

function backspace() {
  if (state.errored) {
    reset();
    return;
  }
  if (state.freshEntry) {
    return;
  }
  const trimmed = state.entry.slice(0, -1);
  state.entry = trimmed === "" || trimmed === "-" ? "0" : trimmed;
}

async function chooseOperator(operator) {
  if (state.errored) {
    reset();
  }
  if (state.freshEntry && state.operator !== null) {
    state.operator = operator;
    render();
    return;
  }
  if (!(await resolvePending())) {
    return;
  }
  state.left = state.entry;
  state.operator = operator;
  state.freshEntry = true;
  render();
}

async function equals() {
  if (state.errored) {
    reset();
    render();
    return;
  }
  if (state.left === null) {
    state.freshEntry = true;
    render();
    return;
  }
  const shown = `${state.left} ${OPERATOR_SYMBOLS[state.operator]} ${state.entry} =`;
  if (await resolvePending()) {
    historyEl.textContent = shown;
    displayEl.textContent = state.entry;
    displayEl.classList.remove("error");
  }
}

const ACTIONS = {
  clear: () => { reset(); render(); },
  backspace: () => { backspace(); render(); },
  negate: () => { negate(); render(); },
  decimal: () => { pushDecimal(); render(); },
  equals,
};

keysEl.addEventListener("click", (event) => {
  const key = event.target.closest(".key");
  if (!key) {
    return;
  }
  if (key.dataset.digit !== undefined) {
    pushDigit(key.dataset.digit);
    render();
  } else if (key.dataset.operator) {
    chooseOperator(key.dataset.operator);
  } else {
    ACTIONS[key.dataset.action]();
  }
});

const KEYBOARD_ACTIONS = {
  Enter: "equals",
  "=": "equals",
  Backspace: "backspace",
  Escape: "clear",
  Delete: "clear",
  c: "clear",
  C: "clear",
  ".": "decimal",
  ",": "decimal",
};

document.addEventListener("keydown", (event) => {
  const { key } = event;
  let button = null;

  if (key >= "0" && key <= "9") {
    button = document.querySelector(`.key[data-digit="${key}"]`);
  } else if (OPERATOR_SYMBOLS[key]) {
    button = document.querySelector(`.key[data-operator="${key}"]`);
  } else if (KEYBOARD_ACTIONS[key]) {
    button = document.querySelector(`.key[data-action="${KEYBOARD_ACTIONS[key]}"]`);
  }

  if (!button) {
    return;
  }
  event.preventDefault();
  button.click();
  button.classList.add("pressed");
  setTimeout(() => button.classList.remove("pressed"), 90);
});

render();
