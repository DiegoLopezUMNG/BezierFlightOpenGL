#include "Camera.h" // Incluye la declaración de la cámara.
#include <glm/gtc/matrix_transform.hpp> // Proporciona lookAt para construir la matriz de vista.
#include <glm/geometric.hpp> // Proporciona operaciones como normalize y cross.
#include <cmath> // Proporciona seno, coseno y otras operaciones trigonométricas.

Camera::Camera(glm::vec3 position) // Define el constructor que recibe la posición inicial.
    : Position(position), Front(0.0f, 0.0f, -1.0f), Up(0.0f, 1.0f, 0.0f), Right(1.0f, 0.0f, 0.0f), WorldUp(0.0f, 1.0f, 0.0f), Yaw(-90.0f), Pitch(-24.0f), MovementSpeed(8.0f) { // Inicializa los vectores y ángulos de la cámara.
    updateCameraVectors(); // Calcula los vectores definitivos a partir del yaw y el pitch.
} // Termina el constructor.

glm::mat4 Camera::GetViewMatrix() const { // Construye la matriz que transforma el mundo al espacio de cámara.
    return glm::lookAt(Position, Position + Front, Up); // Orienta la cámara desde su posición hacia el vector Front.
} // Termina GetViewMatrix.

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) { // Procesa un desplazamiento independiente de la tasa de fotogramas.
    const float velocity = MovementSpeed * deltaTime; // Convierte la velocidad por segundo en desplazamiento del fotograma.
    if (direction == CameraMovement::Forward) { // Comprueba si se solicitó avanzar.
        Position += Front * velocity; // Desplaza la cámara hacia delante.
    } // Cierra el movimiento hacia delante.
    if (direction == CameraMovement::Backward) { // Comprueba si se solicitó retroceder.
        Position -= Front * velocity; // Desplaza la cámara hacia atrás.
    } // Cierra el movimiento hacia atrás.
    if (direction == CameraMovement::Left) { // Comprueba si se solicitó moverse a la izquierda.
        Position -= Right * velocity; // Desplaza la cámara hacia su izquierda local.
    } // Cierra el movimiento a la izquierda.
    if (direction == CameraMovement::Right) { // Comprueba si se solicitó moverse a la derecha.
        Position += Right * velocity; // Desplaza la cámara hacia su derecha local.
    } // Cierra el movimiento a la derecha.
    if (direction == CameraMovement::Up) { // Comprueba si se solicitó subir.
        Position += WorldUp * velocity; // Desplaza la cámara verticalmente hacia arriba.
    } // Cierra el movimiento hacia arriba.
    if (direction == CameraMovement::Down) { // Comprueba si se solicitó bajar.
        Position -= WorldUp * velocity; // Desplaza la cámara verticalmente hacia abajo.
    } // Cierra el movimiento hacia abajo.
} // Termina ProcessKeyboard.


void Camera::ProcessMouseMovement(float xOffset, float yOffset) { // Gira la cámara según el movimiento del ratón.
    constexpr float sensitivity = 0.12f; // Convierte píxeles del cursor en grados de rotación.
    Yaw += xOffset * sensitivity; // Modifica el ángulo horizontal de la mirada.
    Pitch = glm::clamp(Pitch + yOffset * sensitivity, -89.0f, 89.0f); // Limita el giro vertical para evitar singularidades.
    updateCameraVectors(); // Recalcula la base de la cámara con los nuevos ángulos.
} // Termina ProcessMouseMovement.

void Camera::Reset() { // Restablece los valores de la cámara.
    Position = glm::vec3(0.0f, 8.0f, 28.0f); // Recupera la posición inicial.
    Yaw = -90.0f; // Recupera el ángulo horizontal inicial.
    Pitch = -24.0f; // Recupera el ángulo vertical inicial.
    updateCameraVectors(); // Recalcula los vectores de orientación.
} // Termina Reset.

void Camera::updateCameraVectors() { // Calcula la base ortogonal de la cámara a partir de yaw y pitch.
    glm::vec3 front; // Reserva el vector temporal de dirección.
    front.x = std::cos(glm::radians(Yaw)) * std::cos(glm::radians(Pitch)); // Calcula la componente x mediante los ángulos.
    front.y = std::sin(glm::radians(Pitch)); // Calcula la componente y mediante el ángulo vertical.
    front.z = std::sin(glm::radians(Yaw)) * std::cos(glm::radians(Pitch)); // Calcula la componente z mediante los ángulos.
    Front = glm::normalize(front); // Normaliza la dirección para que su longitud sea uno.
    Right = glm::normalize(glm::cross(Front, WorldUp)); // Calcula el eje derecho perpendicular a la mirada y al eje vertical.
    Up = glm::normalize(glm::cross(Right, Front)); // Calcula el eje vertical local perpendicular a los otros dos.
} // Termina updateCameraVectors.
