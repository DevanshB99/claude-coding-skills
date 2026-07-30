'use strict';

const SOLVE_PATH = '/api/solve';

const form = document.getElementById('coefficients');
const notice = document.getElementById('notice');
const result = document.getElementById('result');
const plotPanel = document.getElementById('plot-panel');
const plotCanvas = document.getElementById('plot-canvas');

const fields = {
  equation: document.getElementById('result-equation'),
  roots: document.getElementById('result-roots'),
  discriminant: document.getElementById('result-discriminant'),
  vertex: document.getElementById('result-vertex'),
  axis: document.getElementById('result-axis'),
  direction: document.getElementById('result-direction'),
};

function showError(message) {
  notice.textContent = message;
  notice.hidden = false;
  result.hidden = true;
  plotPanel.hidden = true;
}

function showSolution(payload) {
  notice.hidden = true;
  fields.equation.textContent = payload.equation;
  fields.roots.textContent = payload.roots.join('   ');
  fields.discriminant.textContent = `${payload.discriminant} (${payload.nature})`;
  fields.vertex.textContent = payload.vertex;
  fields.axis.textContent = payload.axis_of_symmetry;
  fields.direction.textContent = payload.direction;
  plotCanvas.innerHTML = payload.plot;
  result.hidden = false;
  plotPanel.hidden = false;
}

async function requestSolution() {
  const query = new URLSearchParams(new FormData(form));
  const response = await fetch(`${SOLVE_PATH}?${query}`);
  const payload = await response.json();
  if (!response.ok) {
    showError(payload.error || 'The server could not solve that.');
    return;
  }
  showSolution(payload);
}

form.addEventListener('submit', (event) => {
  event.preventDefault();
  requestSolution().catch(() => showError('Could not reach the calculator.'));
});

requestSolution().catch(() => showError('Could not reach the calculator.'));
