#include <iostream>
#include <vector>
#include <string>
#include <assert.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

const GLuint WIDTH = 800, HEIGHT = 800;

// Vertex Shader com model e projection
const GLchar* vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texc;

out vec3 vColor;
out vec2 tex_coord;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    vColor = color;
    tex_coord = texc;
    gl_Position = projection * model * vec4(position, 1.0);
}
)";

// Fragment Shader igual
const GLchar* fragmentShaderSource = R"(
#version 400
in vec3 vColor;
in vec2 tex_coord;
out vec4 color;
uniform sampler2D tex_buff;
void main()
{
    color = texture(tex_buff, tex_coord);
}
)";

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint setupShader();
GLuint setupGeometry();
GLuint loadTexture(string filePath);

class Sprite {
public:
    GLuint VAO;
    GLuint textureID;
    GLuint shaderID;

    glm::vec2 position;
    glm::vec2 scale;
    float rotation;

    Sprite(GLuint vao, GLuint tex, GLuint shader)
        : VAO(vao), textureID(tex), shaderID(shader),
          position(0.0f), scale(1.0f), rotation(0.0f) {}

    void Draw(const glm::mat4& projection) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0, 0.0, 1.0));
        model = glm::scale(model, glm::vec3(scale, 1.0f));

        glUseProgram(shaderID);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, &projection[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Múltiplos Sprites", nullptr, nullptr);
    if (!window)
    {
        cerr << "Falha ao criar a janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    GLuint shaderID = setupShader();
    GLuint VAO = setupGeometry();

    GLuint texID1 = loadTexture("../assets/Vampirinho.png");
    GLuint texID2 = loadTexture("../assets/Vampirinho.png");

    vector<Sprite> sprites;
    sprites.emplace_back(VAO, texID1, shaderID);
    sprites.emplace_back(VAO, texID2, shaderID);

    sprites[0].position = glm::vec2(400.0f, 400.0f);
    sprites[0].scale = glm::vec2(100.0f, 100.0f);

    sprites[1].position = glm::vec2(200.0f, 200.0f);
    sprites[1].scale = glm::vec2(150.0f, 150.0f);

    glm::mat4 projection = glm::ortho(0.0f, float(WIDTH), 0.0f, float(HEIGHT), -1.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenha todos os sprites
        for (Sprite& sprite : sprites)
        {
            sprite.Draw(projection);
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLuint setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint setupGeometry()
{
    // Retângulo 1x1 centrado na origem (coordenadas OpenGL de -0.5 a 0.5)
    GLfloat vertices[] = {
        // posição           // cor            // textura
        -0.5f, -0.5f, 0.0f,  1,1,1,           0.0f, 0.0f,  // canto inferior esquerdo
         0.5f, -0.5f, 0.0f,  1,1,1,           1.0f, 0.0f,  // canto inferior direito
         0.5f,  0.5f, 0.0f,  1,1,1,           1.0f, 1.0f,  // canto superior direito
        -0.5f, -0.5f, 0.0f,  1,1,1,           0.0f, 0.0f,  // canto inferior esquerdo
         0.5f,  0.5f, 0.0f,  1,1,1,           1.0f, 1.0f,  // canto superior direito
        -0.5f,  0.5f, 0.0f,  1,1,1,           0.0f, 1.0f   // canto superior esquerdo
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

GLuint loadTexture(string filePath)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
		stbi_set_flip_vertically_on_load(true);


    if (data)
    {
        if (nrChannels == 3) // jpg, bmp
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if (nrChannels == 4) // png com alpha
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else
            cout << "Formato de textura nao suportado (nrChannels=" << nrChannels << ")" << endl;

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Falha ao carregar textura: " << filePath << endl;
    }
    stbi_image_free(data);

    return texID;
}