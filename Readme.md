#  OCR & Crossword Solver

A lightweight C-based application designed to perform Optical Character Recognition (OCR) on crossword puzzle images and automatically resolve them using a custom search-based solver and a GTK graphical user interface.

---

##  Project Overview

This project provides a complete pipeline from raw image input to a fully solved crossword puzzle:

1. **Image Preprocessing:** Grayscale conversion, noise reduction, thresholding, and grid detection.
2. **Optical Character Recognition (OCR):** Segmentation of individual characters and classification using a custom multi-layer perceptron (MLP) built from scratch in C.
3. **Automated Solver:** Algorithmic grid resolution using dictionary lookup and constraint satisfaction backtracking.
4. **User Interface:** Interactive GTK-based GUI allowing users to load images, review detected characters, and visualize the solution step-by-step.

---

##  Key Technical Features

- **Built in C:** High performance with zero heavy external machine learning dependencies.
- **Custom Neural Network:** Multi-Layer Perceptron trained for character detection (A–Z) written entirely from C primitives.
- **Image Processing Pipeline:** Custom algorithms for edge detection, binarization, and grid cell extraction.
- **GTK Graphical Interface:** User-friendly interface to load images, adjust grid parameters, and run the solver.

---

##  Repository Structure

```text
├── src/
│   ├── image_processing/   # Image filtering, thresholding, grid extraction
│   ├── ocr/                # Neural network architecture, weights, character recognition
│   ├── solver/             # Crossword backtracking algorithm & dictionary lookup
│   └── gui/                # GTK interface and event handling
├── data/
│   ├── dictionary.txt      # Lexicon for crossword resolution
│   └── trained_weights.bin # Saved neural network weights
├── Makefile                # Build configuration
└── README.md
