}import { electronApp as ye } from "@electron-toolkit/utils";
import { app as u, screen as h, BrowserWindow as ee, shell as L, ipcMain as W, globalShortcut as _, desktopCapturer as te, systemPreferences as X, Menu as Y, session as ve } from "electron";
import B, { join as y } from "node:path";
import { existsSync as be, writeFileSync as Me } from "node:fs";
import { spawn as F } from "node:child_process";
import { EventEmitter as Ce } from "node:events";
import De from "screenshot-desktop";
import Se from "electron-updater";
import { electronAppUniversalProtocolClient as Z } from "electron-app-universal-protocol-client";
import Ae from "node:module";
const wt = import.meta.filename, C = import.meta.dirname, ft = Ae.createRequire(import.meta.url), oe = y(u.getPath("userData"), "onboarding.done");
let se = be(oe);
function ne() {
  return se;
}
function Te() {
  Me(oe, ""), se = !0, l.recreateWindow();
}
const D = process.platform === "darwin", R = process.platform === "win32";
process.platform;
const S = process.env.NODE_ENV === "development";
process.env.NODE_ENV;
function _e(t, e, o, s) {
  e = Math.floor(e), o = Math.floor(o);
  const a = h.getPrimaryDisplay().displayFrequency;
  let i = Math.min(Math.max(a, 30), 360);
  a > 60 && (i = Math.max(60, Math.floor(a / 2)));
  const c = 1e3 / i, d = t.getBounds(), b = d.width, w = d.height, f = d.x, g = d.y, P = f + Math.floor((b - e) / 2), I = g + Math.floor((w - o) / 2), pe = e - b, we = o - w, fe = P - f, me = I - g, V = Math.floor(s / c);
  let H = 0;
  const ge = Date.now();
  let m = null;
  const j = () => {
    const J = Date.now() - ge;
    if (H = Math.min(V, Math.floor(J / c)), H < V) {
      const A = Ee(J / s), N = Math.floor(b + pe * A), U = Math.floor(w + we * A), $ = Math.floor(f + fe * A), z = Math.floor(g + me * A);
      if (R) {
        const T = t.getBounds();
        (Math.abs(T.width - N) >= 1 || Math.abs(T.height - U) >= 1 || Math.abs(T.x - $) >= 1 || Math.abs(T.y - z) >= 1) && t.setBounds(
          {
            x: $,
            y: z,
            width: N,
            height: U
          },
          !1
        );
      } else
        t.setBounds({
          x: $,
          y: z,
          width: N,
          height: U
        });
      m = setTimeout(j, c);
    } else
      t.setBounds({
        x: P,
        y: I,
        width: e,
        height: o
      }), t.setResizable(!1), m !== null && (clearTimeout(m), m = null);
  }, G = t.isResizable();
  return G || t.setResizable(!0), j(), {
    cancel: () => {
      m !== null && (clearTimeout(m), m = null), t.setBounds({
        x: P,
        y: I,
        width: e,
        height: o
      }), t.setResizable(G);
    }
  };
}
function Ee(t) {
  return t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
}
class ie {
  window;
  constructor(e) {
    this.window = new ee(
      e || {
        show: R,
        // on Windows, we must show immediately for undetectability to work
        // window style options
        alwaysOnTop: !0,
        transparent: !0,
        frame: !1,
        roundedCorners: !1,
        hasShadow: !1,
        // window resize options
        fullscreenable: !1,
        minimizable: !1,
        // macOS specific options
        hiddenInMissionControl: !0,
        // macOs + Windows specific options
        skipTaskbar: !0,
        webPreferences: {
          preload: y(C, "../preload/index.mjs"),
          sandbox: !1
        }
      }
    ), this.window.setContentProtection(!0), this.window.setVisibleOnAllWorkspaces(!0, { visibleOnFullScreen: !0 }), this.window.setResizable(!1), R && (this.window.setAlwaysOnTop(!0, "screen-saver", 1), this.window.webContents.setBackgroundThrottling(!1)), this.moveToPrimaryDisplay(), this.setIgnoreMouseEvents(!0), this.window.once("ready-to-show", () => {
      this.window.show();
    }), this.window.webContents.setWindowOpenHandler((o) => (L.openExternal(o.url), { action: "deny" })), S && process.env.ELECTRON_RENDERER_URL ? this.window.loadURL(process.env.ELECTRON_RENDERER_URL) : this.window.loadFile(y(C, "../renderer/index.html"));
  }
  sendToWebContents(e, o) {
    this.window.isDestroyed() || this.window.webContents.send(e, o);
  }
  setIgnoreMouseEvents(e) {
    this.window.setIgnoreMouseEvents(e, { forward: !0 });
  }
  resizeWindow(e, o, s) {
    _e(this.window, e, o, s);
  }
  focus() {
    this.window.focus();
  }
  close() {
    this.window.isDestroyed() || this.window.close();
  }
  isDestroyed() {
    return this.window.isDestroyed();
  }
  getBounds() {
    return this.window.getBounds();
  }
  moveToPrimaryDisplay() {
    const e = h.getPrimaryDisplay();
    this.window.setPosition(e.workArea.x, e.workArea.y), this.window.setSize(e.workArea.width, e.workArea.height), this.sendToWebContents("display-changed", null);
  }
  moveToDisplay(e) {
    this.window.setPosition(e.workArea.x, e.workArea.y), this.window.setSize(e.workArea.width, e.workArea.height), this.sendToWebContents("display-changed", null);
  }
  reload() {
    this.window.webContents.reload();
  }
  onUnload(e) {
    this.window.webContents.on("did-navigate", e);
  }
  toggleDevTools() {
    this.window.webContents.isDevToolsOpened() ? this.window.webContents.closeDevTools() : (this.window.webContents.openDevTools({ mode: "detach" }), this.window.focus());
  }
}
class Oe extends ie {
  constructor() {
    const e = {
      show: !0,
      alwaysOnTop: !1,
      transparent: !0,
      frame: !1,
      roundedCorners: !1,
      hasShadow: !0,
      fullscreenable: !1,
      minimizable: !1,
      hiddenInMissionControl: !1,
      skipTaskbar: !0,
      webPreferences: {
        preload: y(C, "../preload/index.mjs"),
        sandbox: !1
      }
    };
    super(e), super.moveToPrimaryDisplay();
  }
  setIgnoreMouseEvents(e) {
  }
  moveToPrimaryDisplay() {
    const e = h.getPrimaryDisplay();
    this.window.setPosition(e.workArea.x, e.workArea.y), this.window.center();
  }
}
class We {
  currentWindow = null;
  handlers = /* @__PURE__ */ new Set();
  createWindow() {
    this.currentWindow = new (ne() ? ie : Oe)();
    for (const e of this.handlers)
      e(this.currentWindow);
    return this.currentWindow;
  }
  /** Can only be called after createWindow() */
  getCurrentWindow() {
    if (!this.currentWindow)
      throw new Error("No current window. Did you call createWindow()?");
    return this.currentWindow;
  }
  recreateWindow() {
    this.currentWindow && this.currentWindow.close(), this.createWindow();
  }
  /** Runs the handlers immediately */
  onWindowChange(e) {
    return this.handlers.add(e), this.currentWindow && e(this.currentWindow), () => {
      this.handlers.delete(e);
    };
  }
}
const l = new We();
function re() {
  const e = l.getCurrentWindow().getBounds(), o = h.getDisplayMatching(e);
  return h.getAllDisplays().map((s) => ({
    ...s,
    label: s.label || `Display ${s.id}`,
    primary: s.id === h.getPrimaryDisplay().id,
    current: s.id === o.id
  }));
}
function ae(t) {
  return h.getAllDisplays().find((e) => e.id === t);
}
class Re {
  window;
  displayId;
  constructor(e, o) {
    this.displayId = e.id, this.window = new ee({
      show: !1,
      frame: !1,
      transparent: !0,
      alwaysOnTop: !0,
      skipTaskbar: !0,
      resizable: !1,
      movable: !1,
      minimizable: !1,
      maximizable: !1,
      fullscreenable: !1,
      x: e.bounds.x,
      y: e.bounds.y,
      width: e.bounds.width,
      height: e.bounds.height,
      webPreferences: {
        nodeIntegration: !1,
        contextIsolation: !0,
        sandbox: !1,
        preload: y(C, "../preload/index.mjs")
      }
    }), this.window.setVisibleOnAllWorkspaces(!0, { visibleOnFullScreen: !0 }), this.window.setIgnoreMouseEvents(!1);
    const s = () => {
      console.log(`[DisplayOverlay] Overlay click triggered for display ${this.displayId}`), o(this.displayId);
    }, n = `overlay-click-${this.displayId}`;
    W.on(n, s), this.window.on("closed", () => {
      console.log(`[DisplayOverlay] Cleaning up IPC handler for display ${this.displayId}`), W.removeListener(n, s);
    }), this.loadReactOverlay(e, n);
  }
  async loadReactOverlay(e, o) {
    const s = {
      display: {
        id: e.id,
        label: e.label || `Display ${e.id}`,
        bounds: e.bounds
      },
      ipcChannel: o,
      onOverlayClick: () => {
      }
      // This will be handled via IPC
    }, n = encodeURIComponent(JSON.stringify(s));
    if (S && process.env.ELECTRON_RENDERER_URL) {
      const a = new URL(`${process.env.ELECTRON_RENDERER_URL}/overlay.html`);
      a.searchParams.set("displayData", n), this.window.loadURL(a.toString());
    } else
      this.window.loadFile(y(C, "../renderer/overlay.html"), {
        query: { displayData: n }
      });
  }
  show() {
    this.window.show();
  }
  hide() {
    this.window.hide();
  }
  highlight() {
    this.window.webContents.executeJavaScript(`
      window.dispatchEvent(new CustomEvent('highlight'));
    `).catch(() => {
    });
  }
  unhighlight() {
    this.window.webContents.executeJavaScript(`
      window.dispatchEvent(new CustomEvent('unhighlight'));
    `).catch(() => {
    });
  }
  destroy() {
    console.log(`[DisplayOverlay] Destroying overlay for display ${this.displayId}`), this.window.isDestroyed() || this.window.close();
  }
  getBounds() {
    return this.window.getBounds();
  }
}
class xe {
  overlays = /* @__PURE__ */ new Map();
  isActive = !1;
  showOverlays() {
    console.log("[DisplayOverlayManager] Showing overlays"), this.hideOverlays(), this.isActive = !0;
    const e = h.getAllDisplays(), o = l.getCurrentWindow(), s = o.getBounds(), n = h.getDisplayMatching(s);
    for (const a of e) {
      if (a.id === n.id)
        continue;
      const i = new Re(a, (c) => {
        if (console.log(
          `[DisplayOverlayManager] Display ${c} clicked, checking if active: ${this.isActive}`
        ), !this.isActive) {
          console.log(
            `[DisplayOverlayManager] Ignoring click for display ${c} - overlays are inactive`
          );
          return;
        }
        console.log(`[DisplayOverlayManager] Moving window to display ${c}`);
        const d = ae(c);
        d && o.moveToDisplay(d), this.hideOverlays();
      });
      this.overlays.set(a.id, i), i.show();
    }
  }
  hideOverlays() {
    console.log("[DisplayOverlayManager] Hiding overlays"), this.isActive = !1;
    for (const e of this.overlays.values())
      e.destroy();
    this.overlays.clear();
  }
  highlightDisplay(e) {
    const o = this.overlays.get(e);
    o && o.highlight();
  }
  unhighlightDisplay(e) {
    const o = this.overlays.get(e);
    o && o.unhighlight();
  }
}
const E = new xe();
function r(t, e) {
  W.on(t, e);
}
function O(t, e) {
  W.handle(t, e);
}
const ke = 13, Pe = 24e3;
async function le(t, e) {
  const o = B.basename(e);
  return await new Promise((s, n) => {
    let a = "";
    t.stdout.on("data", (i) => {
      a += i.toString();
    }), t.stderr.on("data", (i) => {
      console.error(`[${o}] stderr: ${i}`);
    }), t.on("close", (i) => {
      i !== 0 ? (console.error(`[${o}] process exited with code ${i}`), n(new Error(`Process exited with code ${i}`))) : s({ stdout: a });
    }), t.on("error", (i) => {
      console.error(`[${o}] process error: ${i}`), n(i);
    });
  });
}
async function Ie() {
  const t = F("sw_vers", ["-productVersion"]), { stdout: e } = await le(t, "sw_vers"), o = Number.parseInt(e.split(".")[0] ?? "", 10);
  return { isSupported: !Number.isNaN(o) && o >= ke };
}
const Ne = B.join(
  // app.getAppPath(): root folder of the electron app
  // process.resourcesPath: the Resources folder in the app's package contents
  S ? u.getAppPath() : process.resourcesPath,
  "macExtraResources"
);
function Ue(t) {
  return B.join(Ne, t);
}
const Q = Ue("nativeMacRecorder");
let M = null;
u.on("before-quit", () => q());
function $e(t) {
  q(), M = F(Q, [Pe.toString()]), ze(M, t), le(M, Q);
}
function q() {
  M?.kill("SIGINT"), M = null;
}
function ze(t, e) {
  let o = "";
  t.stdout.on("data", (s) => {
    const n = s.toString(), i = (o + n).split(`
`);
    o = i.pop() ?? "", e.sendToWebContents("mac-native-recorder-data", {
      base64Data: i.join("")
      // concatenate all complete lines
    });
  });
}
const Le = [
  ["VoiceMemos", "Voice Memos"],
  ["Google Chrome", "Google Chrome"],
  ["firefox", "Mozilla Firefox"],
  ["com.apple.WebKit", "Safari"],
  ["zoom.us", "Zoom"],
  ["GoogleMeet", "Google Meet"],
  ["Slack", "Slack"],
  // below are still untested
  ["FaceTime", "FaceTime"],
  ["Discord", "Discord"],
  ["Teams", "Microsoft Teams"],
  ["QuickTimePlayer", "QuickTime Player"]
];
function Be(t) {
  for (const [e, o] of Le)
    if (t.includes(e))
      return o;
  return null;
}
class Fe extends Ce {
  proc = null;
  // place the most selective rules first to avoid hitting a less specific rule
  // which does not match the app name
  matchRules = [
    {
      type: "mic-used",
      subsystem: "com.apple.coreaudio:as_server",
      matchSubstring: '\\"input_running\\":true',
      regex: /"session":\{[^}]*"name":"([A-Za-z0-9_\. ]+)\(\d+\)".*?"input_running":\s*true/
    },
    {
      type: "mic-off",
      subsystem: "com.apple.coreaudio:as_server",
      matchSubstring: '\\"input_running\\":false',
      regex: /"session":\{[^}]*"name":"([A-Za-z0-9_\. ]+)\(\d+\)".*?"input_running":\s*false/
    },
    {
      type: "mic-used",
      subsystem: "com.apple.audio.AVFAudio",
      matchSubstring: "AVCaptureDevice was used",
      regex: /AVCaptureDevice was used for audio by "(.*?)"/
    },
    {
      type: "mic-off",
      subsystem: "com.apple.audio.AVFAudio",
      matchSubstring: "AVCaptureDevice was stopped",
      regex: /AVCaptureDevice was stopped being used for audio by "(.*?)"/
    },
    {
      type: "mic-used",
      subsystem: "com.apple.audio.ASDT",
      matchSubstring: ": startStream: running state: 1"
    },
    {
      type: "mic-off",
      subsystem: "com.apple.audio.ASDT",
      matchSubstring: ": stopStream: running state: 0"
    },
    // Firefox-specific rules for AUHAL subsystem
    {
      type: "mic-used",
      subsystem: "com.apple.coreaudio:AUHAL",
      matchSubstring: "connecting device"
    },
    {
      type: "mic-off",
      subsystem: "com.apple.coreaudio:AUHAL",
      matchSubstring: "nothing to teardown"
    },
    // Firefox AVCapture rules
    {
      type: "mic-used",
      subsystem: "com.apple.coremedia",
      matchSubstring: "logging capture stack initiator"
    }
  ];
  start() {
    if (this.proc) return;
    console.log("[MicMonitor] start() called");
    const o = ["stream", "--info", "--predicate", this.buildPredicate()], s = F("log", o);
    this.proc = s, this.proc.stdout.on("data", (n) => {
      const a = n.toString(), i = a.split(`
`).filter((c) => c.trim().length > 0);
      for (const c of i)
        for (const d of this.matchRules) {
          const b = d.matchSubstring.replace(/\\"/g, '"');
          if (c.includes(b)) {
            let w = "";
            if (d.regex) {
              const g = d.regex.exec(c);
              g?.[1] && (w = g[1]);
            }
            if (!w) {
              console.log("[MicMonitor] failed to extract the app name from a matching rule");
              continue;
            }
            const f = Be(w);
            if (!f)
              break;
            console.log(`[MicMonitor] matched rule: ${d.type} for app: ${f}`), this.emit(d.type, { app: f, message: c });
            return;
          }
        }
      console.log("[MicMonitor] no rule matched for log message:", a);
    }), this.proc.stderr.on("data", (n) => {
      console.error("[MicMonitor stderr]", n.toString());
    }), this.proc.on("exit", (n) => {
      console.log(`[MicMonitor] exited with code ${n}`), this.proc === s && (this.proc = null);
    });
  }
  buildPredicate() {
    return this.matchRules.map((e) => `(eventMessage CONTAINS "${e.matchSubstring}")`).join(" || ");
  }
  stop() {
    this.proc && (this.proc.kill(), this.proc = null);
  }
}
let p = null;
u.on("before-quit", () => ce());
function qe(t) {
  p || D && (p = new Fe(), p.start(), p.on("mic-used", (e) => {
    t.sendToWebContents("unhide-window", null), t.sendToWebContents("mic-used", e);
  }), p.on("mic-off", (e) => {
    t.sendToWebContents("mic-off", e);
  }));
}
function ce() {
  p && (p.stop(), p = null);
}
async function Ve() {
  return { data: await De({ format: "png" }), contentType: "image/png" };
}
let de = S;
const v = /* @__PURE__ */ new Set();
function He(t) {
  if (v.has(t)) {
    console.warn(`Shortcut already registered: ${t}`);
    return;
  }
  v.add(t), k();
}
function je(t) {
  if (!v.has(t)) {
    console.warn(`Shortcut not registered: ${t}`);
    return;
  }
  v.delete(t), k();
}
function Ge() {
  de = !0, k();
}
function Je() {
  v.clear(), k();
}
function k() {
  _.unregisterAll();
  for (const t of v)
    _.register(t, () => {
      l.getCurrentWindow().sendToWebContents("global-shortcut-triggered", { accelerator: t });
    }) || console.error(`Failed to register global shortcut: ${t}`);
  de && (_.register("CommandOrControl+Alt+R", () => {
    l.getCurrentWindow().reload();
  }), _.register("CommandOrControl+Alt+I", () => {
    l.getCurrentWindow().toggleDevTools();
  }));
}
const { autoUpdater: x } = Se;
function Xe() {
  let t = l.getCurrentWindow();
  l.onWindowChange((e) => {
    t = e;
  }), r("quit-app", () => {
    u.quit();
  }), r("check-for-update", () => {
    x.checkForUpdatesAndNotify();
  }), r("install-update", () => {
    x.quitAndInstall();
  }), O("request-has-onboarded", async () => ({ hasOnboarded: ne() })), O("request-media-permission", async (e, o) => {
    if (process.platform === "darwin") {
      if (o === "screen")
        try {
          return await te.getSources({ types: ["screen"] }), !0;
        } catch {
          return !1;
        }
      try {
        const s = X.getMediaAccessStatus(o);
        return s === "not-determined" ? await X.askForMediaAccess(o) : s === "granted";
      } catch (s) {
        return console.error("Media permission error:", s), !1;
      }
    }
    return !0;
  }), r("set-has-onboarded-true", () => {
    Te();
  }), r("register-global-shortcut", (e, { accelerator: o }) => {
    He(o);
  }), r("unregister-global-shortcut", (e, { accelerator: o }) => {
    je(o);
  }), r("enable-dev-shortcuts", () => {
    Ge();
  }), r("reset-global-shortcuts", () => {
    Je();
  }), r("set-ignore-mouse-events", (e, { ignore: o }) => {
    t.setIgnoreMouseEvents(o);
  }), r("resize-window", (e, { width: o, height: s, duration: n }) => {
    t.resizeWindow(o, s, n);
  }), r("focus-window", () => {
    t.focus();
  }), O("capture-screenshot", async () => {
    const { contentType: e, data: o } = await Ve();
    return { contentType: e, data: o };
  }), r("get-available-displays", () => {
    const e = re();
    t.sendToWebContents("available-displays", { displays: e });
  }), r("move-window-to-display", (e, { displayId: o }) => {
    const s = ae(o);
    s && t.moveToDisplay(s);
  }), r("show-display-overlays", () => {
    E.showOverlays();
  }), r("hide-display-overlays", () => {
    console.log("[IPC] hide-display-overlays called"), E.hideOverlays();
  }), r("highlight-display", (e, { displayId: o }) => {
    E.highlightDisplay(o);
  }), r("unhighlight-display", (e, { displayId: o }) => {
    E.unhighlightDisplay(o);
  }), D && (O("mac-check-macos-version", async () => {
    const { isSupported: e } = await Ie();
    return { isSupported: e };
  }), r("mac-open-system-settings", (e, { section: o }) => {
    o === "privacy > microphone" && L.openExternal(
      "x-apple.systempreferences:com.apple.preference.security?Privacy_Microphone"
    ), o === "privacy > screen-recording" && L.openExternal(
      "x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture"
    );
  }), r("mac-set-native-recorder-enabled", (e, { enabled: o }) => {
    o ? $e(t) : q();
  }), r("mac-set-mic-monitor-enabled", (e, { enabled: o }) => {
    o ? qe(t) : ce();
  }));
}
function Ye() {
  const t = () => {
    l.getCurrentWindow().moveToPrimaryDisplay();
    const e = re();
    l.getCurrentWindow().sendToWebContents("available-displays", { displays: e });
  };
  h.on("display-added", t), h.on("display-removed", t), h.on("display-metrics-changed", t);
}
const ue = "cluely";
function Ze() {
  Z.on("request", (t) => {
    D && u.dock.hide();
    const e = l.getCurrentWindow();
    e.sendToWebContents("unhide-window", null), he(t, e);
  }), Z.initialize({
    protocol: ue,
    mode: S ? "development" : "production"
  }), K(l.getCurrentWindow(), process.argv), u.on("second-instance", (t, e) => {
    const o = l.getCurrentWindow();
    o.sendToWebContents("unhide-window", null), K(o, e);
  }), u.on("activate", () => {
    D && u.dock.hide(), l.getCurrentWindow().sendToWebContents("unhide-window", null);
  });
}
function K(t, e) {
  const o = e.find((s) => s.startsWith(`${ue}://`));
  o && he(o, t);
}
function he(t, e) {
  const o = new URL(t), s = o.hostname, n = Object.fromEntries(o.searchParams);
  e.sendToWebContents("protocol-data", { route: s, params: n });
}
function Qe() {
  const t = Y.buildFromTemplate([
    // preserve Cmd+C, Cmd+V, etc.
    { role: "editMenu" },
    // preserve Cmd+R
    {
      role: "viewMenu",
      submenu: [
        // disable Cmd +/-
        { role: "zoomIn", enabled: !1 },
        { role: "zoomOut", enabled: !1 }
      ]
    }
  ]);
  Y.setApplicationMenu(t);
}
function Ke() {
  ve.defaultSession.setDisplayMediaRequestHandler(
    (t, e) => {
      te.getSources({ types: ["screen"] }).then((o) => {
        e({ video: o[0], audio: "loopback" });
      }).catch(() => {
        e({});
      });
    },
    // always use our custom handler
    { useSystemPicker: !1 }
  );
}
function et() {
  x.on("update-downloaded", () => {
    l.getCurrentWindow().sendToWebContents("update-downloaded", null);
  }), x.checkForUpdatesAndNotify();
}
const tt = "cluely";
D && u.dock.hide();
R && (u.requestSingleInstanceLock() || (u.quit(), process.exit(0)));
async function ot() {
  await u.whenReady(), ye.setAppUserModelId(`com.${tt}`), Qe(), Ke(), l.createWindow(), et(), Xe(), Ye(), Ze();
}
ot();