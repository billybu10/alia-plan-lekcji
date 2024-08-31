# Alia-plan-lekcji

Timetable in C++ written in [alia](alia.dev)

## Disclaimer

This project is written as my first attempt at writing in alia, and focuses mostly on it. I didn't write it in the most optimized way. If you (for some reason) want to use my project, it will need some modifications(css, it's awful, because i just didn't care).

## How to build

These instructions assume you're running Linux (and will work inside the
Windows Subsystem for Linux).

1. Ensure sure that you have [CMake](https://cmake.org/) (3.14 or higher) and
   [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)
   (1.39.18 or higher).

1. Clone this repository locally.

1. Build it. (Be sure to activate Emscripten for your shell before doing this.)

   ```
   mkdir build
   cd build
   emcmake cmake ..
   make -j
   ```

   Another option is to use VS Code with the CMake Tools extension to
   automatically handle this process. Just select the Emscripten kit and it
   should all work.

1. To view app, you'll need to run a web server to serve the files within
   the `build/deploy` directory. A Python 3.x script is included that will do
   this for you:

   ```
   cd build/deploy
   python3 ../../scripts/run-web-server.py
   ```

   With that running, you can navigate to http://localhost:8020/ to see your
   app.
