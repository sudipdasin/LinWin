# Preview 14 — Start, taskbar and desktop GUI overhaul

Owner requested improved Start and taskbar appearance/function plus desktop right-click Close LinWin. Previous verified release Preview 13 remains available until Render completes the next native gates.

Start: adaptive narrow navigation rail with accessible full labels, alphabetical app list and single-click launching with Enter, direct all-app context pin/unpin, persistent Move earlier/later on pinned tiles (skip unavailable registered apps without dropping saved IDs), subtle individual tile surfaces, complete name tooltips and revised menu spacing. Existing search, recent privacy and pins are retained.

Taskbar: dedicated wide-screen Task View button, left-aligned running-window strip instead of stretching buttons, show-desktop control at right, taskbar context menu for actual controls, X11 external-window activate/normal close actions (no process kill), clearer exit hover. Narrow overflow and existing top/bottom placement preserved.

Desktop: Close LinWin on right-click menu including background margins. Start, taskbar and desktop share default-No confirmation, distinct shell-session wording. Existing closeEvent saves tool geometry and hides tool windows. Normal overlay exit does not shut down XFCE or other applications.

GUI: shared theme-aware surfaces, labels, inputs, lists, tables' headers, buttons, focus, disabled states, scrollbars and tooltips for the seven native settings scopes; existing tool-specific styles kept. This is not a complete Windows 10 compositor or native window decoration overhaul.

Tests: actual registered-app pin ordering and restoration, Task View presence, desktop close cancellation and confirmed exit, plus existing 20 native suites, package install/remove, updater and download checks. New candidate unverified until Render passes. No GitHub Actions or CircleCI. No main promotion.
