# STM32 Battleship

Bare-metal Battleship on the STM32F429i: no RTOS, no dynamic allocation in the game stack, all game logic running directly on a Cortex-M4F at 168 MHz. Built a full touchscreen game with a hunt-and-target AI opponent, two-player pass-and-play with hardware-enforced turn privacy, and a Flash-persisted attack heatmap and leaderboard. Display primitives, game state machine, AI, and Flash persistence written from scratch on top of the given HAL and LCD low-level drivers. Zero compiler warnings under `-Wall -Wextra`.

---

## Hardware

- **MCU:** STM32F429ZIT6 (Cortex-M4F, 168 MHz)
- **Display:** 240×320 ILI9341 TFT via LTDC + SDRAM framebuffer
- **Touch:** STMPE811 resistive controller, polled
- **Input:** onboard PA0 blue push button
- **Persistence:** internal Flash sector 11

---

## Design decisions

- **No `HAL_Delay` in the game loop.** Blocking delays stall rendering. Timing uses a TIM6 10 ms flag set in the ISR and read in the main loop; the ISR only sets a `volatile` flag. The remaining `HAL_Delay` calls live in the given init drivers and the 5-second splash in `main.c`, both outside the steady-state loop.
- **Flash writes once per game.** Erasing a Flash sector is blocking and takes roughly 2 seconds at this clock speed. Heatmap and win counters accumulate in RAM during play; the game-over handler erases and rewrites sector 11 once. Mid-game writes would freeze the UI.
- **Touch uses polling, not interrupts.** The STMPE811 interrupt line is unreliable on this board; polling every main-loop iteration is tight enough for a turn-based game and eliminates the ISR complexity entirely.
- **Hunt-and-target AI.** The AI opponent shoots randomly until it scores a hit, then queues adjacent cells for targeted follow-up. The queue filters already-shot cells so the AI never wastes a turn.
- **Two-player turn privacy.** Pass-and-play physically hands the board between players. A full-screen transition state (`STATE_TRANSITION_SCREEN`) blocks the display between turns. The next player presses the hardware button to reveal their view. Used in four places in the 2P flow to prevent each player from seeing the opponent's grid on handoff.

---

## Architecture

Strict three-module layering: ApplicationCode handles init and the main loop; GameDriver owns all state, AI logic, and Flash; DisplayDriver is the only translation unit that includes `LCD_Driver.h`. The hard boundary on display access means any display swap touches one file. Module-internal state stays `static`; cross-module access goes through header-declared getters. The two true globals (the TIM6 handle and the volatile timer flag) exist because the ISR has to reach them.

```
ApplicationCode.c   init, ISR, main loop
GameDriver.c        state machine, game logic, AI, Flash
DisplayDriver.c     rendering (the only file that includes LCD_Driver.h)
```

---

## Build and flash

Open `firmware/` as an STM32CubeIDE project (target part STM32F429ZIT6). Build in Debug config, then Run → Debug to flash over ST-Link. On boot the splash runs for 5 seconds, then the home screen renders.

---

## Demo

1. Home: tap `SINGLEPLAYER`, `MULTIPLAYER`, or `STATS`. Hold blue button for 3 seconds from any state to return here.
2. Placement: tap a cell to select a start, short-press blue button to rotate, tap on-screen `PLACE` to commit. 3 ships on a 7×7 grid. The next ship's ghost previews automatically after each commit.
3. Attack: tap cells on the opponent grid. Hits keep your turn (filled red circle in the cell), misses pass it (blue square, ~2-second result panel so the move is readable). First to sink all 3 ships wins.
4. 2-player: same flow with a full-screen transition between turns so neither player sees the other's board. Stats screen shows persisted win counts and a hit-density heatmap.

---

## Repo layout

```
firmware/    STM32CubeIDE project (Core/, Drivers/, .cproject, .project)
README.md
LICENSE
.gitignore
```
