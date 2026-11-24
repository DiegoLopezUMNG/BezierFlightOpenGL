# Publicar el proyecto en GitHub y abrirlo en otro computador

## 1. Preparar el repositorio

Antes de publicar, verifica que:

- El programa compila en tu computador.
- `README.md`, `CMakeLists.txt`, `CMakePresets.json` y `vcpkg.json` estén incluidos.
- `assets/models/airplane.glb` esté incluido. El archivo ya incorpora las texturas de color utilizadas por el proyecto.
- No se incluyan carpetas `out/`, `build/`, `vcpkg_installed/`, archivos `.exe`, DLL locales ni cachés del IDE. `.gitignore` ya excluye los elementos habituales.

### Crear un repositorio desde la página de GitHub

1. Inicia sesión en GitHub desde el navegador.
2. Crea un repositorio nuevo con un nombre como `BezierFlightOpenGL`.
3. Para evitar una fusión inicial innecesaria, créalo vacío, sin README, licencia ni `.gitignore` adicionales: estos archivos ya están en el proyecto.
4. Copia la URL HTTPS del repositorio.

### Subir el proyecto desde PowerShell

Abre PowerShell en la carpeta raíz del proyecto y ejecuta:

```powershell
git init
git add .
git status
git commit -m "Primer commit: simulacion de vuelo OpenGL"
git branch -M main
git remote add origin https://github.com/TU-USUARIO/BezierFlightOpenGL.git
git push -u origin main
```

Sustituye la URL de ejemplo por la dirección real de tu repositorio. `git status` permite revisar qué archivos van a subirse antes de confirmar el primer commit.

## 2. ¿Debo subir el modelo 3D a GitHub?

Depende de la licencia y del tamaño del modelo:

El proyecto ya contiene `assets/models/airplane.glb`, que reúne la geometría y las texturas de color en un solo archivo. El archivo generado ocupa aproximadamente 13 MB, por lo que se incluye directamente en el ZIP. Antes de subirlo a un repositorio público, comprueba que la licencia del modelo original permita compartirlo. Si no tienes permiso para redistribuirlo, quita el GLB del repositorio y acuerda con el equipo una forma autorizada de obtenerlo.

No subas credenciales, tokens, claves privadas ni datos personales.

## 3. Cómo abrir el proyecto en otro PC

En el computador de otro integrante:

1. Instalar Visual Studio 2026 con la carga de trabajo **Desarrollo para el escritorio con C++**, Windows SDK y herramientas de CMake.
2. Instalar Git.
3. Instalar vcpkg y configurar la variable de usuario `VCPKG_ROOT`, como se explica en el README.
4. Clonar el repositorio:

```powershell
git clone https://github.com/TU-USUARIO/BezierFlightOpenGL.git
Set-Location BezierFlightOpenGL
```

5. El modelo ya está en `assets/models/airplane.glb` si se incluyó al publicar el repositorio. Si el GLB se excluyó por licencia, cada integrante tendrá que conseguirlo por la vía autorizada y colocarlo en esa ruta.
6. Abrir la carpeta desde Visual Studio con **Archivo > Abrir > Carpeta**.
7. Seleccionar el preset `windows-vs2026-debug`, esperar a que vcpkg instale las dependencias y compilar.
8. Ejecutar `BezierFlight`.

Cada PC instala sus propias dependencias en `vcpkg_installed/`. Esa carpeta se genera localmente y no necesita subirse al repositorio. El manifiesto `vcpkg.json` permite que todos instalen el conjunto de bibliotecas declarado por el proyecto.

## 4. Actualizar el proyecto entre integrantes

Antes de comenzar una sesión:

```powershell
git pull
```

Después de modificar el código:

```powershell
git status
git add .
git commit -m "Describe brevemente el cambio"
git push
```

Eviten modificar simultáneamente el mismo archivo principal. Para cambios grandes, creen una rama:

```powershell
git switch -c nombre-de-la-mejora
```

Después pueden subirla y crear un Pull Request para revisar el cambio antes de integrarlo a `main`.

## 5. Recomendación de entrega

En el repositorio conviene incluir el código fuente, shaders, documentación y configuración CMake/vcpkg. El GLB de este proyecto también está incluido en el paquete entregado; publícalo en GitHub únicamente si la licencia del modelo permite redistribuirlo. Para la entrega académica, adjunten también una versión ejecutable o un video de demostración solo si la consigna lo solicita y si el entorno de evaluación es compatible. La opción reproducible principal es compilar el código fuente en el PC de destino.
