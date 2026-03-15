#!/usr/bin/env python3
"""RFC scraper for the iojournal logging library project.

Searches the IETF Datatracker API for RFCs and active Internet-Drafts relevant
to syslog, observability, JSON, and timestamps, then emits a structured
registry for `docs/rfc/`.

Recommended workflow:
    python3 scripts/rfc-scraper.py -o docs/rfc/registry.md
    python3 scripts/rfc-scraper.py --download docs/rfc
    python3 scripts/rfc-scraper.py --download-all docs/rfc
"""

from __future__ import annotations

import argparse
import json
import logging
import sys
import time
from collections import defaultdict
from dataclasses import dataclass, field
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import requests
from requests.adapters import HTTPAdapter, Retry

log = logging.getLogger("rfc-scraper")

# -- Datatracker API ----------------------------------------------------------

DATATRACKER_BASE = "https://datatracker.ietf.org"
DATATRACKER_API = f"{DATATRACKER_BASE}/api/v1"
RFC_EDITOR_BASE = "https://www.rfc-editor.org"
USER_AGENT = "iojournal-rfc-scraper/1.0"
REQUEST_TIMEOUT = 15
RATE_LIMIT_DELAY = 0.3

@dataclass
class RfcEntry:
    number: int
    title: str
    status: str = ""
    pages: int | None = None
    categories: set[str] = field(default_factory=set)

    @property
    def url(self) -> str:
        return f"{RFC_EDITOR_BASE}/rfc/rfc{self.number}"

    @property
    def txt_url(self) -> str:
        return f"{RFC_EDITOR_BASE}/rfc/rfc{self.number}.txt"

@dataclass
class DraftEntry:
    name: str
    title: str
    status: str = "ACTIVE DRAFT"
    rev: str = ""
    categories: set[str] = field(default_factory=set)

    @property
    def url(self) -> str:
        return f"{DATATRACKER_BASE}/doc/{self.name}/"

CATEGORIES: dict[str, list[str]] = {
    "Syslog": [
        "Syslog",
        "Syslog Protocol",
        "Syslog Message",
    ],
    "Observability & Tracing": [
        "OpenTelemetry",
        "Trace Context",
    ],
    "JSON & Formatting": [
        "JSON",
        "JSON Lines",
    ],
    "Time": [
        "Date and Time on the Internet",
    ]
}

KNOWN_CRITICAL: dict[int, tuple[str, str]] = {
    # Syslog
    5424: ("The Syslog Protocol", "PROPOSED STANDARD"),
    5425: ("Transport Layer Security (TLS) Transport Mapping for Syslog", "PROPOSED STANDARD"),
    5426: ("Transmission of Syslog Messages over UDP", "PROPOSED STANDARD"),
    6587: ("Transmission of Syslog Messages over TCP", "PROPOSED STANDARD"),
    3164: ("The BSD syslog Protocol", "INFORMATIONAL"),
    # Timestamps
    3339: ("Date and Time on the Internet: Timestamps", "PROPOSED STANDARD"),
    # JSON
    8259: ("The JavaScript Object Notation (JSON) Data Interchange Format", "INTERNET STANDARD"),
}

IMPORTANT_DRAFTS: dict[str, str] = {}

CRITICAL_GROUPS: dict[str, list[int]] = {
    "Syslog Family": [5424, 5425, 5426, 6587, 3164],
    "Data & Time Formats": [8259, 3339],
}

PROTOCOL_MATRIX: dict[str, list[str]] = {
    "Message Encoding": [
        "RFC 5424 (Syslog Message Format)",
        "RFC 8259 (JSON Data Interchange Format)",
        "RFC 3339 (Date and Time on the Internet)",
    ],
    "Transports": [
        "RFC 5426 (Syslog over UDP)",
        "RFC 6587 (Syslog over TCP)",
        "RFC 5425 (Syslog over TLS)",
    ],
}

HIGH_RELEVANCE_KEYWORDS: list[str] = ["syslog", "logging", "tracing", "opentelemetry"]
MEDIUM_RELEVANCE_KEYWORDS: list[str] = ["json", "timestamp", "tls", "udp", "tcp"]
DRAFT_RELEVANCE_KEYWORDS: list[str] = ["syslog", "telemetry"]

def _create_session() -> requests.Session:
    session = requests.Session()
    session.headers["User-Agent"] = USER_AGENT
    adapter = HTTPAdapter(
        max_retries=Retry(
            total=3,
            backoff_factor=1,
            status_forcelist=[429, 500, 502, 503, 504],
        ),
    )
    session.mount("https://", adapter)
    return session

def _parse_rfc_number(name: str) -> int | None:
    if name.startswith("rfc"):
        try:
            return int(name[3:])
        except ValueError:
            pass
    return None

def _today_utc_iso() -> str:
    return datetime.now(tz=timezone.utc).date().isoformat()

def search_datatracker_rfcs(session: requests.Session, query: str, *, max_results: int = 10) -> list[dict[str, Any]]:
    params: dict[str, str | int] = {
        "title__icontains": query,
        "type__slug": "rfc",
        "limit": max_results,
        "format": "json",
    }
    try:
        resp = session.get(f"{DATATRACKER_API}/doc/document/", params=params, timeout=REQUEST_TIMEOUT)
        resp.raise_for_status()
        data = resp.json()
        results = []
        for obj in data.get("objects", []):
            rfc_num = _parse_rfc_number(obj.get("name", ""))
            if rfc_num is not None:
                results.append({
                    "rfc": rfc_num,
                    "title": obj.get("title", ""),
                    "status": obj.get("std_level", ""),
                    "pages": obj.get("pages"),
                })
        return results
    except requests.RequestException as exc:
        log.warning("Datatracker RFC search failed for %r: %s", query, exc)
        return []

def search_datatracker_drafts(session: requests.Session, query: str, *, max_results: int = 5) -> list[dict[str, Any]]:
    params: dict[str, str | int] = {
        "title__icontains": query,
        "type__slug": "draft",
        "expires__gt": _today_utc_iso(),
        "limit": max_results,
        "format": "json",
    }
    try:
        resp = session.get(f"{DATATRACKER_API}/doc/document/", params=params, timeout=REQUEST_TIMEOUT)
        resp.raise_for_status()
        data = resp.json()
        return [{
            "name": obj.get("name", ""),
            "title": obj.get("title", ""),
            "rev": obj.get("rev", ""),
        } for obj in data.get("objects", [])]
    except requests.RequestException as exc:
        log.warning("Datatracker draft search failed for %r: %s", query, exc)
        return []

def download_rfc_txt(session: requests.Session, rfc_num: int, dest_dir: Path) -> Path | None:
    dest = dest_dir / f"rfc{rfc_num}.txt"
    if dest.exists():
        log.debug("Already exists: %s", dest)
        return dest
    url = f"{RFC_EDITOR_BASE}/rfc/rfc{rfc_num}.txt"
    try:
        resp = session.get(url, timeout=30)
        resp.raise_for_status()
        dest.write_bytes(resp.content)
        log.info("Downloaded rfc%d (%d bytes)", rfc_num, len(resp.content))
        return dest
    except requests.RequestException as exc:
        log.warning("Failed to download rfc%d: %s", rfc_num, exc)
        return None

def collect_rfcs(session: requests.Session) -> tuple[dict[int, RfcEntry], dict[str, DraftEntry]]:
    rfcs: dict[int, RfcEntry] = {}
    drafts: dict[str, DraftEntry] = {}
    for cat_name, queries in CATEGORIES.items():
        for query in queries:
            for r in search_datatracker_rfcs(session, query):
                num = r["rfc"]
                if num not in rfcs:
                    rfcs[num] = RfcEntry(number=num, title=r["title"], status=r.get("status", ""), pages=r.get("pages"))
                rfcs[num].categories.add(cat_name)
            for d in search_datatracker_drafts(session, query):
                name = d["name"]
                if name not in drafts:
                    drafts[name] = DraftEntry(name=name, title=d["title"], rev=d.get("rev", ""))
                drafts[name].categories.add(cat_name)
            time.sleep(RATE_LIMIT_DELAY)
    for rfc_num, (title, status) in KNOWN_CRITICAL.items():
        if rfc_num not in rfcs:
            rfcs[rfc_num] = RfcEntry(number=rfc_num, title=title, status=status)
        rfcs[rfc_num].categories.add("Curated critical")
    return rfcs, drafts

def relevance_score(entry: RfcEntry) -> int:
    score = 0
    title_lower = entry.title.lower()
    for kw in HIGH_RELEVANCE_KEYWORDS:
        if kw in title_lower: score += 10
    for kw in MEDIUM_RELEVANCE_KEYWORDS:
        if kw in title_lower: score += 5
    if entry.number in KNOWN_CRITICAL: score += 20
    status_lower = entry.status.lower() if entry.status else ""
    if "standard" in status_lower: score += 3
    elif "proposed" in status_lower: score += 2
    return score

def _status_label(rfc_num: int, entry: RfcEntry) -> str:
    if rfc_num in KNOWN_CRITICAL: return KNOWN_CRITICAL[rfc_num][1]
    status = entry.status
    if isinstance(status, str) and "/" in status:
        return status.rsplit("/", maxsplit=1)[-1].strip()
    return status or ""

def generate_markdown(rfcs: dict[int, RfcEntry], drafts: dict[str, DraftEntry]) -> str:
    now = datetime.now(tz=timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    lines: list[str] = []
    w = lines.append
    w("# iojournal Logging Library — RFC Registry\n")
    w(f"**Generated**: {now}")
    w(f"**RFCs found**: {len(rfcs)}")
    w(f"**Active Internet-Drafts found**: {len(drafts)}")
    w("")
    status_counts: dict[str, int] = defaultdict(int)
    for entry in rfcs.values():
        label = (entry.status or "UNKNOWN").upper()
        if "/" in label: label = label.rsplit("/", maxsplit=1)[-1].strip()
        status_counts[label] += 1
    w("---\n## Statistics\n")
    for status, count in sorted(status_counts.items(), key=lambda x: -x[1]):
        w(f"- **{status}**: {count}")
    w("\n---\n## Critical RFCs (must-implement)\n")
    for group_name, rfc_nums in CRITICAL_GROUPS.items():
        w(f"\n### {group_name}\n")
        w("| RFC | Title | Status | Relevance |")
        w("|-----|-------|--------|-----------|")
        for num in rfc_nums:
            entry = rfcs.get(num)
            title = entry.title if entry else KNOWN_CRITICAL.get(num, ("?",))[0]
            status = _status_label(num, entry) if entry else KNOWN_CRITICAL.get(num, ("", ""))[1]
            score = relevance_score(entry) if entry else 0
            w(f"| [{num}]({RFC_EDITOR_BASE}/rfc/rfc{num}) | {title} | {status} | {score} |")
    critical_set: set[int] = set()
    for nums in CRITICAL_GROUPS.values(): critical_set.update(nums)
    scored = sorted(((relevance_score(e), e) for e in rfcs.values()), key=lambda x: (-x[0], x[1].number))
    w("\n---\n## High-relevance RFCs (not in critical list)\n")
    w("| RFC | Title | Status | Score |")
    w("|-----|-------|--------|-------|")
    count = 0
    for score, entry in scored:
        if entry.number in critical_set or score < 5: continue
        status = _status_label(entry.number, entry)
        w(f"| [{entry.number}]({entry.url}) | {entry.title[:80]} | {status} | {score} |")
        count += 1
        if count >= 60: break
    w("\n---\n## Protocol-to-RFC matrix for iojournal\n")
    for component, rfc_refs in PROTOCOL_MATRIX.items():
        w(f"\n### {component}\n")
        for ref in rfc_refs:
            w(f"- {ref}")
    return "\n".join(lines)

def generate_json(rfcs: dict[int, RfcEntry], drafts: dict[str, DraftEntry]) -> str:
    data = {
        "generated": datetime.now(tz=timezone.utc).isoformat(),
        "rfcs": {num: {"title": e.title, "status": e.status, "pages": e.pages, "categories": sorted(e.categories), "relevance": relevance_score(e), "url": e.url} for num, e in sorted(rfcs.items())},
        "drafts": {e.name: {"title": e.title, "rev": e.rev, "categories": sorted(e.categories), "url": e.url} for e in sorted(drafts.values(), key=lambda x: x.name)},
    }
    return json.dumps(data, indent=2, ensure_ascii=False)

def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate or refresh the iojournal RFC registry and local RFC text mirror.",
        epilog=(
            "Typical usage:\n"
            "  python3 scripts/rfc-scraper.py -o docs/rfc/registry.md\n"
            "  python3 scripts/rfc-scraper.py --download docs/rfc\n"
            "  python3 scripts/rfc-scraper.py --download-all docs/rfc"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("-o", "--output", type=Path, default=None)
    parser.add_argument("--json", action="store_true", dest="json_output")
    parser.add_argument("--download", type=Path, default=None, metavar="DIR")
    parser.add_argument("--download-all", type=Path, default=None, metavar="DIR")
    parser.add_argument("--skip-search", action="store_true")
    parser.add_argument("-v", "--verbose", action="count", default=0)
    return parser.parse_args(argv)

def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    level = logging.WARNING
    if args.verbose >= 2: level = logging.DEBUG
    elif args.verbose >= 1: level = logging.INFO
    logging.basicConfig(level=level, format="%(levelname)-5s %(message)s", stream=sys.stderr)
    session = _create_session()
    if args.skip_search:
        rfcs: dict[int, RfcEntry] = {}
        for rfc_num, (title, status) in KNOWN_CRITICAL.items():
            rfcs[rfc_num] = RfcEntry(number=rfc_num, title=title, status=status, categories={"Curated critical"})
        drafts: dict[str, DraftEntry] = {}
    else:
        rfcs, drafts = collect_rfcs(session)
    download_dir = args.download_all or args.download
    if download_dir is not None:
        download_dir.mkdir(parents=True, exist_ok=True)
        nums_to_download = sorted(rfcs.keys()) if args.download_all else sorted(KNOWN_CRITICAL.keys())
        for rfc_num in nums_to_download:
            download_rfc_txt(session, rfc_num, download_dir)
            time.sleep(RATE_LIMIT_DELAY)
    output = generate_json(rfcs, drafts) if args.json_output else generate_markdown(rfcs, drafts)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(output, encoding="utf-8")
    else:
        sys.stdout.write(output + "\n")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
