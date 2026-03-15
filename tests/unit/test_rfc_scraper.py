import importlib.util
import re
import sys
import unittest
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve().parents[2] / "scripts" / "rfc-scraper.py"
SPEC = importlib.util.spec_from_file_location("rfc_scraper", SCRIPT_PATH)
assert SPEC is not None
assert SPEC.loader is not None
RFC_SCRAPER = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = RFC_SCRAPER
SPEC.loader.exec_module(RFC_SCRAPER)


class FakeResponse:
    def __init__(self, objects):
        self._payload = {"objects": objects}

    def raise_for_status(self):
        return None

    def json(self):
        return self._payload


class FakeSession:
    def __init__(self, objects):
        self.objects = objects
        self.calls = []

    def get(self, url, params=None, timeout=None):
        self.calls.append({
            "url": url,
            "params": params or {},
            "timeout": timeout,
        })
        return FakeResponse(self.objects)


class DatatrackerQueryTests(unittest.TestCase):
    def test_search_datatracker_rfcs_uses_slug_filter(self):
        session = FakeSession([
            {
                "name": "rfc5424",
                "title": "The Syslog Protocol",
                "std_level": "ps",
                "pages": 38,
            }
        ])

        RFC_SCRAPER.search_datatracker_rfcs(session, "Syslog")

        params = session.calls[0]["params"]
        self.assertEqual(params["type__slug"], "rfc")
        self.assertNotIn("type", params)
        self.assertNotIn("order_by", params)

    def test_search_datatracker_drafts_filters_for_unexpired_drafts(self):
        session = FakeSession([
            {
                "name": "draft-ietf-netmod-syslog-model",
                "title": "A YANG Data Model for Syslog Configuration",
                "rev": "33",
            }
        ])

        RFC_SCRAPER.search_datatracker_drafts(session, "Syslog")

        params = session.calls[0]["params"]
        self.assertEqual(params["type__slug"], "draft")
        self.assertRegex(params["expires__gt"], r"^\d{4}-\d{2}-\d{2}$")
        self.assertNotIn("states__slug__in", params)
        self.assertNotIn("order_by", params)


if __name__ == "__main__":
    unittest.main()
