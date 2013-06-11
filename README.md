Game of Drones
Keep Calm and Shoot Your Way Out
================================

It’s 3033, and humans and aliens are at war in a race to conquer the galaxy! Stranded on the planet of Monstradoom, you’ve been taken prisoner of war by your malevolent hosts. Not one to get held up, you’ve managed to escape - and what’s more - by stealing a serious piece of extraterrestrial flying technology. You’re on the run, and they’re on the hunt!

Developers
==========

Compilation instructions for Linux:

1. Install (and compile) dependent libraries. Please refer to the instructions
   at the library web sites below.
   
    a. Boost (Networking and timers) - http://www.boost.org/
    b. GLM (Math) - http://glm.g-truc.net/
    c. FMOD (Sound) - http://www.fmod.org/
    d. FTGL (Font rendering) - http://ftgl.sourceforge.net/docs/htm
    e. GLFW (Windowing) - http://www.glfw.org/
    f. GLEW (OpenGL extension) - http://glew.sourceforge.net/
    
2. Install g++ 4.7 or higher for C++11 support

3. Install the Go programming language: http://golang.org/

4. cd into the project root directory in the command line and build the source  by typing "make" to invoke the provided Makefile. This command will produce two files in the root directory: Server.out and GameOfDrones.out. Server.out is the server program, and GameOfDrones.out is the main game.

There are also XCode and Visual Studio 11 projects (what we used to develop the game in), but in our lengthy attempts to get them working outside of our regular development environments, we encountered significant difficulties trying to get our game to compile properly. As a result, we have included windows executables with our submission.


Running the Game:

Start the server by typing: ./Server.out
In a separate terminal session, start the main game by typing: ./GameOfDrones.out

To check the setup, hit enter at the main screen to start a new game and then hit enter on the next screen to connect to the server. This will connect the game to a local server. It should load the level and you should be able to dodge enemies with the arrow keys.

To play as a co-op, two players must start their own instances of the game. When on the new game screen, if one or more players is on a different computer from the main server, the player(s) must select the IP address field using the up and down arrow keys, hit enter, and type in the ip address. Then, in the player field, the pilot must be player “1” (no quotes), and the gunner must be player “2”. The gunner looks around using the mouse, and has two main abilities: the primary blaster (left mouse button) and a lightning strike (right mouse button).

IMPORTANT: The server pairs an incoming player 2 with the last player 1, so whoever is player 1 MUST hit connect before player 2.

Additionally, you’ll notice that the initial field for player is “1s”. If the player field is “1s” or “2s”, then the player is able to play alone in his respective role. Note that because Game of Drones is a co-op game, not all features are enabled when playing without a partner.


Enjoy!
