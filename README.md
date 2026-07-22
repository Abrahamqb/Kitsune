# Kitsune

Manager para consolas PlayStation con jailbreak. Aplicacion de escritorio escrita en C++ con interfaz grafica basada en ImGui, renderizada via OpenGL 3.3 y gestionada con GLFW.

Proyecto personal con fines educativos. Desarrollado como ejercicio de aprendizaje en programacion de redes, protocolos FTP, servidores HTTP embebidos y interfaces graficas en C++.

---

## Descripcion General

Kitsune permite gestionar juegos y aplicaciones instalados en una PlayStation conectada a la misma red local. La comunicacion con la consola se realiza mediante protocolo FTP, y el envio de archivos PKG se ejecuta por dos vias: servidor HTTP local por WiFi y conexion directa por cable LAN via protocolo RPI en el puerto 12800.

---

## Funcionalidades

### Conexion FTP y Listado de Contenido

El modulo `FTPManager` establece una conexion FTP con la consola usando las credenciales por defecto (`ps4ftp` / `12345`). La conexion se configura en modo pasivo con timeout de 5 segundos.

**Funciones principales del modulo FTP:**

| Funcion | Descripcion |
|---------|-------------|
| `FTPConnect(IP, Port)` | Verifica conexion FTP con la consola. Retorna cadena vacia si la conexion es exitosa. |
| `FTPListDirectory(IP, Port, RemotePath)` | Lista el contenido de un directorio remoto via FTP. Retorna el listado como cadena de texto. |
| `GetLocalIP()` | Obtiene la IP local del PC haciendo una peticion a google.com y extrayendo la interfaz de origen. |

### PKG Sender por WiFi

Kitsune ejecuta un servidor HTTP local en el puerto 8080 usando la libreria `cpp-httplib`. El flujo es:

1. El usuario selecciona un archivo PKG desde su PC usando el dialogo nativo de Windows
2. El servidor HTTP se inicia en un hilo separado en `0.0.0.0:8080`
3. Se construye una URL apuntando al servidor local con el nombre del archivo PKG
4. Desde la consola (usando GoldHEN o herramienta compatible), se accede a la URL para descargar el archivo
5. El servidor entrega el PKG como `application/octet-stream`

El servidor se ejecuta en un hilo separado, permitiendo que la interfaz grafica siga respondiendo durante la transferencia.

**Funciones del servidor local:**

| Funcion | Descripcion |
|---------|-------------|
| `StartLocalWebServer(PathPKG)` | Inicia servidor HTTP en puerto 8080 sirviendo el archivo PKG indicado. Detiene cualquier instancia previa antes de iniciar. |
| `StopLocalWebServer()` | Detiene el servidor HTTP activo y espera 400ms para limpieza del socket. |

### PKG Sender por LAN - RPI (Puerto 12800)

Para transferencias por cable LAN directo, el modulo `GoldHEN` usa el protocolo RPI (Remote Package Installer) en el puerto 12800. Este metodo ofrece mayor velocidad y estabilidad que WiFi.

El flujo es:

1. Se construye un payload JSON con la URL del PKG en el servidor local
2. Se envia una peticion POST a `http://<PS_IP>:12800/api/install`
3. La consola recibe la URL y descarga el PKG desde el servidor HTTP del PC

**Payload JSON enviado:**

```json
{
  "type": "direct",
  "packages": ["http://<PC_IP>:8080/<nombre_archivo.pkg>"]
}
```

**Funcion RPI:**

| Funcion | Descripcion |
|---------|-------------|
| `SendRPICommand(PsIp, PCIP, Port, PKGName)` | Envia comando de instalacion PKG via RPI a la consola. Retorna `true` si la consola confirma recepcion exitosa. |

### WinLanSetup - Configuracion de Red

Modulo dedicado a configurar la conexion de red entre el PC y la consola. Usa las APIs de Windows `GetAdaptersAddresses` e `iphlpapi` para enumerar adaptadores de red, y PowerShell con COM (`HNetCfg.HNetShare`) para gestionar Internet Connection Sharing (ICS).

Esto permite compartir la conexion a internet del PC con la consola a traves de un cable LAN directo.

**Estructura de datos:**

```cpp
struct EthernetAdapter {
    std::wstring NameW;  // Nombre del adaptador en formato wide string
    std::string  Name;   // Nombre del adaptador en formato UTF-8
};
```

**Funciones del modulo:**

| Funcion | Descripcion |
|---------|-------------|
| `GetEthernetAdapters()` | Enumera adaptadores ethernet del sistema. Retorna vector de `EthernetAdapter`. |
| `EnableLanMethod(adapterName)` | Activa ICS en el adaptador seleccionado. Configura WiFi como publico y ethernet como privado. |
| `DisableLanMethod(adapterName)` | Desactiva ICS en todos los adaptadores del sistema. |

### Selector de Archivos

Dialogo nativo de Windows para seleccionar archivos PKG desde el sistema de archivos local, implementado con la libreria `portable-file-dialogs`.

**Funcion:**

| Funcion | Descripcion |
|---------|-------------|
| `OpenFileDialog(outPath, title, filters)` | Abre dialogo de seleccion de archivos. Retorna la ruta seleccionada via parametro de salida. |

---

## Interfaz Grafica

La interfaz esta dividida en tres paneles principales:

### Panel de Configuracion

- Campo de IP de la consola (default: `192.168.18.17`)
- Campo de IP del PC
- Campo de puerto FTP (default: `2121`)
- Campo de ruta remota (default: `/data/GoldHEN`)
- Boton de conexion/desconexion FTP
- Boton para detectar IP local automaticamente

### Panel de Logs

- Registro de eventos de la aplicacion
- Mensajes de conexion FTP
- Estados de transferencia PKG
- Errores de la consola

---

## Arquitectura del Proyecto

```
Kitsune/
  src/
    main.cpp              Punto de entrada. Inicializacion de GLFW, OpenGL, ImGui.
    Interface.*           Logica de la interfaz grafica. Paneles, tabs, renderizado.
    FTPManager.*          Conexion y operaciones FTP con la consola.
    GoldHEN.*             servidor HTTP local, comando RPI.
    WinLanSetup.*         Configuracion de red y sharing LAN via PowerShell/COM.
    FileDialog.*          Selector de archivos nativo Windows.
    http/
      httplib.h           Servidor HTTP embebido (cpp-httplib).
    Other/
      portable-file-dialogs.h   Dialogos de archivo nativos.
  thirdparty/
    imgui/                Framework de interfaz grafica inmediata.
    include/
      GLFW/               Ventana, contexto OpenGL y manejo de input.
      glad/               Loader de funciones OpenGL.
```

---

## Dependencias

| Componente | Uso |
|---|---|
| **ImGui** | Framework de interfaz grafica inmediata. Renderizado de paneles, botones, listas. |
| **GLFW** | Creacion de ventana, contexto OpenGL 3.3 Core y manejo de eventos de input. |
| **GLAD** | Loader dinamico de funciones OpenGL. |
| **libcurl** | Operaciones FTP (conexion, listado de directorios) y peticiones HTTP (comando RPI). |
| **cpp-httplib** | Servidor HTTP embebido para el PKG Sender WiFi. |
| **portable-file-dialogs** | Dialogos nativos de Windows para seleccion de archivos. |
| **WinSock2** | API de Windows para operaciones de red de bajo nivel. |
| **iphlpapi** | API de Windows para enumeracion de adaptadores de red. |

---

## Compilacion

Proyecto configurado para Visual Studio (solution `Kitsune.slnx`).

### Requisitos

- Windows 10 o superior
- Visual Studio 2022 o compatible
- libcurl configurada en el proyecto

### Pasos

1. Abrir `Kitsune.slnx` en Visual Studio
2. Seleccionar configuracion Debug o Release
3. Compilar con `Ctrl+Shift+B`
4. Ejecutable generado en `x64/Debug/` o `x64/Release/`

---

## Uso

1. Ejecutar `Kitsune.exe`
2. Ingresar la IP de la consola PlayStation
3. Hacer clic en **Conectar** para establecer la conexion FTP
4. Para enviar un PKG, seleccionar el metodo (WiFi o LAN) y el archivo

---

## Disclaimer

Este proyecto es un ejercicio personal de aprendinaje. Destinado exclusivamente para fines educativos y de uso personal.

Kitsune no promueve ni facilita la pirateria de software. El envio de archivos PKG solo debe realizarse con contenido que el usuario posea legalmente. El uso de jailbreak en consolas puede anular la garantia del fabricante y violar los terminos de servicio de Sony.

El autor no se hace responsable del uso que se le de a esta herramienta.
