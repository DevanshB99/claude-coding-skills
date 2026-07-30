'use strict';

const SVG_NS = 'http://www.w3.org/2000/svg';
const PLOT_WIDTH = 640;
const PLOT_HEIGHT = 420;
const MARGIN = {left: 54, right: 16, top: 16, bottom: 34};
const TICK_COUNT = 5;
const SIGNIFICANT_DIGITS = 6;
const ROOT_MARKER_RADIUS = 5;
const VERTEX_MARKER_RADIUS = 4;

const NATURE_TEXT = {
  TWO_REAL: 'two distinct real roots',
  ONE_REPEATED_REAL: 'one repeated real root',
  COMPLEX_PAIR: 'a complex conjugate pair',
};

const elements = {
  form: document.getElementById('coefficient-form'),
  notice: document.getElementById('notice'),
  results: document.getElementById('results'),
  equation: document.getElementById('out-equation'),
  nature: document.getElementById('out-nature'),
  discriminant: document.getElementById('out-discriminant'),
  rootOne: document.getElementById('out-root-1'),
  rootTwo: document.getElementById('out-root-2'),
  vertex: document.getElementById('out-vertex'),
  plot: document.getElementById('plot'),
};

function formatNumber(value) {
  if (Number.isInteger(value)) {
    return String(value);
  }
  return String(Number(value.toPrecision(SIGNIFICANT_DIGITS)));
}

function formatRoot(root) {
  if (root.imaginary === 0) {
    return formatNumber(root.real);
  }
  const sign = root.imaginary < 0 ? '−' : '+';
  return `${formatNumber(root.real)} ${sign} ${formatNumber(Math.abs(root.imaginary))}i`;
}

function formatEquation(coefficients) {
  const term = (value, suffix) => {
    const sign = value < 0 ? '−' : '+';
    return ` ${sign} ${formatNumber(Math.abs(value))}${suffix}`;
  };
  return `${formatNumber(coefficients.a)}x²${term(coefficients.b, 'x')}${term(coefficients.c, '')} = 0`;
}

function createElement(name, attributes, className) {
  const node = document.createElementNS(SVG_NS, name);
  Object.entries(attributes).forEach(([key, value]) => {
    node.setAttribute(key, String(value));
  });
  if (className) {
    node.setAttribute('class', className);
  }
  return node;
}

function makeProjector(viewport) {
  const spanX = viewport.xMax - viewport.xMin || 1;
  const spanY = viewport.yMax - viewport.yMin || 1;
  const usableWidth = PLOT_WIDTH - MARGIN.left - MARGIN.right;
  const usableHeight = PLOT_HEIGHT - MARGIN.top - MARGIN.bottom;
  return {
    toX: (x) => MARGIN.left + ((x - viewport.xMin) / spanX) * usableWidth,
    toY: (y) => MARGIN.top + (1 - (y - viewport.yMin) / spanY) * usableHeight,
  };
}

function tickValues(minimum, maximum) {
  const step = (maximum - minimum) / (TICK_COUNT - 1);
  return Array.from({length: TICK_COUNT}, (unused, index) => minimum + step * index);
}

function appendLabel(svg, attributes, text) {
  const label = createElement('text', attributes, 'tick-label');
  label.textContent = text;
  svg.append(label);
}

function drawGridLines(svg, viewport, project) {
  const bottom = PLOT_HEIGHT - MARGIN.bottom;
  tickValues(viewport.xMin, viewport.xMax).forEach((value) => {
    const x = project.toX(value);
    svg.append(createElement('line', {x1: x, y1: MARGIN.top, x2: x, y2: bottom}, 'grid'));
    appendLabel(svg, {x: x, y: bottom + 16, 'text-anchor': 'middle'}, formatNumber(value));
  });
  tickValues(viewport.yMin, viewport.yMax).forEach((value) => {
    const y = project.toY(value);
    svg.append(createElement(
        'line', {x1: MARGIN.left, y1: y, x2: PLOT_WIDTH - MARGIN.right, y2: y}, 'grid'));
    appendLabel(svg, {x: MARGIN.left - 8, y: y + 4, 'text-anchor': 'end'},
                formatNumber(value));
  });
}

function drawAxes(svg, viewport, project) {
  if (viewport.yMin <= 0 && viewport.yMax >= 0) {
    const y = project.toY(0);
    svg.append(createElement(
        'line', {x1: MARGIN.left, y1: y, x2: PLOT_WIDTH - MARGIN.right, y2: y}, 'axis'));
  }
  if (viewport.xMin <= 0 && viewport.xMax >= 0) {
    const x = project.toX(0);
    svg.append(createElement(
        'line',
        {x1: x, y1: MARGIN.top, x2: x, y2: PLOT_HEIGHT - MARGIN.bottom},
        'axis'));
  }
}

function drawCurve(svg, points, project) {
  const path = points
      .map((point) => `${project.toX(point.x).toFixed(2)},${project.toY(point.y).toFixed(2)}`)
      .join(' ');
  svg.append(createElement('polyline', {points: path}, 'curve'));
}

function drawMarkers(svg, solution, project) {
  svg.append(createElement(
      'circle',
      {cx: project.toX(solution.vertexX), cy: project.toY(solution.vertexY),
       r: VERTEX_MARKER_RADIUS},
      'marker-vertex'));
  solution.roots
      .filter((root) => root.imaginary === 0)
      .forEach((root) => {
        svg.append(createElement(
            'circle',
            {cx: project.toX(root.real), cy: project.toY(0), r: ROOT_MARKER_RADIUS},
            'marker-root'));
      });
}

function renderPlot(solution) {
  const svg = elements.plot;
  svg.replaceChildren();
  const project = makeProjector(solution);
  drawGridLines(svg, solution, project);
  drawAxes(svg, solution, project);
  drawCurve(svg, solution.points, project);
  drawMarkers(svg, solution, project);
}

function renderSummary(coefficients, solution) {
  elements.equation.textContent = formatEquation(coefficients);
  elements.nature.textContent = NATURE_TEXT[solution.rootKind] || solution.rootKind;
  elements.discriminant.textContent = formatNumber(solution.discriminant);
  elements.rootOne.textContent = formatRoot(solution.roots[0]);
  elements.rootTwo.textContent = formatRoot(solution.roots[1]);
  elements.vertex.textContent =
      `(${formatNumber(solution.vertexX)}, ${formatNumber(solution.vertexY)})`;
}

function showNotice(message) {
  elements.notice.textContent = message;
  elements.notice.hidden = false;
  elements.results.hidden = true;
}

function showSolution(coefficients, solution) {
  elements.notice.hidden = true;
  elements.results.hidden = false;
  renderSummary(coefficients, solution);
  renderPlot(solution);
}

// Returns the parsed payload as {data} or {error}; a transport failure is
// reported as an error payload so callers have a single shape to handle.
async function requestSolution(coefficients) {
  const query = new URLSearchParams(coefficients).toString();
  try {
    const response = await fetch(`/api/solve?${query}`);
    return await response.json();
  } catch (networkError) {
    return {error: {message: `cannot reach the calculator service: ${networkError.message}`}};
  }
}

async function handleSubmit(event) {
  event.preventDefault();
  const form = new FormData(elements.form);
  const raw = {a: form.get('a'), b: form.get('b'), c: form.get('c')};
  const payload = await requestSolution(raw);
  if (payload.error) {
    showNotice(payload.error.message);
    return;
  }
  const coefficients = {a: Number(raw.a), b: Number(raw.b), c: Number(raw.c)};
  showSolution(coefficients, payload.data);
}

elements.form.addEventListener('submit', handleSubmit);
elements.form.requestSubmit();
