#include <vector>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// A common.cpp elvárja ezeket a konstansokat a beemelés előtt!
const int VAOCount = 2; // 0: Kontrollpontok/Poligon, 1: Bézier-görbe
const int BOCount = 2;
const int ProgramCount = 1;
const int TextureCount = 1;

#include <common.cpp> // vagy "common.cpp", ahogy nálad az előzőekben működött

GLchar windowTitle[] = "N-ed foku Bezier (Beadando)";
int draggedIndex = -1; // Épp vonszolt pont indexe

// Kezdő 4 kontrollpont (A feladat kéri)
std::vector<glm::vec3> controlPoints = {
    glm::vec3(-0.6f, -0.4f, 0.0f),
    glm::vec3(-0.2f,  0.6f, 0.0f),
    glm::vec3(0.2f,  0.6f, 0.0f),
    glm::vec3(0.6f, -0.4f, 0.0f)
};
// Ide számoljuk ki a görbét alkotó sűrű vonalakat
std::vector<glm::vec3> curvePoints;

// --- MATEMATIKA: Binomiális együttható ---
long long nCr(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n / 2) k = n - k;
    long long res = 1;
    for (int i = 1; i <= k; ++i) res = res * (n - i + 1) / i;
    return res;
}

// --- GÖRBE SZÁMÍTÁSA ---
void calculateBezier() {
    curvePoints.clear();
    int n = controlPoints.size() - 1; // Fokszám
    if (n < 1) return; // Ha nincs elég pont, nem rajzolunk görbét

    int segments = 200; // Részletesség
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / segments;
        glm::vec3 p(0.0f);
        for (int j = 0; j <= n; ++j) {
            float b = (float)nCr(n, j) * pow(t, j) * pow(1.0f - t, n - j);
            p += controlPoints[j] * b;
        }
        curvePoints.push_back(p);
    }
}

// --- GPU MEMÓRIA FRISSÍTÉSE ---
void updateGPU() {
    calculateBezier();

    // 1. Kontrollpontok (VAO 0)
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[0]);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec3), controlPoints.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // 2. Görbe (VAO 1)
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[1]);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}

// --- ESEMÉNYKEZELŐK ---
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width; windowHeight = height;
    glViewport(0, 0, width, height);
    // Beállítjuk a common.cpp mátrixait ortografikus 2D vetítésre
    float aspectRatio = (float)width / (float)height;
    if (width < height) matProjection = glm::ortho(-1.0f, 1.0f, -1.0f / aspectRatio, 1.0f / aspectRatio, -1.0f, 1.0f);
    else matProjection = glm::ortho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    matModelView = glm::mat4(1.0f);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
}

// Kurzorkoordináták konvertálása OpenGL világba (-1-től 1-ig)
glm::vec3 getMousePos(double xpos, double ypos) {
    float x = (float)(2.0 * xpos) / windowWidth - 1.0f;
    float y = 1.0f - (float)(2.0 * ypos) / windowHeight;

    glm::mat4 inverseVP = glm::inverse(matProjection * matModelView);
    glm::vec4 worldPos = inverseVP * glm::vec4(x, y, 0.0f, 1.0f);
    return glm::vec3(worldPos.x, worldPos.y, 0.0f);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    glm::vec3 m = getMousePos(x, y);
    float clickRadius = 0.05f; // Érzékenység

    if (action == GLFW_PRESS) {
        // Megkeressük, melyik pontra kattintottunk
        int clickedIndex = -1;
        for (int i = 0; i < controlPoints.size(); ++i) {
            if (glm::distance(m, controlPoints[i]) < clickRadius) {
                clickedIndex = i;
                break;
            }
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (clickedIndex != -1) {
                draggedIndex = clickedIndex; // Vonszolás kezdése
            }
            else {
                controlPoints.push_back(m); // Új pont létrehozása
                updateGPU();
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (clickedIndex != -1 && controlPoints.size() > 2) {
                controlPoints.erase(controlPoints.begin() + clickedIndex); // Pont törlése
                updateGPU();
            }
        }
    }
    else if (action == GLFW_RELEASE) {
        draggedIndex = -1; // Vonszolás vége
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (draggedIndex != -1) {
        controlPoints[draggedIndex] = getMousePos(xpos, ypos);
        updateGPU();
    }
}

// --- KIRAJZOLÁS ---
void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program[0]);

    glUniformMatrix4fv(glGetUniformLocation(program[0], "matProjection"), 1, GL_FALSE, glm::value_ptr(matProjection));
    glUniformMatrix4fv(glGetUniformLocation(program[0], "matModelView"), 1, GL_FALSE, glm::value_ptr(matModelView));

    GLuint colorLoc = glGetUniformLocation(program[0], "objectColor");
    GLuint isPointLoc = glGetUniformLocation(program[0], "isPoint");

    // 1. Kontrollpoligon (Sárga)
    glUniform1i(isPointLoc, 0); // Vonal (ne vágja körré)
    glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_LINE_STRIP, 0, controlPoints.size());

    // 2. Bézier-görbe (Kék)
    glLineWidth(3.0f); // Vastagabb vonal
    glUniform3f(colorLoc, 0.0f, 0.0f, 1.0f);
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());
    glLineWidth(1.0f); // Visszaállítás

    // 3. Kontrollpontok (Piros, kerek)
    glPointSize(8.0f); // 3 és 9 pixel közötti érték a feladat alapján
    glUniform1i(isPointLoc, 1); // PONT VÁGÁSA BEKAPCSOLVA!
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_POINTS, 0, controlPoints.size());
}

int main() {

    cout << "Bezier Curve Editor" << endl;
    cout << "  Left click  : add control point" << endl;
    cout << "  Left drag   : move control point" << endl;
    cout << "  Right click : remove control point" << endl;
    cout << "  ESC         : exit" << endl;
    
    init(3, 3, GLFW_OPENGL_CORE_PROFILE);

    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "vertexShader.glsl", 0 },
        { GL_FRAGMENT_SHADER, "fragmentShader.glsl", 0 },
        { GL_NONE, nullptr, 0 }
    };
    program[0] = LoadShaders(shaders);

    updateGPU();
    framebufferSizeCallback(window, windowWidth, windowHeight); // Mátrixok inicializálása

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cleanUpScene(0);
    return 0;
}
