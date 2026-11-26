# Neon Fluent widgets and Start menu

Native desktop widgets show the local clock, calendar, CPU utilization, available-memory based RAM utilization, and root filesystem usage. Values are read from the actual system, unavailable values are labeled, and CPU use is computed from successive samples. The timer stops when hidden. Widgets are displayed only in Neon Fluent and on sufficiently large windows. The Start menu has a real pinned grid backed by the existing installed-app registry and existing user pin settings, with shared search and activation. No invented app or weather data is shown.

Standalone metrics parser regression passed with g++ C++20. Native Qt runtime layout, performance and DEB generation remain unverified. Weather, recommended documents, glass composition, power and tray parity are still pending. No CI invoked.
