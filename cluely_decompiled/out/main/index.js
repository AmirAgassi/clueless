import { electronApp as ce } from "@electron-toolkit/utils";
import { ipcMain as Y, app as i, screen as u, BrowserWindow as de, shell as O, desktopCapturer as G, systemPreferences as B, globalShortcut as p, Menu as j, session as le } from "electron";
import { spawn as J } from "node:child_process";
import I, { join as y } from "node:path";
import { existsSync as ue, writeFileSync as pe } from "node:fs";
import he from "screenshot-desktop";
import we from "electron-updater";
import { electronAppUniversalProtocolClient as H } from "electron-app-universal-protocol-client";
import fe from "node:module";
const $e = import.meta.filename, x = import.meta.dirname, Xe = fe.createRequire(import.meta.url);
function r(n, e) {
  Y.on(n, e);
}
function V(n, e) {
  Y.handle(n, e);
}
const me = 13, ye = 24e3;
async function K(n, e) {
  const t = I.basename(e);
  return await new Promise((o, c) => {
    let d = "";
    n.stdout.on("data", (s) => {
      d += s.toString();
    }), n.stderr.on("data", (s) => {
      console.error(`[${t}] stderr: ${s}`);
    }), n.on("close", (s) => {
      s !== 0 ? (console.error(`[${t}] process exited with code ${s}`), c(new Error(`Process exited with code ${s}`))) : o({ stdout: d });
    }), n.on("error", (s) => {
      console.error(`[${t}] process error: ${s}`), c(s);
    });
  });
}
async function ge() {
  const n = J("sw_vers", ["-productVersion"]), { stdout: e } = await K(n, "sw_vers"), t = Number.parseInt(e.split(".")[0] ?? "", 10);
  return { isSupported: !Number.isNaN(t) && t >= me };
}
const g = process.platform === "darwin", b = process.platform === "win32";
process.platform;
const M = process.env.NODE_ENV === "development";
process.env.NODE_ENV;
const be = I.join(
  // app.getAppPath(): root folder of the electron app
  // process.resourcesPath: the Resources folder in the app's package contents
  M ? i.getAppPath() : process.resourcesPath,
  "macExtraResources"
);
function ve(n) {
  return I.join(be, n);
}
const $ = ve("nativeMacRecorder");
let h = null;
i.on("before-quit", () => N());
function Me(n) {
  N(), h = J($, [ye.toString()]), Ce(h, n), K(h, $);
}
function N() {
  h?.kill("SIGINT"), h = null;
}
function Ce(n, e) {
  let t = "";
  n.stdout.on("data", (o) => {
    const c = o.toString(), s = (t + c).split(`
`);
    t = s.pop() ?? "", e.sendToWebContents("mac-native-recorder-data", {
      base64Data: s.join("")
      // concatenate all complete lines
    });
  });
}
function Te(n, e, t, o) {
  e = Math.floor(e), t = Math.floor(t);
  const d = u.getPrimaryDisplay().displayFrequency;
  let s = Math.min(Math.max(d, 30), 360);
  d > 60 && (s = Math.max(60, Math.floor(d / 2)));
  const C = 1e3 / s, w = n.getBounds(), T = w.width, W = w.height, _ = w.x, R = w.y, D = _ + Math.floor((T - e) / 2), E = R + Math.floor((W - t) / 2), oe = e - T, se = t - W, re = D - _, ie = E - R, U = Math.floor(o / C);
  let z = 0;
  const ae = Date.now();
  let l = null;
  const q = () => {
    const L = Date.now() - ae;
    if (z = Math.min(U, Math.floor(L / C)), z < U) {
      const f = We(L / o), A = Math.floor(T + oe * f), P = Math.floor(W + se * f), S = Math.floor(_ + re * f), k = Math.floor(R + ie * f);
      if (b) {
        const m = n.getBounds();
        (Math.abs(m.width - A) >= 1 || Math.abs(m.height - P) >= 1 || Math.abs(m.x - S) >= 1 || Math.abs(m.y - k) >= 1) && n.setBounds(
          {
            x: S,
            y: k,
            width: A,
            height: P
          },
          !1
        );
      } else
        n.setBounds({
          x: S,
          y: k,
          width: A,
          height: P
        });
      l = setTimeout(q, C);
    } else
      n.setBounds({
        x: D,
        y: E,
        width: e,
        height: t
      }), n.setResizable(!1), l !== null && (clearTimeout(l), l = null);
  }, F = n.isResizable();
  return F || n.setResizable(!0), q(), {
    cancel: () => {
      l !== null && (clearTimeout(l), l = null), n.setBounds({
        x: D,
        y: E,
        width: e,
        height: t
      }), n.setResizable(F);
    }
  };
}
function We(n) {
  return n < 0.5 ? 4 * n * n * n : 1 - Math.pow(-2 * n + 2, 3) / 2;
}
class Q {
  window;
  constructor(e) {
    this.window = new de(
      e || {
        show: b,
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
          preload: y(x, "../preload/index.mjs"),
          sandbox: !1
        }
      }
    ), this.window.setContentProtection(!0), this.window.setVisibleOnAllWorkspaces(!0, { visibleOnFullScreen: !0 }), this.window.setResizable(!1), b && (this.window.setAlwaysOnTop(!0, "screen-saver", 1), this.window.webContents.setBackgroundThrottling(!1)), this.moveToPrimaryDisplay(), this.setIgnoreMouseEvents(!0), this.window.once("ready-to-show", () => {
      this.window.show();
    }), this.window.webContents.setWindowOpenHandler((t) => (O.openExternal(t.url), { action: "deny" })), M && process.env.ELECTRON_RENDERER_URL ? this.window.loadURL(process.env.ELECTRON_RENDERER_URL) : this.window.loadFile(y(x, "../renderer/index.html"));
  }
  sendToWebContents(e, t) {
    this.window.isDestroyed() || this.window.webContents.send(e, t);
  }
  setIgnoreMouseEvents(e) {
    this.window.setIgnoreMouseEvents(e, { forward: !0 });
  }
  resizeWindow(e, t, o) {
    Te(this.window, e, t, o);
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
  moveToPrimaryDisplay() {
    const e = u.getPrimaryDisplay();
    this.window.setPosition(e.workArea.x, e.workArea.y), this.window.setSize(e.workArea.width, e.workArea.height);
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
class _e extends Q {
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
        preload: y(x, "../preload/index.mjs"),
        sandbox: !1
      }
    };
    super(e), super.moveToPrimaryDisplay();
  }
  setIgnoreMouseEvents(e) {
  }
  moveToPrimaryDisplay() {
    const e = u.getPrimaryDisplay();
    this.window.setPosition(e.workArea.x, e.workArea.y), this.window.center();
  }
}
class Re {
  currentWindow = null;
  handlers = /* @__PURE__ */ new Set();
  createWindow() {
    this.currentWindow = new (ne() ? Q : _e)();
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
const a = new Re(), Z = y(i.getPath("userData"), "onboarding.done");
let ee = ue(Z);
function ne() {
  return ee;
}
function De() {
  pe(Z, ""), ee = !0, a.recreateWindow();
}
async function Ee() {
  return { data: await he({ format: "png" }), contentType: "image/png" };
}
const { autoUpdater: v } = we;
function Ae() {
  let n = a.getCurrentWindow();
  a.onWindowChange((e) => {
    n = e;
  }), r("quit-app", () => {
    i.quit();
  }), r("check-for-update", () => {
    v.checkForUpdatesAndNotify();
  }), r("install-update", () => {
    v.quitAndInstall();
  }), r("request-app-version", () => {
    n.sendToWebContents("app-version", {
      version: i.getVersion()
    });
  }), V("request-has-onboarded", async () => ({ hasOnboarded: ne() })), V("request-media-permission", async (e, t) => {
    if (process.platform === "darwin") {
      if (t === "screen")
        try {
          return await G.getSources({ types: ["screen"] }), !0;
        } catch {
          return !1;
        }
      try {
        const o = B.getMediaAccessStatus(t);
        return o === "not-determined" ? await B.askForMediaAccess(t) : o === "granted";
      } catch (o) {
        return console.error("Media permission error:", o), !1;
      }
    }
    return !0;
  }), r("set-has-onboarded-true", () => {
    De();
  }), r("register-global-shortcut", (e, { accelerator: t }) => {
    p.register(t, () => {
      n.sendToWebContents("global-shortcut-triggered", { accelerator: t });
    }) || console.error(`Failed to register global shortcut: ${t}`);
  }), r("unregister-global-shortcut", (e, { accelerator: t }) => {
    p.unregister(t);
  }), r("set-ignore-mouse-events", (e, { ignore: t }) => {
    n.setIgnoreMouseEvents(t);
  }), r("resize-window", (e, { width: t, height: o, duration: c }) => {
    n.resizeWindow(t, o, c);
  }), r("focus-window", () => {
    n.focus();
  }), r("capture-screenshot", () => {
    Ee().then(({ contentType: e, data: t }) => {
      n.sendToWebContents("new-screenshot", { contentType: e, data: t });
    });
  }), g && (r("mac-check-macos-version", async () => {
    const { isSupported: e } = await ge();
    n.sendToWebContents("mac-check-macos-version-result", {
      isSupported: e
    });
  }), r("mac-open-system-settings", (e, { section: t }) => {
    t === "privacy > microphone" && O.openExternal(
      "x-apple.systempreferences:com.apple.preference.security?Privacy_Microphone"
    ), t === "privacy > screen-recording" && O.openExternal(
      "x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture"
    );
  }), r("mac-set-native-recorder-enabled", (e, { enabled: t }) => {
    t ? Me(n) : N();
  }));
}
function Pe() {
  const n = () => {
    a.getCurrentWindow().moveToPrimaryDisplay();
  };
  u.on("display-added", n), u.on("display-removed", n), u.on("display-metrics-changed", n);
}
function Se() {
  H.on("request", (n) => {
    g && i.dock.hide(), te(n, a.getCurrentWindow());
  }), H.initialize({
    protocol: "cluely",
    mode: M ? "development" : "production"
  }), X(a.getCurrentWindow(), process.argv), i.on("second-instance", (n, e) => {
    const t = a.getCurrentWindow();
    t.sendToWebContents("unhide-window", null), X(t, e);
  }), i.on("activate", () => {
    g && i.dock.hide(), a.getCurrentWindow().sendToWebContents("unhide-window", null);
  });
}
function X(n, e) {
  const t = e.find((o) => o.startsWith("cluely://"));
  t && te(t, n);
}
function te(n, e) {
  const t = new URL(n), o = t.hostname, c = Object.fromEntries(t.searchParams);
  e.sendToWebContents("protocol-data", { route: o, params: c });
}
function ke() {
  const n = j.buildFromTemplate([
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
  j.setApplicationMenu(n);
}
function Oe() {
  le.defaultSession.setDisplayMediaRequestHandler(
    (n, e) => {
      G.getSources({ types: ["screen"] }).then((t) => {
        e({ video: t[0], audio: "loopback" });
      }).catch(() => {
        e({});
      });
    },
    // always use our custom handler
    { useSystemPicker: !1 }
  );
}
function xe() {
  v.on("update-downloaded", () => {
    a.getCurrentWindow().sendToWebContents("update-downloaded", null);
  }), v.checkForUpdatesAndNotify();
}
g && i.dock.hide();
b && (i.requestSingleInstanceLock() || (i.quit(), process.exit(0)));
async function Ie() {
  await i.whenReady(), ce.setAppUserModelId("com.cluely"), ke(), Oe(), a.createWindow(), xe(), Ae(), Pe(), Se(), M && a.onWindowChange((n) => {
    function e() {
      p.unregisterAll(), p.register("CommandOrControl+Alt+R", () => n.reload()), p.register("CommandOrControl+Alt+I", () => n.toggleDevTools());
    }
    e(), n.onUnload(e);
  });
}
Ie();
