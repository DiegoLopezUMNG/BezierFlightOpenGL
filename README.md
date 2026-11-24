# Vuelo 3D con OpenGL y curva de Bézier

Proyecto educativo en C++ y OpenGL 3.3 Core. Importa un avión desde glTF/GLB, lo anima a lo largo de una curva cúbica de Bézier, orienta el morro según la tangente de la curva y aplica iluminación Phong mediante shaders externos.

 El modelo se centró alrededor del origen, sus unidades originales en centímetros se convirtieron a metros y el morro apunta al eje local +X.

## 1. Arquitectura

```text
BezierFlightOpenGL/
├── CMakeLists.txt              # Configuración y enlace de dependencias
├── CMakePresets.json            # Presets de CMake para Visual Studio 2026 usando Ninja
├── vcpkg.json                   # Dependencias declaradas para vcpkg
├── src/
│   ├── main.cpp                 # Ventana, entradas, actualización y renderizado
│   ├── BezierPath.h/.cpp        # Curva cúbica y tabla de longitud de arco
│   ├── Camera.h/.cpp            # Cámara y desplazamiento por teclado
│   ├── Model.h/.cpp             # Importación glTF/GLB, mallas y texturas
│   └── Shader.h/.cpp            # Lectura, compilación y enlace de GLSL
├── shaders/
│   ├── phong.vert               # Transformación de vértices del avión
│   ├── phong.frag               # Iluminación ambiental, difusa y especular
│   ├── line.vert                # Transformación de vértices de la trayectoria
│   └── line.frag                # Color de la trayectoria
├── assets/
│   └── models/
│       ├── airplane.glb         # Avión incluido; texturas incrustadas
│       └── INSTRUCCIONES.txt    # Detalles de conversión y ejes locales
└── docs/
    ├── ARQUITECTURA.md           # Explicación técnica del diseño
    ├── GUIÓN_EXPOSICIÓN.md       # Guion para presentar el proyecto
    └── GITHUB_Y_EQUIPO.md        # Publicación y reproducción en otro PC
```

### Flujo de ejecución

1. `main.cpp` inicializa GLFW, crea el contexto OpenGL 3.3 y carga GLAD.
2. `Shader` lee los archivos `.vert` y `.frag`, los compila y enlaza los programas.
3. `Model` usa Assimp para recorrer la jerarquía del archivo glTF/GLB, convertir las mallas y cargar las texturas.
4. `BezierPath` evalúa la curva cúbica y precalcula una tabla de longitud aproximada.
5. En cada fotograma se actualiza el tiempo, la distancia recorrida y la cámara.
6. La posición se obtiene evaluando la Bézier; la tangente determina la orientación del avión.
7. Se construyen las matrices de modelo, vista y proyección y se dibuja la trayectoria y el avión.
8. El fragment shader calcula los términos ambiental, difuso y especular del modelo Phong.

## 2. Requisitos para Windows

- Windows 10 u 11 de 64 bits.
- Visual Studio 2026 Community, Professional o Enterprise, actualizado. El proyecto usa `CMakePresets.json` con el generador Ninja para no depender del generador específico de Visual Studio 2022.
- En el instalador de Visual Studio 2026, selecciona **Desarrollo para el escritorio con C++**. Comprueba que estén incluidos **MSVC Build Tools para x64/x86 (Latest)**, Windows 10/11 SDK y **C++ CMake tools for Windows**. En VS 2026 el toolset MSVC usa una nomenclatura distinta a la de VS 2022; no necesitas instalar v143 para este proyecto.
- Git para Windows.
- CMake 3.24 o posterior. La instalación de Visual Studio puede incluir una versión adecuada.
- Una tarjeta gráfica y controlador compatibles con OpenGL 3.3.
- vcpkg para instalar las bibliotecas declaradas en `vcpkg.json`.

> **Nota para Visual Studio 2026:** este paquete usa el generador `Ninja` en sus presets. Visual Studio puede trabajar con proyectos CMake usando Ninja. Esta elección evita exigir que el CMake integrado reconozca el generador nativo `Visual Studio 18 2026`; ese generador se incorporó en CMake 4.2, mientras que Visual Studio 2026 18.11 documenta CMake 4.1.2. No hace falta instalar CMake por separado para seguir el flujo de este README.

## 3. Instalar vcpkg

Abre PowerShell y ejecuta estos comandos (puedes elegir otra carpeta si prefieres):

```powershell
New-Item -ItemType Directory -Force C:\dev | Out-Null
Set-Location C:\dev
git clone https://github.com/microsoft/vcpkg.git
Set-Location C:\dev\vcpkg
.\bootstrap-vcpkg.bat
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\dev\vcpkg", "User")
```

Cierra y vuelve a abrir Visual Studio y PowerShell para que reconozcan `VCPKG_ROOT`. Puedes comprobarlo con:

```powershell
$env:VCPKG_ROOT
```

El manifiesto `vcpkg.json` declara Assimp, GLAD, GLFW, GLM y stb_image. CMake y vcpkg resolverán esas dependencias al configurar el proyecto. No es necesario descargar cada biblioteca manualmente.

## 4. Abrir y ejecutar en Visual Studio

1. Descomprime el proyecto en una carpeta local.
2. El modelo `assets/models/airplane.glb` ya viene incluido; no necesitas convertir ni copiar el OBJ original.
3. Abre Visual Studio 2026.
4. Selecciona **Archivo > Abrir > Carpeta** y elige la carpeta `BezierFlightOpenGL`.
5. Visual Studio detectará `CMakeLists.txt` y `CMakePresets.json`. Selecciona el preset `windows-vs2026-debug` para compilar en Debug. El preset usa Ninja, que está admitido por la integración de CMake de Visual Studio.
6. Espera a que CMake configure el proyecto y vcpkg instale las dependencias la primera vez.
7. Como Ninja es un generador de una sola configuración, Debug o Release se elige mediante el preset de CMake: `windows-vs2026-debug` o `windows-vs2026-release`. Para comenzar, usa Debug y compila con **Compilar > Compilar todo**.
8. Selecciona `BezierFlight` como elemento de inicio y ejecuta con **Depurar > Iniciar sin depurar** o `Ctrl+F5`.

También puedes compilar desde una terminal de desarrollador de Visual Studio, ubicada en la carpeta del proyecto:

```powershell
cmake --preset windows-vs2026-debug
cmake --build --preset debug
.\out\build\windows-vs2026-debug\BezierFlight.exe
```

Para compilar en modo optimizado:

```powershell
cmake --preset windows-vs2026-release
cmake --build --preset release
```

### Usar un archivo `.gltf` o una ruta distinta

El ejecutable recibe la ruta del modelo como primer argumento. Por ejemplo:

```powershell
.\out\build\windows-vs2026-debug\BezierFlight.exe "assets/models/mi_avion/scene.gltf"
```

En Visual Studio, configura ese texto como argumento del programa en la configuración de depuración de CMake. Si usas el nombre predeterminado, deja el archivo binario en `assets/models/airplane.glb`.

## 5. Modelo 3D incluido y cómo sustituirlo

El proyecto ya incluye `assets/models/airplane.glb`. Puedes ejecutar directamente el programa sin descargar ni convertir otro modelo. Para sustituirlo por un `.glb` propio:

1. Haz una copia de seguridad del modelo incluido.
2. Copia tu archivo a `assets/models/` y nómbralo `airplane.glb`, o pasa su ruta como argumento al ejecutar.
3. Comprueba que las texturas estén incrustadas en el GLB o que las rutas externas sean correctas.

### Si tienes un `.gltf`

Un `.gltf` puede depender de un archivo `.bin` y de imágenes externas. Copia **toda la carpeta exportada**, sin mover ni renombrar sus archivos internos. Por ejemplo:

```text
assets/models/mi_avion/
├── scene.gltf
├── scene.bin
└── textures/
    ├── baseColor.png
    └── normal.png
```

Luego inicia el programa pasando `assets/models/mi_avion/scene.gltf` como argumento. Assimp recorre la escena, sus mallas y materiales; stb_image carga las imágenes utilizadas como textura de color.

### Ajustar orientación y tamaño

En `src/main.cpp` hay dos valores pensados para adaptar el modelo:

```cpp
constexpr float MODEL_SCALE = 1.0f; // Multiplicador del tamaño del avión.
const glm::vec3 MODEL_FORWARD_AXIS(1.0f, 0.0f, 0.0f); // El modelo incluido apunta hacia su eje local +X.
```

El morro del modelo incluido apunta al eje local **+X**. El GLB se centró alrededor del origen y se convirtió de centímetros a metros. Si sustituyes el avión, vuelve a comprobar el eje frontal y cambia `MODEL_FORWARD_AXIS` según corresponda. Ajusta `MODEL_SCALE` solo si el nuevo modelo aparece demasiado grande o pequeño.

Los cuatro puntos de control se editan en `src/main.cpp`, en el arreglo `controlPoints`. Cada punto es una posición `(x, y, z)` en unidades del mundo.

## 6. Controles durante la ejecución

| Tecla | Acción |
|---|---|
| W / S | Mover la cámara hacia delante / atrás |
| A / D | Mover la cámara a izquierda / derecha |
| Q / E | Bajar / subir la cámara |
| P | Pausar / reanudar el avión |
| + o = | Aumentar la velocidad |
| - | Reducir la velocidad |
| T | Reiniciar el vuelo desde el punto inicial |
| R | Reiniciar la cámara |
| Esc | Cerrar la aplicación |

El avión realiza un recorrido de P0 a P3 y se detiene al llegar al final. Pulsa `T` para comenzar de nuevo. La trayectoria se dibuja como una línea azul clara.

## 7. Decisiones y límites conocidos

- La curva es una **Bézier cúbica** con cuatro puntos de control.
- La tabla de longitud de arco aproxima la distancia recorrida y reduce los cambios de velocidad visual provocados por el parámetro `t`.
- La orientación se obtiene alineando el eje frontal local del avión con la tangente de la curva. No se incluye todavía inclinación lateral o banking.
- El shader implementa Phong clásico por componentes ambiental, difuso y especular. No es un renderizador PBR completo.
- La carga de materiales utiliza la textura base color/difusa y un color difuso de reserva. No se pretende reproducir todas las extensiones PBR de glTF.
- El punto de pivote y la escala original del modelo dependen de cómo se exportó el archivo 3D.
- Los shaders, el código C++ y la configuración de CMake están separados por responsabilidad.
- Los archivos JSON (`CMakePresets.json` y `vcpkg.json`) no admiten comentarios según su sintaxis; por eso no es posible comentar literalmente cada línea de esos dos archivos. Las líneas de C++, GLSL y CMake sí llevan comentarios explicativos.

## 8. Solución rápida de problemas

**No encuentra el modelo.** Verifica la ruta y extensión. El nombre predeterminado es `assets/models/airplane.glb`. Para `.gltf`, pasa la ruta de `scene.gltf` como argumento.

**El avión no tiene textura.** En un `.gltf`, comprueba que las imágenes referenciadas estén dentro de la estructura de carpetas original. Comprueba también que el material tenga una textura base color o difusa.

**El avión aparece girado.** Cambia `MODEL_FORWARD_AXIS` hasta que coincida con el eje local hacia el morro del modelo.

**El avión es demasiado grande o pequeño.** Cambia `MODEL_SCALE`.

**CMake no encuentra vcpkg.** Verifica que `VCPKG_ROOT` apunta a la carpeta raíz de vcpkg y reinicia Visual Studio después de crear la variable de entorno.

**La ventana se abre pero no aparece el avión.** Revisa la consola de Visual Studio. Assimp imprime el error de importación y el programa continúa mostrando la trayectoria para facilitar el diagnóstico.

**Los shaders no se encuentran.** Ejecuta el programa desde el proyecto compilado con CMake; el ejecutable busca los archivos en la carpeta `shaders` del proyecto y CMake también los copia junto al ejecutable.

## 9. Documentación adicional

- [Arquitectura y explicación técnica](docs/ARQUITECTURA.md)
- [Guion para la exposición](docs/GUIÓN_EXPOSICIÓN.md)
- [Publicación en GitHub y trabajo en otro computador](docs/GITHUB_Y_EQUIPO.md)

## 10. Referencias oficiales

- [Visual Studio: proyectos CMake](https://learn.microsoft.com/cpp/build/cmake-projects-in-visual-studio)
- [vcpkg: dependencias en modo manifiesto](https://learn.microsoft.com/vcpkg/consume/manifest-mode)
- [Assimp: formatos compatibles](https://github.com/assimp/assimp/blob/master/doc/Fileformats.md)
- [GLFW: guía de compilación y enlace](https://www.glfw.org/docs/latest/build_guide.html)


## Nota de configuración para Visual Studio 2026 (Ninja)

Los presets de Windows indican explícitamente `CC=cl` y `CXX=cl`, y usan la arquitectura x64 como entorno externo. Visual Studio debe preparar el entorno MSVC antes de configurar CMake. Si aparece `CMAKE_CXX_COMPILER not set`, verifica que esté instalado el componente MSVC x64/x86 Build Tools y configura desde Developer PowerShell for VS 2026.
