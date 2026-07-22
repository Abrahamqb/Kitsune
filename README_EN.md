# Kitsune

A manager for jailbroken PlayStation consoles. Kitsune is a desktop application written in **C++** with a graphical interface built using **Dear ImGui**, rendered with **OpenGL 3.3**, and managed through **GLFW**.

This is a personal project created for educational purposes, focused on learning **network programming**, **FTP protocols**, **embedded HTTP servers**, and **C++ GUI development**.

---

# Overview

Kitsune allows you to manage games and applications installed on a PlayStation console connected to the same local network. Communication with the console is handled through **FTP**, while PKG transfers can be performed using two methods:

- **Wi-Fi** via a local HTTP server
- **Direct LAN connection** using the **Remote Package Installer (RPI)** protocol on port **12800**

---

# Features

## FTP Connection & Directory Browser

The `FTPManager` module establishes an FTP connection using the default PlayStation FTP credentials:

- Username: `ps4ftp`
- Password: `12345`

Connections use **Passive Mode** with a **5-second timeout**.

### FTP Functions

| Function | Description |
|----------|-------------|
| `FTPConnect(IP, Port)` | Tests the FTP connection with the console. Returns an empty string on success. |
| `FTPListDirectory(IP, Port, RemotePath)` | Lists the contents of a remote directory over FTP and returns them as a string. |
| `GetLocalIP()` | Retrieves the PC's local IP address by opening a connection to `google.com` and determining the outgoing network interface. |

---

## PKG Sender (Wi-Fi)

Kitsune starts a local HTTP server on **port 8080** using **cpp-httplib**.

### Transfer Flow

1. Select a PKG file using the native Windows file picker.
2. A local HTTP server starts on `0.0.0.0:8080`.
3. Kitsune generates a URL pointing to the selected PKG.
4. The PlayStation (through GoldHEN or another compatible installer) accesses the generated URL.
5. The HTTP server streams the PKG file as `application/octet-stream`.

The server runs in a separate thread, allowing the user interface to remain responsive during transfers.

### HTTP Server Functions

| Function | Description |
|----------|-------------|
| `StartLocalWebServer(PathPKG)` | Starts the embedded HTTP server on port 8080 serving the specified PKG file. Stops any previous instance before starting. |
| `StopLocalWebServer()` | Stops the active HTTP server and waits 400 ms to ensure the socket is fully released. |

---

## PKG Sender (LAN) - Remote Package Installer (Port 12800)

For direct Ethernet transfers, Kitsune communicates with **GoldHEN's Remote Package Installer (RPI)** on **port 12800**.

This method offers significantly faster and more stable transfers compared to Wi-Fi.

### Transfer Flow

1. A JSON payload containing the PKG URL is created.
2. An HTTP POST request is sent to:

```
http://<PS_IP>:12800/api/install
```

3. The PlayStation receives the request and downloads the PKG directly from the PC's HTTP server.

### JSON Payload

```json
{
    "type": "direct",
    "packages": [
        "http://<PC_IP>:8080/<package_name.pkg>"
    ]
}
```

### RPI Function

| Function | Description |
|----------|-------------|
| `SendRPICommand(PsIP, PCIP, Port, PKGName)` | Sends an installation request through RPI. Returns `true` if the console successfully acknowledges the command. |

---

## WinLanSetup - LAN Configuration

The `WinLanSetup` module configures the network connection between the PC and the PlayStation.

It uses the Windows APIs:

- `GetAdaptersAddresses`
- `iphlpapi`

to enumerate network adapters, and PowerShell with the COM interface (`HNetCfg.HNetShare`) to configure **Internet Connection Sharing (ICS)**.

This allows the PC to share its internet connection with the console through a direct Ethernet cable.

### Data Structure

```cpp
struct EthernetAdapter
{
    std::wstring NameW;
    std::string Name;
};
```

### Functions

| Function | Description |
|----------|-------------|
| `GetEthernetAdapters()` | Enumerates all Ethernet adapters installed on the system. |
| `EnableLanMethod(adapterName)` | Enables Internet Connection Sharing. Sets Wi-Fi as the public interface and Ethernet as the private interface. |
| `DisableLanMethod(adapterName)` | Disables Internet Connection Sharing on all network adapters. |

---

## Native File Picker

Kitsune uses **portable-file-dialogs** to provide the native Windows file selection dialog.

### Function

| Function | Description |
|----------|-------------|
| `OpenFileDialog(outPath, title, filters)` | Opens a native file picker and returns the selected file path through an output parameter. |

---

# User Interface

The application is organized into several panels.

## Configuration Panel

- PlayStation IP Address
- PC IP Address
- FTP Port (default: `2121`)
- Remote FTP Path (default: `/data/GoldHEN`)
- Connect / Disconnect button
- Automatic Local IP detection

## Log Panel

Displays:

- FTP connection logs
- PKG transfer status
- HTTP server events
- Console responses
- Error messages

---

# Project Structure

```
Kitsune/
│
├── src/
│   ├── main.cpp
│   ├── Interface.*
│   ├── FTPManager.*
│   ├── GoldHEN.*
│   ├── WinLanSetup.*
│   ├── FileDialog.*
│   │
│   ├── http/
│   │   └── httplib.h
│   │
│   └── Other/
│       └── portable-file-dialogs.h
│
└── thirdparty/
    ├── imgui/
    └── include/
        ├── GLFW/
        └── glad/
```

---

# Dependencies

| Component | Purpose |
|-----------|---------|
| **Dear ImGui** | Immediate-mode graphical user interface. |
| **GLFW** | Window creation, OpenGL context, and input handling. |
| **GLAD** | OpenGL function loader. |
| **libcurl** | FTP operations and HTTP requests (RPI). |
| **cpp-httplib** | Embedded HTTP server used for Wi-Fi PKG transfers. |
| **portable-file-dialogs** | Native Windows file dialogs. |
| **WinSock2** | Low-level Windows networking API. |
| **iphlpapi** | Windows API for network adapter enumeration. |

---

# Building

The project is configured for **Visual Studio** (`Kitsune.slnx`).

## Requirements

- Windows 10 or newer
- Visual Studio 2022 (or compatible)
- libcurl configured in the project

## Build Steps

1. Open `Kitsune.slnx`
2. Select **Debug** or **Release**
3. Build the solution (`Ctrl + Shift + B`)
4. The executable will be generated in:

```
x64/Debug/
```

or

```
x64/Release/
```

---

# Usage

1. Launch **Kitsune.exe**
2. Enter your PlayStation's IP address.
3. Click **Connect** to establish the FTP connection.
4. Select a PKG file.
5. Choose either **Wi-Fi** or **LAN** transfer mode.
6. Start the installation.

---

# Disclaimer

This project was developed as a personal learning exercise and is intended for **educational and personal use only**.

Kitsune does **not** promote or facilitate software piracy. PKG transfers should only be performed with content that you legally own.

Using jailbreak software on a PlayStation console may void the manufacturer's warranty and may violate Sony's Terms of Service.

The author assumes **no responsibility** for any misuse of this software.
