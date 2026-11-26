# LinWin 50-point completion ledger

This is an acceptance ledger, not a claim of 50 newly delivered major features. Preview 6 is the last passing baseline (Render deployment dep-dc4arvl9el5s73ao9j6g). Preview 7 initially failed C++ test compilation; the repaired candidate includes new native regression gates. Scope is based on the original Windows-like UX objectives and approved Neon Fluent reference. Component tests do not establish physical XFCE or visual acceptance.

## Baseline verified in Preview 6

| ID | Milestone | Evidence or remaining condition |
|---:|---|---|
| 1 | Monotonic Debian upgrade ordering | Render package gate |
| 2 | Exact package and executable version identity | Render package gate |
| 3 | APT install and removal in disposable Ubuntu builder | Render package gate |
| 4 | Native X11 overlay startup | Xvfb smoke gate |
| 5 | Native X11 shell startup | Xvfb smoke gate |
| 6 | Seven selectable theme profiles | theme-catalog |
| 7 | Minimal theme effects policy | responsive-taskbar |
| 8 | Snap geometry | snap-geometry |
| 9 | Persistent workspace profiles | workspace-profiles |
| 10 | Recent-document filtering | recent-documents |
| 11 | Weather JSON validation | weather-forecast |
| 12 | Local battery status parsing | desktop-status |
| 13 | CPU and RAM metric parsing | system-metrics |
| 14 | Task Manager tabs | task-manager-tabs |
| 15 | Clipboard center | clipboard-center |
| 16 | Searchable Settings | settings-hub-search |
| 17 | Setup wizard review | setup-wizard-review |
| 18 | File Explorer quick access | file-manager-quick-access |
| 19 | XFCE dependency planning | xfce-dependency-plan |
| 20 | Responsive taskbar and menu controls | responsive-taskbar |
| 21 | X11 session wrapper | x11-session-wrapper |
| 22 | Embedded updater regression | 45 tests |
| 23 | HTTP download handling | 4 tests |
| 24 | Native captures at both target resolutions | Render 4-capture gate |

## Locally component-tested; candidate integration pending

| ID | Milestone | Evidence or remaining condition |
|---:|---|---|
| 25 | Textured terrain determinism and alpha behavior | Prior standalone C++ render; Preview 7 integration not yet passed |
| 26 | Installed-app suggestions and duplicate prevention | New C++ fixture test; exact source blobs verified |
| 27 | Release evidence rejection and integrity | New Python suite: 9 tests; exact source blobs verified |

## Candidate changes committed; native verification pending

| ID | Milestone | Evidence or remaining condition |
|---:|---|---|
| 28 | Repair Preview 7 regression compile collision | b96572a |
| 29 | Shaded terrain composited into native desktop | Preview 7 candidate |
| 30 | Original desktop place icons | Preview 7 candidate |
| 31 | Functional widget toolbar | Preview 7 candidate |
| 32 | Preserve existing pin ordering | 78883d5 |
| 33 | Add suggested installed apps through Edit | 78883d5 |
| 34 | Keyboard navigation in Start search | 40932ec |
| 35 | Order-independent multiword search | 40932ec |
| 36 | Dismiss Start only after successful application launch | 40932ec |
| 37 | Actual registered-app launch test | 48d3ee6 |
| 38 | Native package/test evidence manifest | f77d495 |

## Open acceptance requirements

| ID | Milestone | Evidence or remaining condition |
|---:|---|---|
| 39 | Photorealistic reference-quality mountain and lake | Procedural terrain is not exact reference match |
| 40 | Reference-quality ribbon and logo | Visual comparison and correction |
| 41 | Exact Start, taskbar and widget fidelity | Human visual acceptance at both resolutions |
| 42 | Consistent native window decorations and compositor effects | XFCE integration and host support |
| 43 | Full native notification/status-notifier tray | Host protocol integration |
| 44 | Live weather on physical desktop | Opt-in network verification |
| 45 | System-wide clipboard/middle-click behavior | Scope and toolkit limitations |
| 46 | Universal drag/drop and shell integration | Host application interoperability |
| 47 | Unified app/package discovery and management | Host package types and UX |
| 48 | Signed in-app preview updates and recovery | Signing key and trusted release metadata required |
| 49 | Physical Mint XFCE performance and usability | Actual laptop or authorized native runner |
| 50 | Final visual approval and stable release | No release until acceptance |

No GitHub Actions or CircleCI; main remains unchanged. Only Render deploys the isolated build candidate. Do not promote an unverified candidate or infer visual fidelity from test counts.
