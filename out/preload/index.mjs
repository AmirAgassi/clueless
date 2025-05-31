import { electronAPI as e } from "@electron-toolkit/preload";
import { contextBridge as r } from "electron";
if (process.contextIsolated)
  try {
    r.exposeInMainWorld("electron", e);
  } catch (o) {
    console.error(o);
  }
else
  window.electron = e;
