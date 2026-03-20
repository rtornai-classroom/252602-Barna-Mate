// Konstansek, amiket a common.cpp elvár, mielőtt betöltjük
const int VAOCount = 2;
const int BOCount = 2;
const int ProgramCount = 1;
const int TextureCount = 1;



#include "common.cpp"

// Az ablak címe
GLchar windowTitle[] = "Szamitogepes Grafika Beadando";

// --- GLOBÁLIS VÁLTOZÓK A FELADATHOZ ---
const float radius = 50.0f;
float circleX = 300.0f; // windowWidth / 2.0f, középpont, 2a)
float circleY = 300.0f;
float circleVx = 5.0f; //3a)
float circleVy = 0.0f;

float lineWidth = 200.0f; // windowWidth / 3.0f 
float lineHeight = 3.0f;
float lineX = 300.0f;
float lineY = 300.0f;

bool isStarted = true;

// --- KÖTELEZŐ ESEMÉNYKEZELŐK (A common.cpp elvárja őket) ---
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

// Itt kezeljük a gombnyomásokat
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Kilépés ESC gombra
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Indítás S gombbal (25 fok, 10px sebesség) //7)
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {

        float vektor = 10.0f;

        float angleRad = 25.0f * (asin(1.0)*2.0f / 180.0f);
        circleVx = glm::cos(angleRad) * vektor;
        circleVy = glm::sin(angleRad) * vektor;
    }

    // Vízszintesre állítás V gombbal
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        circleVx = 5.0f;
        circleVy = 0.0f;
    }
}

// --- FŐPROGRAM ---
int main() {

    //  Tájékoztatás
    cout << "Billentyu gombok" << endl;
    cout << "[ESC]:\tkilepes" << endl;
    cout << "[fel nyil]:\tszakasz felfele mozgatasa" << endl;
    cout << "[le nyil]:\tszakasz lefele mozgatasa" << endl;
    cout << "[S]:\telinditja 25 fokos szogben a kort" << endl;
    cout << "[V]:\tvizszintesen indul el a kor" << endl;


    //  Inicializálás a common.cpp beépített függvényével (OpenGL 3.3 Core Profile)
    init(3, 3, GLFW_OPENGL_CORE_PROFILE);

    //  Shaderek betöltése a sablon LoadShaders függvényével
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "circlevert.glsl", 0 },
        { GL_FRAGMENT_SHADER, "circlefrag.glsl", 0 },
        { GL_NONE, NULL, 0 }
    };
    program[0] = LoadShaders(shaders);

    //  Geometria definiálása
    float quadVertices[] = {
        -radius,  radius,  -radius, -radius,   radius, -radius,
        -radius,  radius,   radius, -radius,   radius,  radius
    };
    float lineVertices[] = {
        -100.0f, 0.0f,    100.0f, 0.0f
    };

    // Kör (Quad) VAO/VBO beállítása
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Vonal VAO/VBO beállítása
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //  Projekciós mátrix beállítása (A sablon windowWidth/windowHeight változóival)
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
    glUseProgram(program[0]);
    glUniformMatrix4fv(glGetUniformLocation(program[0], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //  Render ciklus
    while (!glfwWindowShouldClose(window)) {

        // --- INPUT: Szakasz folyamatos mozgatása ---
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) { //5)
            lineY += 5.0f;
            if (lineY > windowHeight) lineY = windowHeight;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            lineY -= 5.0f;
            if (lineY < 0.0f) lineY = 0.0f;
        }

        // --- FIZIKA ÉS ÜTKÖZÉS ---
        if (isStarted) {
            circleX += circleVx;
            circleY += circleVy;

            // Arányos visszapattanás
            if (circleX + radius >= windowWidth) {
                circleX = windowWidth - radius - ((circleX + radius) - windowWidth); //3b)
                circleVx = -circleVx;
            }
            else if (circleX - radius <= 0.0f) {
                circleX = radius + (0.0f - (circleX - radius));
                circleVx = -circleVx;
            }

            if (circleY + radius >= windowHeight) {
                circleY = windowHeight - radius - ((circleY + radius) - windowHeight);
                circleVy = -circleVy;
            }
            else if (circleY - radius <= 0.0f) {
                circleY = radius + (0.0f - (circleY - radius));
                circleVy = -circleVy;
            }
        }

        // Ütközésvizsgálat a vonal és a kör között //6)
        float closestX = glm::clamp(circleX, lineX - lineWidth / 2.0f, lineX + lineWidth / 2.0f);
        float closestY = glm::clamp(circleY, lineY - lineHeight / 2.0f, lineY + lineHeight / 2.0f);
        bool isIntersecting = ((circleX - closestX) * (circleX - closestX) + (circleY - closestY) * (circleY - closestY)) < (radius * radius);

        // --- KIRAJZOLÁS ---
        glClearColor(1.0f, 1.0f, 0.0f, 1.0f); // Sárga háttér /1) mértet a common.cpp-ben
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program[0]);
        glUniform2f(glGetUniformLocation(program[0], "circleCenter"), circleX, circleY);

        //  Kör rajzolása /2b)
        if (!isIntersecting) {
            glUniform3f(glGetUniformLocation(program[0], "colorCenter"), 0.0f, 1.0f, 0.0f); // Zöld
            glUniform3f(glGetUniformLocation(program[0], "colorEdge"), 1.0f, 0.0f, 0.0f);   // Piros
        }
        else {
            glUniform3f(glGetUniformLocation(program[0], "colorCenter"), 1.0f, 0.0f, 0.0f); // Piros
            glUniform3f(glGetUniformLocation(program[0], "colorEdge"), 0.0f, 1.0f, 0.0f);   // Zöld
        }
        glUniform1f(glGetUniformLocation(program[0], "radius"), radius);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(circleX, circleY, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(program[0], "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //  Vonal rajzolása // 4)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(lineX, lineY, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(program[0], "model"), 1, GL_FALSE, glm::value_ptr(model));

        glUniform3f(glGetUniformLocation(program[0], "colorCenter"), 0.0f, 0.0f, 1.0f); // Kék
        glUniform3f(glGetUniformLocation(program[0], "colorEdge"), 0.0f, 0.0f, 1.0f);
        glUniform1f(glGetUniformLocation(program[0], "radius"), 9999.0f); // Kikapcsoljuk a shader vágását

        glLineWidth(lineHeight);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_LINES, 0, 2);

        // Képfrissítés
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Takarítás a keretrendszer függvényével
    cleanUpScene(0);


    return 0;
}
