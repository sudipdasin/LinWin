# Native Snap Layouts

Seven geometry zones and a restore command are available inside each independent LinWin tool window. Alt+Shift+Left/Right/Up/Down are window-local shortcuts for half screen, fill, and restore. The engine accounts for monitor work areas, odd dimensions, negative coordinates, native frame insets and minimum content size. An undersized zone is rejected. This is not a compositor or external-app snapping engine. Tested pure C++ geometry: 12 checks. Qt integration and physical XFCE behavior pending.
