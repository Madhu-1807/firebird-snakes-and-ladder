# 🐍 Paramapadham (Snakes and Ladders) – Firebird V Robot Game

This project implements the classic game of *Snakes and Ladders (Paramapadham)* on the **Firebird V robot** using AVR C in Microchip Studio. The robot navigates a grid-based board, rolls a virtual die, and responds to snakes and ladders using sensor input and programmed logic.

---

## 🎮 Features

- 🔁 Autonomous movement along a predefined grid path
- 🎲 Dice roll logic for player turns
- 🐍 Snakes and ladders logic with position-based transitions
- 📟 LCD display for game info
- 🔊 Buzzer feedback for events
- 🔍 Line following using IR sensors

---

## ⚙️ Hardware Used

- **Firebird V Robot Platform**
- Atmega2560 Microcontroller
- IR Proximity Sensors (line tracking)
- LCD Module
- Buzzer
- Push button (for dice roll)

---

## 🛠 Development Tools

- [Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio) (formerly Atmel Studio)
- AVR-GCC Toolchain
- USB AVR Programmer

---

## 🚀 How to Run

1. Clone or download this repository.
2. Open the project in **Microchip Studio**.
3. Build the project and flash the `.hex` file to the Firebird V robot.
4. Place the robot at the start of the game board.
5. Use the push button to simulate a dice roll and begin movement.

---

## 📁 Project Contents

- `main.c` – Main control logic for gameplay and movement
- `Black_Line_Following.hex` – Compiled HEX file ready for flashing
- `Black_Line_Following.atsln` – Microchip Studio solution file
- Supporting `.c` and `.h` files for LCD, buzzer, motion, etc.

---

## 📸 Demo 

Video will be avaiable soon

---

## 📄 License

This project is for educational and non-commercial use. Feel free to fork, modify, and experiment!

---

## 🙌 Acknowledgments

- This project was mentored by Prof. Saraswathi Kritivasan
- NEX Robotics for the Firebird V platform
