#!/usr/bin/env python3
"""Bootstrap-safe docs linter for the stable docs surface."""

from __future__ import annotations

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parent.parent
DOCS_DIR = ROOT / "docs"
LANG_DIRS = [ROOT / "docs" / "en", ROOT / "docs" / "ru"]
NUMBERED_RE = re.compile(r"^\d{2}-[a-z0-9-]+\.md$")
BOX_DRAWING_RE = re.compile(r"[\u2500-\u257f]")
MERMAID_START = "```mermaid"
FENCE = "```"


def fail(msg: str) -> None:
    print(f"FAIL: {msg}")
    raise SystemExit(1)


def lint_numbered_doc(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines()

    if "\t" in text:
        fail(f"{path}: contains tab characters")

    if re.search(r"[ \t]+$", text, flags=re.MULTILINE):
        fail(f"{path}: contains trailing whitespace")

    if BOX_DRAWING_RE.search(text):
        fail(f"{path}: contains box-drawing characters; use Mermaid for diagrams")

    h1_count = sum(1 for line in lines if line.startswith("# "))
    if h1_count != 1:
        fail(f"{path}: expected exactly one H1, found {h1_count}")

    fence_stack: list[str] = []
    mermaid_blocks = 0
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line.startswith(FENCE):
            if fence_stack:
                fence_stack.pop()
            else:
                fence_stack.append(line)
                if line == MERMAID_START:
                    mermaid_blocks += 1
                    j = i + 1
                    while j < len(lines) and not lines[j].strip():
                        j += 1
                    if j >= len(lines) or lines[j].strip().startswith(FENCE):
                        fail(f"{path}: empty Mermaid block")
            i += 1
            continue
        i += 1

    if fence_stack:
        fail(f"{path}: unbalanced fenced code blocks")

    if mermaid_blocks == 0:
        fail(f"{path}: expected at least one Mermaid diagram")


def main() -> int:
    if not DOCS_DIR.is_dir():
        fail("docs directory is missing")

    required_dirs = [
        DOCS_DIR / "plans",
        DOCS_DIR / "rfc",
        DOCS_DIR / "tmp",
    ]
    for path in required_dirs:
        if not path.exists():
            fail(f"{path}: required bootstrap docs path is missing")

    if not all(lang_dir.is_dir() for lang_dir in LANG_DIRS):
        print("SKIP: stable docs surface (docs/en and docs/ru) is not initialized yet")
        return 0

    if all(not any(lang_dir.iterdir()) for lang_dir in LANG_DIRS):
        print("SKIP: stable docs surface exists but numbered docs are not initialized yet")
        return 0

    expected_sets: list[list[str]] = []
    min_numbered_docs = 1

    for lang_dir in LANG_DIRS:
        numbered = sorted(p.name for p in lang_dir.iterdir() if p.is_file() and NUMBERED_RE.match(p.name))
        if len(numbered) < min_numbered_docs:
            fail(f"{lang_dir}: expected at least {min_numbered_docs} numbered docs, found {len(numbered)}")

        expected_sets.append(numbered)
        for name in numbered:
            lint_numbered_doc(lang_dir / name)

    if expected_sets[0] != expected_sets[1]:
        fail("docs/en and docs/ru numbered docs do not match")

    print("PASS: numbered docs structure and Mermaid fences look valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
