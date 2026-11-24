#version 330 core // Declara que este shader usa GLSL 3.30 Core.
out vec4 FragColor; // Define el color que se escribirá para cada fragmento de la línea.
uniform vec3 uLineColor; // Recibe el color elegido para la trayectoria.
void main() { // Define el cálculo de color de la línea.
    FragColor = vec4(uLineColor, 1.0); // Dibuja la trayectoria con opacidad completa.
} // Termina main del fragment shader de línea.
