Project Overview
- Currently we already implemented walking with frame, right now we doesn't have the run frame yet, but if you hold shift it will move faster.
- We also have a menu that is very simple, well the escape button to go back to menu not working yet
- My next plan is to make the tileset or the map for the character, since we still using green background with border

How to Compile
- The project is configured for Visual Studio Code on Windows using the MinGW-w64 compiler.

- Compiler Version    : GCC 12.0+ (MinGW-w64).
- Required Libraries  : raylib (v5.0 recommended), raymath.h.
- Exact Compile Command: CTRL+SHIFT+B, then type make, then type .\BlindLoyalty.exe

How to Run
- Launch  : click BlindLoyalty.exe
- Controls:
   -Arrow Keys: Move Reuben around.
   -Left Shift: Hold to Run (increases speed and animation play rate).
   -Mouse Left Click: Select "PLAY" or "QUIT" in the menu.
   -ESC Key: not working yet

Project Structure Explanation
So There is 3 Folder in this file, the first one is .vscode

In .vscode folder, mostly consist of json, this json is the starter template, like inside of it there is setting for windows, linux, mac etc, but currently we are using windows, we dont know what happen if you use other OS, since our team use windows device.

The most important in that folder is launch.json. I look at that file for 2 hours to find why my code are not working. That file consist of path and debugger, thats why that file pretty crucial for coding.

The next Folder is Images, there is 2 folder inside it, background and character, in Character there are many sprites, but our main character is Reuben, while the other one botak and tuyul is actually other people asset from google, we use them for testing, like we are thinking how to make animation based on that sprite.

The last Folder is .src, this is type of thing from starter template that I dont dare to touch, because we dont know whats in that thing, the only thing i know is that there is license and readme that shows the original creator of template, I dont delete the readme, incase I got some problem, I might watch the video again

Maybe You will think why all our code outside the folder. Actually I tried to put in a folder named coding, but when i try to run it, it always have a problem, I dont found the solution yet but yeah I put all the code outside other folder.

character.h for character movement frame etc
gameplay.h for the playground testing(the future map)
menu.h for the menu when first time you open the game
main.c where all the code works