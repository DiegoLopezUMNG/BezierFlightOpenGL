# Guion para explicar el proyecto

Duración sugerida: 5 a 7 minutos. Se puede repartir entre integrantes, asignando un módulo a cada persona.

## 1. Introducción

“Este proyecto consiste en una simulación tridimensional desarrollada en C++ y OpenGL. Importamos un avión desde un archivo en formato glTF o GLB y lo animamos para que recorra una trayectoria definida mediante una curva cúbica de Bézier. Además, implementamos una cámara controlada por teclado y un sistema de iluminación Phong. La arquitectura está dividida por módulos y los shaders se encuentran en archivos externos.”

## 2. Arquitectura del software

“Separamos el programa según sus responsabilidades. `main.cpp` coordina el bucle principal; `Model` se encarga de importar la escena 3D con Assimp; `BezierPath` calcula el recorrido; `Camera` construye la vista y procesa desplazamientos; y `Shader` carga, compila y enlaza los programas GLSL. Esta separación mejora la legibilidad, el mantenimiento y la posibilidad de modificar una parte sin alterar todo el programa.”

**Términos técnicos:** modularidad, encapsulamiento, importación de activos, jerarquía de escena, pipeline gráfico y shader program.

## 3. Importación del avión

“Para importar el modelo usamos Assimp, una biblioteca de importación de recursos 3D. Assimp lee el archivo glTF o GLB y entrega una escena compuesta por nodos, mallas y materiales. Recorremos la jerarquía de nodos y acumulamos sus transformaciones. Luego convertimos los vértices, normales, coordenadas UV e índices a estructuras propias. Los vértices y los índices se envían a la GPU mediante VBO y EBO, y el VAO conserva la configuración de sus atributos.”

“Las texturas se cargan con stb_image. Así podemos utilizar el color base o difuso del material para dar apariencia al avión.”

**Términos técnicos:** Assimp, glTF, GLB, malla o mesh, nodo, jerarquía, coordenadas UV, normal, VAO, VBO, EBO y textura.

## 4. Recorrido con Bézier

“El movimiento está definido por una curva cúbica de Bézier, que utiliza cuatro puntos de control: P0, P1, P2 y P3. Los extremos son el inicio y el final del recorrido; los dos puntos intermedios controlan la curvatura. La ecuación evalúa una posición para cada parámetro t entre cero y uno.”

“También calculamos la derivada de la curva. Esta derivada es la tangente, es decir, el vector que indica la dirección instantánea del movimiento. La usamos para orientar el avión hacia donde avanza.”

“Como t no equivale a distancia, precalculamos una tabla de longitud de arco. Así relacionamos la distancia recorrida con el parámetro de la curva y el avance se ve más uniforme.”

**Términos técnicos:** curva paramétrica, puntos de control, polinomio de Bernstein, parámetro t, derivada, tangente, longitud de arco e interpolación.

## 5. Matrices de transformación y orientación

“Para colocar el avión en la escena construimos tres matrices: escala, rotación y traslación. La matriz de modelo es T por R por S. La escala adapta las dimensiones; la rotación alinea el eje frontal local del avión con la tangente; y la traslación lo ubica sobre la curva.”

“Después aplicamos la matriz de vista de la cámara y la matriz de proyección. En el vertex shader el orden de transformación es proyección por vista por modelo por posición local.”

**Términos técnicos:** espacio local, espacio de mundo, matriz de modelo, matriz de vista, matriz de proyección, cuaternión, perspectiva y coordenadas de recorte.

## 6. Cámara y teclado

“La cámara se implementa con una posición y tres vectores: Front, Right y Up. Estos vectores describen hacia dónde mira y cuáles son sus ejes locales. La función `lookAt` genera la matriz de vista. GLFW consulta el teclado y permite trasladar la cámara con WASD y Q/E. También podemos pausar, cambiar la velocidad y reiniciar el vuelo.”

“Usamos `deltaTime` para que la velocidad de desplazamiento sea independiente de la tasa de fotogramas.”

**Términos técnicos:** cámara virtual, vector de dirección, base ortogonal, matriz de vista, entrada de usuario, evento y delta time.

## 7. Iluminación Phong y shaders

“La iluminación se calcula en un fragment shader separado del código C++. El modelo Phong se compone de tres términos. El ambiental representa una iluminación general; el difuso depende del ángulo entre la normal y la luz; y el especular produce reflejos brillantes según la posición de la cámara.”

“El vertex shader transforma los vértices y calcula la posición y normal en el mundo. El fragment shader combina textura, color e iluminación para obtener el color final de cada fragmento.”

**Términos técnicos:** vertex shader, fragment shader, GLSL, uniforme, normal, reflexión, componente ambiental, Lambert/difuso y especular.

## 8. Cierre

“En conclusión, integramos importación de modelos, geometría paramétrica, álgebra lineal, control de cámara y renderizado en tiempo real. La arquitectura modular permite cambiar el modelo, modificar los puntos de control, ajustar la iluminación o reemplazar los shaders sin reescribir el programa completo.”

## Preguntas que podrían hacerles

**¿Por qué se usa una curva de Bézier?**  
Porque permite definir una trayectoria suave usando pocos puntos de control y controlar la forma del recorrido sin almacenar una lista larga de posiciones.

**¿Para qué se usa la derivada?**  
Para obtener la tangente de la curva. Su dirección indica hacia dónde debe orientarse el morro del avión.

**¿Por qué se usa una tabla de longitud de arco?**  
Porque el parámetro t no avanza a velocidad espacial constante. La tabla aproxima la distancia recorrida y reduce esa variación.

**¿Qué diferencia hay entre Model, View y Projection?**  
Model ubica el objeto en el mundo; View representa la cámara; Projection transforma la escena al volumen de visión perspectiva.

**¿Qué hace VAO, VBO y EBO?**  
VBO almacena los atributos de vértice; EBO almacena índices que reutilizan vértices; VAO conserva cómo están organizados esos atributos y buffers.

**¿Qué diferencia hay entre vertex shader y fragment shader?**  
El vertex shader procesa vértices y sus transformaciones. El fragment shader calcula el color de los fragmentos que pueden formar los píxeles visibles.

**¿Qué aporta Phong?**  
Una aproximación de iluminación que combina ambiente, reflexión difusa y reflexión especular.

**¿El avión gira o se inclina como un avión real?**  
La versión base alinea su eje frontal con la tangente de la trayectoria. No calcula banking, fuerzas ni aerodinámica; eso sería una ampliación posterior.

**¿Por qué glTF?**  
Es un formato de intercambio de escenas 3D que puede incluir geometría, materiales y recursos de textura. GLB es su variante binaria, útil para distribuir un único archivo.
