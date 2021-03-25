fastsim (extended)
=======

Fastsim is a fast, lightweight simulator of a wheeled robot (khepera-like), primarily designed for use in the Sferes evolutionary framework (https://github.com/jbmouret/sferes2). This specific distribution has been extensively modified and documented by Christopher Stanton of the Evolving AI Lab at the University of Wyoming. Notable changes include:
- A door mechanism (temporary walls that obstruct movement until deactivated)
- A grid of "tiles" visited by the robot, typically used for statistical analysis (the tile size can be user-specified)
- A tile sensor, allowing the robot to sense the number of unvisited tiles within a configurable pie-slice area
- Enhancements to the existing light sensor, which now uses a pie-slice area similar to the tile sensor
- Framerate control during visualization, to prevent jittery videos
- The visualization of individual robot sensors can be manually enabled or disabled
- Various functions were renamed and recommented for a clearer indication of their intended use
- Minor performance enhancements and bug fixes

*If you use this software in an academic article, please cite:*

Mouret, J.-B. and Doncieux, S. (2012). Encouraging Behavioral Diversity in Evolutionary Robotics: an Empirical Study. Evolutionary Computation. Vol 20 No 1 Pages 91-133.

Dependencies:
--------------------

- Boost libraries (most versions should work)
- SDL 1.2 (http://www.libsdl.org/download-1.2.php); do *not* use any other versions, or Fastsim will not work properly

If the available binaries do not work, you will need to compile SDL from source.
During compilation, there is a known error on some systems concerning X11.
If you get this error (Linux users):
- Perform the fix listed here: http://hg.libsdl.org/SDL/rev/91ad7b43317a
- After applying the fix, you will need to run autoconf (if you don't have this program, download it), from within the SDL root directory
- Run autogen.sh, after which compilation should work without any further problems
This fix should also work for Mac users, but has not been tested. The exact procedure for Windows users is unknown at this time.

Usage (Sferes):
--------------------
- Copy fastsim to the "modules" directory in the sferes2 root directory
- Add fastsim in modules.conf in the sferes2 root directory (if modules.conf does not exist, create it)
- Run ./waf configure and ./waf build

*If SDL cannot be found,* copy the files from "sferesModifications" into your Sferes root directory, overwriting any existing files. These modified files have better SDL detection and will likely be included in future versions of Sferes.

You may also need to modify the fastsim wscript file (specifically, "model.includes") to point to your SDL includes folder.

Note: SDL will not be activated unless USE_SDL is defined (Sferes automatically takes care of this when detecting SDL). SDL is only required for visualization and can be safely disabled without affecting the core functionality of fastsim.

To see how fastsim is actually used, take a look at example.cpp.

Usage (Standalone):
--------------------

Fastsim should still be usable without Sferes. However, simu_fastsim.hpp must be modified to avoid any references to Sferes. No files other than simu_fastsim.hpp should require modification.

For standalone users, the fastsim wscript file and files in the "sferesModifications" folder will not be used.

Note: SDL will not be activated unless USE_SDL is manually defined either using flags or by manually defining USE_SDL in display.hpp. SDL is only required for visualization and can be safely disabled without affecting the core functionality of fastsim.

To see how fastsim is actually used, take a look at example.cpp.

Map Creation:
--------------------

- Fastsim requires that all maps use the PBM (portable bitmap) image format
- All PBM files *must* use the grayscale mode and be encoded in ASCII format (using a P1 header)
- PBM files with embedded comments cannot currently be processed by fastsim; open these files in a text or hex editor and remove any lines starting with the comment tag "#"
- Valid PBM files will have "P1" on the first line of the file, two ASCII numbers representing width and height on the second line, and a block of ASCII 1's and 0's, representing the black and white pixels of the map
- Because Photoshop does not seem to allow for ASCII encodings, the suggested editor for these maps is Gimp

In all maps, black pixels are considered obstacles (impassable) and white pixels represent empty space. Pixels can be modified in real-time using the drawRegion() function of Map.hpp. Conditional pixels (such as doors) can also be achieved using the functions in Door.hpp.