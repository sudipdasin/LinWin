# Controlling owner blueprint — 2026-11-27

This supersedes feature-order decisions in ROADMAP.md, the 50-point ledger and older wallpaper-focused plans. Windows 10 22H2 desktop and Settings are the interaction reference, with original LinWin branding/assets. Recreate behavior, navigation, discoverability, and visual controls, not hardcoded screenshot scenery or proprietary assets. Keep XFCE/X11 and normal Linux recovery available.

## Mandatory delivery order

P0 Visual: finish every applicable visual setting and desktop feature first. P1 Ease of use: finish the practical daily Windows workflows second. P2 all other settings: preserve their Windows 10 category positions and visible labels, but disable actions until explicitly authorized for the subsequent phase. Pending P0/P1 actions are also disabled until real implementations exist. Do not disguise unavailable functions as working toggles. Existing implemented native tools and safe update/recovery paths must remain accessible during migration; do not delete working features. No unrelated new features while P0/P1 remain unfinished.

## Research basis and scope

Microsoft Windows 10 sections in https://support.microsoft.com/en-us/accessibility/windows/understand-and-explore-windows-settings ; page names and version applicability in https://learn.microsoft.com/en-us/windows/apps/develop/launch/launch-settings ; visual behavior in https://support.microsoft.com/en-us/windows/experience/personalization/customize-the-taskbar-in-windows and https://support.microsoft.com/en-us/accessibility/windows/make-windows-easier-to-see . Sources cover multiple Windows versions: exclude Windows 11-only controls, record optional build/hardware/edition differences separately. The 113-page catalog below is a broad initial inventory, not a claim of exhaustive control-level parity. Use actual 22H2 reference observations to expand each page into individual controls, defaults, state changes, keyboard behavior, host scope, and tests. Cortana and Mixed Reality are optional/retired Windows-specific pages; keep visible if applicable to a reference edition, with disabled actions. Never pretend Microsoft-specific services are available on Linux.

## P0 workstreams and gates

1. Settings shell: home categories, global Find a setting, category sidebar, breadcrumb/back, direct desktop routes, focus/keyboard/screen reader support, consistent responsive layout.
2. Background: Picture, Solid color, Slideshow, recent pictures, Browse, interval, shuffle, battery policy, Fill/Fit/Stretch/Tile/Center/Span; independent from theme; no forced logo; real multi-monitor scope.
3. Colors: Light/Dark/Custom, independent system/app modes, accent picker/custom RGB, wallpaper-derived accent, transparency, accent on Start/taskbar/title bars/borders; contrast checks.
4. Lock screen: picture/slideshow, preview, clock/status, screen timeout link; use real XFCE locker and never imitate secure authentication with a fake overlay. Unsupported host capabilities disabled.
5. Themes: background/colors/sounds/cursor profiles, save/apply/delete/import/export with safe format, desktop icon settings. Keep seven existing themes including Classic Minimal Fast.
6. Fonts: installed font list/search/preview, supported user font install/uninstall with confirmation, interface typeface/text size and fallback; no proprietary font redistribution.
7. Start: menu size/full screen, tiles/pins/reorder/groups, app list, recently added/most used/recent items, folder shortcuts, search, account and power actions; privacy and accessibility.
8. Taskbar: lock, auto-hide, small buttons, location, combine buttons, badges, tray icon visibility, system icons, Peek/show desktop, multi-monitor options and right-click settings; preserve safe return path.
9. Display: resolution, scale, orientation, monitor layout, refresh, brightness/night light/HDR where supported; confirm/revert risky display changes; accurate capability detection.
10. Visual accessibility: text scaling, animations/transparency/background toggles, pointer size/color, text cursor, magnifier, contrast, color filters, captions. Scope honestly to LinWin or verified host backend.
11. Shell surfaces: desktop icons/context menus/view/sort/refresh, title bars/minimize/maximize/close, window borders, shadows, task view, notifications/action center, dialogs, selection/focus/hover, original cohesive assets. No fake weather, fake volume or invented device state.

## P1 workstreams

Original LinWin requirements: standard explicit clipboard and history, safe middle-click behavior, auto-scroll where supported without breaking browser middle click; Alt+Tab, Win+D, Win+E, Win+I, Win+V and snap; real file/folder drag/drop, shortcut creation, conflict/undo, File Explorer navigation/breadcrumbs, quick access, properties, search, defaults and startup controls, keyboard/mouse/touchpad, notifications/focus, everyday accessibility, consistent discoverability. Preserve X11 PRIMARY and Wayland limitations as explicit host capabilities, not global promises. Hardware and app package management outside everyday workflows deferred.

## Inventory
Priority V=visual, E=ease, D=deferred. Only route-enabled rows have an existing native route; partial controls are not completed settings.

| Category | Page | Priority | Initial route |
|---|---|---|---|
| System | Display | V | display |
| System | Sound | E | Disabled |
| System | Notifications & actions | V | Disabled |
| System | Focus assist | E | Disabled |
| System | Power & sleep | D | Disabled |
| System | Battery | D | Disabled |
| System | Storage | D | Disabled |
| System | Tablet | V | Disabled |
| System | Multitasking | E | Disabled |
| System | Projecting to this PC | D | Disabled |
| System | Shared experiences | D | Disabled |
| System | Clipboard | E | Disabled |
| System | Remote Desktop | D | Disabled |
| System | About | D | Disabled |
| Devices | Bluetooth & other devices | D | Disabled |
| Devices | Printers & scanners | D | Disabled |
| Devices | Mouse | E | Disabled |
| Devices | Touchpad | E | Disabled |
| Devices | Typing | E | Disabled |
| Devices | Pen & Windows Ink | E | Disabled |
| Devices | AutoPlay | E | Disabled |
| Devices | USB | D | Disabled |
| Phone | Phone Link | D | Disabled |
| Network & Internet | Status | D | Disabled |
| Network & Internet | Wi-Fi | D | Disabled |
| Network & Internet | Ethernet | D | Disabled |
| Network & Internet | Dial-up | D | Disabled |
| Network & Internet | VPN | D | Disabled |
| Network & Internet | Airplane mode | D | Disabled |
| Network & Internet | Mobile hotspot | D | Disabled |
| Network & Internet | Data usage | D | Disabled |
| Network & Internet | Proxy | D | Disabled |
| Network & Internet | Cellular | D | Disabled |
| Personalization | Background | V | background |
| Personalization | Colors | V | Disabled |
| Personalization | Lock screen | V | Disabled |
| Personalization | Themes | V | themes |
| Personalization | Fonts | V | Disabled |
| Personalization | Start | V | Disabled |
| Personalization | Taskbar | V | Disabled |
| Apps | Apps & features | E | Disabled |
| Apps | Default apps | E | Disabled |
| Apps | Offline maps | D | Disabled |
| Apps | Apps for websites | D | Disabled |
| Apps | Video playback | V | Disabled |
| Apps | Startup | E | Disabled |
| Apps | Optional features | D | Disabled |
| Accounts | Your info | V | Disabled |
| Accounts | Email & accounts | D | Disabled |
| Accounts | Sign-in options | D | Disabled |
| Accounts | Access work or school | D | Disabled |
| Accounts | Family & other users | D | Disabled |
| Accounts | Sync your settings | D | Disabled |
| Time & Language | Date & time | E | Disabled |
| Time & Language | Region | E | Disabled |
| Time & Language | Language | E | Disabled |
| Time & Language | Speech | E | Disabled |
| Gaming | Xbox Game Bar | D | Disabled |
| Gaming | Captures | D | Disabled |
| Gaming | Game Mode | D | Disabled |
| Gaming | Xbox Networking | D | Disabled |
| Ease of Access | Display | V | text |
| Ease of Access | Mouse pointer | V | Disabled |
| Ease of Access | Text cursor | V | Disabled |
| Ease of Access | Magnifier | V | Disabled |
| Ease of Access | Color filters | V | Disabled |
| Ease of Access | High contrast | V | Disabled |
| Ease of Access | Narrator | E | Disabled |
| Ease of Access | Audio | E | Disabled |
| Ease of Access | Closed captions | V | Disabled |
| Ease of Access | Speech | E | Disabled |
| Ease of Access | Keyboard | E | Disabled |
| Ease of Access | Mouse | E | Disabled |
| Ease of Access | Eye control | E | Disabled |
| Search | Permissions & history | D | Disabled |
| Search | Searching Windows | E | Disabled |
| Privacy | General | D | Disabled |
| Privacy | Speech | D | Disabled |
| Privacy | Inking & typing personalization | D | Disabled |
| Privacy | Diagnostics & feedback | D | Disabled |
| Privacy | Activity history | D | Disabled |
| Privacy | Location | D | Disabled |
| Privacy | Camera | D | Disabled |
| Privacy | Microphone | D | Disabled |
| Privacy | Voice activation | D | Disabled |
| Privacy | Notifications | D | Disabled |
| Privacy | Account info | D | Disabled |
| Privacy | Contacts | D | Disabled |
| Privacy | Calendar | D | Disabled |
| Privacy | Phone calls | D | Disabled |
| Privacy | Call history | D | Disabled |
| Privacy | Email | D | Disabled |
| Privacy | Tasks | D | Disabled |
| Privacy | Messaging | D | Disabled |
| Privacy | Radios | D | Disabled |
| Privacy | Other devices | D | Disabled |
| Privacy | Background apps | D | Disabled |
| Privacy | App diagnostics | D | Disabled |
| Privacy | Automatic file downloads | D | Disabled |
| Privacy | Documents | D | Disabled |
| Privacy | Pictures | D | Disabled |
| Privacy | Videos | D | Disabled |
| Privacy | File system | D | Disabled |
| Update & Security | Windows Update equivalent | D | Disabled |
| Update & Security | Delivery Optimization | D | Disabled |
| Update & Security | Windows Security equivalent | D | Disabled |
| Update & Security | Backup | D | Disabled |
| Update & Security | Troubleshoot | D | Disabled |
| Update & Security | Recovery | D | Disabled |
| Update & Security | Activation | D | Disabled |
| Update & Security | Find my device | D | Disabled |
| Update & Security | For developers | D | Disabled |
| Update & Security | Windows Insider Program equivalent | D | Disabled |

## Release conditions

Each control must have an implemented backend, persistence where applicable, real observable state change, recovery/defaults, keyboard navigation, appropriate permission/host scope, and native test evidence. Disabled entries may be navigated/read but cannot perform actions. Visual acceptance requires actual installed captures and user review; compare layout/typography/interaction, not wallpaper pixels. Test default and classic themes, physical Mint 22.3 XFCE 1920x1080 and smaller resolution, installation/upgrade, rollback and no regression. No GitHub Actions or CircleCI; isolated branches, Render verification, monotonic DEBs, no final release until both priorities are accepted. No paid infrastructure without authorization.
