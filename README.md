# Captain Claw
Realization of the beloved game "Claw", released by Monolith Productions in 1997

_by Ariel Halili_

### Overview
Captain Claw is a modern C++ re-implementation of the classic 2D platformer game *Claw* (1997), originally developed by Monolith Productions. This project recreates the game's rich world, vibrant characters, and challenging gameplay, staying faithful to the original, while utilizing modern technologies and techniques.

### Gameplay
Captain Claw follows the same mechanics and design as the original game. A **HELP** screen is available in the main menu, and additional guides are accessible online as this is a realization of the original game. Players take on the role of Captain Claw, navigating challenging levels, battling enemies, and collecting treasures.

### Modules
- **Framework** – A general library shared across all modules.
- **RezParser** – Classes and methods for parsing the REZ file and its content, based on documentation at [The Claw Recluse](https://captainclaw.net/) and [OpenClaw/libwap](https://github.com/pjasicek/OpenClaw/tree/master/libwap).
- **ExtractRez** – A tool to extract and save the parsed REZ file to a local directory.
- **GameEngine** – A library responsible for handling GUI and audio.
- **MyClaw** – The main game implementation, putting all modules together with game's logic.

### Technologies
- **WinAPI** – Handles native Windows functionality such as window management and system interactions.
- **DirectX (Direct2D, DirectSound)** – For high-performance 2D rendering and sound management.
- **Custom Physics Engine** – Physics is fully implemented from scratch, offering accurate and optimized in-game mechanics.
- **Custom Game Engine** – The game engine is entirely written from the ground up to manage game logic, graphics, and audio in a seamless manner.

### Building & Running
To build the project, use Visual Studio with support for WinAPI and DirectX. All dependencies should be included in the repository, and the game should compile with minimal configuration required.

1. Clone the repository
2. Open the project in Visual Studio
3. Build and run

### Contributing
Contributions are welcome! If you'd like to help, please check out the open issues or submit your suggestions. When contributing:
- Follow the existing coding style and structure.
- Make sure your contributions are tested before submitting.
- Open pull requests for review.

### License
This project is licensed under the MIT License. See the LICENSE file for more details.

### Credits
- https://captainclaw.net/ - the Claw Recluse
- https://github.com/pjasicek/OpenClaw/tree/master/libwap - REZ file parser and its content (.PID, .ANI, etc.)
- https://github.com/freudi74/mfimage - PCX file parser
- https://blog.fourthwoods.com/2012/02/24/playing-midi-files-in-windows-part-5/ - MIDI player
- https://github.com/BuildSucceeded/2D-Platformer - the initial idea for the realization of the project

### Screenshot
<img src="./screenshot.jpg" alt="screenshot of level 1" height="400"/>
