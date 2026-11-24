#pragma once // Evita inclusiones repetidas de este encabezado.

#include <glm/glm.hpp> // Proporciona vectores tridimensionales.
#include <array> // Permite guardar exactamente cuatro puntos de control.
#include <vector> // Permite guardar la tabla de longitudes acumuladas.

class BezierPath { // Representa una curva cúbica de Bézier en tres dimensiones.
public: // Expone las operaciones de evaluación y recorrido.
    explicit BezierPath(const std::array<glm::vec3, 4>& controlPoints, int samples = 300); // Construye la curva y precalcula su longitud aproximada.
    glm::vec3 position(float t) const; // Evalúa la posición de la curva para un parámetro entre cero y uno.
    glm::vec3 tangent(float t) const; // Evalúa la derivada, que indica la dirección de avance.
    glm::vec3 positionAtDistance(float distance) const; // Devuelve la posición para una distancia recorrida sobre la curva.
    glm::vec3 tangentAtDistance(float distance) const; // Devuelve la dirección de la curva en una distancia recorrida.
    float getLength() const; // Devuelve la longitud aproximada total.
    const std::array<glm::vec3, 4>& getControlPoints() const; // Permite consultar los cuatro puntos de control.

private: // Guarda los datos usados para aproximar la longitud de arco.
    std::array<glm::vec3, 4> points; // Conserva P0, P1, P2 y P3.
    std::vector<float> cumulativeLengths; // Conserva la distancia acumulada de cada muestra.
    float totalLength = 0.0f; // Conserva la longitud aproximada total de la curva.
    int sampleCount = 300; // Conserva la cantidad de segmentos de la aproximación.
    float parameterAtDistance(float distance) const; // Convierte una distancia de recorrido al parámetro t.
    void buildArcLengthTable(); // Construye la tabla de longitudes acumuladas.
}; // Termina la declaración de BezierPath.
