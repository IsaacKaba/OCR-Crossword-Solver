# 🖋️ OCR Project (C)

An OCR (Optical Character Recognition) project written in **C**, capable of resolving a crossword game.  

![C](https://img.shields.io/badge/Language-C-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Build](https://img.shields.io/badge/build-passing-success)

---

## ✨ Features

- 🧠 Neural network implemented in C for detecting letters and numbers  
- 📝 Solver that generates a list of possible words
- ⚡ Full program combining detection and solving in a graphical interface made with GTK  

---

## 📦 Installation

```bash
# Build Neural Network
cd NeuralNetwork
make ocr

# Use Neural Network
./ocr <PathToImage> # Image must represent a letter or number 

# Train Neural Network
./ocr -t <Epochs>
```
