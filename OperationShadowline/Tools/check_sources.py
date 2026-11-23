#!/usr/bin/env python3
"""
Offline sanity checks for the Operation Shadowline sources.

This is NOT a compiler. It cannot see Unreal's headers, so it cannot tell you whether an
engine API call is correct. What it does catch is the class of mistake that costs the most
time on a first build, and that is detectable from the text alone:

  * a reflected type with no matching .generated.h include, or one that is not last
  * UCLASS / USTRUCT without GENERATED_BODY()
  * a header with no #pragma once
  * unbalanced braces, parens or brackets (comment- and string-aware)
  * a project #include that points at a file which does not exist
  * a .cpp that does not include its own header
  * a UPROPERTY / UFUNCTION macro not followed by a declaration
  * malformed .ini lines, and invalid .uproject JSON

Run from the project root:  python3 Tools/check_sources.py
Exit code 0 = clean, 1 = problems found.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "Source"
CONFIG = ROOT / "Config"

problems: list[str] = []
notes: list[str] = []


def fail(path: Path, line: int | None, message: str) -> None:
    where = f"{path.relative_to(ROOT)}"
    if line is not None:
        where += f":{line}"
    problems.append(f"{where}: {message}")


def strip_code(text: str) -> str:
    """Blank out comments and string/char literals, preserving newlines and length."""
    out = []
    i, n = 0, len(text)
    state = None  # None | 'line' | 'block' | 'str' | 'chr'
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""

        if state is None:
            if c == "/" and nxt == "/":
                state, i = "line", i + 2
                out.append("  ")
                continue
            if c == "/" and nxt == "*":
                state, i = "block", i + 2
                out.append("  ")
                continue
            if c == '"':
                state, i = "str", i + 1
                out.append(" ")
                continue
            if c == "'":
                state, i = "chr", i + 1
                out.append(" ")
                continue
            out.append(c)
            i += 1
            continue

        if state == "line":
            if c == "\n":
                state = None
                out.append("\n")
            else:
                out.append(" ")
            i += 1
            continue

        if state == "block":
            if c == "*" and nxt == "/":
                state, i = None, i + 2
                out.append("  ")
                continue
            out.append("\n" if c == "\n" else " ")
            i += 1
            continue

        # inside a string or char literal
        if c == "\\":
            out.append("  ")
            i += 2
            continue
        if (state == "str" and c == '"') or (state == "chr" and c == "'"):
            state = None
            out.append(" ")
            i += 1
            continue
        out.append("\n" if c == "\n" else " ")
        i += 1

    return "".join(out)


def check_balance(path: Path, code: str) -> None:
    pairs = {"}": "{", ")": "(", "]": "["}
    stack: list[tuple[str, int]] = []
    line = 1
    for ch in code:
        if ch == "\n":
            line += 1
        elif ch in "{([":
            stack.append((ch, line))
        elif ch in ")}]":
            if not stack:
                fail(path, line, f"unmatched closing '{ch}'")
                return
            opener, opened_at = stack.pop()
            if opener != pairs[ch]:
                fail(path, line, f"'{ch}' closes '{opener}' opened at line {opened_at}")
                return
    if stack:
        opener, opened_at = stack[-1]
        fail(path, opened_at, f"'{opener}' is never closed")


REFLECT_RE = re.compile(r"^\s*(UCLASS|USTRUCT|UENUM|UINTERFACE|UDELEGATE)\s*\(", re.M)
INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)
TYPE_DECL_RE = re.compile(
    r"^\s*(?:class|struct)\s+(?:[A-Z_]+_API\s+)?([A-Za-z_]\w*)", re.M
)


def module_include_roots() -> list[Path]:
    # PublicIncludePaths adds the module folder itself, so includes are written
    # relative to Source/OperationShadowline (e.g. "Player/ShadowlineCharacter.h").
    roots = [SOURCE]
    for mod in SOURCE.iterdir():
        if mod.is_dir():
            roots.append(mod)
    return roots


def check_header(path: Path, text: str, code: str) -> None:
    if "#pragma once" not in text:
        fail(path, 1, "header has no '#pragma once'")

    includes = INCLUDE_RE.findall(text)
    has_reflection = bool(REFLECT_RE.search(code))
    generated = f"{path.stem}.generated.h"

    if has_reflection:
        if generated not in includes:
            fail(path, None, f"declares reflected types but does not include \"{generated}\"")
        elif includes[-1] != generated:
            fail(
                path,
                None,
                f"\"{generated}\" must be the LAST include (currently followed by "
                f"{includes[includes.index(generated) + 1]!r})",
            )
    elif generated in includes:
        fail(path, None, f"includes \"{generated}\" but declares no reflected type")

    # Every UCLASS/USTRUCT needs GENERATED_BODY(). UENUM does not.
    lines = code.splitlines()
    for idx, line in enumerate(lines):
        m = re.match(r"\s*(UCLASS|USTRUCT)\s*\(", line)
        if not m:
            continue
        window = "\n".join(lines[idx : idx + 14])
        if "GENERATED_BODY()" not in window and "GENERATED_UCLASS_BODY()" not in window:
            fail(path, idx + 1, f"{m.group(1)} with no GENERATED_BODY() within 14 lines")

    # A UPROPERTY/UFUNCTION must be followed by an actual declaration.
    for idx, line in enumerate(lines):
        if not re.match(r"\s*(UPROPERTY|UFUNCTION)\s*\(", line):
            continue
        # skip continuation lines of a multi-line macro
        j = idx
        depth = 0
        while j < len(lines):
            depth += lines[j].count("(") - lines[j].count(")")
            if depth <= 0:
                break
            j += 1
        nxt = next((l for l in lines[j + 1 :] if l.strip()), "")
        if not nxt.strip() or re.match(r"\s*(UPROPERTY|UFUNCTION|UCLASS|USTRUCT)\s*\(", nxt):
            fail(path, idx + 1, "UPROPERTY/UFUNCTION is not followed by a declaration")


def check_includes_resolve(path: Path, text: str) -> None:
    roots = module_include_roots()
    for inc in INCLUDE_RE.findall(text):
        if inc.endswith(".generated.h"):
            continue
        # Only validate includes that look like they belong to this project.
        stem = Path(inc).name
        if not stem.startswith("Shadowline") and not stem.startswith("OperationShadowline"):
            continue
        if any((root / inc).is_file() for root in roots):
            continue
        # Also allow a bare filename resolving anywhere in the module.
        if any(p.name == stem for p in SOURCE.rglob("*.h")):
            fail(path, None, f'include "{inc}" exists as {stem} but not at that path')
        else:
            fail(path, None, f'include "{inc}" does not resolve to any file in Source/')


def main() -> int:
    headers = sorted(SOURCE.rglob("*.h"))
    sources = sorted(SOURCE.rglob("*.cpp"))

    if not headers:
        problems.append("no headers found under Source/ - wrong working directory?")
        return 1

    for path in headers + sources:
        text = path.read_text(encoding="utf-8")
        code = strip_code(text)
        check_balance(path, code)
        check_includes_resolve(path, text)
        if path.suffix == ".h":
            check_header(path, text, code)
        else:
            own = f"{path.stem}.h"
            includes = [Path(i).name for i in INCLUDE_RE.findall(text)]
            if own not in includes:
                # Module impl .cpp legitimately includes the module header instead.
                if path.stem != path.parent.name:
                    fail(path, None, f'does not include its own header "{own}"')

    # Reflected class names should match their file, so UHT output lands where expected.
    for path in headers:
        code = strip_code(path.read_text(encoding="utf-8"))
        if not REFLECT_RE.search(code):
            continue
        decls = TYPE_DECL_RE.findall(code)
        # Strip the leading U/A/F/I prefix Unreal requires on reflected types.
        bare = {re.sub(r"^[UAFIE]", "", d) for d in decls}
        if path.stem not in bare and decls:
            notes.append(
                f"{path.relative_to(ROOT)}: no reflected type matches the filename "
                f"(found {sorted(decls)}) - fine for a types-only header, otherwise check it"
            )

    # Config files.
    for ini in sorted(CONFIG.glob("*.ini")):
        section = None
        for n, raw in enumerate(ini.read_text(encoding="utf-8").splitlines(), 1):
            line = raw.strip()
            if not line or line.startswith((";", "#")):
                continue
            if line.startswith("["):
                if not line.endswith("]"):
                    fail(ini, n, f"malformed section header: {line!r}")
                section = line
                continue
            if section is None:
                fail(ini, n, f"key outside any section: {line!r}")
            if "=" not in line:
                fail(ini, n, f"no '=' in setting line: {line!r}")

    # .uproject
    for up in sorted(ROOT.glob("*.uproject")):
        try:
            data = json.loads(up.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            fail(up, exc.lineno, f"invalid JSON: {exc.msg}")
            continue
        for key in ("FileVersion", "EngineAssociation", "Modules"):
            if key not in data:
                fail(up, None, f"missing required key {key!r}")
        for mod in data.get("Modules", []):
            if not (SOURCE / mod["Name"]).is_dir():
                fail(up, None, f"module {mod['Name']!r} has no Source/{mod['Name']}/ folder")
            if not (SOURCE / mod["Name"] / f"{mod['Name']}.Build.cs").is_file():
                fail(up, None, f"module {mod['Name']!r} has no Build.cs")

    print(f"Checked {len(headers)} headers, {len(sources)} sources, "
          f"{len(list(CONFIG.glob('*.ini')))} config files.\n")

    for note in notes:
        print(f"  note: {note}")
    if notes:
        print()

    if problems:
        print(f"{len(problems)} problem(s):\n")
        for p in problems:
            print(f"  ERROR {p}")
        print("\nThese are text-level problems only. A clean run does NOT mean the")
        print("project compiles - only a real engine build can tell you that.")
        return 1

    print("No text-level problems found.")
    print("\nIMPORTANT: this is not a compile. The sources have never been built against")
    print("Unreal's headers, so engine API errors are still expected on first build.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
