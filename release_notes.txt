Qoat of the Hill 
================

This is a Nokia Developer example game implemented with Qt and OpenGL ES 2.0.
The project contains two solutions. The first one utilises Qt GameEnabler, and
the second one, the more recent, utilises Games API.

The game is a classic Scorched Earth / Worms clone with two players trying to 
knock each other out off a randomly generated mountain with the shockwaves 
from their exploding ammunition.

All the graphics in the game are rendered with native OpenGL ES 2.0.
  
The application has been tested on Symbian Anna (Nokia N8-00, Nokia E6-00, and 
Nokia E7-00), Maemo 5 (Nokia N900), MeeGo 1.2 Harmattan (Nokia N950),
Windows XP, and Windows with Qt 4.7.3 and PowerVR OpenGL 2.0 emulation
libraries.

This example application is hosted in Nokia Developer Projects:
- http://projects.developer.nokia.com/qoatofthehill

-------------------------------------------------------------------------------

PREREQUISITES 

- Qt basics 
- OpenGL basics
- GLSL basics

-------------------------------------------------------------------------------

PROJECT STRUCTURE

Folders
-------

 |                          The root folder contains the project files for both
 |                          the Qt GameEnabler and the Games API solution,
 |                          the license information and this file (release
 |                          notes).
 |
 |- bin                     Contains the compiled binaries of the project.
 |
 |- ge_src                  Contains the Qt GameEnabler source files.
 |
 |- icons                   Contains the project icons.
 |
 |- images                  Contains the game graphics.
 |
 |- qtc_packaging
 |  |
 |  |- debian_fremantle     Debian packaging files for Maemo 5.
 |  |
 |  |- debian_harmattan     Debian packaging files for MeeGo 1.2 Harmattan.
 |
 |- sounds                  Contains the game audio files.
 |
 |- src                     Contains the general game source files.
 |
 |- src_gameenabler         Contains the source files of the Qt GameEnabler
 |                          specific solution.
 |
 |- src_gamesapi            Contains the source files of the Games API specific
 |                          solution.


Important Classes
-----------------

- GameLevel: A class for the game level. Contains a generated 2D version of 
  the landscape and functionality/structures to generate a renderable 3D mesh 
  from it. 

- GameMenu: A very simple menu containing a title and two buttons. 
  All texts are rendered as static images.

- GameObject: An interface for a dynamic object in the game. Contains 
  information for position, moving direction, sprite index (texture), and lots 
  of other information attached to a single object. Both players, ammunition, 
  trees, tooltips, and so on inherit from this.

- GameObjectManager: A class holding a list of objects inherited from 
  GameObject and methods to manipulate them. These objects will be 
  run/rendered.

- GamePlayer: A class inherited from GameObject. Implements functionality 
  for a single player. NOTE: GamePlayer.h and GamePlayer.cpp contain several 
  small, separate classes which are all inherited from GameObject and 
  implement object-specific functionality. For example, GameTree, 
  GameBurningPiece, GameAmmunition, etc. 

- ParticleType: Defines the behaviour/attributes of a single particle. 

- ParticleEngine: An engine running/rendering all the particles.

- TextureManager: A helper class for loading/caching/automatic releasing of 
  textures.

- GameInstance: Complete game instance, encapsuling GameLevel, 2 players, 
  ParticleEngine, GameObjectManager, and other objects required. A pointer 
  to this class is spread through an object interacting in the game. 

- MyGameWindow: The main game window derived from Qt GameEnabler's 
  GE::MainWindow. This is the 'replacement QGLWidget' providing pre-
  initialised OpenGL ES 2.0 for us.

-------------------------------------------------------------------------------

BUILD & INSTALLATION INSTRUCTIONS 

Preparations 
~~~~~~~~~~~~ 
Check that you have at least the latest Qt SDK installed in the development
environment and on the device.

The application can be compiled directly to any device with OpenGL ES 2.0 
support, for example, Maemo or Symbian Anna. However, if you wish to compile it 
on another platform such as desktop Windows, you must have external 
OpenGL ES 2.0 emulation libraries. With Windows or Linux (32bit) desktop, you 
can use Khronos OpenGL ES 2.0 PC emulation libraries available from www.imgtec.com 
(http://www.imgtec.com/powervr/insider/sdkdownloads/index.asp#GLES2). 


Build & installation instructions using Qt SDK 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

1. Open the project file: 
   Select File > Open File or Project, then select qoatofthehill.pro. 

2. Select target(s), for example Desktop and Symbian Anna, and press Finish. 

3. Compile the project. If you are building for Desktop, modify your .pro 
   file's 'windows' section's GLES2 include and library paths according to 
   where you have installed the SDK: 
   'INCLUDEPATH += /PowerVRSDK/Builds/OGLES2/Include' and
   'LIBS +=  -L/PowerVRSDK/Builds/OGLES2/WindowsPC/Lib' must point to valid 
   locations.

4. You can now run the software. Have fun! 

-------------------------------------------------------------------------------

COMPATIBILITY 

- Qt 4.7.0 (Maemo)
- Qt 4.7.4 or higher (Symbian Anna, MeeGo 1.2 Harmattan, and desktop)
- Qt Mobility 1.2.1 (Symbian Anna and MeeGo 1.2 Harmattan)
- OpenGL ES 2.0

Tested on:  

- Nokia E6-00
- Nokia E7-00
- Nokia N8-00
- Nokia N900 (PR1.2, PR1.3 firmware)
- Nokia N950
- Windows XP
- Windows 7 

Developed with: 
- Qt SDK 1.1

-------------------------------------------------------------------------------

LICENSE

See the license text file delivered with this project. The license file is also
available online at
https://projects.developer.nokia.com/qoatofthehill/browser/trunk/Licence.txt

-------------------------------------------------------------------------------

VERSION HISTORY 
1.3 Bug fixes.
1.2 Added Games API build
1.1 Added MeeGo 1.2 Harmattan support
1.0 First release
0.1 First version