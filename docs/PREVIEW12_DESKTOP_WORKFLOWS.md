# Preview 12: desktop interaction and settings home

Preview 11 compiled, but Render deployment dep-dc4bod15efls73dktph0 failed responsive-taskbar at "Fixture must be selectable for pinning"; 19/20 CTest suites passed. Preview 10 remains the live baseline. Fix the search backend to select the first visible match only when the typed query has no valid selection, preserving explicit user selection and never auto-launching. Retain the original failing fixture assertion.

New taskbar location control moves the actual LinWin panel to top/bottom and adjusts Start anchor and reserved desktop area. Left/right, auto-hide, and grouping remain disabled. Local GCC C++17 policy regression passed 850248 geometry combinations, including both anchors; exact source/test blob hashes 74ac7800aada11c292d2843ff25b094720ad5194 and 25df3dd8f5496b37725039f4cbd8326b2dc4eef5. This does not prove every native widget fits.

Windows 10 settings home now has 13 responsive icon category buttons, real category selection, home navigation and global search. The previous 113-entry inventory and disabled-action gates remain, and unsupported entries cannot execute. New native tests check category navigation and actual taskbar geometry. Existing wallpaper and themes remain unchanged. Native build and physical Mint XFCE acceptance are pending. No GitHub Actions/CircleCI, no main promotion.
