#include "BezierPath.h" // Incluye la curva que define el recorrido del avión.
#include "Camera.h" // Incluye la cámara que se desplaza por el escenario.
#include "Model.h" // Incluye el cargador y dibujador del modelo 3D.
#include "Shader.h" // Incluye la gestión de los programas GLSL.
#include <glad/glad.h> // Carga las funciones OpenGL usadas por la aplicación.
#include <GLFW/glfw3.h> // Proporciona la ventana, el contexto y las entradas de teclado.
#include <glm/gtc/matrix_transform.hpp> // Proporciona translate, scale y perspective.
#include <glm/gtc/quaternion.hpp> // Proporciona la conversión de cuaterniones a matrices.
#include <glm/gtc/constants.hpp> // Proporciona constantes matemáticas como pi.
#include <glm/geometric.hpp> // Proporciona productos escalares y vectoriales para construir la orientación.
#include <algorithm> // Proporciona clamp y min.
#include <array> // Permite declarar los cuatro puntos de control de la curva.
#include <cmath> // Proporciona fmod y operaciones matemáticas.
#include <filesystem> // Proporciona rutas de archivos independientes del sistema.
#include <iostream> // Muestra mensajes de estado y errores.
#include <stdexcept> // Permite capturar errores de inicialización.
#include <unordered_map> // Guarda el estado anterior de las teclas para detectar pulsaciones.
#include <vector> // Almacena los puntos que dibujan la curva.

namespace { // Agrupa constantes y funciones privadas del archivo principal.
constexpr int WINDOW_WIDTH = 1280; // Define el ancho inicial de la ventana en píxeles.
constexpr int WINDOW_HEIGHT = 720; // Define la altura inicial de la ventana en píxeles.
constexpr float LANDSCAPE_SCALE = 120.0f; // Amplía el paisaje importado hasta unos 19.3 metros virtuales de ancho.
const glm::vec3 LANDSCAPE_POSITION(0.345f, -9.2f, 1.888f); // Centra el paisaje y deja su punto más alto por debajo del avión.
constexpr float MODEL_SCALE = 1.0f; // Permite ajustar el tamaño del modelo importado sin modificar el archivo 3D.
// Según la prueba visual, el morro real del avión apunta hacia el eje local -X.

void FramebufferSizeCallback(GLFWwindow*, int width, int height) { // Recibe los cambios de tamaño del área de dibujo.
    glViewport(0, 0, width, height); // Actualiza el área de representación al tamaño real de la ventana.
} // Termina FramebufferSizeCallback.

glm::mat4 FlightRotation(const glm::vec3& direction) { // Construye una orientación estable para el modelo cuyo morro apunta a -X.
    const glm::vec3 x = -glm::normalize(direction); // Alinea el eje local negativo X con la dirección de vuelo.
    const glm::vec3 z = glm::normalize(glm::cross(x, glm::vec3(0.0f, 1.0f, 0.0f))); // Calcula el eje lateral perpendicular a la vertical global.
    const glm::vec3 y = glm::normalize(glm::cross(z, x)); // Recupera el eje superior manteniendo el avión derecho.
    return glm::mat4(glm::vec4(x, 0.0f), glm::vec4(y, 0.0f), glm::vec4(z, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Forma la matriz de rotación con los ejes como columnas.
} // Termina FlightRotation.

bool WasPressed(GLFWwindow* window, int key, std::unordered_map<int, bool>& previousState) { // Detecta el instante inicial de una pulsación.
    const bool isDown = glfwGetKey(window, key) == GLFW_PRESS; // Consulta si la tecla está presionada en este fotograma.
    const bool justPressed = isDown && !previousState[key]; // Detecta una transición de liberada a presionada.
    previousState[key] = isDown; // Guarda el estado actual para compararlo en el fotograma siguiente.
    return justPressed; // Devuelve verdadero solo una vez por pulsación.
} // Termina WasPressed.

void ProcessInput(GLFWwindow* window, Camera& camera, float deltaTime, bool& paused, float& flightSpeed, float& travelledDistance, std::unordered_map<int, bool>& previousState) { // Actualiza cámara y controles del vuelo.
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, true); } // Cierra la ventana cuando se pulsa Escape.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Forward, deltaTime); } // Mueve la cámara hacia delante con W.
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Backward, deltaTime); } // Mueve la cámara hacia atrás con S.
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Left, deltaTime); } // Mueve la cámara hacia la izquierda con A.
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Right, deltaTime); } // Mueve la cámara hacia la derecha con D.
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Down, deltaTime); } // Mueve la cámara hacia abajo con Q.
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Up, deltaTime); } // Mueve la cámara hacia arriba con E.
    if (WasPressed(window, GLFW_KEY_P, previousState)) { paused = !paused; } // Pausa o reanuda la animación cuando se pulsa P.
    if (WasPressed(window, GLFW_KEY_R, previousState)) { camera.Reset(); } // Restablece la posición de la cámara cuando se pulsa R.
    if (WasPressed(window, GLFW_KEY_T, previousState)) { travelledDistance = 0.0f; paused = false; } // Reinicia el recorrido del avión desde P0 cuando se pulsa T.
    if (WasPressed(window, GLFW_KEY_EQUAL, previousState)) { flightSpeed = std::min(flightSpeed + 0.5f, 15.0f); } // Aumenta la velocidad con la tecla + o =.
    if (WasPressed(window, GLFW_KEY_MINUS, previousState)) { flightSpeed = std::max(flightSpeed - 0.5f, 0.5f); } // Reduce la velocidad con la tecla - sin detenerla.
} // Termina ProcessInput.

void CreatePathBuffer(const std::vector<BezierPath>& paths, GLuint& vao, GLuint& vbo, GLsizei& vertexCount) { // Convierte la curva en una línea dibujable.
    constexpr int LINE_SAMPLES = 200; // Define la cantidad de segmentos de la representación visual.
    std::vector<glm::vec3> points; // Reserva una lista de posiciones que formarán la línea.
    points.reserve(paths.size() * (LINE_SAMPLES + 1)); // Reserva capacidad para todas las muestras.
    for (const BezierPath& path : paths) { // Recorre cada tramo del circuito cerrado.
    for (int i = 0; i <= LINE_SAMPLES; ++i) { // Recorre el parámetro desde cero hasta uno.
        const float t = static_cast<float>(i) / static_cast<float>(LINE_SAMPLES); // Calcula el parámetro actual de la curva.
        points.push_back(path.position(t)); // Añade la posición de la curva a la lista.
    } // Termina la generación de puntos de un tramo.
    } // Termina la generación del circuito completo.
    vertexCount = static_cast<GLsizei>(points.size()); // Guarda cuántos vértices tendrá la línea.
    glGenVertexArrays(1, &vao); // Crea el VAO de la trayectoria.
    glGenBuffers(1, &vbo); // Crea el VBO de la trayectoria.
    glBindVertexArray(vao); // Activa el VAO para registrar su atributo.
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // Activa el VBO que recibirá las posiciones.
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(points.size() * sizeof(glm::vec3)), points.data(), GL_STATIC_DRAW); // Copia la curva muestreada a la GPU.
    glEnableVertexAttribArray(0); // Habilita el atributo cero para la posición.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr); // Describe tres componentes flotantes por punto.
    glBindVertexArray(0); // Desactiva el VAO después de configurarlo.
} // Termina CreatePathBuffer.

void DrawPath(const Shader& shader, GLuint vao, GLsizei vertexCount, const glm::mat4& view, const glm::mat4& projection) { // Dibuja la curva como una línea continua.
    shader.use(); // Activa el shader sencillo de color.
    shader.setMat4("uView", view); // Envía la matriz de vista.
    shader.setMat4("uProjection", projection); // Envía la matriz de proyección.
    shader.setVec3("uLineColor", glm::vec3(0.15f, 0.75f, 1.0f)); // Define un color azul claro para distinguir la trayectoria.
    glBindVertexArray(vao); // Activa los datos de vértices de la curva.
    glLineWidth(2.0f); // Solicita un grosor de línea visible.
    glDrawArrays(GL_LINE_STRIP, 0, vertexCount); // Dibuja los puntos como una secuencia conectada.
    glBindVertexArray(0); // Desactiva el VAO de la curva.
} // Termina DrawPath.
} // Termina el espacio de nombres auxiliar.

int main(int argc, char* argv[]) { // Define el punto de entrada del programa y recibe argumentos opcionales.
    try { // Permite capturar errores de shaders y de inicialización.
        if (glfwInit() != GLFW_TRUE) { // Inicializa GLFW y comprueba su resultado.
            throw std::runtime_error("No fue posible inicializar GLFW."); // Informa que no se pudo crear el sistema de ventanas.
        } // Cierra la comprobación de GLFW.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Solicita la versión principal 3 de OpenGL.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Solicita la versión secundaria 3 de OpenGL.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Solicita el perfil moderno Core.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Permite cambiar el tamaño de la ventana.
        GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vuelo 3D - Curva de Bezier", nullptr, nullptr); // Crea la ventana y su contexto OpenGL.
        if (window == nullptr) { // Comprueba si se pudo crear la ventana.
            glfwTerminate(); // Libera los recursos inicializados por GLFW.
            throw std::runtime_error("No fue posible crear la ventana OpenGL."); // Informa del fallo de creación.
        } // Cierra la comprobación de ventana.
        glfwMakeContextCurrent(window); // Activa el contexto OpenGL en el hilo actual.
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback); // Registra la función para reaccionar al redimensionamiento.
        glfwSwapInterval(1); // Activa la sincronización vertical para evitar exceso de fotogramas.
        if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // Carga las funciones de OpenGL usando GLFW.
            glfwDestroyWindow(window); // Destruye la ventana si no se pudieron cargar las funciones.
            glfwTerminate(); // Cierra GLFW después del fallo de GLAD.
            throw std::runtime_error("No fue posible inicializar GLAD."); // Informa del fallo al cargar OpenGL.
        } // Cierra la comprobación de GLAD.
        glEnable(GL_DEPTH_TEST); // Activa la prueba de profundidad para ocultar superficies detrás de otras.
        glClearColor(0.55f, 0.73f, 0.88f, 1.0f); // Define un fondo azul claro que acompaña el paisaje.
        std::cout << "OpenGL: " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << '\n'; // Imprime la versión de OpenGL del controlador.
        const std::filesystem::path projectRoot(APP_ROOT_DIR); // Recupera la carpeta del proyecto definida por CMake.
        const std::filesystem::path suppliedModelPath = argc > 1 ? std::filesystem::path(argv[1]) : std::filesystem::path("assets/models/airplane.glb"); // Obtiene la ruta recibida o la ruta predeterminada del avión.
        const std::filesystem::path modelPath = suppliedModelPath.is_absolute() ? suppliedModelPath : projectRoot / suppliedModelPath; // Resuelve rutas relativas desde la carpeta raíz del proyecto.
        { // Delimita el tiempo de vida de los objetos OpenGL para destruirlos antes de cerrar el contexto.
            Shader phongShader((projectRoot / "shaders" / "phong.vert").string(), (projectRoot / "shaders" / "phong.frag").string()); // Carga y enlaza los shaders Phong.
            Shader lineShader((projectRoot / "shaders" / "line.vert").string(), (projectRoot / "shaders" / "line.frag").string()); // Carga y enlaza los shaders de la línea de trayectoria.
            Model airplane; // Crea el contenedor que recibirá las mallas del avión.
            if (!airplane.Load(modelPath)) { std::cerr << "No se encontró un modelo válido. Copia tu avión a assets/models/airplane.glb o pasa su ruta como argumento.\n"; } // Continúa mostrando la escena aunque todavía no exista el modelo.
            Model landscape; // Crea un segundo modelo independiente del avión.
            const bool landscapeLoaded = landscape.Load(projectRoot / "assets/models/landscape.glb"); // Importa una sola vez el paisaje estático con sus texturas incrustadas.
            if (!landscapeLoaded) { std::cerr << "No se pudo cargar assets/models/landscape.glb; el vuelo continuara sin paisaje.\n"; } // Informa el fallo sin impedir la animación del avión.
            const glm::mat4 landscapeTranslation = glm::translate(glm::mat4(1.0f), LANDSCAPE_POSITION); // Construye la traslación fija del paisaje.
            const glm::mat4 landscapeScale = glm::scale(glm::mat4(1.0f), glm::vec3(LANDSCAPE_SCALE)); // Construye su escala uniforme.
            const glm::mat4 landscapeMatrix = landscapeTranslation * landscapeScale; // Combina las transformaciones estáticas fuera del bucle de renderizado.
            const float k = 0.55228475f; // Factor de control de Bézier para aproximar un cuarto de elipse.
            const float rx = 10.0f, rz = 6.0f, h = 2.0f; // Define los radios horizontal y longitudinal y la altura del circuito.
            using V = glm::vec3; // Abrevia el tipo de vector tridimensional de los puntos de control.
            using Controls = std::array<V, 4>; // Define un conjunto de cuatro puntos para cada Bézier cúbica.
            const std::vector<BezierPath> flightPaths = { // Construye cuatro curvas con extremos y tangentes coincidentes.
                BezierPath(Controls{V(-rx,h,0), V(-rx,h,k*rz), V(-k*rx,h,rz), V(0,h,rz)}), // Primer cuarto del óvalo.
                BezierPath(Controls{V(0,h,rz), V(k*rx,h,rz), V(rx,h,k*rz), V(rx,h,0)}), // Segundo cuarto del óvalo.
                BezierPath(Controls{V(rx,h,0), V(rx,h,-k*rz), V(k*rx,h,-rz), V(0,h,-rz)}), // Tercer cuarto del óvalo.
                BezierPath(Controls{V(0,h,-rz), V(-k*rx,h,-rz), V(-rx,h,-k*rz), V(-rx,h,0)}) // Cuarto tramo que regresa al inicio sin saltos.
            }; // Termina la definición del recorrido cerrado.
            float totalLength = 0.0f; // Inicializa la longitud de todo el circuito.
            for (const auto& path : flightPaths) { totalLength += path.getLength(); } // Suma las longitudes de los cuatro tramos.
            GLuint pathVAO = 0; // Reserva el identificador del VAO de la línea.
            GLuint pathVBO = 0; // Reserva el identificador del VBO de la línea.
            GLsizei pathVertexCount = 0; // Guarda la cantidad de vértices de la curva.
            CreatePathBuffer(flightPaths, pathVAO, pathVBO, pathVertexCount); // Sube la representación de la curva a la GPU.
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Captura el cursor para permitir rotaciones ilimitadas.
            double lastMouseX = 0.0, lastMouseY = 0.0; // Guarda la última posición virtual del cursor.
            bool firstMouse = true; // Evita saltos de orientación al iniciar o recuperar el foco.
            Camera camera; // Inicializa la cámara que permitirá observar el recorrido.
            bool paused = false; // Guarda si la animación del avión está pausada.
            float flightSpeed = 4.0f; // Define la velocidad inicial del avión en unidades de mundo por segundo.
            float travelledDistance = 0.0f; // Guarda la distancia acumulada sobre la curva.
            float lastFrameTime = static_cast<float>(glfwGetTime()); // Guarda el instante del fotograma anterior.
            std::unordered_map<int, bool> previousKeyState; // Conserva el estado de teclas que se detectan por pulsación.
            std::cout << "Controles: Mouse mirar | WASD mover camara | Q/E bajar/subir | P pausar | +/- velocidad | T reiniciar vuelo | R reiniciar camara | ESC salir\n"; // Muestra las instrucciones al iniciar.
            while (glfwWindowShouldClose(window) == GLFW_FALSE) { // Ejecuta el bucle mientras la ventana permanezca abierta.
                const float currentFrameTime = static_cast<float>(glfwGetTime()); // Lee el reloj de GLFW.
                const float deltaTime = std::min(currentFrameTime - lastFrameTime, 0.05f); // Calcula el tiempo del fotograma y limita saltos grandes.
                lastFrameTime = currentFrameTime; // Actualiza el instante de referencia.
                glfwPollEvents(); // Procesa eventos de ventana y teclado.
                double mouseX = 0.0, mouseY = 0.0; // Reserva las coordenadas actuales del cursor.
                glfwGetCursorPos(window, &mouseX, &mouseY); // Consulta la posición virtual del ratón.
                if (glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_FALSE) { firstMouse = true; } // Descarta desplazamientos mientras la ventana no tiene foco.
                else { // Procesa el ratón únicamente cuando la ventana está activa.
                    if (!firstMouse) { camera.ProcessMouseMovement(static_cast<float>(mouseX - lastMouseX), static_cast<float>(lastMouseY - mouseY)); } // Aplica diferencias de cursor e invierte el eje vertical de pantalla.
                    firstMouse = false; // Habilita la lectura de diferencias para el siguiente fotograma.
                } // Termina el procesamiento del ratón.
                lastMouseX = mouseX; lastMouseY = mouseY; // Conserva las coordenadas para el siguiente fotograma.
                ProcessInput(window, camera, deltaTime, paused, flightSpeed, travelledDistance, previousKeyState); // Aplica las entradas del usuario.
                if (!paused) { travelledDistance = std::fmod(travelledDistance + flightSpeed * deltaTime, totalLength); } // Reinicia la distancia al completar una vuelta sin saltos espaciales.
                float localDistance = travelledDistance; // Copia la distancia global para localizar el tramo activo.
                std::size_t segment = 0; // Comienza la búsqueda en el primer tramo.
                while (segment + 1 < flightPaths.size() && localDistance >= flightPaths[segment].getLength()) { // Avanza mientras la distancia exceda el tramo actual.
                    localDistance -= flightPaths[segment].getLength(); // Resta la longitud del tramo ya recorrido.
                    ++segment; // Selecciona el tramo siguiente.
                } // Termina la búsqueda del tramo activo.
                const glm::vec3 planePosition = flightPaths[segment].positionAtDistance(localDistance); // Evalúa la posición local sobre el tramo activo.
                const glm::vec3 flightDirection = flightPaths[segment].tangentAtDistance(localDistance); // Evalúa la dirección de avance sobre ese tramo.
                const glm::mat4 translation = glm::translate(glm::mat4(1.0f), planePosition); // Construye la matriz de traslación del avión.
                const glm::mat4 rotation = FlightRotation(flightDirection); // Orienta el morro hacia la tangente sin invertir el avión.
                const glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(MODEL_SCALE)); // Construye la matriz de escala uniforme del modelo.
                const glm::mat4 modelMatrix = translation * rotation * scale; // Combina escala, rotación y traslación en el orden correcto.
                const glm::mat4 view = camera.GetViewMatrix(); // Calcula la matriz de vista a partir de la cámara actual.
                int framebufferWidth = 0; // Reserva el ancho real del framebuffer.
                int framebufferHeight = 0; // Reserva el alto real del framebuffer.
                glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight); // Consulta el tamaño actual del área de dibujo.
                const float aspectRatio = framebufferHeight > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight) : 1.0f; // Calcula la relación de aspecto evitando una división por cero.
                const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 200.0f); // Construye la proyección en perspectiva.
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpia el color y la profundidad del fotograma anterior.
                DrawPath(lineShader, pathVAO, pathVertexCount, view, projection); // Dibuja primero la trayectoria de referencia.
                phongShader.use(); // Activa el programa que calcula la iluminación Phong.
                phongShader.setMat4("uView", view); // Envía la matriz de vista de la cámara.
                phongShader.setMat4("uProjection", projection); // Envía la matriz de proyección perspectiva.
                phongShader.setVec3("uLightPosition", glm::vec3(-4.0f, 10.0f, 6.0f)); // Sitúa la fuente de luz en el escenario.
                phongShader.setVec3("uLightColor", glm::vec3(1.0f, 0.96f, 0.88f)); // Define un color ligeramente cálido para la luz.
                phongShader.setVec3("uViewPosition", camera.Position); // Envía la posición de la cámara para calcular el reflejo especular.
                if (landscapeLoaded) { // Dibuja el paisaje únicamente si pudo importarse correctamente.
                    phongShader.setMat4("uModel", landscapeMatrix); // Envía la transformación fija del paisaje, sin animación.
                    phongShader.setFloat("uAmbientStrength", 0.40f); // Ilumina suavemente las zonas en sombra del terreno.
                    phongShader.setFloat("uSpecularStrength", 0.12f); // Reduce el brillo para dar un acabado menos metálico.
                    phongShader.setFloat("uShininess", 16.0f); // Define un reflejo especular más amplio en el paisaje.
                    landscape.Draw(phongShader); // Renderiza las mallas y texturas del escenario estático con el mismo shader Phong.
                } // Termina el dibujado del paisaje.
                phongShader.setMat4("uModel", modelMatrix); // Restablece la matriz de modelo animada del avión.
                phongShader.setFloat("uAmbientStrength", 0.22f); // Define la intensidad del término ambiental.
                phongShader.setFloat("uSpecularStrength", 0.55f); // Define la intensidad del reflejo especular.
                phongShader.setFloat("uShininess", 32.0f); // Define la concentración del brillo especular.
                airplane.Draw(phongShader); // Dibuja las mallas del avión con su material y la iluminación.
                glfwSwapBuffers(window); // Presenta el fotograma terminado en la ventana.
            } // Termina el bucle de renderizado.
            glDeleteVertexArrays(1, &pathVAO); // Libera el VAO de la trayectoria mientras el contexto sigue activo.
            glDeleteBuffers(1, &pathVBO); // Libera el VBO de la trayectoria mientras el contexto sigue activo.
        } // Destruye los shaders y el modelo antes de destruir el contexto OpenGL.
        glfwDestroyWindow(window); // Libera la ventana y su contexto.
        glfwTerminate(); // Cierra GLFW y libera sus recursos globales.
    } catch (const std::exception& error) { // Recibe errores de inicialización, archivos o shaders.
        std::cerr << "Error: " << error.what() << '\n'; // Imprime el detalle del error para facilitar su diagnóstico.
        return 1; // Devuelve un código distinto de cero para indicar que el programa falló.
    } // Termina el bloque de captura de errores.
    return 0; // Informa al sistema operativo que la aplicación terminó correctamente.
} // Termina main.
