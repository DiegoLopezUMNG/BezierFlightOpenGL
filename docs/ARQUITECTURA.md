# Arquitectura del software y funcionamiento técnico

## 1. Objetivo

La aplicación representa un avión 3D que recorre una trayectoria en el espacio. El programa importa la geometría desde glTF/GLB, calcula la posición y orientación en cada fotograma, transforma el modelo con matrices 4x4 y lo renderiza usando OpenGL 3.3 Core y un modelo de iluminación Phong.

## 2. Arquitectura por módulos

| Módulo | Responsabilidad | Elementos clave |
|---|---|---|
| `main.cpp` | Orquestar la aplicación y el bucle de renderizado. | GLFW, entradas, `deltaTime`, matrices, dibujo |
| `BezierPath` | Evaluar la curva y convertir distancia recorrida en parámetro. | P0-P3, Bernstein, derivada, tabla de longitud |
| `Camera` | Mantener la posición y orientación de cámara. | Front, Right, Up, `lookAt` |
| `Model` | Importar y convertir el archivo 3D. | Assimp, nodos, mallas, materiales, texturas |
| `Mesh` | Gestionar buffers y dibujar una malla. | VAO, VBO, EBO, `glDrawElements` |
| `Shader` | Cargar y gestionar programas GLSL. | Compilación, enlazado, uniformes |
| `phong.vert/.frag` | Transformar vértices y calcular iluminación. | Model/View/Projection, normales, Phong |
| `line.vert/.frag` | Representar la trayectoria. | `GL_LINE_STRIP`, color uniforme |

La dependencia principal entre módulos es:

```text
                   +----------------+
                   |    main.cpp    |
                   +----------------+
                    /      |       \
                   /       |        \
          +-----------+ +------+ +----------+
          | BezierPath| |Camera| |  Model   |
          +-----------+ +------+ +----------+
                                     |
                                +---------+
                                |  Mesh   |
                                +---------+

                 +-------------------+
                 |      Shader       |
                 +-------------------+
                     |           |
                phong.*       line.*
```

## 3. Carga del modelo 3D

`Model::Load()` crea un `Assimp::Importer` y lee el archivo. Los indicadores de procesamiento convierten las caras a triángulos, generan normales suaves cuando hacen falta, comparten vértices equivalentes y validan la estructura.

El archivo incluido es `assets/models/airplane.glb`. El ZIP original contenía el OBJ y las imágenes, pero no el `AVION.mtl` que el OBJ declaraba. Para convertirlo se reconstruyó el vínculo entre cada material y su imagen `*_Base_Color.png`. El resultado verificado contiene 21 mallas, 18 materiales y 17 imágenes incrustadas. Dos materiales que no tenían textura de color en el paquete original usan un color neutro de reserva. Los mapas adicionales de normal, altura, rugosidad y metalicidad no se incorporaron: el shader de esta aplicación implementa iluminación Phong y utiliza la textura difusa/base color.

El OBJ declaraba coordenadas en centímetros. Durante la conversión, el modelo se trasladó para centrar su caja envolvente en el origen y sus coordenadas se escalaron a metros. Se mantuvo la orientación del archivo: el morro apunta a +X y la vertical es +Y.

Assimp organiza el modelo como una escena jerárquica. `ProcessNode()` visita cada nodo y acumula la transformación local con la del padre. `ProcessMesh()` transforma las posiciones, corrige las normales con la matriz normal, copia UV e índices y recupera el material. Cada malla se convierte en un `Mesh` con VAO, VBO y EBO.

Las texturas externas se leen con stb_image. Para imágenes embebidas en GLB, Assimp proporciona sus bytes y dimensiones; si llegan comprimidos, stb_image los decodifica desde memoria. El proyecto carga el canal de color base/difuso y conserva el color difuso como alternativa.

## 4. Curva cúbica de Bézier

Una curva cúbica necesita cuatro puntos de control: P0, P1, P2 y P3. P0 y P3 son los extremos; P1 y P2 controlan la forma y la tangencia de entrada y salida.

La ecuación usada es:

\[
B(t)=(1-t)^3P_0+3(1-t)^2tP_1+3(1-t)t^2P_2+t^3P_3,\qquad 0\leq t\leq1
\]

La derivada es:

\[
B'(t)=3(1-t)^2(P_1-P_0)+6(1-t)t(P_2-P_1)+3t^2(P_3-P_2)
\]

La posición del avión es `B(t)`. La derivada `B'(t)` marca la dirección de la curva y se normaliza para obtener la tangente.

### Velocidad aproximadamente uniforme

El parámetro `t` no representa una distancia física. Incrementar `t` uniformemente puede generar aceleraciones visuales. Para reducir ese efecto, `BezierPath::buildArcLengthTable()` muestrea la curva y acumula la distancia entre puntos consecutivos. En cada fotograma se busca en esa tabla la distancia acumulada más cercana y se interpola el parámetro `t` correspondiente.

El avance del avión es:

\[
s_{nuevo}=s_{anterior}+v\Delta t
\]

Donde `s` es la distancia recorrida, `v` la velocidad en unidades del mundo por segundo y `Δt` el tiempo transcurrido desde el fotograma anterior. Al alcanzar la longitud aproximada de la curva, el avión permanece en P3; la tecla T devuelve el avance a cero.

## 5. Orientación del avión

La tangente normalizada representa el vector hacia el que debe apuntar el morro. `RotationBetweenVectors(ejeLocal, tangente)` calcula un cuaternión que alinea el eje frontal del modelo con la dirección del recorrido usando productos vectoriales y escalares. `glm::mat4_cast()` convierte ese cuaternión en matriz 4x4.

El modelo incluido tiene su morro orientado hacia el eje local +X. Por eso `MODEL_FORWARD_AXIS` se define como `(1, 0, 0)` en `main.cpp`. Si se sustituye el modelo, se debe volver a comprobar esta dirección y modificar la constante para que el vector local apunte desde el avión hacia su morro.

La orientación no calcula banking o alabeo. Es una alineación directa con la tangente, suficiente para que el avión siga la trayectoria sin una simulación aerodinámica.

## 6. Transformaciones y matrices

El vertex shader transforma cada vértice con:

\[
P_{clip}=P\,V\,M\,P_{local}
\]

Donde:

- **M (Model):** escala, rota y traslada el avión desde su espacio local hasta el espacio del mundo.
- **V (View):** representa la posición y orientación de la cámara.
- **P (Projection):** aplica la proyección perspectiva y el volumen de visión.

Con vectores columna, el producto se escribe `Projection * View * Model * position`. Por ello el código construye:

\[
M=T\,R\,S
\]

Primero se escala el objeto en su espacio local, luego se rota y finalmente se traslada a la posición de la Bézier.

## 7. Cámara

`Camera` conserva posición y tres vectores ortogonales: `Front`, `Right` y `Up`. La matriz de vista se obtiene con `glm::lookAt(Position, Position + Front, Up)`. El teclado mueve la cámara en sus ejes locales y en el eje vertical global.

La entrada se procesa con GLFW. Los movimientos multiplican la velocidad por `deltaTime`, de modo que el desplazamiento no dependa directamente de los fotogramas por segundo.

## 8. Iluminación Phong

El shader calcula tres componentes:

**Ambiental:** iluminación de fondo aproximada que evita zonas totalmente negras.

\[
I_a=k_aL_a
\]

**Difusa:** depende del ángulo entre la normal de la superficie N y la dirección hacia la luz L.

\[
I_d=k_dL_d\max(N\cdot L,0)
\]

**Especular:** produce un brillo que depende de la dirección de reflexión R y la dirección hacia la cámara V.

\[
I_s=k_sL_s\max(R\cdot V,0)^n
\]

La suma de las tres contribuciones produce el color iluminado. `uShininess` controla la concentración del brillo especular, `uAmbientStrength` la intensidad ambiental y `uSpecularStrength` el peso especular.

## 9. Shaders externos

Los shaders no están incrustados en C++. `Shader` lee archivos de texto, crea objetos GLSL, compila cada etapa, enlaza el programa y muestra los registros cuando falla una compilación.

- `phong.vert`: recibe posición, normal y UV; transforma la posición y normal y pasa datos al fragment shader.
- `phong.frag`: elige color de textura o material y calcula Phong.
- `line.vert`: transforma los puntos muestreados de Bézier.
- `line.frag`: pinta la trayectoria con un color uniforme.

## 10. Bucle de renderizado

Cada iteración hace lo siguiente:

1. Calcula `deltaTime`.
2. Consulta eventos y procesa entradas.
3. Actualiza la distancia recorrida si el vuelo no está pausado.
4. Calcula posición `B(t)` y tangente `B'(t)`.
5. Construye las matrices de traslación, rotación, escala, vista y proyección.
6. Limpia los buffers de color y profundidad.
7. Dibuja la curva como `GL_LINE_STRIP`.
8. Activa el shader Phong, envía uniformes y dibuja las mallas del avión.
9. Intercambia los buffers con `glfwSwapBuffers()`.

## 11. Preparación para otro computador

CMake define los archivos que se compilan y vcpkg administra las dependencias a partir de `vcpkg.json`. `CMakePresets.json` indica a Visual Studio cómo configurar el generador y dónde encontrar el toolchain de vcpkg. De esta forma se versiona la configuración y no las carpetas generadas, los binarios ni las dependencias instaladas.

El modelo `assets/models/airplane.glb` ya forma parte de este proyecto. Antes de publicar el repositorio de forma pública, comprueba que la licencia de la fuente del modelo permita redistribuirlo. La guía de GitHub detalla la publicación y la reconstrucción en otro computador.
