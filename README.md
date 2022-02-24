![logo](https://github.com/kaluma-project/kaluma/blob/master/logo.png?raw=true)

[![Current Version](https://img.shields.io/github/tag/kaluma-project/kaluma.svg)](https://github.com/kaluma-project/kaluma/tags)
[![GitHub license](https://img.shields.io/github/license/kaluma-project/kaluma)](https://github.com/kaluma-project/kaluma/blob/master/LICENSE)
[![Docs Status](https://img.shields.io/badge/docs-ready-orange.svg)](https://kalumajs.org/docs/)

# Overview

**Official website: [kalumajs.org](https://kalumajs.org/)**

__Kaluma__ is a tiny and efficient **JavaScript runtime** for [RP2040 (Raspberry Pi Pico)](https://www.raspberrypi.org/products/raspberry-pi-pico/). The main features are:

- **Small footprint**. Runs minimally on microcontrollers with 300KB ROM with 64KB RAM.
- Support **modern JavaScript** standards (ECMAScript 5/6/6+). Powered by [JerryScript](https://jerryscript.net/).
- Has internal event loop like as Node.js for **asynchronous**.
- Has **built-in modules** including file systems (LittleFS, FAT), graphics, networking and more.
- Support RP2's **PIO (Programmable I/O) assembly** embeddable in JavaScript code.
- Provides very friendly API that resembles **Node.js** and **Arduino**.
