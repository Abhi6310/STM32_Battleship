# STM32 Battleship

Bare-metal Battleship on the STM32F429i: no RTOS, no heap allocation, all game logic running directly on a Cortex-M4F at 168 MHz. Built a full touchscreen game with a hunt-and-target AI opponent, two-player pass-and-play with hardware-enforced turn privacy, and a Flash-persisted attack heatmap and leaderboard. All peripheral drivers written from scratch; zero compiler warnings under `-Wall -Wextra`.

---

## Hardware

- **MCU:** STM32F429ZIT6 (Cortex-M4F, 168 MHz)
- **Display:** 240×320 ILI9341 TFT via LTDC + SDRAM framebuffer
- **Touch:** STMPE811 resistive controller, polled
- **Input:** onboard PA0 push button
- **Persistence:** internal Flash sector 11

---

## Design decisions

- **No `HAL_Delay` outside `main.c`.** Blocking delays anywhere in the game loop stall rendering. Timing uses a TIM6 10 ms flag set in the ISR and read in the main loop. The ISR only sets a `volatile` flag; nothing else runs inside it.
- **Flash writes once per game.** Erasing a Flash sector is blocking and takes roughly 2 seconds at this clock speed. Heatmap and win counters accumulate in RAM during play; the game-over handler erases and rewrites sector 11 once. Mid-game writes would freeze the UI.
- **Touch uses polling, not interrupts.** The STMPE811 interrupt line is unreliable on this board; polling every main-loop iteration is tight enough for a turn-based game and eliminates the ISR complexity entirely.
- **Hunt-and-target AI.** The AI opponent shoots randomly until it scores a hit, then queues adjacent cells for targeted follow-up. The queue filters already-shot cells so the AI never wastes a turn.
- **Two-player turn privacy.** Pass-and-play physically hands the board between players. A full-screen transition state (`STATE_TRANSITION_SCREEN`) blocks the display between turns. The next player presses the hardware button to reveal their view. Used in four places in the 2P flow to prevent each player from seeing the opponent's grid on handoff.

---

## Architecture

Strict three-module layering: ApplicationCode handles init and the main loop; GameDriver owns all state, AI logic, and Flash; DisplayDriver is the only translation unit that includes `LCD_Driver.h`. The hard boundary on display access means any display swap touches one file. Cross-module state goes through header-declared getters. No globals, no accidental coupling.

```
ApplicationCode.c   init, ISR, main loop
GameDriver.c        state machine, game logic, AI, Flash
DisplayDriver.c     rendering (the only file that includes LCD_Driver.h)
```

---

## Build

Build with `-Wall -Wextra` (zero warnings). Open `firmware/` in STM32CubeIDE and flash via Run → STM32 C/C++ Application.
