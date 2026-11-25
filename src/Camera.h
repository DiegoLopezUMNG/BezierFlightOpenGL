#pragma once // Evita incluir este encabezado más de una vez.

#include <glm/glm.hpp> // Proporciona los tipos vectoriales y matriciales usados por la cámara.

enum class CameraMovement { // Define los sentidos de movimiento que admite la cámara.
    Forward, // Desplaza la cámara hacia donde está mirando.
    Backward, // Desplaza la cámara en sentido contrario a su mirada.
    Left, // Desplaza la cámara hacia su izquierda local.
    Right, // Desplaza la cámara hacia su derecha local.
    Up, // Desplaza la cámara hacia arriba en el mundo.
    Down // Desplaza la cámara hacia abajo en el mundo.
}; // Termina la enumeración de movimientos.

class Camera { // Agrupa la posición, orientación y velocidad de la cámara virtual.
public: // Expone los métodos usados por el programa principal.
    glm::vec3 Position; // Guarda la posición de la cámara en coordenadas del mundo.
    glm::vec3 Front; // Guarda el vector unitario hacia donde apunta la cámara.
    glm::vec3 Up; // Guarda el vector vertical local de la cámara.
    glm::vec3 Right; // Guarda el vector horizontal derecho de la cámara.
    glm::vec3 WorldUp; // Guarda la dirección vertical global del escenario.
    float Yaw; // Guarda el ángulo horizontal de orientación.
    float Pitch; // Guarda el ángulo vertical de orientación.
    float MovementSpeed; // Guarda la velocidad de desplazamiento en unidades por segundo.

    Camera(glm::vec3 position = glm::vec3(0.0f, 8.0f, 28.0f)); // Inicializa la cámara en una posición que mira al escenario.
    glm::mat4 GetViewMatrix() const; // Construye la matriz de vista con lookAt.
    void ProcessKeyboard(CameraMovement direction, float deltaTime); // Actualiza la posición según el movimiento y el tiempo.
    void ProcessMouseMovement(float xOffset, float yOffset); // Actualiza yaw y pitch con el desplazamiento del ratón.
    void Reset(); // Devuelve la cámara a su posición inicial.

private: // Oculta los cálculos que mantienen coherente la orientación.
    void updateCameraVectors(); // Recalcula Front, Right y Up a partir de los ángulos.
}; // Termina la declaración de Camera.
