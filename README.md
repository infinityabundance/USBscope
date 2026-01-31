# USBscope

<p align="center">
  <a href="https://github.com/infinityabundance/USBscope">
    <img src="assets/usbscope.svg" alt="USBscope" width="400" height="400">
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white" alt="C++17">
  <img src="https://img.shields.io/badge/License-Apache%202.0-blue.svg" alt="Apache 2.0">
  <img src="https://img.shields.io/badge/Arch%20Linux-1793D1?logo=arch-linux&logoColor=white" alt="Arch Linux">
  <img src="https://img.shields.io/badge/CachyOS-0A7A7A?logo=linux&logoColor=white" alt="CachyOS">
</p>

USBscope is a live USB event dashboard for CachyOS / ArchLinux. It ships a small system daemon that tails kernel USB logs and tracks connected USB devices, then exposes that data over D-Bus to a desktop UI and tray app. The result is a real-time view of what is happening on your USB bus, plus a quick way to surface errors as they happen.

## Why it is useful
- Debug flaky peripherals by correlating hotplug events with kernel messages.
- Catch error bursts quickly via tray notifications and a visible error timeline.
- Get an at-a-glance view of currently connected devices with vendor/product IDs.
- Export filtered logs to CSV for bug reports or deeper analysis.

## What it does
- Tails kernel logs (`journalctl -k -f`) and classifies USB-related events.
- Monitors the live USB device list via `udev`.
- Publishes events and device snapshots over the system D-Bus.
- Provides a Qt UI with filtering, search, timeline visualization, and CSV export.
- Provides a tray icon for quick status and error burst notifications.

## Components
- `usbscoped`: daemon that reads kernel logs and udev, emits D-Bus signals.
- `usbscope-ui`: Qt desktop app with log table, device list, and timeline view.
- `usbscope-tray`: tray app that watches for USB errors and shows notifications.
- `usbscopecore`: shared library for USB event/device types and D-Bus helpers.

## How it works (high level)
1. `usbscoped` tails kernel logs and parses each line into a structured `UsbEvent`.
2. It tracks connected USB devices via `udev` enumeration and hotplug updates.
3. Events and device snapshots are broadcast on D-Bus (`org.cachyos.USBscope1`).
4. The UI and tray clients subscribe to those signals and request history on startup.

## Build
Dependencies: Qt6 (Core, Widgets, DBus), CMake, a C++17 compiler.
```bash
sudo pacman -S --needed cmake make gcc qt6-base qt6-tools 
```

```bash
git clone https://github.com/infinityabundance/USBscope.git
cd USBscope
cmake -S . -B build
cmake --build build
```

## Run
Start the daemon first, then launch the UI or tray app:

```bash
./build/usbscope-ui
```

There is also a sample systemd service file at `data/usbscoped.service`.

## D-Bus API
Interface: `org.cachyos.USBscope1` on the session bus by default.

If the session bus is unavailable, USBscope falls back to the system bus. You can also force system bus use with:

```bash
USBSCOPE_BUS=system usbscope-ui
USBSCOPE_BUS=system usbscoped
USBSCOPE_BUS=system usbscope-tray
```

Note: system bus registration may require an appropriate D-Bus policy.

Methods:
- `GetVersion()`
- `GetRecentEvents(limit)`
- `GetCurrentDevices()`
- `GetStateSummary()`

Signals:
- `LogEvent`
- `DevicesChanged`
- `ErrorBurst`
