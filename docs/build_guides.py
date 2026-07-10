#!/usr/bin/env python3
"""Render the hand-written guide pages (docs/pages/*.md) into the modern
front-website style, as static HTML under docs/html/guides/.

This is the "pages we translate to html" half of the documentation, kept
deliberately separate from the Doxygen API reference (the "code docs"). Doxygen
owns the generated class/function reference under docs/html/reference/; this
script owns the narrative guides and styles them to match the landing page.

No third-party packages: standard library only. Run it *after* Doxygen so the
reference class pages exist to cross-link into.

    python docs/build_guides.py

Output: docs/html/guides/<slug>.html, one per guide, sharing docs/site assets.
"""

import html
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent           # docs/
PAGES_DIR = HERE / "pages"
OUT_DIR = HERE / "html" / "guides"
REFERENCE_DIR = HERE / "html" / "reference"

# --- guide ordering and metadata ---------------------------------------------
# (source markdown stem, url slug, sidebar title, one-line blurb)
GUIDES = [
    ("getting_started",    "getting-started",     "Getting started",     "Build the project and write your first game."),
    ("drawing",            "drawing",             "Drawing",             "Sprites, sheets, shapes and text."),
    ("input",              "input",               "Input",               "Reading the keyboard and mouse."),
    ("textures_and_fonts", "textures-and-fonts",  "Textures & fonts",    "Images, procedural textures, bitmap fonts."),
    ("utilities",          "utilities",           "Utilities",           "Math, rects, timers, random, noise, paths."),
    ("logging_and_data",   "logging-and-asserts", "Logging & asserts",   "Logging, asserts and fatal errors."),
    ("json",               "json",                "JSON files",          "Reading and writing json."),
    ("under_the_hood",     "under-the-hood",      "Under the hood",      "How the engine works inside."),
    ("guided_experiences", "guided-experiences",  "Guided experiences",  "Build small systems yourself."),
    ("camera_2d",          "camera-2d",           "A 2D camera",         "A guided exercise: scrolling camera."),
    ("sprite_animation",   "sprite-animation",    "Sprite animation",    "A guided exercise: animation helper."),
]

# Which guides belong to the "Guided experiences" group in the sidebar.
GUIDED = {"guided_experiences", "camera_2d", "sprite_animation"}

# Doxygen page id (@ref / @subpage target) -> url slug
PAGE_TO_SLUG = {f"page_{stem}": slug for stem, slug, _, _ in GUIDES}
STEM_TO_SLUG = {stem: slug for stem, slug, _, _ in GUIDES}


def build_symbol_map():
    """Map a buz:: symbol to its Doxygen reference page, using whatever class
    and struct pages actually exist. Members (buz::x::y) resolve to their type."""
    mapping = {}
    if not REFERENCE_DIR.is_dir():
        return mapping
    for f in REFERENCE_DIR.glob("*.html"):
        m = re.fullmatch(r"(class|struct)buz_1_1([a-z0-9_]+?)(?:-members)?", f.stem)
        if not m or f.stem.endswith("-members"):
            continue
        # doxygen doubles underscores in the mangled name
        name = m.group(2).replace("__", "_")
        mapping[f"buz::{name}"] = f.name
    return mapping


SYMBOLS = build_symbol_map()

# --- inline markdown ---------------------------------------------------------

_CODE_SPAN = re.compile(r"`([^`]+)`")
_LINK = re.compile(r"\[([^\]]+)\]\(([^)]+)\)")
_AUTOLINK = re.compile(r"<(https?://[^>\s]+)>")
_BOLD = re.compile(r"\*\*([^*]+)\*\*")
# single-asterisk emphasis, applied after bold so it never touches "**...**".
# Only "*...*" is treated as italic; "_..._" is left alone so snake_case
# identifiers in prose are not mangled.
_ITALIC = re.compile(r"\*([^*\n]+)\*")
_SYMBOL = re.compile(r"\bbuz::[A-Za-z_][A-Za-z0-9_:]*[A-Za-z0-9_]")
_DOXY_REF = re.compile(r"@(?:ref|subpage)\s+([A-Za-z0-9_]+)")


def symbol_link(sym):
    """Return an <a> for a buz:: symbol if it resolves to a reference page,
    else a plain <code> span."""
    lookup = sym
    if lookup not in SYMBOLS and "::" in sym:
        # trim a trailing member (buz::rect::from_center_size -> buz::rect)
        lookup = sym.rsplit("::", 1)[0]
    target = SYMBOLS.get(lookup)
    text = html.escape(sym)
    if target:
        return f'<a href="../reference/{target}"><code>{text}</code></a>'
    return f"<code>{text}</code>"


def inline(text):
    """Render inline markdown (code, links, bold, buz:: auto-links) to HTML."""
    # Resolve @ref / @subpage first so their bare page ids never leak through.
    def _ref(m):
        slug = PAGE_TO_SLUG.get(m.group(1))
        return f"@@LINK@@{slug}.html@@" if slug else m.group(1)
    text = _DOXY_REF.sub(_ref, text)

    # Pull code spans and links out so their contents are left untouched.
    stash = []

    def _stash(htmlfrag):
        stash.append(htmlfrag)
        return f"\x00{len(stash) - 1}\x00"

    text = _CODE_SPAN.sub(lambda m: _stash(f"<code>{html.escape(m.group(1))}</code>"), text)

    def _link(m):
        label, url = m.group(1), m.group(2)
        ext = url.startswith("http")
        attrs = ' target="_blank" rel="noopener"' if ext else ""
        return _stash(f'<a href="{html.escape(url, quote=True)}"{attrs}>{html.escape(label)}</a>')

    text = _LINK.sub(_link, text)

    # Bare angle-bracket autolinks: <https://example.com>
    text = _AUTOLINK.sub(
        lambda m: _stash(
            f'<a href="{html.escape(m.group(1), quote=True)}" target="_blank" rel="noopener">{html.escape(m.group(1))}</a>'
        ),
        text,
    )

    # Everything left is plain prose: escape, then auto-link and embolden it.
    text = html.escape(text)
    text = _SYMBOL.sub(lambda m: _stash(symbol_link(m.group(0))), text)
    text = _BOLD.sub(lambda m: f"<strong>{m.group(1)}</strong>", text)
    text = _ITALIC.sub(lambda m: f"<em>{m.group(1)}</em>", text)

    # Restore internal @ref links produced above.
    text = re.sub(r"@@LINK@@([^@]+)@@", lambda m: _stash(f'<a href="{m.group(1)}">the guide</a>'), text)

    # Splice the stashed fragments back in.
    return re.sub(r"\x00(\d+)\x00", lambda m: stash[int(m.group(1))], text)


# @ref/@subpage often appear as "@ref page_drawing for ..." mid-sentence; the
# generic handler above turns them into a bare "the guide" link, which reads
# badly. Replace with the guide's real title inline instead.
def inline_with_refs(text):
    def _named(m):
        slug = PAGE_TO_SLUG.get(m.group(1))
        if not slug:
            return m.group(0)
        title = next((t for _, s, t, _ in GUIDES if s == slug), slug)
        return f"[{title}]({slug}.html)"
    text = _DOXY_REF.sub(_named, text)
    return inline(text)


# --- C++ syntax highlighting -------------------------------------------------
# A small, self-contained tokenizer (no third-party highlighter). It reuses the
# tok-* colour classes already defined in docs/site/assets/site.css, the same
# ones the landing-page code card uses.

_CPP_LANGS = {"cpp", "c", "cc", "cxx", "h", "hpp", "hlsl"}

_CPP_KEYWORDS = {
    "alignas", "alignof", "and", "auto", "bool", "break", "case", "catch",
    "char", "char8_t", "char16_t", "char32_t", "class", "const", "consteval",
    "constexpr", "constinit", "const_cast", "continue", "decltype", "default",
    "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit",
    "export", "extern", "false", "float", "for", "friend", "goto", "if",
    "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
    "nullptr", "operator", "or", "override", "private", "protected", "public",
    "register", "reinterpret_cast", "return", "short", "signed", "sizeof",
    "static", "static_cast", "struct", "switch", "template", "this", "throw",
    "true", "try", "typedef", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while",
}

_CPP_TYPES = {
    "std", "buz", "size_t", "ptrdiff_t", "intptr_t", "uintptr_t",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "shared_ptr", "unique_ptr", "weak_ptr", "string", "wstring",
    "string_view", "vector", "array", "span", "map", "unordered_map",
    "optional", "function", "filesystem", "path", "json",
}

_HL_TOKEN = re.compile(
    r"(?P<com>//[^\n]*|/\*.*?\*/)"
    r"|(?P<str>\"(?:\\.|[^\"\\\n])*\"|'(?:\\.|[^'\\\n])*')"
    r"|(?P<pre>\#[A-Za-z_]+)"
    r"|(?P<num>\b0[xX][0-9A-Fa-f]+[uUlL]*|\b\d+\.?\d*(?:[eE][+-]?\d+)?[fFuUlL]*)"
    r"|(?P<id>[A-Za-z_]\w*)",
    re.S,
)


def highlight_cpp(code):
    out = []
    pos = 0
    for m in _HL_TOKEN.finditer(code):
        if m.start() > pos:
            out.append(html.escape(code[pos:m.start()]))
        kind = m.lastgroup
        text = m.group()
        cls = None
        if kind == "com":
            cls = "tok-com"
        elif kind == "str":
            cls = "tok-str"
        elif kind == "pre":
            cls = "tok-key"
        elif kind == "num":
            cls = "tok-num"
        elif kind == "id":
            if text in _CPP_KEYWORDS:
                cls = "tok-key"
            elif text in _CPP_TYPES:
                cls = "tok-type"
            else:
                j = m.end()
                while j < len(code) and code[j] in " \t":
                    j += 1
                if j < len(code) and code[j] == "(":
                    cls = "tok-fn"
        esc = html.escape(text)
        out.append(f'<span class="{cls}">{esc}</span>' if cls else esc)
        pos = m.end()
    out.append(html.escape(code[pos:]))
    return "".join(out)


_CMAKE_LANGS = {"cmake"}

# Argument keywords (not commands) that read well in blue.
_CMAKE_KEYWORDS = {
    "PRIVATE", "PUBLIC", "INTERFACE", "STATUS", "WARNING", "FATAL_ERROR",
    "PROPERTIES", "GLOB", "GLOB_RECURSE", "CONFIGURE_DEPENDS", "REQUIRED",
    "QUIET", "REQUIRED", "CACHE", "PARENT_SCOPE", "ON", "OFF", "TRUE", "FALSE",
    "AND", "OR", "NOT", "IF", "ELSE", "ENDIF", "FOREACH", "ENDFOREACH",
}

_CMAKE_TOKEN = re.compile(
    r"(?P<com>#[^\n]*)"
    r"|(?P<str>\"[^\"\n]*\")"
    r"|(?P<var>\$\{[^}\n]*\}|\$<[^>\n]*>|\$ENV\{[^}\n]*\})"
    r"|(?P<num>\b\d+\b)"
    r"|(?P<id>[A-Za-z_][A-Za-z0-9_]*)",
)


def highlight_cmake(code):
    out = []
    pos = 0
    for m in _CMAKE_TOKEN.finditer(code):
        if m.start() > pos:
            out.append(html.escape(code[pos:m.start()]))
        kind = m.lastgroup
        text = m.group()
        cls = None
        if kind == "com":
            cls = "tok-com"
        elif kind == "str":
            cls = "tok-str"
        elif kind == "var":
            cls = "tok-type"
        elif kind == "num":
            cls = "tok-num"
        elif kind == "id":
            if text in _CMAKE_KEYWORDS:
                cls = "tok-key"
            else:
                j = m.end()
                while j < len(code) and code[j] in " \t":
                    j += 1
                if j < len(code) and code[j] == "(":
                    cls = "tok-fn"  # command invocation
        esc = html.escape(text)
        out.append(f'<span class="{cls}">{esc}</span>' if cls else esc)
        pos = m.end()
    out.append(html.escape(code[pos:]))
    return "".join(out)


# --- block markdown ----------------------------------------------------------

def render_blocks(lines, base_indent=0):
    """Render a list of markdown lines (already de-indented to base_indent) to
    HTML. Recurses for list-item bodies."""
    out = []
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        stripped = line.strip()

        if not stripped:
            i += 1
            continue

        # fenced code block
        if stripped.startswith("```"):
            lang = stripped[3:].strip()
            body = []
            i += 1
            while i < n and not lines[i].strip().startswith("```"):
                body.append(lines[i])
                i += 1
            i += 1  # closing fence
            raw = "\n".join(_dedent(body))
            if lang in _CPP_LANGS:
                code = highlight_cpp(raw)
            elif lang in _CMAKE_LANGS:
                code = highlight_cmake(raw)
            else:
                code = html.escape(raw)
            cls = f' class="lang-{lang}"' if lang else ""
            btn = '<button class="copy" type="button" aria-label="Copy code to clipboard">Copy</button>'
            out.append(f'<pre class="code">{btn}<code{cls}>{code}</code></pre>')
            continue

        # heading
        m = re.match(r"(#{2,4})\s+(.*)", stripped)
        if m:
            level = len(m.group(1))
            htext = re.sub(r"\s*\{#[^}]+\}\s*$", "", m.group(2))
            out.append(f"<h{level}>{inline_with_refs(htext)}</h{level}>")
            i += 1
            continue

        # table
        if stripped.startswith("|") and i + 1 < n and re.match(r"^\s*\|?[\s:\-|]+\|?\s*$", lines[i + 1]):
            rows = []
            while i < n and lines[i].strip().startswith("|"):
                rows.append(lines[i].strip())
                i += 1
            out.append(_render_table(rows))
            continue

        # blockquote
        if stripped.startswith(">"):
            body = []
            while i < n and lines[i].strip().startswith(">"):
                body.append(re.sub(r"^\s*>\s?", "", lines[i]))
                i += 1
            inner = render_blocks(body)
            out.append(f'<blockquote class="note">{inner}</blockquote>')
            continue

        # list (unordered, ordered, task)
        lm = re.match(r"(\s*)([-*]|\d+\.)\s+(.*)", line)
        if lm:
            block, i = _collect_list(lines, i)
            out.append(block)
            continue

        # paragraph: gather until blank / block boundary
        para = []
        while i < n and lines[i].strip() and not _is_block_start(lines, i):
            para.append(lines[i].strip())
            i += 1
        out.append(f"<p>{inline_with_refs(' '.join(para))}</p>")

    return "\n".join(out)


def _is_block_start(lines, i):
    s = lines[i].strip()
    if s.startswith(("```", ">", "|")) or re.match(r"#{2,4}\s", s):
        return True
    if re.match(r"(\s*)([-*]|\d+\.)\s+", lines[i]):
        return True
    return False


def _collect_list(lines, i):
    """Parse a contiguous list starting at line i. Returns (html, next_index)."""
    n = len(lines)
    first = re.match(r"(\s*)([-*]|\d+\.)\s+", lines[i])
    indent = len(first.group(1))
    ordered = bool(re.match(r"\d+\.", first.group(2)))
    items = []          # list of (marker_text, body_lines)
    task = False

    while i < n:
        line = lines[i]
        m = re.match(r"(\s*)([-*]|\d+\.)\s+(.*)", line)
        if m and len(m.group(1)) == indent:
            body = [m.group(3)]
            i += 1
            # continuation lines: deeper indent or blank-then-deeper
            while i < n:
                if not lines[i].strip():
                    # keep a blank if more indented content follows
                    j = i + 1
                    while j < n and not lines[j].strip():
                        j += 1
                    if j < n and (len(lines[j]) - len(lines[j].lstrip())) > indent:
                        body.append("")
                        i += 1
                        continue
                    break
                cur_indent = len(lines[i]) - len(lines[i].lstrip())
                nxt = re.match(r"(\s*)([-*]|\d+\.)\s+", lines[i])
                if nxt and len(nxt.group(1)) == indent:
                    break  # next sibling
                if cur_indent > indent:
                    body.append(lines[i])
                    i += 1
                else:
                    break
            items.append(body)
        else:
            break

    # task list?
    lis = []
    for body in items:
        head = body[0]
        tm = re.match(r"\[([ xX])\]\s+(.*)", head)
        if tm:
            task = True
            checked = tm.group(1).lower() == "x"
            body = [tm.group(2)] + body[1:]
            box = f'<input type="checkbox" disabled{" checked" if checked else ""}> '
            lis.append(f"<li class=\"task\">{box}{_render_item_body(body)}</li>")
        else:
            lis.append(f"<li>{_render_item_body(body)}</li>")

    tag = "ol" if ordered and not task else "ul"
    cls = ' class="tasklist"' if task else ""
    return f"<{tag}{cls}>\n" + "\n".join(lis) + f"\n</{tag}>", i


def _render_item_body(body):
    """Render a list item body (lead line + any nested blocks). A body that is
    just one paragraph is emitted without <p> wrappers so simple bullets stay
    tight; multi-block items keep full block markup."""
    inner = render_blocks(_dedent(body)).strip()
    # Unwrap only when the body is exactly one paragraph and nothing else, so
    # simple bullets stay tight while multi-block items keep full markup.
    single = (inner.startswith("<p>") and inner.endswith("</p>")
              and inner.count("<p>") == 1
              and not re.search(r"<(pre|ul|ol|table|blockquote|h[1-6])\b", inner))
    return inner[3:-4] if single else inner


def _dedent(lines):
    """Strip the common leading indent from a block of lines (ignoring blanks)."""
    indents = [len(l) - len(l.lstrip()) for l in lines if l.strip()]
    cut = min(indents) if indents else 0
    return [l[cut:] if len(l) >= cut else l for l in lines]


def _render_table(rows):
    def cells(r):
        r = r.strip()
        # drop the outer pipes, then split on unescaped pipes only so a literal
        # "\|" inside a cell (e.g. `[-2022\|-2026]`) stays in one cell.
        if r.startswith("|"):
            r = r[1:]
        if r.endswith("|") and not r.endswith("\\|"):
            r = r[:-1]
        parts = re.split(r"(?<!\\)\|", r)
        return [c.strip().replace("\\|", "|") for c in parts]
    header = cells(rows[0])
    body = rows[2:]
    out = ["<div class=\"tablewrap\"><table>", "<thead><tr>"]
    out += [f"<th>{inline(c)}</th>" for c in header]
    out.append("</tr></thead><tbody>")
    for r in body:
        out.append("<tr>" + "".join(f"<td>{inline(c)}</td>" for c in cells(r)) + "</tr>")
    out.append("</tbody></table></div>")
    return "\n".join(out)


# --- page assembly -----------------------------------------------------------

def sidebar(active_stem):
    def link(stem, slug, title):
        cls = ' class="active"' if stem == active_stem else ""
        return f'<a href="{slug}.html"{cls}>{html.escape(title)}</a>'
    main = [link(s, sl, t) for s, sl, t, _ in GUIDES if s not in GUIDED]
    exp = [link(s, sl, t) for s, sl, t, _ in GUIDES if s in GUIDED]
    return f"""<nav class="side-nav">
  <div class="side-group">Guides</div>
  {''.join(main)}
  <div class="side-group">Guided experiences</div>
  {''.join(exp)}
</nav>"""


def nav_bar():
    return """<header class="nav">
  <div class="wrap">
    <a class="brand" href="../index.html">
      <svg class="mark" viewBox="0 0 32 32" fill="none" aria-hidden="true">
        <rect x="1.5" y="1.5" width="29" height="29" rx="8" stroke="url(#g)" stroke-width="2.2"/>
        <path d="M10 9h7a4 4 0 0 1 0 8h-7z M10 15h8a4 4 0 0 1 0 8h-8z" stroke="url(#g)" stroke-width="2.2" stroke-linejoin="round"/>
        <defs><linearGradient id="g" x1="0" y1="0" x2="32" y2="32"><stop stop-color="#1c86e0"/><stop offset="1" stop-color="#38bdf8"/></linearGradient></defs>
      </svg>
      BuzEngine
    </a>
    <nav class="nav-links">
      <a href="getting-started.html">Get started</a>
      <a href="getting-started.html" class="hide-sm">Guides</a>
      <a href="../reference/annotated.html">Reference</a>
      <a href="../about.html">About</a>
      <a class="ghbtn" href="https://github.com/dyronix/buz-engine">GitHub</a>
    </nav>
  </div>
</header>"""


def prev_next(idx):
    parts = []
    if idx > 0:
        s, sl, t, _ = GUIDES[idx - 1]
        parts.append(f'<a class="pn prev" href="{sl}.html"><span>&larr; Previous</span><strong>{html.escape(t)}</strong></a>')
    else:
        parts.append("<span></span>")
    if idx < len(GUIDES) - 1:
        s, sl, t, _ = GUIDES[idx + 1]
        parts.append(f'<a class="pn next" href="{sl}.html"><span>Next &rarr;</span><strong>{html.escape(t)}</strong></a>')
    return f'<div class="pager">{"".join(parts)}</div>'


PAGE = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title} BuzEngine guides</title>
<meta name="description" content="{blurb}">
<link rel="icon" href="../assets/favicon.svg" type="image/svg+xml">
<link rel="stylesheet" href="../assets/site.css">
</head>
<body class="guide-page">
{nav}
<div class="docwrap wrap">
  <aside class="side">{sidebar}</aside>
  <main class="doc prose">
    <div class="crumb"><a href="../index.html">Home</a> <span>/</span> <a href="getting-started.html">Guides</a> <span>/</span> {title}</div>
    <h1>{h1}</h1>
{content}
    {pager}
  </main>
</div>
<footer>
  <div class="wrap">
    <a class="brand" href="../index.html" style="font-size:1rem">BuzEngine</a>
    <a href="../reference/index.html">Reference</a>
    <a href="../about.html">About</a>
    <a href="https://github.com/dyronix/buz-engine">GitHub</a>
    <span class="sep">Windows 10/11 &middot; DirectX 12 &middot; MIT licensed</span>
  </div>
</footer>
<script src="../assets/site.js" defer></script>
</body>
</html>
"""


def render_guide(idx):
    stem, slug, title, blurb = GUIDES[idx]
    md = (PAGES_DIR / f"{stem}.md").read_text(encoding="utf-8").splitlines()

    # first heading is the page title (drop it; we render our own <h1>)
    h1 = title
    start = 0
    for k, l in enumerate(md):
        m = re.match(r"#\s+(.*)", l.strip())
        if m:
            h1 = re.sub(r"\s*\{#[^}]+\}\s*$", "", m.group(1))
            start = k + 1
            break

    content = render_blocks(md[start:])
    return PAGE.format(
        title=html.escape(title),
        blurb=html.escape(blurb),
        h1=inline_with_refs(h1),
        nav=nav_bar(),
        sidebar=sidebar(stem),
        content=content,
        pager=prev_next(idx),
    )


def main():
    if not PAGES_DIR.is_dir():
        print(f"guide source not found: {PAGES_DIR}", file=sys.stderr)
        return 1
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for idx, (stem, slug, _title, _blurb) in enumerate(GUIDES):
        src = PAGES_DIR / f"{stem}.md"
        if not src.exists():
            print(f"  skip (missing): {src.name}", file=sys.stderr)
            continue
        (OUT_DIR / f"{slug}.html").write_text(render_guide(idx), encoding="utf-8")
        print(f"  guides/{slug}.html")
    print(f"built {len(GUIDES)} guide pages -> {OUT_DIR}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
