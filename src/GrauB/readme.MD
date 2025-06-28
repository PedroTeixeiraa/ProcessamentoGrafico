# 🧛‍♂️ Tilemap Isométrico com Vampirão

Trabalho do Grau B – **Processamento Gráfico**  
Implementação de um jogo simples com tilemap isométrico (formato diamond), utilizando **OpenGL**, **GLFW**, **GLM**, e **STB Image**.

## 👥 Integrantes do Grupo

- Diuliano Vargas
- Felipe Bergamo
- Pedro Teixeira Alves

## 📌 Objetivo

Desenvolver um protótipo jogável com:

- Tilemap isométrico (formato diamond) carregado via `.txt`
- Personagem com animação baseada em spritesheet
- Movimento em 8 direções (N, S, L, O, NE, NO, SE, SO)
- Itens colecionáveis (ex: moedas)
- Obstáculos (ex: lava)
- Troca visual de tile ao pisar
- Condições de vitória e derrota

---

## 🚀 Como Executar
As instruções detalhadas de compilação e execução (incluindo dependências e comandos) estão descritas no arquivo [README_CONFIGURACAO.md](./README_CONFIGURACAO.md).

---

## 🧠 Estrutura do Projeto

```
📁 assets/
┣ 📁 tilesets/
┃ ┗ tilesetIso.png
┣ 📁 sprites/
┃ ┗ Vampires1_Walk_full.png
📁 src/
┗ 📁 GrauB/
  ┣ Desafio.cpp
  ┣ README.md
  ┗ README_CONFIGURACAO.md
📄 CMakeLists.txt
📄 README.md
📄 map.txt
```
---

## 🗺️ Arquivo de Mapa (`map.txt`)

```txt
tileset.png 10 64 64      // tileset, nº de tiles, largura e altura
15 15                     // largura e altura do mapa

0 1 1 1 0 ...             // matriz de IDs dos tiles

--                       // separador de seções
TileProperties           // início da seção de propriedades

0 0 0                    // Tile 0: não troca, não é obstáculo, não é item
1 0 1                    // Tile 1: não troca, não é obstáculo, é item
2 1 0                    // Tile 2: troca, não é obstáculo, não é item
...
```

Propriedades dos tiles:

* `changeTile` → muda visualmente ao pisar
* `hazard` → causa derrota
* `collectible` → pode ser coletado (ex: moeda)

---

## ✨ Funcionalidades Implementadas

✅ Mapa 15x15 carregado via `.txt`  
✅ Sprites animados com base em frames e direções  
✅ Movimento nas 8 direções permitidas  
✅ Itens coletáveis (moedas) com contador  
✅ Tiles perigosos (lava/morte)  
✅ Troca de tile (mudança visual ao pisar)  
✅ Vitória ao coletar todas as moedas  
✅ Lógica de tiles (caminhável, não caminhável) definida no `.txt`

---

## 🎮 Controles

* **W / S / A / D**: Norte, Sul, Oeste, Leste
* **Q / E / Z / C**: Diagonais (NO, NE, SO, SE)

---
