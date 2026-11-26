# Workspace visibility

The Desktop taskbar button and application-local Ctrl+Alt+D now hide and restore independent LinWin tools. Only previously visible, non-minimized tools are restored, and their existing window geometries are retained. Disabled tools and tools independently reopened are removed from the saved set; returning to Linux or closing LinWin discards it. Context menu exposes explicit restore. This is an in-memory workspace toggle, not persistent session recovery or control of other Linux applications. Native regression added; runtime validation pending.
