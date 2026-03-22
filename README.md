# q-neural-snake-c
Autonomous Snake game driven by a Deep Q-Learning Neural Network built from scratch in C. Uses Raylib for rendering and 8-direction sensory input.
# Q-REINFORCEMENT NEURAL NETWORK SNAKE

Developed entirely by me. This project implements a Neural Network for autonomous Snake game management using Deep Q-Learning (DQN). The agent learns optimal strategies to maximize the score by interacting with the environment without pre-set movement rules.

The Neural Network code (Forward and Backpropagation) and the learning logic were written from scratch in C.

## Requirements

RAYLIB must be installed to compile and run the project.

## Key Features

- Neural Network: Custom implementation without external Machine Learning libraries.
- 8-Direction Sensory Input: Perception of apple, obstacles, and body in 8 directions (24 total inputs).
- Performance: Achieved a Score of 46 after 270,000 training episodes.
- Graphics: Rendering handled via Raylib.

## State Representation (Input)

The Neural Network receives a 24-dimensional input vector:

1. Wall Distance: Inverse distance (1/dist) in 8 directions.
2. Apple Detection: Binary flag (1.0 if visible) in 8 directions.
3. Self-collision: Inverse distance (1/dist) from its own body in 8 directions.

## Neural Network Architecture

- Input Layer: 24 neurons (Perception)
- Hidden Layer: 128 neurons (ReLU activation)
- Output Layer: 3 neurons (Actions: Left, Forward, Right)

## Reward System

- Apple eaten: +65.0
- Moving closer to apple: +2.0
- Moving away from apple: -1.5
- Time penalty (per step): -0.25
- Collision (Wall/Body): -25.0

## Installation and Usage

### Compilation (Windows/MinGW)

gcc snake.c -o snake.exe -lraylib -lgdi32 -lwinmm -lm

### Execution

./snake.exe

## Controls

- Q: Toggle between Neural Network and Manual Mode.
- Arrow Keys: Manual control (when Neural Network is deactivated).

## License

Open-source project. Free to use and modify.
