import json
import pathlib
import threading
import urllib.error
import urllib.parse
import urllib.request

import pytest

from quadratic import server

WEB_ROOT = pathlib.Path(__file__).resolve().parents[1] / "web"


@pytest.fixture(name="base_url")
def fixture_base_url():
    listener = server.create_server("127.0.0.1", 0, WEB_ROOT)
    thread = threading.Thread(target=listener.serve_forever, daemon=True)
    thread.start()
    yield f"http://127.0.0.1:{listener.server_address[1]}"
    listener.shutdown()
    listener.server_close()
    thread.join()


def _get(base_url, path):
    with urllib.request.urlopen(f"{base_url}{path}") as response:
        content_type = response.headers["Content-Type"]
        return response.status, content_type, response.read()


def test_solve_payload_returns_description_and_plot():
    status, body = server.solve_payload({"a": "1", "b": "-3", "c": "2"})
    assert status == 200
    assert body["equation"] == "x^2 - 3x + 2 = 0"
    assert body["plot"].startswith("<svg")


def test_solve_payload_rejects_invalid_coefficients():
    status, body = server.solve_payload({"a": "0", "b": "1", "c": "2"})
    assert status == 400
    assert "must not be zero" in body["error"]


def test_create_server_requires_an_index_page(tmp_path):
    with pytest.raises(FileNotFoundError):
        server.create_server("127.0.0.1", 0, tmp_path)


def test_root_serves_the_page(base_url):
    status, content_type, body = _get(base_url, "/")
    assert status == 200
    assert content_type.startswith("text/html")
    assert b"<h1>Quadratic calculator</h1>" in body


def test_stylesheet_is_served_with_its_type(base_url):
    _, content_type, _ = _get(base_url, "/styles/main.css")
    assert content_type.startswith("text/css")


def test_solve_endpoint_returns_json(base_url):
    query = urllib.parse.urlencode({"a": "1", "b": "0", "c": "-4"})
    _, content_type, body = _get(base_url, f"/api/solve?{query}")
    payload = json.loads(body)
    assert content_type.startswith("application/json")
    assert set(payload["roots"]) == {"x = 2", "x = -2"}


def test_solve_endpoint_reports_bad_input(base_url):
    with pytest.raises(urllib.error.HTTPError) as caught:
        _get(base_url, "/api/solve?a=1&b=2")
    assert caught.value.code == 400
    assert "Enter a value for c." in json.loads(caught.value.read())["error"]


def test_unknown_path_is_not_found(base_url):
    with pytest.raises(urllib.error.HTTPError) as caught:
        _get(base_url, "/nope.css")
    assert caught.value.code == 404


def test_path_traversal_is_refused(base_url):
    with pytest.raises(urllib.error.HTTPError) as caught:
        _get(base_url, "/../src/quadratic/server.py")
    assert caught.value.code == 404
