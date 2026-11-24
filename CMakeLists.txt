cmake_minimum_required(VERSION 3.24) # Exige una versión moderna de CMake para usar los presets del proyecto.
project(BezierFlightOpenGL VERSION 1.0.0 LANGUAGES CXX) # Declara el nombre, versión y lenguaje del programa.

set(CMAKE_CXX_STANDARD 17) # Usa C++17 para disponer de filesystem y características modernas.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Impide compilar con un estándar C++ anterior.
set(CMAKE_CXX_EXTENSIONS OFF) # Mantiene el estándar C++ sin extensiones específicas del compilador.

find_package(OpenGL REQUIRED) # Localiza la implementación OpenGL instalada en el sistema.
find_package(glfw3 CONFIG REQUIRED) # Localiza GLFW para crear la ventana y recibir entradas.
find_package(glad CONFIG REQUIRED) # Localiza GLAD para cargar las funciones de OpenGL.
find_package(glm CONFIG REQUIRED) # Localiza GLM para vectores, matrices y cuaterniones.
find_package(assimp CONFIG REQUIRED) # Localiza Assimp para importar modelos glTF y GLB.
find_path(STB_INCLUDE_DIR NAMES stb_image.h PATH_SUFFIXES stb REQUIRED) # Busca stb_image, usado para cargar texturas.

add_executable(BezierFlight # Crea el ejecutable principal de la simulación.
    src/main.cpp # Incluye el punto de entrada y el bucle principal de renderizado.
    src/Shader.cpp # Incluye la compilación y gestión de shaders.
    src/Camera.cpp # Incluye el movimiento y las matrices de la cámara.
    src/BezierPath.cpp # Incluye la evaluación de la curva y su longitud aproximada.
    src/Model.cpp # Incluye la importación y el dibujado del modelo 3D.
) # Cierra la lista de archivos que forman el ejecutable.

target_include_directories(BezierFlight PRIVATE src ${STB_INCLUDE_DIR}) # Añade las carpetas de encabezados necesarias.
target_link_libraries(BezierFlight PRIVATE OpenGL::GL glfw glad::glad glm::glm assimp::assimp) # Enlaza las bibliotecas requeridas.
target_compile_definitions(BezierFlight PRIVATE "APP_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"") # Guarda la ruta del proyecto para localizar shaders y modelo.

if(MSVC) # Comprueba si el compilador activo es Microsoft Visual C++.
    target_compile_options(BezierFlight PRIVATE /W4 /permissive-) # Activa advertencias y una compilación C++ más estricta.
else() # Configura opciones equivalentes para otros compiladores.
    target_compile_options(BezierFlight PRIVATE -Wall -Wextra -Wpedantic) # Activa advertencias habituales de GCC y Clang.
endif() # Termina la configuración específica del compilador.

add_custom_command(TARGET BezierFlight POST_BUILD # Ejecuta una acción después de compilar correctamente.
    COMMAND ${CMAKE_COMMAND} -E copy_directory # Usa la utilidad portable de CMake para copiar una carpeta.
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders # Indica la carpeta fuente que contiene los shaders.
    $<TARGET_FILE_DIR:BezierFlight>/shaders # Copia los shaders junto al ejecutable compilado.
) # Cierra la acción posterior a la compilación.
