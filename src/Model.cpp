#define STB_IMAGE_IMPLEMENTATION // Genera la implementación de stb_image en este único archivo de traducción.
#include <stb_image.h> // Proporciona la carga de imágenes PNG, JPEG y otros formatos comunes.
#include "Model.h" // Incluye las clases Model, Mesh y Vertex.
#include <assimp/Importer.hpp> // Proporciona el importador principal de Assimp.
#include <assimp/postprocess.h> // Proporciona los pasos de triangulación y generación de normales.
#include <assimp/material.h> // Proporciona las propiedades de los materiales importados.
#include <glm/gtc/matrix_inverse.hpp> // Proporciona la inversa-transpuesta para transformar normales.
#include <glm/gtc/type_ptr.hpp> // Permite pasar matrices y vectores a OpenGL.
#include <cstddef> // Proporciona offsetof para configurar atributos de vértice.
#include <iostream> // Permite informar el progreso y los errores de importación.
#include <utility> // Proporciona exchange para transferir identificadores durante movimientos.

namespace { // Agrupa funciones auxiliares privadas de este archivo.
glm::mat4 AssimpToGlm(const aiMatrix4x4& matrix) { // Convierte una matriz de Assimp al formato de GLM.
    return glm::mat4( // Construye una matriz GLM indicando sus cuatro columnas.
        matrix.a1, matrix.b1, matrix.c1, matrix.d1, // Copia la primera columna de la matriz de Assimp.
        matrix.a2, matrix.b2, matrix.c2, matrix.d2, // Copia la segunda columna de la matriz de Assimp.
        matrix.a3, matrix.b3, matrix.c3, matrix.d3, // Copia la tercera columna de la matriz de Assimp.
        matrix.a4, matrix.b4, matrix.c4, matrix.d4  // Copia la cuarta columna de la matriz de Assimp.
    ); // Devuelve la matriz de transformación convertida.
} // Termina AssimpToGlm.
} // Termina el espacio de nombres auxiliar.

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, GLuint textureId, const glm::vec3& color) // Construye una malla a partir de sus datos de CPU.
    : indexCount(static_cast<GLsizei>(indices.size())), diffuseTexture(textureId), diffuseColor(color) { // Copia la cantidad de índices y los datos de material.
    glGenVertexArrays(1, &VAO); // Crea el objeto que almacena la configuración de atributos.
    glGenBuffers(1, &VBO); // Crea el buffer que almacenará los vértices.
    glGenBuffers(1, &EBO); // Crea el buffer que almacenará los índices.
    glBindVertexArray(VAO); // Activa el VAO para registrar su configuración.
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Activa el VBO como buffer de datos de vértices.
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STATIC_DRAW); // Copia los vértices a la memoria de la GPU.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Asocia el EBO al VAO activo.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW); // Copia los índices a la GPU.
    glEnableVertexAttribArray(0); // Habilita el atributo cero para la posición.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Position))); // Describe cómo leer la posición dentro de cada vértice.
    glEnableVertexAttribArray(1); // Habilita el atributo uno para las normales.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal))); // Describe cómo leer la normal dentro de cada vértice.
    glEnableVertexAttribArray(2); // Habilita el atributo dos para las coordenadas UV.
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, TexCoords))); // Describe cómo leer las coordenadas UV.
    glBindVertexArray(0); // Desactiva el VAO para evitar modificarlo por accidente.
} // Termina el constructor de Mesh.

Mesh::~Mesh() { // Libera los objetos gráficos propiedad de esta malla.
    release(); // Elimina los buffers y reinicia sus identificadores.
} // Termina el destructor de Mesh.

Mesh::Mesh(Mesh&& other) noexcept // Define el constructor de movimiento.
    : VAO(std::exchange(other.VAO, 0)), VBO(std::exchange(other.VBO, 0)), EBO(std::exchange(other.EBO, 0)), indexCount(std::exchange(other.indexCount, 0)), diffuseTexture(std::exchange(other.diffuseTexture, 0)), diffuseColor(other.diffuseColor) { // Transfiere los recursos y vacía la instancia anterior.
} // Termina el constructor de movimiento.

Mesh& Mesh::operator=(Mesh&& other) noexcept { // Define la asignación por movimiento.
    if (this != &other) { // Evita asignar el objeto consigo mismo.
        release(); // Libera primero los recursos que pertenecían a esta malla.
        VAO = std::exchange(other.VAO, 0); // Transfiere el VAO y lo elimina de la instancia origen.
        VBO = std::exchange(other.VBO, 0); // Transfiere el VBO y lo elimina de la instancia origen.
        EBO = std::exchange(other.EBO, 0); // Transfiere el EBO y lo elimina de la instancia origen.
        indexCount = std::exchange(other.indexCount, 0); // Transfiere el número de índices.
        diffuseTexture = std::exchange(other.diffuseTexture, 0); // Transfiere el identificador de textura sin duplicar su propiedad.
        diffuseColor = other.diffuseColor; // Copia el color alternativo del material.
    } // Cierra la comprobación de autoasignación.
    return *this; // Devuelve la malla ya actualizada.
} // Termina operator=.

void Mesh::release() { // Implementa la liberación segura de recursos.
    if (VAO != 0) { glDeleteVertexArrays(1, &VAO); VAO = 0; } // Elimina el VAO si existe y limpia su identificador.
    if (VBO != 0) { glDeleteBuffers(1, &VBO); VBO = 0; } // Elimina el VBO si existe y limpia su identificador.
    if (EBO != 0) { glDeleteBuffers(1, &EBO); EBO = 0; } // Elimina el EBO si existe y limpia su identificador.
} // Termina release.

void Mesh::Draw(const Shader& shader) const { // Dibuja la malla con su textura o color alternativo.
    shader.setBool("uHasTexture", diffuseTexture != 0); // Informa al fragment shader si existe una textura difusa.
    shader.setVec3("uObjectColor", diffuseColor); // Envía el color alternativo del material.
    if (diffuseTexture != 0) { // Comprueba si esta malla tiene una textura cargada.
        glActiveTexture(GL_TEXTURE0); // Selecciona la unidad de textura cero.
        glBindTexture(GL_TEXTURE_2D, diffuseTexture); // Asocia la imagen a la unidad seleccionada.
        shader.setInt("uDiffuseMap", 0); // Indica al shader que lea la textura desde la unidad cero.
    } // Cierra la configuración de textura.
    glBindVertexArray(VAO); // Activa el VAO con los atributos y buffers de esta malla.
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr); // Dibuja los triángulos usando el buffer de índices.
    glBindVertexArray(0); // Desactiva el VAO después del dibujado.
    glBindTexture(GL_TEXTURE_2D, 0); // Desasocia la textura para dejar limpio el estado.
} // Termina Draw de Mesh.

Model::~Model() { // Libera las texturas creadas por esta instancia del modelo.
    if (!ownedTextures.empty()) { // Comprueba si el modelo posee texturas.
        glDeleteTextures(static_cast<GLsizei>(ownedTextures.size()), ownedTextures.data()); // Elimina todas las texturas en una sola llamada.
    } // Cierra la comprobación de texturas.
} // Termina el destructor de Model.

bool Model::Load(const std::filesystem::path& path) { // Importa el archivo 3D indicado por el usuario.
    meshes.clear(); // Elimina cualquier malla que hubiera cargado anteriormente.
    if (!ownedTextures.empty()) { glDeleteTextures(static_cast<GLsizei>(ownedTextures.size()), ownedTextures.data()); } // Libera texturas de una carga anterior.
    ownedTextures.clear(); // Reinicia el listado de texturas propias.
    textureCache.clear(); // Reinicia la caché de imágenes.
    modelDirectory = path.parent_path(); // Guarda la carpeta para resolver rutas de texturas externas.
    Assimp::Importer importer; // Crea el objeto responsable de leer el archivo 3D.
    const unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_ValidateDataStructure; // Solicita triángulos, normales, vértices compartidos y validación.
    currentScene = importer.ReadFile(path.string(), flags); // Lee el archivo y ejecuta los pasos de procesamiento indicados.
    if (currentScene == nullptr || currentScene->mRootNode == nullptr || (currentScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0) { // Comprueba si Assimp pudo importar una escena completa.
        std::cerr << "Assimp no pudo cargar el modelo: " << importer.GetErrorString() << '\n'; // Muestra el error detallado de Assimp.
        currentScene = nullptr; // Borra la referencia temporal para evitar usar una escena inválida.
        return false; // Indica que la carga falló.
    } // Cierra la comprobación de importación.
    ProcessNode(currentScene->mRootNode, glm::mat4(1.0f)); // Recorre desde el nodo raíz aplicando sus transformaciones.
    currentScene = nullptr; // El importador y su escena dejan de utilizarse al salir de esta función.
    std::cout << "Modelo cargado: " << path.string() << " | Mallas: " << meshes.size() << '\n'; // Informa el resultado de la importación.
    return !meshes.empty(); // Informa éxito únicamente si se creó al menos una malla.
} // Termina Load.

void Model::ProcessNode(const aiNode* node, const glm::mat4& parentTransform) { // Recorre recursivamente la jerarquía de nodos del modelo.
    const glm::mat4 nodeTransform = parentTransform * AssimpToGlm(node->mTransformation); // Combina la transformación del padre con la transformación local.
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) { // Recorre las mallas asociadas al nodo actual.
        const unsigned int meshIndex = node->mMeshes[i]; // Obtiene el índice de la malla dentro de la escena.
        ProcessMesh(currentScene->mMeshes[meshIndex], nodeTransform); // Convierte la malla aplicando la transformación acumulada.
    } // Termina el recorrido de mallas del nodo.
    for (unsigned int i = 0; i < node->mNumChildren; ++i) { // Recorre todos los nodos hijos.
        ProcessNode(node->mChildren[i], nodeTransform); // Continúa el recorrido conservando la transformación del padre.
    } // Termina el recorrido de nodos hijos.
} // Termina ProcessNode.

void Model::ProcessMesh(const aiMesh* mesh, const glm::mat4& nodeTransform) { // Convierte los vértices e índices de una malla de Assimp.
    std::vector<Vertex> vertices; // Reserva el contenedor de vértices que se enviará a la GPU.
    std::vector<unsigned int> indices; // Reserva el contenedor de índices triangulados.
    vertices.reserve(mesh->mNumVertices); // Reserva memoria para evitar realocaciones mientras se importan vértices.
    const glm::mat3 normalTransform = glm::mat3(glm::transpose(glm::inverse(nodeTransform))); // Calcula la matriz correcta para transformar normales.
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) { // Recorre cada vértice importado.
        Vertex vertex{}; // Crea e inicializa los atributos del vértice actual.
        const aiVector3D& position = mesh->mVertices[i]; // Obtiene la posición original desde Assimp.
        vertex.Position = glm::vec3(nodeTransform * glm::vec4(position.x, position.y, position.z, 1.0f)); // Aplica la transformación jerárquica a la posición.
        if (mesh->HasNormals()) { // Comprueba si Assimp proporcionó normales.
            const aiVector3D& normal = mesh->mNormals[i]; // Obtiene la normal original.
            vertex.Normal = glm::normalize(normalTransform * glm::vec3(normal.x, normal.y, normal.z)); // Transforma y normaliza la normal.
        } else { // Define un comportamiento de reserva si no existen normales.
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f); // Asigna una normal vertical básica.
        } // Cierra la comprobación de normales.
        if (mesh->mTextureCoords[0] != nullptr) { // Comprueba si existe un canal de coordenadas UV.
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y); // Copia las coordenadas UV del primer canal.
        } else { // Define valores si el modelo no contiene coordenadas UV.
            vertex.TexCoords = glm::vec2(0.0f); // Asigna coordenadas cero para evitar leer memoria no inicializada.
        } // Cierra la comprobación de coordenadas.
        vertices.push_back(vertex); // Añade el vértice convertido a la lista de CPU.
    } // Termina el recorrido de vértices.
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) { // Recorre las caras producidas por la triangulación.
        const aiFace& face = mesh->mFaces[i]; // Obtiene la cara actual.
        for (unsigned int j = 0; j < face.mNumIndices; ++j) { // Recorre los índices que forman la cara.
            indices.push_back(face.mIndices[j]); // Añade cada índice a la lista que usará OpenGL.
        } // Termina el recorrido de índices de la cara.
    } // Termina el recorrido de caras.
    glm::vec3 materialColor(0.8f, 0.8f, 0.8f); // Define un color neutro de reserva para el material.
    GLuint textureId = 0; // Inicializa la textura como inexistente.
    if (mesh->mMaterialIndex < currentScene->mNumMaterials) { // Comprueba que el índice de material sea válido.
        const aiMaterial* material = currentScene->mMaterials[mesh->mMaterialIndex]; // Obtiene el material asociado a la malla.
        aiColor4D importedColor; // Reserva el color difuso que pueda contener el material.
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &importedColor) == AI_SUCCESS) { // Intenta leer el color difuso importado.
            materialColor = glm::vec3(importedColor.r, importedColor.g, importedColor.b); // Convierte el color de Assimp a vector GLM.
        } // Cierra la lectura del color.
        textureId = LoadMaterialTexture(material); // Carga o recupera la textura asociada al material.
    } // Cierra la lectura de material.
    meshes.emplace_back(std::move(vertices), std::move(indices), textureId, materialColor); // Crea una malla GPU con los datos procesados.
} // Termina ProcessMesh.

GLuint Model::CreateTexture() { // Crea una textura OpenGL y configura el muestreo.
    GLuint textureId = 0; // Inicializa el identificador de textura.
    glGenTextures(1, &textureId); // Solicita a OpenGL un nuevo objeto de textura.
    glBindTexture(GL_TEXTURE_2D, textureId); // Activa el objeto para configurar sus parámetros.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repite la imagen cuando la coordenada U supera el intervalo.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repite la imagen cuando la coordenada V supera el intervalo.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Usa mipmaps con filtrado lineal al reducir la textura.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Usa filtrado lineal al ampliar la textura.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Permite cargar filas de imagen cuyo tamaño no sea múltiplo de cuatro bytes.
    ownedTextures.push_back(textureId); // Registra el identificador para liberarlo al destruir el modelo.
    return textureId; // Devuelve la textura preparada para recibir píxeles.
} // Termina CreateTexture.

GLuint Model::LoadExternalTexture(const std::filesystem::path& path) { // Carga una imagen desde un archivo externo.
    const std::string cacheKey = path.lexically_normal().string(); // Normaliza la ruta para identificar la imagen en la caché.
    const auto cached = textureCache.find(cacheKey); // Busca si la imagen ya fue cargada.
    if (cached != textureCache.end()) { return cached->second; } // Reutiliza la textura existente cuando está en caché.
    int width = 0; // Recibirá el ancho de la imagen.
    int height = 0; // Recibirá la altura de la imagen.
    int channels = 0; // Recibirá el número original de canales de color.
    stbi_set_flip_vertically_on_load(1); // Invierte las filas para adaptar el origen de imagen al uso habitual de OpenGL.
    unsigned char* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 4); // Lee la imagen y fuerza cuatro canales RGBA.
    if (pixels == nullptr) { // Comprueba si la imagen pudo leerse.
        std::cerr << "No se pudo cargar la textura: " << path.string() << " | " << stbi_failure_reason() << '\n'; // Muestra el error de stb_image.
        return 0; // Devuelve cero para que la malla use su color de reserva.
    } // Cierra la comprobación de lectura.
    const GLuint textureId = CreateTexture(); // Crea y registra el objeto de textura.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels); // Copia los píxeles RGBA a la memoria de la GPU.
    glGenerateMipmap(GL_TEXTURE_2D); // Genera niveles de resolución inferiores para mejorar el muestreo.
    stbi_image_free(pixels); // Libera la memoria temporal que stb_image reservó.
    textureCache[cacheKey] = textureId; // Guarda la relación entre ruta e identificador en la caché.
    return textureId; // Devuelve el identificador listo para usar.
} // Termina LoadExternalTexture.

GLuint Model::LoadEmbeddedTexture(const aiTexture* texture) { // Carga una imagen almacenada dentro del propio archivo 3D.
    int width = 0; // Recibirá el ancho de una imagen comprimida.
    int height = 0; // Recibirá la altura de una imagen comprimida.
    int channels = 0; // Recibirá los canales originales de una imagen comprimida.
    const GLuint textureId = CreateTexture(); // Crea un objeto OpenGL para la imagen embebida.
    if (texture->mHeight == 0) { // En Assimp, altura cero indica que mWidth representa el tamaño de datos comprimidos.
        unsigned char* pixels = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(texture->pcData), static_cast<int>(texture->mWidth), &width, &height, &channels, 4); // Descomprime la imagen directamente desde memoria.
        if (pixels == nullptr) { // Comprueba si stb_image pudo descomprimir los bytes.
            std::cerr << "No se pudo descomprimir una textura embebida: " << stbi_failure_reason() << '\n'; // Muestra el motivo del fallo.
            return 0; // Indica que no hay una textura utilizable.
        } // Cierra la comprobación de descompresión.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels); // Copia la imagen RGBA descomprimida a la GPU.
        stbi_image_free(pixels); // Libera los píxeles temporales.
    } else { // La textura ya contiene texeles sin comprimir.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(texture->mWidth), static_cast<GLsizei>(texture->mHeight), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture->pcData); // Copia los texeles BGRA de Assimp a una textura RGBA.
    } // Cierra la selección según el tipo de almacenamiento.
    glGenerateMipmap(GL_TEXTURE_2D); // Genera mipmaps para reducir el aliasing al ver la textura a distancia.
    return textureId; // Devuelve el identificador de la textura embebida.
} // Termina LoadEmbeddedTexture.

GLuint Model::LoadMaterialTexture(const aiMaterial* material) { // Localiza la textura principal del material.
    aiTextureType textureType = aiTextureType_DIFFUSE; // Usa el tipo difuso como opción compatible de reserva.
    if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0) { textureType = aiTextureType_BASE_COLOR; } // Prefiere la textura base color cuando el importador la proporciona.
    if (material->GetTextureCount(textureType) == 0) { return 0; } // Devuelve cero si el material no tiene textura de color.
    aiString texturePath; // Reserva la ruta o identificador interno de la imagen.
    if (material->GetTexture(textureType, 0, &texturePath) != AI_SUCCESS) { return 0; } // Lee la primera textura del tipo seleccionado.
    const std::string textureKey = texturePath.C_Str(); // Convierte el identificador de Assimp a una cadena estándar.
    const auto cached = textureCache.find(textureKey); // Busca una textura idéntica ya cargada.
    if (cached != textureCache.end()) { return cached->second; } // Reutiliza la textura si ya fue procesada.
    const aiTexture* embedded = currentScene->GetEmbeddedTexture(texturePath.C_Str()); // Comprueba si la referencia pertenece a una imagen embebida.
    GLuint textureId = 0; // Inicializa el identificador que devolverá la función.
    if (embedded != nullptr) { // Comprueba si Assimp encontró la imagen dentro del modelo.
        textureId = LoadEmbeddedTexture(embedded); // Descomprime y sube la imagen embebida a OpenGL.
    } else { // La referencia corresponde a un archivo externo.
        const std::filesystem::path externalPath = modelDirectory / std::filesystem::path(texturePath.C_Str()); // Resuelve la ruta desde la carpeta del modelo.
        textureId = LoadExternalTexture(externalPath); // Lee la imagen externa y crea su textura OpenGL.
    } // Cierra la selección entre imagen interna y externa.
    if (textureId != 0) { textureCache[textureKey] = textureId; } // Guarda la textura cargada para reutilizarla.
    return textureId; // Devuelve la textura o cero cuando no fue posible cargarla.
} // Termina LoadMaterialTexture.

void Model::Draw(const Shader& shader) const { // Dibuja todas las mallas que componen el modelo.
    for (const Mesh& mesh : meshes) { // Recorre cada malla importada.
        mesh.Draw(shader); // Dibuja la malla con su material.
    } // Termina el recorrido de mallas.
} // Termina Draw de Model.

std::size_t Model::GetMeshCount() const { // Expone la cantidad de mallas disponibles.
    return meshes.size(); // Devuelve el tamaño del vector de mallas.
} // Termina GetMeshCount.
