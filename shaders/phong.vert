#version 330 core // Declara que este shader usa GLSL 3.30 Core.
layout (location = 0) in vec3 aPosition; // Recibe la posición del vértice desde el VAO.
layout (location = 1) in vec3 aNormal; // Recibe la normal del vértice desde el VAO.
layout (location = 2) in vec2 aTexCoords; // Recibe las coordenadas UV del vértice desde el VAO.
uniform mat4 uModel; // Recibe la matriz de modelo que posiciona el objeto en el mundo.
uniform mat4 uView; // Recibe la matriz de vista que representa la cámara.
uniform mat4 uProjection; // Recibe la matriz de proyección perspectiva.
out vec3 FragPos; // Entrega la posición del fragmento en coordenadas del mundo.
out vec3 Normal; // Entrega la normal transformada al espacio del mundo.
out vec2 TexCoords; // Entrega las coordenadas UV para consultar la textura.
void main() { // Define la etapa que transforma cada vértice.
    vec4 worldPosition = uModel * vec4(aPosition, 1.0); // Transforma la posición local al espacio del mundo.
    FragPos = worldPosition.xyz; // Guarda la posición tridimensional para el cálculo de iluminación.
    Normal = mat3(transpose(inverse(uModel))) * aNormal; // Aplica la matriz normal para conservar la orientación bajo escalados.
    TexCoords = aTexCoords; // Pasa las coordenadas UV a la etapa de fragmentos.
    gl_Position = uProjection * uView * worldPosition; // Transforma el vértice hasta el espacio de recorte.
} // Termina main del vertex shader.
