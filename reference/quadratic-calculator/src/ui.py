"""Tkinter front end for the quadratic calculator."""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

import coefficients
import plotting
import solver

WINDOW_TITLE = "Quadratic Equation Calculator"
FIGURE_SIZE = (8, 5)
FIGURE_DPI = 100
ENTRY_WIDTH = 10
ERROR_COLOUR = "#c53030"
RESULT_COLOUR = "#1a202c"


class CalculatorWindow:
    """Main window: three coefficient entries, a result line, and a plot."""

    def __init__(self, root: tk.Tk) -> None:
        self._root = root
        self._root.title(WINDOW_TITLE)
        self._entries: dict[str, ttk.Entry] = {}

        self._build_input_row()
        self._result = ttk.Label(root, text="Enter coefficients and press Solve.",
                                 foreground=RESULT_COLOUR, font=("TkDefaultFont", 11))
        self._result.pack(pady=(4, 8))

        self._figure = Figure(figsize=FIGURE_SIZE, dpi=FIGURE_DPI)
        self._axes = self._figure.add_subplot(111)
        self._canvas = FigureCanvasTkAgg(self._figure, master=root)
        self._canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def _build_input_row(self) -> None:
        """Creates the coefficient entries and the Solve button."""
        frame = ttk.Frame(self._root, padding=12)
        frame.pack()
        ttk.Label(frame, text="a·x² + b·x + c = 0").grid(row=0, column=0, columnspan=6,
                                                         pady=(0, 8))
        for index, name in enumerate(coefficients.COEFFICIENT_NAMES):
            ttk.Label(frame, text=f"{name} =").grid(row=1, column=index * 2, padx=(8, 2))
            entry = ttk.Entry(frame, width=ENTRY_WIDTH, justify="right")
            entry.grid(row=1, column=index * 2 + 1)
            entry.bind("<Return>", lambda _event: self._on_solve())
            self._entries[name] = entry

        ttk.Button(frame, text="Solve & Plot", command=self._on_solve).grid(
            row=1, column=6, padx=(16, 0))

    def _raw_inputs(self) -> tuple[str, str, str]:
        """Returns the current text of the a, b, and c entries."""
        return tuple(self._entries[name].get() for name in coefficients.COEFFICIENT_NAMES)

    def _show_error(self, message: str) -> None:
        """Displays `message` in place of a result and clears the plot."""
        self._result.config(text=message, foreground=ERROR_COLOUR)
        self._axes.clear()
        self._canvas.draw()

    def _show_solution(self, equation: solver.Quadratic, solution: solver.Solution) -> None:
        """Displays the root summary and redraws the parabola."""
        self._result.config(text=solver.describe(solution), foreground=RESULT_COLOUR)
        plotting.draw(self._axes, equation, solution)
        self._canvas.draw()

    def _on_solve(self) -> None:
        """Validates the entries, then solves and plots, or reports why it cannot."""
        raw_a, raw_b, raw_c = self._raw_inputs()
        try:
            equation = coefficients.parse(raw_a, raw_b, raw_c)
        except coefficients.CoefficientError as error:
            self._show_error(str(error))
            return

        self._show_solution(equation, solver.solve(equation))


def run() -> int:
    """Opens the calculator window and blocks until it closes. Returns an exit code."""
    root = tk.Tk()
    CalculatorWindow(root)
    root.mainloop()
    return 0
