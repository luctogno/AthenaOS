from pathlib import Path

root = Path.cwd()
try:
    Import("env")
    root = Path(str(env["PROJECT_DIR"]))
except Exception:
    pass

web = root / "web"
out = root / "firmware" / "include" / "web_embed.h"


def pack(symbol, text, delim):
    marker = ")" + delim
    if marker in text:
        raise RuntimeError("delimiter %s found in %s" % (delim, symbol))
    return 'static const char %s[] PROGMEM = R"%s(%s)%s";\n\n' % (
        symbol, delim, text, delim)


html = (web / "index.html").read_text(encoding="utf-8")
css = (web / "style.css").read_text(encoding="utf-8")
js = (web / "app.js").read_text(encoding="utf-8")
setupHtml = (web / "setup.html").read_text(encoding="utf-8")
setupJs = (web / "setup.js").read_text(encoding="utf-8")
body = (
    "#ifndef ATHENAOS_WEB_EMBED_H\n"
    "#define ATHENAOS_WEB_EMBED_H\n"
    "#include <Arduino.h>\n\n"
    + pack("WEB_INDEX_HTML", html, "IDX")
    + pack("WEB_STYLE_CSS", css, "CSS")
    + pack("WEB_APP_JS", js, "JS0")
    + pack("WEB_SETUP_HTML", setupHtml, "SET")
    + pack("WEB_SETUP_JS", setupJs, "SJS")
    + "#endif\n"
)
if not out.exists() or out.read_text(encoding="utf-8") != body:
    out.write_text(body, encoding="utf-8")
