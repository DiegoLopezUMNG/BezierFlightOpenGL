#version 330 core // Declara que este shader usa GLSL 3.30 Core.
layout (location = 0) in vec3 aPosition; // Recibe la posición de cada punto de la trayectoria.
uniform mat4 uView; // Recibe la matriz de vista de la cámara.
uniform mat4 uProjection; // Recibe la matriz de proyección perspectiva.
void main() { // Define la etapa de vértices de la trayectoria.
    gl_Position = uProjection * uView * vec4(aPosition, 1.0); // Transforma la posición de mundo al espacio de recorte.
} // Termina main del vertex shader de línea.
