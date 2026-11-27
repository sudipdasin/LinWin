#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
test "${LINWIN_ISOLATED_VISUAL_QA:-}" = 1
test "${LINWIN_QA_WINDOW_ACTIONS:-}" = 1
case "$HOME" in /tmp/linwin-qa-*/home) ;; *) exit 2;; esac
xfwm4 --compositor=off >/tmp/linwin-qa-xfwm/wm.log 2>&1 &
wm=$!
a= b=
cleanup(){ for pid in "$a" "$b" "$wm"; do if [ -n "$pid" ]; then kill "$pid" 2>/dev/null || :; fi; done; }
trap cleanup EXIT
for attempt in $(seq 1 60); do if wmctrl -m >/dev/null 2>&1; then break; fi; sleep .1; done
wmctrl -m
xterm -T 'LinWin QA Window A' -geometry 80x20+80+80 >/dev/null 2>&1 &
a=$!
xterm -T 'LinWin QA Window B' -geometry 80x20+240+240 >/dev/null 2>&1 &
b=$!
for attempt in $(seq 1 60); do
    if wmctrl -l | grep -F 'LinWin QA Window A' >/dev/null && wmctrl -l | grep -F 'LinWin QA Window B' >/dev/null; then break; fi
    sleep .1
done
wmctrl -l | grep -F 'LinWin QA Window A'
wmctrl -l | grep -F 'LinWin QA Window B'
code=0
/usr/bin/linwin --capture-native "$1" || code=$?
if [ -f "$1/layout.json" ]; then cat "$1/layout.json"; fi
test "$code" -eq 0
