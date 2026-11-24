#include "Shader.h" // Incluye la declaración de la clase que implementa este archivo.
#include <glm/gtc/type_ptr.hpp> // Permite enviar los datos internos de matrices y vectores a OpenGL.
#include <fstream> // Permite abrir los archivos de shaders.
#include <iostream> // Permite mostrar errores de lectura, compilación y enlazado.
#include <sstream> // Permite acumular el contenido de los archivos en una cadena.
#include <stdexcept> // Permite detener la inicialización ante un shader inválido.

std::string Shader::readFile(const std::string& path) { // Define la función que carga el texto GLSL desde disco.
    std::ifstream file(path); // Abre el archivo indicado en modo lectura.
    if (!file.is_open()) { // Comprueba si el archivo pudo abrirse correctamente.
        throw std::runtime_error("No se pudo abrir el shader: " + path); // Informa de una ruta incorrecta.
    } // Cierra la comprobación del archivo.
    std::stringstream buffer; // Crea un flujo temporal para acumular el texto.
    buffer << file.rdbuf(); // Copia todo el contenido del archivo al flujo temporal.
    return buffer.str(); // Devuelve el texto completo del shader.
} // Termina la función readFile.

GLuint Shader::compileStage(GLenum type, const std::string& source, const std::string& path) { // Compila una etapa del pipeline gráfico.
    const GLuint shader = glCreateShader(type); // Solicita a OpenGL un objeto shader del tipo indicado.
    const char* sourcePointer = source.c_str(); // Obtiene un puntero al texto GLSL.
    glShaderSource(shader, 1, &sourcePointer, nullptr); // Entrega el código fuente al objeto shader.
    glCompileShader(shader); // Solicita al controlador gráfico compilar el código fuente.
    GLint success = GL_FALSE; // Inicializa el estado de compilación como fallido.
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // Consulta si la compilación terminó correctamente.
    if (success != GL_TRUE) { // Comprueba si el compilador encontró errores.
        GLint logLength = 0; // Reserva una variable para conocer el tamaño del mensaje de error.
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength); // Consulta el tamaño del registro de compilación.
        std::string log(static_cast<std::size_t>(logLength), '\0'); // Reserva memoria para el mensaje del controlador.
        glGetShaderInfoLog(shader, logLength, nullptr, log.data()); // Copia el mensaje de error al texto reservado.
        glDeleteShader(shader); // Libera el objeto shader que no pudo compilarse.
        throw std::runtime_error("Error compilando " + path + ":\n" + log); // Detiene la creación del programa e informa el error.
    } // Cierra la comprobación de compilación.
    return shader; // Devuelve el identificador de la etapa compilada.
} // Termina la función compileStage.

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) { // Construye un programa a partir de dos archivos GLSL.
    const std::string vertexSource = readFile(vertexPath); // Lee el código del vertex shader.
    const std::string fragmentSource = readFile(fragmentPath); // Lee el código del fragment shader.
    const GLuint vertex = compileStage(GL_VERTEX_SHADER, vertexSource, vertexPath); // Compila la etapa de vértices.
    GLuint fragment = 0; // Reserva el identificador de la etapa de fragmentos.
    try { // Protege el shader de vértices si falla la compilación de la otra etapa.
        fragment = compileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath); // Compila la etapa de fragmentos.
    } catch (...) { // Recibe cualquier error de compilación del fragment shader.
        glDeleteShader(vertex); // Libera el vertex shader que ya se había compilado.
        throw; // Propaga el error original para que lo gestione main.
    } // Termina la protección de la etapa de vértices.
    ID = glCreateProgram(); // Crea el programa OpenGL que reunirá ambas etapas.
    glAttachShader(ID, vertex); // Adjunta el vertex shader compilado al programa.
    glAttachShader(ID, fragment); // Adjunta el fragment shader compilado al programa.
    glLinkProgram(ID); // Enlaza las etapas para comprobar que sus interfaces coincidan.
    GLint success = GL_FALSE; // Inicializa el estado de enlazado como fallido.
    glGetProgramiv(ID, GL_LINK_STATUS, &success); // Consulta si el programa pudo enlazarse.
    glDeleteShader(vertex); // Libera el objeto vertex shader porque ya quedó enlazado.
    glDeleteShader(fragment); // Libera el objeto fragment shader porque ya quedó enlazado.
    if (success != GL_TRUE) { // Comprueba si el enlazador encontró incompatibilidades.
        GLint logLength = 0; // Reserva una variable para el tamaño del registro.
        glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &logLength); // Consulta el tamaño del mensaje de error.
        std::string log(static_cast<std::size_t>(logLength), '\0'); // Reserva espacio para recibir el registro.
        glGetProgramInfoLog(ID, logLength, nullptr, log.data()); // Copia el detalle del error de enlazado.
        glDeleteProgram(ID); // Libera el programa que no pudo enlazarse.
        ID = 0; // Restablece el identificador para indicar que no hay programa válido.
        throw std::runtime_error("Error enlazando shaders:\n" + log); // Informa del problema al iniciar la aplicación.
    } // Cierra la comprobación de enlazado.
} // Termina el constructor de Shader.

Shader::~Shader() { // Define la liberación automática del programa.
    if (ID != 0) { // Comprueba que exista un programa válido.
        glDeleteProgram(ID); // Libera el recurso administrado por OpenGL.
    } // Cierra la comprobación del identificador.
} // Termina el destructor de Shader.

void Shader::use() const { // Define cómo activar el programa actual.
    glUseProgram(ID); // Indica a OpenGL que use este programa en los siguientes dibujados.
} // Termina la función use.

void Shader::setBool(const std::string& name, bool value) const { // Define el envío de un uniforme booleano.
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value ? 1 : 0); // Convierte el booleano a entero, como espera GLSL.
} // Termina la función setBool.

void Shader::setInt(const std::string& name, int value) const { // Define el envío de un uniforme entero.
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); // Actualiza el entero asociado al nombre del uniforme.
} // Termina la función setInt.

void Shader::setFloat(const std::string& name, float value) const { // Define el envío de un uniforme flotante.
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); // Actualiza el valor flotante del uniforme.
} // Termina la función setFloat.

void Shader::setVec3(const std::string& name, const glm::vec3& value) const { // Define el envío de un vector de tres componentes.
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); // Envía las componentes x, y, z a GLSL.
} // Termina la función setVec3.

void Shader::setMat3(const std::string& name, const glm::mat3& value) const { // Define el envío de una matriz 3x3.
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); // Envía la matriz sin transponerla en OpenGL.
} // Termina la función setMat3.

void Shader::setMat4(const std::string& name, const glm::mat4& value) const { // Define el envío de una matriz 4x4.
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); // Envía la matriz de transformación al shader.
} // Termina la función setMat4.
