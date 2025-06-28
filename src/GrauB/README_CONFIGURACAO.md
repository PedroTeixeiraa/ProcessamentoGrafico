# Configuração do Ambiente para Computação Gráfica com OpenGL

Este tutorial irá guiá-lo pela instalação e configuração do ambiente para rodar projetos OpenGL com **CMake** e **VS Code**, utilizando o **MinGW-UCRT64** como compilador.

## ⚠️ Importante: Diferenças entre Windows, Linux e macOS

Este tutorial foi desenvolvido para Windows utilizando MSYS2 UCRT64 como ambiente de compilação. Caso esteja utilizando Linux ou macOS, algumas configurações podem ser diferentes, especialmente na escolha do compilador C/C++ e na configuração do CMake.

Para configurar corretamente o compilador no VS Code no Linux ou no MacOS, siga os guias oficiais:

🔗 [Linux: Configuração do VS Code para C++ no Linux](https://code.visualstudio.com/docs/cpp/config-linux)  
🔗 [macOS: Configuração do VS Code para C++ no macOS](https://code.visualstudio.com/docs/cpp/config-clang-mac)  

Caso tenha dificuldades na configuração do CMake, consulte a documentação oficial:  
🔗 [CMake Documentation](https://cmake.org/documentation/)

---

## 📌 1. Instalando as Ferramentas Necessárias

Antes de começar, certifique-se de ter os seguintes programas instalados:

### 1️⃣ Instalar o CMake

Baixe e instale o **CMake** a partir do site oficial:
🔗 [CMake Download](https://cmake.org/download/)

Durante a instalação, **habilite a opção "Add CMake to system PATH"** para facilitar o uso no terminal.

---

### 2️⃣ Instalar o Compilador MinGW-UCRT64 através do MSYS2

Baixe o **MSYS2** através do link:
🔗 [MSYS2 Download](https://www.msys2.org/)

Provavelmente ao terminar de instalar, abrirá um terminal.

Execute o seguinte comando para instalar os pacotes necessários:

```sh
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
```

Caso queira ou necessite de mais suporte nesta etapa, consulte o manual oficial:

🔗 [Configuração do VS Code para C++ no Windows](https://code.visualstudio.com/docs/cpp/config-mingw)

### Configurando a variável de ambiente no Sistema Operacional (Windows)

Esse passo garante que o sistema operacional encontre o compilador automaticamente ao rodar comandos no terminal, sem precisar especificar o caminho completo. A forma mais simples de fazer isso (se você tiver permissão de administrador do sistema) é a seguinte:

 - Edite a variável de caminhos do sistema operacional (`PATH`), acrescentando o caminho onde ficaram os executáveis do compilador (provavelmente ficaram em `C:\msys64\ucrt64\bin` - ou onde você escolheu instalar)
 - Se estiver em um computador que não tenha acesso de adm, adicionar temporariamente ao path com este comando (via terminal CMD): 
```sh
   set PATH=%PATH%;C:\msys64\ucrt64\bin
```
---

### 3️⃣ Instalar o VS code

Baixe e instale o **VS Code** pelo link:
🔗 [VS Code Download](https://code.visualstudio.com/)

Após a instalação, abra o **VS Code** e instale as seguintes extensões:

- **CMake Tools** ➝ Para integração com o CMake.
- **C/C++** ➝ Para suporte à IntelliSense e depuração.
  
Para isso, você pode ir no menu View -> Extensions ou clicar no ícone da interface do Visual Studio Code.

---

## 📌 2. Clonando o Repositório de Exemplo

Agora vamos baixar o código de exemplo:

1️⃣ **Clone o repositório** no diretório de sua escolha:

```sh
git clone https://github.com/PedroTeixeiraa/ProcessamentoGrafico.git
```
Se você nunca usou o git, pode fazê-lo utilizando o 🔗 [Github Desktop](https://desktop.github.com/download/). Para isso, consulte o seguinte material:
- [Tutorial de Entregas pelo Github](TutorialEntregasGithub.pdf)

2️⃣ Abra o VS Code e vá em `Arquivo -> Abrir Diretório` (ou `File -> Open Folder`). Selecione o diretório onde você clonou o repositório de exemplo.

⚠️ É muito importante entrar no diretório do projeto pelo comando `Open Folder`. Caso contrário, o VS Code não reconhece o diretório `${workspaceFolder}`, o que poderá causar problemas no passo de compilação.

---

## 📌 3. Configurando o CMake no VS Code

1️⃣ No VS Code, pressione `Ctrl + Shift + P` e procure por `CMake: Scan for kit`

2️⃣ Logo após, pressione novamente `Ctrl + Shift + P` e procure por `CMake: Select a Kit`.  

3️⃣ **Selecione o compilador correto**:
   - Escolha `GCC for MSYS2 UCRT64` (ou similar, conforme sua instalação)

4️⃣ Este passo costuma acontecer automaticamente. Mas caso não ocorra nada após selecionar o kit, pressione `Ctrl + Shift + P` e execute `CMake: Configure`
   - Isso fará o **CMake detectar o compilador correto e preparar o projeto**. Espera-se que, após esta etapa, tenha-se uma saída similar a esta no terminal do VS Studio Code:

## 📌 4. Compilando e Executando o Projeto

Da mesma forma que o **CMake: Configure** pode executar automaticamente, a compilação pode ocorrer em sequência também de forma automática.
Caso não ocorra ou você pretenda compilar novamente:

1️⃣ Pressione `Ctrl + Shift + P` e execute `CMake: Build`
   - Ou rode manualmente no terminal:

   ```sh
   cd build
   cmake --build .
   ```

2️⃣ **Execute o programa**:
   ```sh
   ./Desafio.exe
   ```

Se tudo estiver correto, o projeto será compilado e executado com sucesso! 🚀