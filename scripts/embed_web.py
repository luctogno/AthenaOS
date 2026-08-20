from pathlib import Path

root = Path.cwd()
try:
    Import("env")
    root = Path(str(env["PROJECT_DIR"]))
except Exception:
    pass

web = root / "web"
out_web = root / "firmware" / "include" / "web_embed.h"
out_splash = root / "firmware" / "include" / "splash_embed.h"
src_logo = root / "docs" / "assets" / "athenaos.jpg"
splash_jpg = root / "firmware" / "assets" / "splash.jpg"


def pack(symbol, text, delim):
    marker = ")" + delim
    if marker in text:
        raise RuntimeError("delimiter %s found in %s" % (delim, symbol))
    return 'static const char %s[] PROGMEM = R"%s(%s)%s";\n\n' % (
        symbol, delim, text, delim)


def pack_bin(symbol, data):
    lines = ["static const uint8_t %s[] PROGMEM = {\n" % symbol]
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        lines.append("  " + ", ".join("0x%02x" % b for b in chunk) + ",\n")
    lines.append("};\n")
    lines.append("static const size_t %s_LEN = sizeof(%s);\n\n" % (symbol, symbol))
    return "".join(lines)


def maybe_compress_splash():
    if not src_logo.exists():
        return
    try:
        from PIL import Image
    except ImportError:
        return
    if splash_jpg.exists() and splash_jpg.stat().st_mtime >= src_logo.stat().st_mtime:
        return
    splash_jpg.parent.mkdir(parents=True, exist_ok=True)
    img = Image.open(src_logo).convert("RGB")
    img = img.resize((368, 368), Image.Resampling.LANCZOS)
    img.save(splash_jpg, "JPEG", quality=48, optimize=True, progressive=False)


maybe_compress_splash()
if not splash_jpg.exists():
    raise RuntimeError("missing %s" % splash_jpg)

html = (web / "index.html").read_text(encoding="utf-8")
css = (web / "style.css").read_text(encoding="utf-8")
js = (web / "app.js").read_text(encoding="utf-8")
setupHtml = (web / "setup.html").read_text(encoding="utf-8")
web_body = (
    "#ifndef ATHENAOS_WEB_EMBED_H\n"
    "#define ATHENAOS_WEB_EMBED_H\n"
    "#include <Arduino.h>\n\n"
    + pack("WEB_INDEX_HTML", html, "IDX")
    + pack("WEB_STYLE_CSS", css, "CSS")
    + pack("WEB_APP_JS", js, "JS0")
    + pack("WEB_SETUP_HTML", setupHtml, "SET")
    + "#endif\n"
)
if not out_web.exists() or out_web.read_text(encoding="utf-8") != web_body:
    out_web.write_text(web_body, encoding="utf-8")

splash_body = (
    "#ifndef ATHENAOS_SPLASH_EMBED_H\n"
    "#define ATHENAOS_SPLASH_EMBED_H\n"
    "#include <Arduino.h>\n\n"
    + pack_bin("SPLASH_JPG", splash_jpg.read_bytes())
    + "#endif\n"
)
if not out_splash.exists() or out_splash.read_text(encoding="utf-8") != splash_body:
    out_splash.write_text(splash_body, encoding="utf-8")
