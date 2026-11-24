#pragma once // Evita incluir este encabezado más de una vez durante la compilación.

#include <glad/glad.h> // Proporciona los tipos y las funciones modernas de OpenGL.
#include <glm/glm.hpp> // Proporciona los tipos de vectores y matrices de GLM.
#include <string> // Permite recibir nombres de uniformes y rutas de archivos.

class Shader { // Encapsula un programa formado por un vertex shader y un fragment shader.
public: // Expone las operaciones que necesita el resto de la aplicación.
    GLuint ID = 0; // Guarda el identificador que OpenGL asigna al programa enlazado.

    Shader(const std::string& vertexPath, const std::string& fragmentPath); // Lee, compila y enlaza los dos archivos GLSL.
    ~Shader(); // Libera el programa de OpenGL cuando el objeto deja de existir.
    Shader(const Shader&) = delete; // Impide copiar un identificador de programa OpenGL.
    Shader& operator=(const Shader&) = delete; // Impide asignar copias del mismo programa.

    void use() const; // Activa el programa para los siguientes dibujados.
    void setBool(const std::string& name, bool value) const; // Envía un valor booleano a un uniforme.
    void setInt(const std::string& name, int value) const; // Envía un entero a un uniforme.
    void setFloat(const std::string& name, float value) const; // Envía un flotante a un uniforme.
    void setVec3(const std::string& name, const glm::vec3& value) const; // Envía un vector de tres componentes.
    void setMat3(const std::string& name, const glm::mat3& value) const; // Envía una matriz 3x3.
    void setMat4(const std::string& name, const glm::mat4& value) const; // Envía una matriz 4x4.

private: // Oculta las funciones auxiliares de compilación.
    static std::string readFile(const std::string& path); // Lee el contenido de un archivo de texto.
    static GLuint compileStage(GLenum type, const std::string& source, const std::string& path); // Compila una etapa GLSL y comprueba errores.
}; // Termina la declaración de la clase Shader.
