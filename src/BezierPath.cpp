#include "BezierPath.h" // Incluye la declaración de la curva de Bézier.
#include <algorithm> // Proporciona clamp y lower_bound.
#include <iterator> // Proporciona distance para convertir iteradores en índices.

BezierPath::BezierPath(const std::array<glm::vec3, 4>& controlPoints, int samples) // Define el constructor de la curva.
    : points(controlPoints), sampleCount(std::max(samples, 2)) { // Copia los puntos y garantiza un mínimo de muestras.
    buildArcLengthTable(); // Precalcula las distancias para recorrer la curva de forma más uniforme.
} // Termina el constructor.

glm::vec3 BezierPath::position(float t) const { // Evalúa la ecuación de Bézier cúbica.
    const float u = 1.0f - glm::clamp(t, 0.0f, 1.0f); // Calcula el complemento de t dentro del intervalo válido.
    const float tClamped = glm::clamp(t, 0.0f, 1.0f); // Limita t para evitar extrapolaciones no deseadas.
    return (u * u * u) * points[0] + // Aporta el peso Bernstein del primer punto de control.
           (3.0f * u * u * tClamped) * points[1] + // Aporta el peso Bernstein del segundo punto de control.
           (3.0f * u * tClamped * tClamped) * points[2] + // Aporta el peso Bernstein del tercer punto de control.
           (tClamped * tClamped * tClamped) * points[3]; // Aporta el peso Bernstein del cuarto punto de control.
} // Termina position.

glm::vec3 BezierPath::tangent(float t) const { // Evalúa la derivada de la curva cúbica.
    const float tClamped = glm::clamp(t, 0.0f, 1.0f); // Limita el parámetro al intervalo de la curva.
    const float u = 1.0f - tClamped; // Calcula el complemento del parámetro.
    const glm::vec3 derivative = 3.0f * u * u * (points[1] - points[0]) + // Calcula la contribución del primer tramo de control.
                                 6.0f * u * tClamped * (points[2] - points[1]) + // Calcula la contribución del tramo central.
                                 3.0f * tClamped * tClamped * (points[3] - points[2]); // Calcula la contribución del último tramo de control.
    if (glm::length(derivative) < 0.0001f) { // Detecta una derivada casi nula.
        return glm::vec3(0.0f, 0.0f, 1.0f); // Devuelve una dirección segura para evitar normalizar un vector nulo.
    } // Cierra la comprobación de derivada nula.
    return glm::normalize(derivative); // Devuelve la tangente unitaria de la curva.
} // Termina tangent.

void BezierPath::buildArcLengthTable() { // Construye una tabla para aproximar la longitud de arco.
    cumulativeLengths.clear(); // Elimina muestras anteriores si se reconstruye la curva.
    cumulativeLengths.reserve(static_cast<std::size_t>(sampleCount + 1)); // Reserva espacio para las muestras y el punto inicial.
    cumulativeLengths.push_back(0.0f); // Define la distancia acumulada inicial como cero.
    glm::vec3 previous = position(0.0f); // Evalúa el punto inicial de la curva.
    totalLength = 0.0f; // Reinicia la longitud acumulada.
    for (int i = 1; i <= sampleCount; ++i) { // Recorre los parámetros uniformemente espaciados.
        const float t = static_cast<float>(i) / static_cast<float>(sampleCount); // Calcula el parámetro de la muestra actual.
        const glm::vec3 current = position(t); // Evalúa la posición de la curva en la muestra.
        totalLength += glm::length(current - previous); // Acumula la distancia entre dos muestras consecutivas.
        cumulativeLengths.push_back(totalLength); // Guarda la distancia acumulada hasta esta muestra.
        previous = current; // Actualiza el punto anterior para la siguiente iteración.
    } // Termina el recorrido de muestras.
} // Termina buildArcLengthTable.

float BezierPath::parameterAtDistance(float distance) const { // Busca qué parámetro t corresponde a una distancia recorrida.
    if (totalLength <= 0.0001f) { // Comprueba que la curva tenga una longitud útil.
        return 0.0f; // Evita divisiones o búsquedas sobre una tabla degenerada.
    } // Cierra la comprobación de longitud.
    const float wrappedDistance = glm::clamp(distance, 0.0f, totalLength); // Limita la distancia al recorrido para que el avión se detenga al llegar al final.
    const auto upper = std::lower_bound(cumulativeLengths.begin(), cumulativeLengths.end(), wrappedDistance); // Encuentra la primera muestra que alcanza la distancia solicitada.
    if (upper == cumulativeLengths.begin()) { // Comprueba si la distancia está al comienzo de la curva.
        return 0.0f; // Devuelve el primer parámetro.
    } // Cierra la comprobación inicial.
    if (upper == cumulativeLengths.end()) { // Comprueba si la búsqueda alcanzó el final de la tabla.
        return 1.0f; // Devuelve el último parámetro.
    } // Cierra la comprobación final.
    const std::size_t upperIndex = static_cast<std::size_t>(std::distance(cumulativeLengths.begin(), upper)); // Convierte el iterador superior a índice.
    const std::size_t lowerIndex = upperIndex - 1; // Obtiene la muestra inmediatamente anterior.
    const float lowerDistance = cumulativeLengths[lowerIndex]; // Lee la distancia acumulada inferior.
    const float upperDistance = cumulativeLengths[upperIndex]; // Lee la distancia acumulada superior.
    const float segmentLength = upperDistance - lowerDistance; // Calcula la longitud entre ambas muestras.
    const float fraction = segmentLength > 0.0001f ? (wrappedDistance - lowerDistance) / segmentLength : 0.0f; // Interpola dentro del segmento sin dividir entre cero.
    const float lowerT = static_cast<float>(lowerIndex) / static_cast<float>(sampleCount); // Convierte el índice inferior a parámetro t.
    const float upperT = static_cast<float>(upperIndex) / static_cast<float>(sampleCount); // Convierte el índice superior a parámetro t.
    return glm::mix(lowerT, upperT, fraction); // Devuelve el parámetro interpolado para la distancia deseada.
} // Termina parameterAtDistance.

glm::vec3 BezierPath::positionAtDistance(float distance) const { // Evalúa la posición usando distancia recorrida.
    return position(parameterAtDistance(distance)); // Convierte distancia a t y evalúa la curva.
} // Termina positionAtDistance.

glm::vec3 BezierPath::tangentAtDistance(float distance) const { // Evalúa la dirección usando distancia recorrida.
    return tangent(parameterAtDistance(distance)); // Convierte distancia a t y evalúa la derivada.
} // Termina tangentAtDistance.

float BezierPath::getLength() const { // Expone la longitud total calculada.
    return totalLength; // Devuelve la suma de los segmentos aproximados.
} // Termina getLength.

const std::array<glm::vec3, 4>& BezierPath::getControlPoints() const { // Expone los puntos de control para consulta.
    return points; // Devuelve una referencia constante a P0, P1, P2 y P3.
} // Termina getControlPoints.
