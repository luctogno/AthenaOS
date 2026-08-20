const $ = (id) => document.getElementById(id);
let wifiOn = true;

function toast(msg) {
  const el = $("toast");
  el.textContent = msg;
  el.hidden = false;
  setTimeout(() => { el.hidden = true; }, 1800);
}

function post(url, body) {
  return fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body
  });
}

function setText(el, v) {
  if (!el) return;
  const s = v == null ? "" : String(v);
  if (el.textContent !== s) el.textContent = s;
}

function setHtml(el, html) {
  if (!el) return;
  if (el.dataset.html === html) return;
  el.dataset.html = html;
  el.innerHTML = html;
}

function showTab() {
  const tab = (location.hash || "#apps").slice(1);
  ["apps", "settings", "install", "serial"].forEach((id) => {
    const el = $(id);
    if (el) el.hidden = id !== tab;
  });
  document.querySelectorAll("nav a").forEach((a) => {
    a.classList.toggle("on", a.getAttribute("href") === "#" + tab);
  });
}

async function refresh() {
  const j = await (await fetch("/api/status")).json();
  const s = j.system || {};
  const st = j.settings || {};
  wifiOn = !!st.wifiOn;
  const connected = !!(s.wifi && s.wifi.connected);
  const wifiEl = $("wifiState");
  setText(wifiEl, !wifiOn ? "WiFi off" : (connected ? ("Connected: " + (s.wifi.ssid || "")) : "Not connected"));
  if (wifiEl) wifiEl.style.color = !wifiOn ? "" : (connected ? "var(--cyan)" : "var(--err)");
  const guide = $("wifiGuide");
  if (!wifiOn || connected) {
    guide.hidden = true;
  } else {
    const ap = s.ap || {};
    guide.hidden = false;
    setHtml(guide, "Join hotspot <b>" + (ap.ssid || "AthenaOS") +
      "</b> then open <a href='/setup'>/setup</a> (" + (ap.url || "") + ")");
  }
  setText($("meta"), (s.ip || "") + " · " + Math.floor((s.uptimeMs || 0) / 1000) + "s");
  setHtml($("sys"), [
    s.board, (s.width || "") + "x" + (s.height || ""),
    (s.wifi && s.wifi.connected) ? (s.wifi.ssid || "wifi") : "wifi off",
    s.currentApp || "-"
  ].map((v) => "<span>" + v + "</span>").join(""));
  setHtml($("appRows"), (j.apps || []).map((a) => {
    const kill = a.killable ? "<button class='kill' data-kill='" + a.id + "'>Kill</button>" : "";
    return "<tr><td>" + a.name + "<br><small>" + a.id + "</small></td><td>" +
      (a.current ? "foreground" : a.state) +
      "</td><td><button data-open='" + a.id + "'>Open</button>" + kill + "</td></tr>";
  }).join(""));
  const vol = $("vol");
  if (document.activeElement !== vol && vol.value !== String(st.volume)) vol.value = st.volume;
  setText($("volVal"), st.volume);
  setText($("wifiBtn"), wifiOn ? "On" : "Off");
  if (document.activeElement !== $("ssid") && $("ssid").value !== (st.ssid || "")) {
    $("ssid").value = st.ssid || "";
  }
  if ($("tz") && document.activeElement !== $("tz") && $("tz").value !== (st.tz || "Rome")) {
    $("tz").value = st.tz || "Rome";
  }
  if ($("ntpBtn")) setText($("ntpBtn"), st.ntpOn ? "On" : "Off");
  if ($("ntpState")) {
    if (!st.ntpOn) setText($("ntpState"), "NTP off");
    else if (st.ntpSynced) setText($("ntpState"), "NTP synced · " + (st.tz || "Rome"));
    else setText($("ntpState"), "NTP waiting (needs WiFi)");
  }
}

document.body.addEventListener("click", async (e) => {
  const t = e.target;
  if (t.dataset.open) {
    await post("/api/apps/open", "id=" + encodeURIComponent(t.dataset.open));
    toast("opened");
    refresh();
  }
  if (t.dataset.kill) {
    await post("/api/apps/kill", "id=" + encodeURIComponent(t.dataset.kill));
    toast("killed");
    refresh();
  }
});

$("vol").oninput = () => { setText($("volVal"), $("vol").value); };
$("vol").onchange = async () => {
  await post("/api/settings", "volume=" + $("vol").value);
  toast("volume saved");
};
$("wifiBtn").onclick = async () => {
  await post("/api/settings", "wifiOn=" + (wifiOn ? "0" : "1"));
  toast(wifiOn ? "wifi off" : "wifi on");
  refresh();
};
$("wifiSave").onclick = async () => {
  await post("/api/settings",
    "ssid=" + encodeURIComponent($("ssid").value) +
    "&password=" + encodeURIComponent($("pass").value));
  $("pass").value = "";
  toast("wifi saved");
};
$("tz").onchange = async () => {
  await post("/api/settings", "tz=" + encodeURIComponent($("tz").value));
  toast("timezone saved");
  refresh();
};
$("ntpBtn").onclick = async () => {
  await post("/api/settings", "ntpOn=" + (stNtpOn() ? "0" : "1"));
  toast("ntp saved");
  refresh();
};
function stNtpOn() {
  return $("ntpBtn").textContent === "On";
}
$("reboot").onclick = async () => {
  if (!confirm("Restart now?")) return;
  await post("/api/restart", "");
};
$("poweroff").onclick = async () => {
  if (!confirm("Power off?")) return;
  await post("/api/poweroff", "");
};
const pkg = $("pkg");
if (pkg) {
  pkg.onchange = async (e) => {
    const f = e.target.files[0];
    if (!f) return;
    const r = await fetch("/api/apps/install", { method: "POST", body: new FormData() });
    const j = await r.json();
    toast(j.hint || j.error || "install");
  };
}

window.addEventListener("hashchange", () => {
  showTab();
  refreshSerial();
});
showTab();
refresh();
refreshSerial();
setInterval(refresh, 2000);

async function refreshSerial() {
  if ((location.hash || "") !== "#serial") return;
  const t = await (await fetch("/api/log")).text();
  const el = $("serialOut");
  if (el.textContent === t) return;
  const atEnd = el.scrollHeight - el.scrollTop - el.clientHeight < 40;
  el.textContent = t;
  if (atEnd) el.scrollTop = el.scrollHeight;
}
$("serial").addEventListener("click", refreshSerial);
setInterval(refreshSerial, 1000);
