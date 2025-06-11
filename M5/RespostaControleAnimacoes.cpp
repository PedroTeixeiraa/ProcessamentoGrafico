#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

const GLuint WIDTH = 800, HEIGHT = 800;

// Vertex Shader
const char* vertexShaderSource = R"(
#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 model;
uniform mat4 projection;
uniform vec2 uvOffset;
uniform vec2 uvScale;

out vec2 TexCoords;

void main()
{
    TexCoords = texCoords * uvScale + uvOffset;
    gl_Position = projection * model * vec4(position, 1.0);
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 400 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;

void main()
{
    color = texture(image, TexCoords);
}
)";

float playerX = 0.0f, playerY = 0.0f;
const float moveSpeed = 0.01f;

// Utilitário: Compilar shaders e linkar programa
GLuint compileShaderProgram(const char* vSrc, const char* fSrc)
{
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vSrc, nullptr);
    glCompileShader(vertex);

    GLint success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "Erro no Vertex Shader:\n" << infoLog << "\n";
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fSrc, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "Erro no Fragment Shader:\n" << infoLog << "\n";
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

// Carregamento de textura com STB
GLuint loadTexture(const std::string& path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (data) {
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Wrapping & Filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        std::cerr << "Falha ao carregar textura: " << path << "\n";
    }

    stbi_image_free(data);
    return textureID;
}

struct Layer {
    GLuint texture;
    float parallaxFactor;
    glm::vec2 basePosition;
};

// Classe para desenhar sprites
class SpriteRenderer {
public:
    SpriteRenderer(GLuint shader) : shader(shader)
    {
        initRenderData();
    }

    ~SpriteRenderer() {
        glDeleteVertexArrays(1, &quadVAO);
    }

    void DrawSprite(GLuint texture, glm::vec2 position, glm::vec2 size, float rotate = 0.0f)
    {
        glUseProgram(shader);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
        model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));

        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader, "image"), 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

private:
    GLuint shader, quadVAO;

    void initRenderData()
    {
        float vertices[] = {
            // pos        // tex
            0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
            0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
            1.0f, 0.0f, 0.0f,  1.0f, 0.0f
        };

        GLuint VBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

class CharacterController {
public:
    CharacterController(GLuint texture, GLuint shader, int nAnimations, int nFrames)
        : texture(texture), shader(shader), nAnimations(nAnimations), nFrames(nFrames),
          iAnimation(0), iFrame(0), frameTimer(0.0f), frameDuration(1.0f / 12.0f)
    {
        ds = 1.0f / nFrames;
		dt = 1.0f / nAnimations;
    }

    void Update(float deltaTime, GLFWwindow* window) {
		frameTimer += deltaTime;
		float actualSpeed = speed * deltaTime;

		bool moved = false;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			position.y += actualSpeed;
			playerY += actualSpeed; // Atualiza parallax
			iAnimation = 1;
			moved = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			position.y -= actualSpeed;
			playerY -= actualSpeed;
			iAnimation = 0;
			moved = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			position.x += actualSpeed;
			playerX += actualSpeed;
			iAnimation = 2;
			moved = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			position.x -= actualSpeed;
			playerX -= actualSpeed;
			iAnimation = 3;
			moved = true;
		}

		if (moved && frameTimer >= frameDuration) {
			iFrame = (iFrame + 1) % nFrames;
			frameTimer = 0.0f;
		}
	}

    void Draw(SpriteRenderer& renderer) {
        float offsetS = iFrame * ds;
		float offsetT = iAnimation * dt;

        glUseProgram(shader);
		glUniform2f(glGetUniformLocation(shader, "uvOffset"), offsetS, offsetT);
		glUniform2f(glGetUniformLocation(shader, "uvScale"), ds, dt);
		renderer.DrawSprite(texture, position, size);
    }

    glm::vec2 position = glm::vec2(WIDTH / 2 - 200, HEIGHT / 2);
    glm::vec2 size = glm::vec2(100.0f);
    float speed = 2.0f;

private:
    GLuint texture, shader;
    int nAnimations, nFrames;
    int iAnimation, iFrame;
    float frameTimer, frameDuration;
    float ds, dt;
};

const float minY = 0.0f;
const float maxY = HEIGHT - 100.0f;

void key_callback(GLFWwindow* window, int key, int, int action, int)
{
    if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Parallax Scene", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint shader = compileShaderProgram(vertexShaderSource, fragmentShaderSource);
    SpriteRenderer renderer(shader);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WIDTH),
                                      static_cast<float>(HEIGHT), 0.0f, -1.0f, 1.0f);
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &projection[0][0]);

    // Personagem
    GLuint playerTexture = loadTexture("../assets/sprites/Vampires1_Walk_full.png");

	CharacterController player(playerTexture, shader, 4, 6);

    std::vector<Layer> layers = {
		{ loadTexture("../assets/backgrounds/layers/1.png"), 0.1f, glm::vec2(0, 0) },
		{ loadTexture("../assets/backgrounds/layers/2.png"), 0.2f, glm::vec2(0, 0) },
		{ loadTexture("../assets/backgrounds/layers/3.png"), 0.4f, glm::vec2(0, 0) },
		{ loadTexture("../assets/backgrounds/layers/4.png"), 0.6f, glm::vec2(0, 0) },
		{ loadTexture("../assets/backgrounds/layers/5.png"), 0.8f, glm::vec2(0, 0) },
		{ loadTexture("../assets/backgrounds/layers/6.png"), 1.0f, glm::vec2(0, 0) }
	};

	const float worldMoveSpeed = 0.3f;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader);
		
		float backgroundMoveSpeed = 500.0f;

		for (const Layer& layer : layers) {
			float offsetX = fmod(-playerX * backgroundMoveSpeed * layer.parallaxFactor, WIDTH);
			if (offsetX < 0) offsetX += WIDTH;

			float offsetY = fmod(-playerY * backgroundMoveSpeed * layer.parallaxFactor, HEIGHT);
			if (offsetY < 0) offsetY += HEIGHT;

			glm::vec2 size = glm::vec2(WIDTH, HEIGHT);
			glm::vec2 uvOffset = glm::vec2(offsetX / WIDTH, offsetY / HEIGHT);

			glUniform2f(glGetUniformLocation(shader, "uvScale"), 1.0f, 1.0f);

			// Bloco 1 (original)
			glUniform2f(glGetUniformLocation(shader, "uvOffset"), uvOffset.x, uvOffset.y);
			renderer.DrawSprite(layer.texture, glm::vec2(-offsetX, -offsetY), size);

			// Bloco 2 (direita)
			glUniform2f(glGetUniformLocation(shader, "uvOffset"), uvOffset.x - 1.0f, uvOffset.y);
			renderer.DrawSprite(layer.texture, glm::vec2(-offsetX + WIDTH, -offsetY), size);

			// Bloco 3 (baixo)
			glUniform2f(glGetUniformLocation(shader, "uvOffset"), uvOffset.x, uvOffset.y - 1.0f);
			renderer.DrawSprite(layer.texture, glm::vec2(-offsetX, -offsetY + HEIGHT), size);

			// Bloco 4 (direita + baixo)
			glUniform2f(glGetUniformLocation(shader, "uvOffset"), uvOffset.x - 1.0f, uvOffset.y - 1.0f);
			renderer.DrawSprite(layer.texture, glm::vec2(-offsetX + WIDTH, -offsetY + HEIGHT), size);
		}


		float currentTime = glfwGetTime();
		static float lastTime = currentTime;
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		player.Update(deltaTime, window);
		player.Draw(renderer);

		glfwSwapBuffers(window);
	}

    glfwTerminate();
    return 0;
}