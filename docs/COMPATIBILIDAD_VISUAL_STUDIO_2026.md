# Compatibilidad con Visual Studio 2026

Este paquete está preparado para Visual Studio 2026 en Windows x64.

## Componentes del instalador

En Visual Studio Installer, modifica la instalación y selecciona la carga de trabajo **Desarrollo para el escritorio con C++**. Comprueba que se encuentren instalados:

- MSVC Build Tools para x64/x86 (Latest).
- Windows 10 SDK o Windows 11 SDK.
- C++ CMake tools for Windows.

## Por qué los presets usan Ninja

La integración de CMake de Visual Studio admite el generador Ninja. El archivo `CMakePresets.json` de este proyecto usa `Ninja` para que el paquete no dependa del generador `Visual Studio 17 2022` que se utilizaba en el preset anterior.

CMake 4.1.2 se documenta como la versión incluida con Visual Studio 2026 18.11. El generador específico `Visual Studio 18 2026` se añadió en CMake 4.2. Usar Ninja evita tener que instalar una versión adicional de CMake para este proyecto.

## Abrir en el IDE

1. Configura `VCPKG_ROOT` para que apunte a la raíz de vcpkg, por ejemplo `C:\dev\vcpkg`.
2. Reinicia Visual Studio para que reciba la variable de entorno.
3. Usa **Archivo > Abrir > Carpeta** y abre la raíz que contiene `CMakeLists.txt`.
4. Selecciona `windows-vs2026-debug` como preset de configuración.
5. Espera a que CMake y vcpkg completen la configuración inicial.
6. Compila el destino `BezierFlight` y ejecútalo con `Ctrl+F5`.

Para Release, selecciona el preset `windows-vs2026-release`.

## Compilar desde Developer PowerShell for VS 2026

Abre **Developer PowerShell for VS 2026** y ejecuta desde la raíz del proyecto:

```powershell
cmake --preset windows-vs2026-debug
cmake --build --preset debug
.\out\build\windows-vs2026-debug\BezierFlight.exe
```

Para Release:

```powershell
cmake --preset windows-vs2026-release
cmake --build --preset release
.\out\build\windows-vs2026-release\BezierFlight.exe
```

Los presets Debug y Release son separados porque Ninja es un generador de una sola configuración: el tipo de compilación se define durante la configuración de CMake.
