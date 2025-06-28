#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//GLM
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct Sprite {
	GLuint VAO;
	GLuint texID;
	vec3 position;
	vec3 dimensions;
	float ds, dt;
	int iAnimation, iFrame;
	int nAnimations, nFrames;
};
	
struct Tile {
	GLuint VAO;
	GLuint texID;
	int iTile;
	vec3 position;
	vec3 dimensions;
	float ds, dt;
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

int setupShader();
int setupSprite(int nAnimations, int nFrames, float &ds, float &dt);
int setupTile(int nTiles, float &ds, float &dt);
int loadTexture(string filePath, int &width, int &height);
void desenharMapa(GLuint shaderID);

const GLuint WIDTH = 800, HEIGHT = 600;

const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 uniform mat4 model;
 uniform mat4 projection;
 void main()
 {
	tex_coord = vec2(texc.s, 1.0 - texc.t);
	gl_Position = projection * model * vec4(position, 1.0);
 }
 )";

const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 uniform vec2 offsetTex;

 void main()
 {
	 color = texture(tex_buff,tex_coord + offsetTex);
 }
 )";

string tilesetFile;
int nTiles;
int tileWidth, tileHeight;
int mapWidth, mapHeight;
vector<vector<int>> mapData;

int playerX;
int playerY;

vector <Tile> tileset;

Sprite vampirao;

struct TileProperties {
    bool isChangeTile;
    bool isHazard;
    bool isCollectible;
};

vector<TileProperties> tileProperties;

int totalMoedas = 0;
int moedasColetadas = 0;

void loadMapConfig(const string& filename) {
    ifstream file(filename);

    if (!file) {
        cerr << "Erro ao abrir arquivo de configuração: " << filename << endl;
        exit(1);
    }

    string line;
    
    getline(file, line);
    stringstream ss1(line);
    ss1 >> tilesetFile >> nTiles >> tileWidth >> tileHeight;

    getline(file, line);
    stringstream ss2(line);
    ss2 >> mapWidth >> mapHeight;

	playerX = 0;
    playerY = 0;

    mapData.resize(mapHeight, vector<int>(mapWidth));

    for (int i = 0; i < mapHeight; i++) {
        getline(file, line);
        stringstream ss(line);
        for (int j = 0; j < mapWidth; j++) {
            ss >> mapData[i][j];
        }
    }

	while (getline(file, line)) {
		if (line == "--") break;
	}

	getline(file, line);
	if (line != "TileProperties") {
		cerr << "Erro: seção TileProperties não encontrada" << endl;
		exit(1);
	}

	tileProperties.clear();

	while (getline(file, line)) {
		if (line.empty()) continue;

		TileProperties props;
		int changeTile, hazard, collectible;
		stringstream ss(line);
		ss >> changeTile >> hazard >> collectible;

		props.isChangeTile = (changeTile != 0);
		props.isHazard = (hazard != 0);
		props.isCollectible = (collectible != 0);

		tileProperties.push_back(props);
	}

    file.close();

	for (int i = 0; i < mapHeight; i++) {
		for (int j = 0; j < mapWidth; j++) {
			if (tileProperties[mapData[i][j]].isCollectible) {
				totalMoedas++;
			}
		}
	}
	cout << "Total de moedas no mapa: " << totalMoedas << endl;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 8);
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Triangulo! -- Rossana", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();

	int imgWidth, imgHeight;

	loadMapConfig("../map.txt");
	GLuint texID = loadTexture("../assets/tilesets/" + tilesetFile, imgWidth, imgHeight);

	vampirao.nAnimations = 4;
	vampirao.nFrames = 6;
	vampirao.VAO = setupSprite(vampirao.nAnimations,vampirao.nFrames,vampirao.ds,vampirao.dt);
	vampirao.position = vec3(0.0, 0.0, 0.0);
	vampirao.dimensions = vec3(tileWidth, tileHeight, 1.0);
	vampirao.texID = loadTexture("../assets/sprites/Vampires1_Walk_full.png", imgWidth, imgHeight);
	vampirao.iAnimation = 0;
	vampirao.iFrame = 0;

	for (int i = 0; i < nTiles; i++){
		Tile tile;
		tile.dimensions = vec3(tileHeight, tileWidth,1.0);
		tile.iTile = i;
		tile.texID = texID;
		tile.VAO = setupTile(nTiles, tile.ds,tile.dt);
		tileset.push_back(tile);
	}

	glUseProgram(shaderID);

	double prev_s = glfwGetTime();
	double title_countdown_s = 0.1;

	float colorValue = 0.0;

	glActiveTexture(GL_TEXTURE0);

	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double lastTime = 0.0;
	double deltaT = 0.0;
	double currTime = glfwGetTime();
	double FPS = 12.0;

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Este trecho de código é totalmente opcional: calcula e mostra a contagem do FPS na barra de título
		{
			double curr_s = glfwGetTime();		// Obtém o tempo atual.
			double elapsed_s = curr_s - prev_s; // Calcula o tempo decorrido desde o último frame.
			prev_s = curr_s;					// Atualiza o "tempo anterior" para o próximo frame.

			// Exibe o FPS, mas não a cada frame, para evitar oscilações excessivas.
			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
			{
				double fps = 1.0 / elapsed_s; // Calcula o FPS com base no tempo decorrido.

				// Cria uma string e define o FPS como título da janela.
				char tmp[256];
				sprintf(tmp, "Ola Triangulo! -- Rossana\tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);

				title_countdown_s = 0.1; // Reinicia o temporizador para atualizar o título periodicamente.
			}
		}

		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);
		
		currTime = glfwGetTime();
		deltaT = currTime - lastTime;

		if (deltaT >= 1.0 / FPS)
		{
			vampirao.iFrame = (vampirao.iFrame + 1) % vampirao.nFrames;
			lastTime = currTime;
		}

		desenharMapa(shaderID);
		glfwSwapBuffers(window);
	}
		
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_PRESS)
    {
        int targetX = playerX;
        int targetY = playerY;

        if (key == GLFW_KEY_W) {
			targetX--;
			vampirao.iAnimation = 2;
		};
        if (key == GLFW_KEY_S) {
			targetX++;
			vampirao.iAnimation = 1;
		}
        if (key == GLFW_KEY_A) {
			targetY--;
			vampirao.iAnimation = 3;
		};
        if (key == GLFW_KEY_D) {
			targetY++;
			vampirao.iAnimation = 0;
		}

        // Diagonais
        if (key == GLFW_KEY_Q) { 
			targetX--; 
			targetY--;
			vampirao.iAnimation = 2; 
		}
        if (key == GLFW_KEY_E) { 
			targetX--; 
			targetY++;
			vampirao.iAnimation = 2; 
		}
        if (key == GLFW_KEY_Z) { 
			targetX++; 
			targetY--;
			vampirao.iAnimation = 1; 
		}
        if (key == GLFW_KEY_C) { 
			targetX++; 
			targetY++;
			vampirao.iAnimation = 1; 
		}

        if (targetX >= 0 && targetX < mapWidth && targetY >= 0 && targetY < mapHeight)
        {
			playerX = targetX;
			playerY = targetY;

            int tileID = mapData[targetX][targetY];			
			
			// Se for hazard
			if (tileProperties[tileID].isHazard) {
				cout << "Você morreu ao pisar na tile " << tileID << "!" << endl;
				glfwSetWindowShouldClose(window, GL_TRUE);
			}

			// Se for item coletável
			if (tileProperties[tileID].isCollectible) {
				cout << "Você coletou uma moeda na posição [" << playerY << "," << playerX << "]!" << endl;
				mapData[playerX][playerY] = 0;

				moedasColetadas++;

				if (moedasColetadas == totalMoedas) {
					cout << "Parabéns! Você coletou todas as moedas e venceu o jogo!" << endl;
					glfwSetWindowShouldClose(window, GL_TRUE);
				}
			}

			// se for para mudar de tile
			if (tileProperties[tileID].isChangeTile) {
				mapData[playerX][playerY] = 1;
			}

        }
        else
        {
            cout << "Tentativa fora do mapa: [" << targetY << ", " << targetX << "]" << endl;
        }
    }
}

// Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupSprite(int nAnimations, int nFrames, float &ds, float &dt)
{

	ds = 1.0 / (float) nFrames;
	dt = 1.0 / (float) nAnimations;
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {
		// x   y    z    s     t
		-0.5,  0.5, 0.0, 0.0, dt, //V0
		-0.5, -0.5, 0.0, 0.0, 0.0, //V1
		 0.5,  0.5, 0.0, ds, dt, //V2
		 0.5, -0.5, 0.0, ds, 0.0  //V3
		};

	GLuint VBO, VAO;
	// Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos
	glBindVertexArray(VAO);
	// Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
	//  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	//  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
	//  Tipo do dado
	//  Se está normalizado (entre zero e um)
	//  Tamanho em bytes
	//  Deslocamento a partir do byte zero

	// Ponteiro pro atributo 0 - Posição - coordenadas x, y, z
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Ponteiro pro atributo 1 - Coordenada de textura s, t
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

int setupTile(int nTiles, float &ds, float &dt)
{
    
	ds = 1.0 / (float) nTiles;
	dt = 1.0;
	
	// Como eu prefiro escalar depois, th e tw serão 1.0
	float th = 1.0, tw = 1.0;

	GLfloat vertices[] = {
		// x   y    z    s     t
		0.0,  th/2.0f,   0.0, 0.0,    dt/2.0f, //A
		tw/2.0f, th,     0.0, ds/2.0f, dt,     //B
		tw/2.0f, 0.0,    0.0, ds/2.0f, 0.0,    //D
		tw,     th/2.0f, 0.0, ds,     dt/2.0f  //C
		};

	GLuint VBO, VAO;
	// Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos
	glBindVertexArray(VAO);
	// Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
	//  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	//  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
	//  Tipo do dado
	//  Se está normalizado (entre zero e um)
	//  Tamanho em bytes
	//  Deslocamento a partir do byte zero

	// Ponteiro pro atributo 0 - Posição - coordenadas x, y, z
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Ponteiro pro atributo 1 - Coordenada de textura s, t
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

int loadTexture(string filePath, int &width, int &height)
{
	GLuint texID;

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

void desenharMapa(GLuint shaderID)
{
	Tile baseTile = tileset[0];
	float tileW = baseTile.dimensions.x;
	float tileH = baseTile.dimensions.y;

	float mapPixelWidth = (mapWidth + mapHeight) * tileW / 8.0f;
	float mapPixelHeight = (mapWidth + mapHeight) * tileH / 2.0f;

	float x0 = (WIDTH - mapPixelWidth) / 2.0f + tileW / 2.0f;
	float y0 = (HEIGHT - mapPixelHeight) / 2.0f + tileH / 4.0f;

	for (int i = 0; i < mapHeight; i++) {
		for (int j = 0; j < mapWidth; j++) {
			// Primeiro: Desenhar o tile de fundo normal
			Tile curr_tile = tileset[mapData[i][j]];

			float x = x0 + (j - i) * curr_tile.dimensions.x / 2.0f;
			float y = y0 + (j + i) * curr_tile.dimensions.y / 2.0f;

			mat4 model = mat4(1.0f);
			model = translate(model, vec3(x, y, 0.0f));
			model = scale(model, curr_tile.dimensions);
			glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

			vec2 offsetTex;
			offsetTex.s = curr_tile.iTile * curr_tile.ds;
			offsetTex.t = 0.0f;
			glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);

			glBindVertexArray(curr_tile.VAO);
			glBindTexture(GL_TEXTURE_2D, curr_tile.texID);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			// Segundo: Se for a posição do player, desenha o vampirão por cima
			if (i == playerX && j == playerY) {
				float tileOffsetX = baseTile.dimensions.x * 0.5f;
				float tileOffsetY = baseTile.dimensions.y * 0.25f;

				float vampX = x + tileOffsetX;
				float vampY = y + tileOffsetY;

				mat4 vampModel = mat4(1.0f);
				vampModel = translate(vampModel, vec3(vampX, vampY, 0.0f));
				vampModel = scale(vampModel, vec3(vampirao.dimensions.x, -vampirao.dimensions.y, 1.0f));
				glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(vampModel));

				float offsetS = vampirao.iFrame * vampirao.ds;
				float offsetT = vampirao.iAnimation * vampirao.dt;
				glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetS, offsetT);

				glBindVertexArray(vampirao.VAO);
				glBindTexture(GL_TEXTURE_2D, vampirao.texID);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
	}
}
