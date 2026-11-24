#version 330 core // Declara que este shader usa GLSL 3.30 Core.
in vec3 FragPos; // Recibe la posición del fragmento en el espacio del mundo.
in vec3 Normal; // Recibe la normal interpolada desde el vertex shader.
in vec2 TexCoords; // Recibe las coordenadas UV interpoladas.
out vec4 FragColor; // Define el color final que se escribirá en el framebuffer.
uniform sampler2D uDiffuseMap; // Representa la textura de color del material.
uniform bool uHasTexture; // Indica si la malla tiene una textura difusa válida.
uniform vec3 uObjectColor; // Proporciona el color del material cuando no hay textura.
uniform vec3 uLightPosition; // Representa la posición de la fuente luminosa en el mundo.
uniform vec3 uLightColor; // Representa el color de la fuente luminosa.
uniform vec3 uViewPosition; // Representa la posición de la cámara en el mundo.
uniform float uAmbientStrength; // Controla la intensidad del término ambiental.
uniform float uSpecularStrength; // Controla la intensidad del reflejo especular.
uniform float uShininess; // Controla la concentración del reflejo especular.
void main() { // Define el cálculo de iluminación que se realiza para cada fragmento.
    vec3 albedo = uHasTexture ? texture(uDiffuseMap, TexCoords).rgb : uObjectColor; // Obtiene el color base desde la textura o el material.
    vec3 normal = normalize(Normal); // Normaliza la normal interpolada para los productos escalares.
    vec3 lightDirection = normalize(uLightPosition - FragPos); // Calcula la dirección desde el fragmento hacia la luz.
    vec3 viewDirection = normalize(uViewPosition - FragPos); // Calcula la dirección desde el fragmento hacia la cámara.
    float diffuseFactor = max(dot(normal, lightDirection), 0.0); // Calcula Lambert y descarta la iluminación desde detrás de la superficie.
    vec3 reflectedDirection = reflect(-lightDirection, normal); // Refleja la dirección de la luz alrededor de la normal.
    float specularFactor = pow(max(dot(viewDirection, reflectedDirection), 0.0), uShininess); // Calcula la intensidad del brillo especular.
    vec3 ambient = uAmbientStrength * uLightColor * albedo; // Calcula la iluminación ambiental aproximada.
    vec3 diffuse = diffuseFactor * uLightColor * albedo; // Calcula la contribución difusa según el ángulo de incidencia.
    vec3 specular = uSpecularStrength * specularFactor * uLightColor; // Calcula el reflejo especular de la superficie.
    vec3 result = ambient + diffuse + specular; // Suma los tres componentes del modelo de iluminación Phong.
    result = pow(result, vec3(1.0 / 2.2)); // Aplica una corrección gamma sencilla para presentar el color.
    FragColor = vec4(result, 1.0); // Escribe el color final con opacidad completa.
} // Termina main del fragment shader.
