const $ = (id) => document.getElementById(id);

function toast(msg) {
  const el = $("toast");
  el.textContent = msg;
  el.hidden = false;
  setTimeout(() => { el.hidden = true; }, 2200);
}

async function loadStatus() {
  const j = await (await fetch("/api/status")).json();
  const ap = j.system && j.system.ap;
  if (ap && ap.ssid) $("ap").textContent = ap.ssid;
  if (j.system && j.system.wifi && j.system.wifi.connected) {
    $("status").textContent = "Connected to " + (j.system.wifi.ssid || "WiFi");
  }
}

async function scan() {
  $("status").textContent = "Scanning...";
  let j = await (await fetch("/api/wifi/scan", { method: "POST" })).json();
  for (let i = 0; i < 12 && j.scanning; i++) {
    await new Promise((r) => setTimeout(r, 500));
    j = await (await fetch("/api/wifi/scan")).json();
  }
  $("status").textContent = "";
  $("nets").innerHTML = (j.networks || []).map((n) =>
    "<button type='button' data-ssid='" + n.ssid + "'>" + n.ssid +
    (n.open ? " (open)" : "") + "</button>"
  ).join("");
}

$("nets").onclick = (e) => {
  const ssid = e.target.dataset.ssid;
  if (!ssid) return;
  $("ssid").value = ssid;
};

$("scan").onclick = scan;

$("save").onclick = async () => {
  const ssid = $("ssid").value.trim();
  if (!ssid) { toast("SSID required"); return; }
  $("status").textContent = "Connecting...";
  await fetch("/api/settings", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: "ssid=" + encodeURIComponent(ssid) + "&password=" + encodeURIComponent($("pass").value)
  });
  for (let i = 0; i < 20; i++) {
    await new Promise((r) => setTimeout(r, 500));
    const j = await (await fetch("/api/status")).json();
    if (j.system && j.system.wifi && j.system.wifi.connected) {
      $("status").textContent = "Connected.";
      if (confirm("WiFi connected. Restart now?")) {
        await fetch("/api/restart", { method: "POST" });
      }
      return;
    }
  }
  $("status").textContent = "Not connected yet. Check password.";
  toast("still connecting");
};

loadStatus();
scan();
