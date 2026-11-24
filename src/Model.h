#pragma once // Evita incluir este encabezado más de una vez.

#include "Shader.h" // Permite recibir el programa GLSL usado para dibujar cada malla.
#include <glm/glm.hpp> // Proporciona vec2 y vec3 para los vértices y colores.
#include <glad/glad.h> // Proporciona los identificadores de buffers, texturas y funciones OpenGL.
#include <assimp/scene.h> // Proporciona los tipos de escena y textura de Assimp.
#include <filesystem> // Permite construir rutas de archivos portables.
#include <string> // Permite recibir la ruta del modelo.
#include <unordered_map> // Permite reutilizar texturas ya cargadas.
#include <vector> // Permite almacenar mallas y recursos de textura.

struct Vertex { // Define el formato de vértice que recibirá el vertex shader.
    glm::vec3 Position; // Guarda la posición del vértice en el espacio del modelo.
    glm::vec3 Normal; // Guarda la normal usada por la iluminación Phong.
    glm::vec2 TexCoords; // Guarda las coordenadas UV de la textura.
}; // Termina la estructura Vertex.

class Mesh { // Representa una malla con sus buffers y su material básico.
public: // Expone la construcción y el dibujado de la malla.
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, GLuint textureId, const glm::vec3& color); // Crea los buffers de una malla.
    ~Mesh(); // Libera los buffers de OpenGL.
    Mesh(const Mesh&) = delete; // Evita duplicar identificadores de buffers por copia.
    Mesh& operator=(const Mesh&) = delete; // Evita asignar copias de identificadores OpenGL.
    Mesh(Mesh&& other) noexcept; // Permite mover los recursos al vector de mallas.
    Mesh& operator=(Mesh&& other) noexcept; // Permite transferir los recursos entre mallas.
    void Draw(const Shader& shader) const; // Dibuja la malla y configura sus datos de material.

private: // Mantiene los recursos gráficos encapsulados.
    GLuint VAO = 0; // Identifica el Vertex Array Object de la malla.
    GLuint VBO = 0; // Identifica el Vertex Buffer Object de los vértices.
    GLuint EBO = 0; // Identifica el Element Buffer Object de los índices.
    GLsizei indexCount = 0; // Guarda cuántos índices deben dibujarse.
    GLuint diffuseTexture = 0; // Guarda la textura de color base o cero si no existe.
    glm::vec3 diffuseColor = glm::vec3(0.8f); // Guarda un color alternativo cuando no hay textura.
    void release(); // Libera los buffers y restablece sus identificadores.
}; // Termina la declaración de Mesh.

class Model { // Importa una escena de Assimp y agrupa todas sus mallas.
public: // Expone la carga y el dibujado del modelo completo.
    bool Load(const std::filesystem::path& path); // Importa un archivo .gltf o .glb y construye las mallas.
    void Draw(const Shader& shader) const; // Dibuja todas las mallas importadas.
    std::size_t GetMeshCount() const; // Devuelve la cantidad de mallas cargadas.
    ~Model(); // Libera las texturas creadas por el modelo.
    Model() = default; // Inicializa un modelo vacío.
    Model(const Model&) = delete; // Evita copiar la propiedad de buffers y texturas OpenGL.
    Model& operator=(const Model&) = delete; // Evita asignar copias de recursos OpenGL.

private: // Encapsula las etapas internas de importación.
    std::vector<Mesh> meshes; // Almacena las mallas que componen la escena.
    std::vector<GLuint> ownedTextures; // Almacena las texturas que esta instancia debe liberar.
    std::unordered_map<std::string, GLuint> textureCache; // Evita cargar dos veces la misma imagen.
    std::filesystem::path modelDirectory; // Guarda la carpeta base para resolver texturas externas.
    const aiScene* currentScene = nullptr; // Referencia temporal a la escena mientras se procesa el importador.
    void ProcessNode(const aiNode* node, const glm::mat4& parentTransform); // Recorre la jerarquía de nodos de Assimp.
    void ProcessMesh(const aiMesh* mesh, const glm::mat4& nodeTransform); // Convierte una malla de Assimp al formato propio.
    GLuint LoadMaterialTexture(const aiMaterial* material); // Obtiene o carga la textura de color del material.
    GLuint LoadExternalTexture(const std::filesystem::path& path); // Carga una textura ubicada fuera del modelo.
    GLuint LoadEmbeddedTexture(const aiTexture* texture); // Carga una imagen almacenada dentro de un archivo GLB.
    GLuint CreateTexture(); // Crea un objeto de textura y configura sus parámetros.
}; // Termina la declaración de Model.
