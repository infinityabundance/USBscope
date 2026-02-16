## Hacking on USBscope

This document is for people who want to read the code or contribute small changes.

### Components overview

- **usbscoped**: daemon that tails kernel logs (`journalctl -k -f`) and watches `udev` for device changes. Emits structured `UsbEvent` and `UsbDeviceInfo` data over D-Bus.
- **usbscope-ui**: Qt6 desktop app that shows a log table, device list, and timeline view. Talks to the daemon over the `org.cachyos.USBscope1` D-Bus interface.
- **usbscope-tray**: system tray app that subscribes to error-related signals and surfaces notifications.
- **usbscopecore**: shared library with common types and D-Bus helpers used by the other components.

### Build

Dependencies: Qt6 (Core, Widgets, DBus), CMake, a C++17 compiler.

```bash
sudo pacman -S --needed cmake make gcc qt6-base qt6-tools

git clone https://github.com/infinityabundance/USBscope.git
cd USBscope
cmake -S . -B build
cmake --build build
```

This produces binaries like `build/usbscope-ui`, `build/usbscoped`, and `build/usbscope-tray`.

### Run

From the repository root after building:

```bash
./build/usbscope-ui
./build/usbscoped
./build/usbscope-tray
```

By default the apps try to use the session D-Bus. You can force the system bus with:

```bash
USBSCOPE_BUS=system usbscope-ui
USBSCOPE_BUS=system usbscoped
USBSCOPE_BUS=system usbscope-tray
```

System bus use may require a suitable D-Bus policy.

### Where to start reading code

- **UI entry point**: `MainWindow` in the UI sources wires up the log table, filters, device list, and timeline view. The `TimelineView`/`TimelineScene` files handle zooming, panning, and drawing.
- **D-Bus client and types**: look for the D-Bus helper / client classes that expose `GetRecentEvents`, `GetCurrentDevices`, `GetStateSummary`, and the `LogEvent` / `DevicesChanged` / `ErrorBurst` signals.
- **Daemon**: the `usbscoped` sources contain the journald tailing and `udev` integration logic that produces `UsbEvent` and device snapshots.

Keeping changes focused and incremental makes review easier. Small, tightly scoped patches (bug fixes, small refactors, or local UI tweaks) are the easiest to land.

