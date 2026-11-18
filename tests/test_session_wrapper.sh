#!/bin/sh
set -eu

script=$1
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
export LINWIN_SESSION_LOG="$tmp/commands.log"

for name in xfwm4 xfsettingsd xfdesktop linwin; do
    cat > "$tmp/$name" <<'STUB'
#!/bin/sh
printf '%s %s\n' "$(basename "$0")" "$*" >> "$LINWIN_SESSION_LOG"
if [ "$(basename "$0")" = linwin ]; then
    [ "${1:-}" = "--shell" ]
    exit
fi
exec sleep 30
STUB
    chmod +x "$tmp/$name"
done

DISPLAY=:99 XDG_SESSION_TYPE=x11 PATH="$tmp:$PATH" sh "$script"
grep -q '^xfsettingsd --replace$' "$LINWIN_SESSION_LOG"
grep -q '^xfwm4 --replace$' "$LINWIN_SESSION_LOG"
grep -q '^xfdesktop --replace$' "$LINWIN_SESSION_LOG"
grep -q '^linwin --shell$' "$LINWIN_SESSION_LOG"

: > "$LINWIN_SESSION_LOG"
if DISPLAY=:99 XDG_SESSION_TYPE=wayland PATH="$tmp:$PATH" sh "$script" 2>/dev/null; then
    echo "The LinWin X11 session wrapper must reject Wayland." >&2
    exit 1
fi
[ ! -s "$LINWIN_SESSION_LOG" ]
