"""Terminal front end: solve and plot a quadratic as ASCII art.

Run:
    python3 cli.py 1 -3 2
"""

import argparse

import solver

_WIDTH = 71
_HEIGHT = 21


def render_ascii(solution: solver.Solution, width: int = _WIDTH, height: int = _HEIGHT) -> str:
    """Draw the parabola on a character grid, with axes and root markers."""
    curve = solver.sample_curve(solution, points=width)
    x_min, x_max = curve["x_min"], curve["x_max"]
    y_min, y_max = curve["y_min"], curve["y_max"]

    grid = [[" "] * width for _ in range(height)]

    def row_of(y):
        fraction = (y_max - y) / (y_max - y_min)
        return min(height - 1, max(0, round(fraction * (height - 1))))

    def column_of(x):
        fraction = (x - x_min) / (x_max - x_min)
        return min(width - 1, max(0, round(fraction * (width - 1))))

    if y_min <= 0 <= y_max:
        grid[row_of(0)] = ["-"] * width
    if x_min <= 0 <= x_max:
        axis_column = column_of(0)
        for row in range(height):
            grid[row][axis_column] = "|" if grid[row][axis_column] == " " else "+"

    for x, y in zip(curve["xs"], curve["ys"]):
        if y_min <= y <= y_max:
            grid[row_of(y)][column_of(x)] = "*"

    vertex_x, vertex_y = solution.vertex
    grid[row_of(vertex_y)][column_of(vertex_x)] = "V"
    for root in solution.roots:
        if root.is_real:
            grid[row_of(0.0)][column_of(root.real)] = "O"

    frame = "+" + "-" * width + "+"
    body = "\n".join("|" + "".join(row) + "|" for row in grid)
    footer = (
        f"x from {x_min:.3g} to {x_max:.3g}   * curve   O root   V vertex"
    )
    return f"{frame}\n{body}\n{frame}\n{footer}"


def main() -> int:
    parser = argparse.ArgumentParser(description="Solve and plot ax^2 + bx + c = 0.")
    parser.add_argument("a", type=float, help="coefficient of x^2 (non-zero)")
    parser.add_argument("b", type=float, help="coefficient of x")
    parser.add_argument("c", type=float, help="constant term")
    parser.add_argument("--no-plot", action="store_true", help="print numbers only")
    args = parser.parse_args()

    if args.a == 0:
        parser.error("'a' must be non-zero — that is a linear equation.")

    solution = solver.solve(args.a, args.b, args.c)
    print(solver.format_equation(args.a, args.b, args.c))
    print()
    for step in solution.steps[1:]:
        print(f"  {step}")
    print()
    print(f"  Nature: {solution.nature}, opens {solution.opens}")
    print(f"  Roots:  {', '.join(root.format() for root in solution.roots)}")
    if not args.no_plot:
        print()
        print(render_ascii(solution))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
