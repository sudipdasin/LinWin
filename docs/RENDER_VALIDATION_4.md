# Render candidate 4

Render.3 is the verified live baseline. Candidate 4 adds native window Snap layouts (seven zones and restore), window-local keyboard shortcuts, a standalone geometry regression, and an actual Qt dialog snap-and-restore regression. The build gate retains 45 updater tests, all existing CTest suites, sixteen screenshot captures, Xvfb startup, and real container APT install/remove. No main promotion; physical XFCE/monitor and external app snapping remain unverified.
