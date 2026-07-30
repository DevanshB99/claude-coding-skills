import re
import xml.etree.ElementTree

from quadratic import plotting
from quadratic import solver

OPTIONS = plotting.PlotOptions()


def _svg(coefficients):
    return plotting.render_svg(solver.solve(coefficients))


def test_render_svg_is_well_formed_xml():
    root = xml.etree.ElementTree.fromstring(_svg(solver.Coefficients(1, -3, 2)))
    assert root.tag.endswith("svg")
    assert root.get("viewBox") == f"0 0 {OPTIONS.width} {OPTIONS.height}"


def test_render_svg_marks_both_real_roots_and_the_vertex():
    markup = _svg(solver.Coefficients(1, -3, 2))
    assert markup.count('class="plot-root"') == 2
    assert markup.count('class="plot-vertex"') == 1


def test_render_svg_omits_markers_for_complex_roots():
    markup = _svg(solver.Coefficients(1, 0, 4))
    assert 'class="plot-root"' not in markup
    assert 'class="plot-vertex"' in markup


def test_curve_points_stay_inside_the_drawing_area():
    markup = _svg(solver.Coefficients(2, 5, -3))
    points = re.search(r'class="plot-curve" points="([^"]+)"', markup).group(1)
    pairs = [pair.split(",") for pair in points.split(" ")]
    assert len(pairs) == OPTIONS.sample_count
    low = OPTIONS.margin - 1
    for x, y in pairs:
        assert low <= float(x) <= OPTIONS.width - OPTIONS.margin + 1
        assert low <= float(y) <= OPTIONS.height - OPTIONS.margin + 1


def test_options_change_the_canvas_size():
    solution = solver.solve(solver.Coefficients(1, 0, -1))
    options = plotting.PlotOptions(width=200, height=100)
    markup = plotting.render_svg(solution, options)
    assert 'viewBox="0 0 200 100"' in markup


def test_label_describes_the_equation():
    assert 'aria-label="Parabola of x^2 - 3x + 2 = 0"' in _svg(
        solver.Coefficients(1, -3, 2)
    )
