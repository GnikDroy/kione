#!/usr/bin/env python3
"""Subset the Material Symbols icon font to just the glyphs the editor references.
The full font is downloaded fresh each run, so only the subset lives in the repo.
"""

import glob
import os
import re
import sys
import tempfile
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONT_URL = (
    "https://raw.githubusercontent.com/google/material-design-icons/master/"
    "variablefont/MaterialSymbolsOutlined%5BFILL%2CGRAD%2Copsz%2Cwght%5D.ttf"
)
OUT = os.path.join(ROOT, "editor", "res", "fonts", "material-symbols-outlined.ttf")
INSTANCE = {"FILL": 0, "wght": 400, "GRAD": 0, "opsz": 24}


def used_codepoints():
    tokens = set()
    for base in ("src", "editor"):
        for dirpath, _, files in os.walk(os.path.join(ROOT, base)):
            for name in files:
                if name.endswith((".cpp", ".hpp")):
                    with open(
                        os.path.join(dirpath, name), encoding="utf-8", errors="ignore"
                    ) as f:
                        tokens.update(re.findall(r"ICON_MS_[A-Z0-9_]+", f.read()))

    headers = glob.glob(
        os.path.join(ROOT, "build", "_deps", "*", "IconsMaterialSymbols.h")
    )
    if not headers:
        sys.exit(
            "IconsMaterialSymbols.h not found under build/_deps; configure the build first."
        )
    header = open(headers[0], encoding="utf-8").read()

    codepoints = set()
    for token in sorted(tokens):
        match = re.search(
            r"#define %s\b.*?U\+([0-9a-fA-F]+)" % re.escape(token), header
        )
        if match:
            codepoints.add(int(match.group(1), 16))
        else:
            print(f"warning: no codepoint found for {token}")
    return sorted(codepoints)


def main():
    from fontTools import subset
    from fontTools.ttLib import TTFont
    from fontTools.varLib.instancer import instantiateVariableFont

    codepoints = used_codepoints()
    print(f"keeping {len(codepoints)} glyphs")

    with tempfile.NamedTemporaryFile(suffix=".ttf", delete=False) as tmp:
        print("downloading full variable font ...")
        urllib.request.urlretrieve(FONT_URL, tmp.name)
        source = tmp.name
    before = os.path.getsize(source)

    font = TTFont(source)
    instantiateVariableFont(font, INSTANCE, inplace=True)
    subsetter = subset.Subsetter()
    subsetter.populate(unicodes=codepoints)
    subsetter.subset(font)
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    font.save(OUT)
    os.unlink(source)

    print(f"{before // 1024} KB -> {os.path.getsize(OUT) // 1024} KB  ({OUT})")


if __name__ == "__main__":
    main()
