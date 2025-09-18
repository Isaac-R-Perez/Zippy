# Zippy
Zippy is a 2D twin-stick arcade game built for Windows. Inspired by Handmade Hero. The player takes on the role of Zippy, a small ship tasked with surviving waves of increasing enemies. You can dodge bullets and reflect them back, as well as shoot and fire off attacks of your own. Your ultimate goal is to face your nemesis, Zappy, and defeat him! 

In this game the player can move, shoot, and dash. Dashing is done in the direction you last moved, not where you are aiming! While dashing, the player can collide with bullets safely and send them back at enemies. You and the enemies become stronger as the waves progress until Zappy finally appears on Wave 13. Defeat Zappy and you win!

# Gamplay Demo
https://youtu.be/8K6ONOFbeAI

# How To Play
I have provided a pre-compiled version within a zip file built on my machine, so all you'd need to do is clone the repository onto a Windows machine then unzip the folder and run the executable inside to start the game.

However, if you'd like to compile from source, I have provided all of the files you'd need to do so, as well as a batch script to build from the command line using MSVC.

# Building From Release_Build.bat
My setup builds Zippy using Visual Studio 2022 Developer Command Prompt v17.14.0 x64. You'll have to clone the repo, initialize your MSVC build tools into your console, and then run Release_Build.bat from your command line to compile and create Zippy.exe as well as IGame.dll. 

The newly created files will be placed in your repo so all you'd need to do to play the game is launch the executable Zippy.exe. 

If you'd like to move the game to another folder then all necessary files are: Zippy.exe, IGame.dll, and the entire Assets folder. Make sure Zippy.exe and IGame.dll are in the same folder alongside your Assets folder (the game looks for Assets/... while running).

# Game Controls
There are two ways to play: with mouse + keyboard and with a gamepad. If you have a supported controller plugged in, keyboard controls will be locked.

# Mouse + Keyboard Controls
W - Move Up

A - Move Left

S - Move Down

D - Move Right

Left Click - Fire Weapon

Right Click - Dash

Esc - Pause Game

Backspace - Display Debug Info

-- Zippy will follow your mouse to aim in keyboard + mouse mode. 

# Gamepad Controls
Left Stick - Move

Right Stick - Aim

Left Shoulder - Dash

Right Shoulder - Fire Weapon

Start - Pause Game

Back - Display Debug Info

# Zippy Software Details
Zippy is split into two pieces, the platform code (win32_platform.cpp) and the game code (IGame.cpp). win32_platform is the main file and it is what gets compiled into Zippy.exe. The platform code is where all Windows functionality is housed, like window mangement, audio control, gathering user input, and allocating memory. This section is where the game is launched and closed from. The platform code loads Zippy's game code dynamically from IGame.dll. 

The game code is where all of the game-specific functionality is done, such as drawing entities or text into a bitmap for rendering, loading textures, processing user input, processing collision, updating all entities, managing the game's state, and much much more. Basically, anything that is independent from the platform the game is running on is done within IGame.dll.

Most of the game's systems were built from scratch. Including the entity management system, the arena memory allocator, entity collisions, software rendering, text rendering, user input, bitmap texture loading, the saving and loading system, vector and matrix math functions within my own math library, and more.

However, I use a couple of standard libraries and several Windows libraries where necessary. For gamepad support I used XInput and for async multi-threaded audio support I used XAudio 2. My psuedo-random number generator for the game is a 32-bit Mersenne-Twister from the random standard library and it is initialized with the current time using the chrono standard library functions.

Zippy was developed in the style of C++ that I learned by going through the first 30 episodes of Handmade Hero on YouTube. All dynamic memory allocation is performed with VirtualAlloc, either upfront like for the GameState and memory arenas or in place for things such as file reading. There are no C++ classes and all custom data types are POD structs. My code makes heavy use of pointers, C macros, and direct memory manipulation to implement the game's functionality.
