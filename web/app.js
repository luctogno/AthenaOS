const $ = (id) => document.getElementById(id);
let wifiOn = true;
let apOn = false;
let ntpOn = true;
let lang = "en";

const I18N = {
  en: {
    navApps: "Apps", navSettings: "Settings", navInstall: "Install", navSerial: "Serial",
    thApp: "App", thState: "State", volume: "Volume", language: "Language", wifi: "WiFi",
    ssid: "SSID", password: "Password", passKeep: "leave empty to keep", wifiSave: "Save WiFi",
    setupLink: "Open WiFi setup", timezone: "Timezone", ntp: "NTP", reboot: "Restart",
    poweroff: "Power off", installHint: "Sideload is not available yet. API:",
    on: "On", off: "Off", wifiOff: "WiFi off", connected: "Connected: ",
    notConnected: "Not connected", wifiOffChip: "wifi off",
    hotspot: "Hotspot", hotspotOn: "Hotspot on: ", hotspotOff: "Hotspot off",
    wifiGuidePre: "Join hotspot ", wifiGuideMid: " then open ", wifiGuideEnd: "",
    foreground: "foreground", open: "Open", kill: "Kill",
    ntpOff: "NTP off", ntpOk: "NTP synced · ", ntpWait: "NTP waiting (needs WiFi)",
    opened: "opened", killed: "killed", volSaved: "volume saved", wifiSaved: "wifi saved",
    apSaved: "hotspot saved", tzSaved: "timezone saved", ntpSaved: "ntp saved", langSaved: "language saved",
    restartQ: "Restart now?", powerQ: "Power off?"
  },
  it: {
    navApps: "App", navSettings: "Impostazioni", navInstall: "Installa", navSerial: "Seriale",
    thApp: "App", thState: "Stato", volume: "Volume", language: "Lingua", wifi: "WiFi",
    ssid: "SSID", password: "Password", passKeep: "vuoto per mantenere", wifiSave: "Salva WiFi",
    setupLink: "Apri setup WiFi", timezone: "Fuso orario", ntp: "NTP", reboot: "Riavvia",
    poweroff: "Spegni", installHint: "Sideload non disponibile. API:",
    on: "On", off: "Off", wifiOff: "WiFi spento", connected: "Connesso: ",
    notConnected: "Non connesso", wifiOffChip: "wifi off",
    hotspot: "Hotspot", hotspotOn: "Hotspot attivo: ", hotspotOff: "Hotspot spento",
    wifiGuidePre: "Collegati all'hotspot ", wifiGuideMid: " poi apri ", wifiGuideEnd: "",
    foreground: "in primo piano", open: "Apri", kill: "Chiudi",
    ntpOff: "NTP spento", ntpOk: "NTP sincronizzato · ", ntpWait: "NTP in attesa (serve WiFi)",
    opened: "aperta", killed: "chiusa", volSaved: "volume salvato", wifiSaved: "wifi salvato",
    apSaved: "hotspot salvato", tzSaved: "fuso salvato", ntpSaved: "ntp salvato", langSaved: "lingua salvata",
    restartQ: "Riavviare ora?", powerQ: "Spegnere ora?"
  }
};

function t(key) {
  return (I18N[lang] && I18N[lang][key]) || I18N.en[key] || key;
}

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

function formatUptime(ms) {
  let sec = Math.max(0, Math.floor((ms || 0) / 1000));
  const d = Math.floor(sec / 86400);
  sec %= 86400;
  const h = Math.floor(sec / 3600);
  sec %= 3600;
  const m = Math.floor(sec / 60);
  sec %= 60;
  const parts = [];
  if (d) parts.push(d + "d");
  if (d || h) parts.push(h + "h");
  if (d || h || m) parts.push(m + "m");
  parts.push(sec + "s");
  return parts.join(" ");
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

function applyStatic() {
  document.documentElement.lang = lang;
  document.querySelectorAll("[data-i18n]").forEach((el) => {
    setText(el, t(el.getAttribute("data-i18n")));
  });
  if ($("pass")) $("pass").placeholder = t("passKeep");
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
  const nextLang = st.lang === "it" ? "it" : "en";
  if (nextLang !== lang) {
    lang = nextLang;
    applyStatic();
  }
  if ($("lang") && document.activeElement !== $("lang") && $("lang").value !== lang) {
    $("lang").value = lang;
  }
  wifiOn = !!st.wifiOn;
  ntpOn = !!st.ntpOn;
  const connected = !!(s.wifi && s.wifi.connected);
  const ap = s.ap || {};
  apOn = !!ap.active;
  const wifiEl = $("wifiState");
  setText(wifiEl, !wifiOn ? t("wifiOff") : (connected ? (t("connected") + (s.wifi.ssid || "")) : t("notConnected")));
  if (wifiEl) wifiEl.style.color = !wifiOn ? "" : (connected ? "var(--cyan)" : "var(--err)");
  setText($("apBtn"), apOn ? t("on") : t("off"));
  if ($("apBtn")) $("apBtn").disabled = !wifiOn;
  setText($("apState"), !wifiOn ? t("wifiOff") : (apOn ? (t("hotspotOn") + (ap.ssid || "AthenaOS")) : t("hotspotOff")));
  const guide = $("wifiGuide");
  if (!wifiOn || !apOn) {
    guide.hidden = true;
  } else {
    guide.hidden = false;
    setHtml(guide, t("wifiGuidePre") + "<b>" + (ap.ssid || "AthenaOS") +
      "</b>" + t("wifiGuideMid") + "<a href='/setup'>/setup</a> (" + (ap.url || "") + ")" + t("wifiGuideEnd"));
  }
  setText($("meta"), (s.ip || "") + " · " + formatUptime(s.uptimeMs));
  setHtml($("sys"), [
    s.board, (s.width || "") + "x" + (s.height || ""),
    (s.wifi && s.wifi.connected) ? (s.wifi.ssid || "wifi") : t("wifiOffChip"),
    s.currentApp || "-"
  ].map((v) => "<span>" + v + "</span>").join(""));
  setHtml($("appRows"), (j.apps || []).map((a) => {
    const kill = a.killable ? "<button class='kill' data-kill='" + a.id + "'>" + t("kill") + "</button>" : "";
    return "<tr><td>" + a.name + "<br><small>" + a.id + "</small></td><td>" +
      (a.current ? t("foreground") : a.state) +
      "</td><td><button data-open='" + a.id + "'>" + t("open") + "</button>" + kill + "</td></tr>";
  }).join(""));
  const vol = $("vol");
  if (document.activeElement !== vol && vol.value !== String(st.volume)) vol.value = st.volume;
  setText($("volVal"), st.volume);
  setText($("wifiBtn"), wifiOn ? t("on") : t("off"));
  if (document.activeElement !== $("ssid") && $("ssid").value !== (st.ssid || "")) {
    $("ssid").value = st.ssid || "";
  }
  if ($("tz") && document.activeElement !== $("tz") && $("tz").value !== (st.tz || "Rome")) {
    $("tz").value = st.tz || "Rome";
  }
  if ($("ntpBtn")) setText($("ntpBtn"), ntpOn ? t("on") : t("off"));
  if ($("ntpState")) {
    if (!ntpOn) setText($("ntpState"), t("ntpOff"));
    else if (st.ntpSynced) setText($("ntpState"), t("ntpOk") + (st.tz || "Rome"));
    else setText($("ntpState"), t("ntpWait"));
  }
}

document.body.addEventListener("click", async (e) => {
  const el = e.target;
  if (el.dataset.open) {
    await post("/api/apps/open", "id=" + encodeURIComponent(el.dataset.open));
    toast(t("opened"));
    refresh();
  }
  if (el.dataset.kill) {
    await post("/api/apps/kill", "id=" + encodeURIComponent(el.dataset.kill));
    toast(t("killed"));
    refresh();
  }
});

$("vol").oninput = () => { setText($("volVal"), $("vol").value); };
$("vol").onchange = async () => {
  await post("/api/settings", "volume=" + $("vol").value);
  toast(t("volSaved"));
};
$("wifiBtn").onclick = async () => {
  await post("/api/settings", "wifiOn=" + (wifiOn ? "0" : "1"));
  toast(wifiOn ? t("wifiOff") : t("on"));
  refresh();
};
$("apBtn").onclick = async () => {
  await post("/api/settings", "apOn=" + (apOn ? "0" : "1"));
  toast(t("apSaved"));
  refresh();
};
$("wifiSave").onclick = async () => {
  await post("/api/settings",
    "ssid=" + encodeURIComponent($("ssid").value) +
    "&password=" + encodeURIComponent($("pass").value));
  $("pass").value = "";
  toast(t("wifiSaved"));
};
$("lang").onchange = async () => {
  await post("/api/settings", "lang=" + encodeURIComponent($("lang").value));
  toast(t("langSaved"));
  refresh();
};
$("tz").onchange = async () => {
  await post("/api/settings", "tz=" + encodeURIComponent($("tz").value));
  toast(t("tzSaved"));
  refresh();
};
$("ntpBtn").onclick = async () => {
  await post("/api/settings", "ntpOn=" + (ntpOn ? "0" : "1"));
  toast(t("ntpSaved"));
  refresh();
};
$("reboot").onclick = async () => {
  if (!confirm(t("restartQ"))) return;
  await post("/api/restart", "");
};
$("poweroff").onclick = async () => {
  if (!confirm(t("powerQ"))) return;
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
  const log = await (await fetch("/api/log")).text();
  const el = $("serialOut");
  if (el.textContent === log) return;
  const atEnd = el.scrollHeight - el.scrollTop - el.clientHeight < 40;
  el.textContent = log;
  if (atEnd) el.scrollTop = el.scrollHeight;
}
$("serial").addEventListener("click", refreshSerial);
setInterval(refreshSerial, 1000);
