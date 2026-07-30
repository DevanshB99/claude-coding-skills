/* Front end for the quadratic calculator. Python does the maths; this draws it. */

(function () {
  "use strict";

  var PAD = { top: 18, right: 18, bottom: 30, left: 52 };
  var form = document.getElementById("coefficients");
  var errorBox = document.getElementById("error");
  var preview = document.getElementById("equation-preview");
  var canvas = document.getElementById("chart");
  var tooltip = document.getElementById("tooltip");
  var state = null;
  var frame = null;

  /* ---------- helpers ---------- */

  function ink(name) {
    return getComputedStyle(document.documentElement).getPropertyValue(name).trim();
  }

  function num(value, places) {
    if (!isFinite(value)) return "—";
    var rounded = Number(value.toFixed(places === undefined ? 4 : places));
    return String(rounded === 0 ? 0 : rounded);
  }

  function readCoefficients() {
    return {
      a: form.elements.a.value,
      b: form.elements.b.value,
      c: form.elements.c.value
    };
  }

  function localEquation() {
    var v = readCoefficients();
    var a = parseFloat(v.a), b = parseFloat(v.b), c = parseFloat(v.c);
    if (!isFinite(a) || !isFinite(b) || !isFinite(c)) return "ax² + bx + c = 0";
    var head = a === 1 ? "x²" : a === -1 ? "−x²" : num(a) + "x²";
    var out = head;
    [[b, "x"], [c, ""]].forEach(function (pair) {
      var value = pair[0];
      if (value === 0) return;
      var mag = Math.abs(value);
      var shown = mag === 1 && pair[1] ? "" : num(mag);
      out += (value < 0 ? " − " : " + ") + shown + pair[1];
    });
    return out + " = 0";
  }

  function showError(message) {
    errorBox.textContent = message;
    errorBox.hidden = !message;
  }

  /* ---------- render ---------- */

  function renderResults(data) {
    var labels = data.roots.map(function (root) {
      return root.label.replace(/-/g, "−").replace(/\+\/-/g, "±");
    });
    document.getElementById("roots-hero").innerHTML =
      "x = " + labels.join('<span class="sep">,</span> ');
    document.getElementById("nature").textContent =
      data.nature.charAt(0).toUpperCase() + data.nature.slice(1) +
      "; parabola opens " + data.opens + ".";

    var stats = [
      ["Discriminant", num(data.discriminant)],
      ["Vertex", "(" + num(data.vertex.x) + ", " + num(data.vertex.y) + ")"],
      ["Axis of symmetry", "x = " + num(data.axisOfSymmetry)],
      ["y-intercept", num(data.yIntercept)]
    ];
    document.getElementById("stats").innerHTML = stats.map(function (row) {
      return "<div><dt>" + row[0] + "</dt><dd>" +
        String(row[1]).replace(/-/g, "−") + "</dd></div>";
    }).join("");

    document.getElementById("steps").innerHTML = data.steps.map(function (step) {
      return "<li>" + step
        .replace(/&/g, "&amp;").replace(/</g, "&lt;")
        .replace(/\^2/g, "²")
        .replace(/\+\/-/g, "±")
        .replace(/->/g, "→") + "</li>";
    }).join("");

    var realRoots = data.roots.filter(function (r) { return r.isReal; }).length;
    document.getElementById("axis-note").textContent = realRoots
      ? "The curve meets the x-axis at " + realRoots +
        (realRoots === 1 ? " point." : " points.")
      : "No real roots, so the curve never meets the x-axis.";

    var rows = [];
    for (var i = 0; i < data.curve.xs.length; i += 8) {
      rows.push("<tr><td>" + num(data.curve.xs[i], 3) +
        "</td><td>" + num(data.curve.ys[i], 3) + "</td></tr>");
    }
    document.querySelector("#points-table tbody").innerHTML = rows.join("");

    ["results", "plot-panel", "steps-panel"].forEach(function (id) {
      document.getElementById(id).hidden = false;
    });
  }

  /* ---------- plotting ---------- */

  function scales(data, width, height) {
    var c = data.curve;
    var plotW = width - PAD.left - PAD.right;
    var plotH = height - PAD.top - PAD.bottom;
    return {
      x: function (v) { return PAD.left + (v - c.x_min) / (c.x_max - c.x_min) * plotW; },
      y: function (v) { return PAD.top + (c.y_max - v) / (c.y_max - c.y_min) * plotH; },
      xInverse: function (px) {
        return c.x_min + (px - PAD.left) / plotW * (c.x_max - c.x_min);
      },
      plotW: plotW,
      plotH: plotH
    };
  }

  function ticks(min, max, count) {
    var raw = (max - min) / count;
    var magnitude = Math.pow(10, Math.floor(Math.log10(raw)));
    var step = [1, 2, 2.5, 5, 10].reduce(function (best, m) {
      return Math.abs(m * magnitude - raw) < Math.abs(best - raw) ? m * magnitude : best;
    }, magnitude);
    var out = [];
    for (var t = Math.ceil(min / step) * step; t <= max + step * 1e-9; t += step) {
      out.push(t);
    }
    return out;
  }

  function draw() {
    frame = null;
    if (!state) return;
    var ctx = canvas.getContext("2d");
    var ratio = window.devicePixelRatio || 1;
    var width = canvas.clientWidth;
    var height = canvas.clientHeight;
    canvas.width = Math.round(width * ratio);
    canvas.height = Math.round(height * ratio);
    ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
    ctx.clearRect(0, 0, width, height);

    var data = state.data;
    var s = scales(data, width, height);
    var c = data.curve;
    var surface = ink("--surface-1");

    ctx.font = '11px system-ui, -apple-system, "Segoe UI", sans-serif';
    ctx.lineJoin = "round";
    ctx.lineCap = "round";

    // Recessive grid + tick labels.
    var xTicks = ticks(c.x_min, c.x_max, 7);
    var yTicks = ticks(c.y_min, c.y_max, 6);
    ctx.strokeStyle = ink("--gridline");
    ctx.lineWidth = 1;
    ctx.fillStyle = ink("--muted");
    ctx.beginPath();
    xTicks.forEach(function (t) {
      var px = Math.round(s.x(t)) + 0.5;
      ctx.moveTo(px, PAD.top);
      ctx.lineTo(px, PAD.top + s.plotH);
    });
    yTicks.forEach(function (t) {
      var py = Math.round(s.y(t)) + 0.5;
      ctx.moveTo(PAD.left, py);
      ctx.lineTo(PAD.left + s.plotW, py);
    });
    ctx.stroke();

    ctx.textAlign = "center";
    ctx.textBaseline = "top";
    xTicks.forEach(function (t) {
      ctx.fillText(num(t, 2).replace("-", "−"), s.x(t), PAD.top + s.plotH + 7);
    });
    ctx.textAlign = "right";
    ctx.textBaseline = "middle";
    yTicks.forEach(function (t) {
      ctx.fillText(num(t, 2).replace("-", "−"), PAD.left - 8, s.y(t));
    });

    // Axes, only where they fall inside the window.
    ctx.strokeStyle = ink("--axis");
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    if (c.y_min <= 0 && c.y_max >= 0) {
      ctx.moveTo(PAD.left, s.y(0));
      ctx.lineTo(PAD.left + s.plotW, s.y(0));
    }
    if (c.x_min <= 0 && c.x_max >= 0) {
      ctx.moveTo(s.x(0), PAD.top);
      ctx.lineTo(s.x(0), PAD.top + s.plotH);
    }
    ctx.stroke();

    // The curve.
    ctx.strokeStyle = ink("--series-1");
    ctx.lineWidth = 2;
    ctx.beginPath();
    for (var i = 0; i < c.xs.length; i++) {
      var px = s.x(c.xs[i]);
      var py = s.y(c.ys[i]);
      if (i === 0) ctx.moveTo(px, py); else ctx.lineTo(px, py);
    }
    ctx.stroke();

    // Vertex, then real roots on top; 2px surface ring keeps overlaps legible.
    drawMarker(ctx, s.x(data.vertex.x), s.y(data.vertex.y), ink("--series-3"), surface);
    labelPoint(ctx, s, data.vertex.x, data.vertex.y, "vertex", ink("--series-3"), height);

    data.roots.filter(function (r) { return r.isReal; }).forEach(function (root) {
      drawMarker(ctx, s.x(root.real), s.y(0), ink("--series-2"), surface);
      labelPoint(ctx, s, root.real, 0, num(root.real, 3).replace("-", "−"),
        ink("--series-2"), height);
    });

    if (state.hover !== null) drawCrosshair(ctx, s, data, state.hover, height);
  }

  function drawMarker(ctx, px, py, colour, surface) {
    ctx.beginPath();
    ctx.arc(px, py, 5, 0, Math.PI * 2);
    ctx.fillStyle = colour;
    ctx.fill();
    ctx.strokeStyle = surface;
    ctx.lineWidth = 2;
    ctx.stroke();
  }

  // Direct labels in muted ink, never in the series colour.
  function labelPoint(ctx, s, x, y, text, colour, height) {
    var px = s.x(x);
    var py = s.y(y);
    ctx.fillStyle = ink("--text-secondary");
    ctx.textAlign = px > PAD.left + s.plotW - 60 ? "right" : "left";
    ctx.textBaseline = py > height - PAD.bottom - 26 ? "bottom" : "top";
    var dx = ctx.textAlign === "right" ? -9 : 9;
    var dy = ctx.textBaseline === "bottom" ? -9 : 9;
    ctx.fillText(text, px + dx, py + dy);
  }

  function drawCrosshair(ctx, s, data, index, height) {
    var c = data.curve;
    var px = s.x(c.xs[index]);
    var py = s.y(c.ys[index]);
    ctx.save();
    ctx.strokeStyle = ink("--axis");
    ctx.lineWidth = 1;
    ctx.setLineDash([3, 3]);
    ctx.beginPath();
    ctx.moveTo(px, PAD.top);
    ctx.lineTo(px, PAD.top + s.plotH);
    ctx.stroke();
    ctx.restore();
    drawMarker(ctx, px, py, ink("--series-1"), ink("--surface-1"));

    tooltip.hidden = false;
    tooltip.innerHTML = "x " + num(c.xs[index], 3).replace("-", "−") +
      "<br>y " + num(c.ys[index], 3).replace("-", "−");
    tooltip.style.left = px + "px";
    tooltip.style.top = Math.max(py, PAD.top + 8) + "px";
  }

  function schedule() {
    if (frame === null) frame = requestAnimationFrame(draw);
  }

  /* ---------- events ---------- */

  function solve(event) {
    if (event) event.preventDefault();
    showError("");
    fetch("/api/solve", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(readCoefficients())
    }).then(function (response) {
      return response.json().then(function (body) {
        if (!response.ok) throw new Error(body.error || "Request failed.");
        return body;
      });
    }).then(function (data) {
      state = { data: data, hover: null };
      preview.textContent = localEquation();
      renderResults(data);
      schedule();
    }).catch(function (err) {
      showError(err.message || "Could not reach the solver.");
    });
  }

  form.addEventListener("submit", solve);
  form.addEventListener("input", function () {
    preview.textContent = localEquation();
  });

  form.addEventListener("click", function (event) {
    var chip = event.target.closest("[data-preset]");
    if (!chip) return;
    var parts = chip.getAttribute("data-preset").split(",");
    form.elements.a.value = parts[0];
    form.elements.b.value = parts[1];
    form.elements.c.value = parts[2];
    preview.textContent = localEquation();
    solve();
  });

  canvas.addEventListener("pointermove", function (event) {
    if (!state) return;
    var rect = canvas.getBoundingClientRect();
    var x = state.data.curve;
    var s = scales(state.data, rect.width, rect.height);
    var value = s.xInverse(event.clientX - rect.left);
    var index = Math.round((value - x.x_min) / (x.x_max - x.x_min) * (x.xs.length - 1));
    index = Math.min(x.xs.length - 1, Math.max(0, index));
    if (index !== state.hover) { state.hover = index; schedule(); }
  });

  canvas.addEventListener("pointerleave", function () {
    if (!state) return;
    state.hover = null;
    tooltip.hidden = true;
    schedule();
  });

  window.addEventListener("resize", schedule);

  var toggle = document.getElementById("theme-toggle");
  var label = toggle.querySelector("[data-theme-label]");
  function currentlyDark() {
    var stamped = document.documentElement.getAttribute("data-theme");
    if (stamped) return stamped === "dark";
    return window.matchMedia("(prefers-color-scheme: dark)").matches;
  }
  function syncLabel() { label.textContent = currentlyDark() ? "Light" : "Dark"; }
  toggle.addEventListener("click", function () {
    document.documentElement.setAttribute("data-theme", currentlyDark() ? "light" : "dark");
    syncLabel();
    schedule();
  });
  syncLabel();

  solve();
})();
