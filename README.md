# LOZ #

This project is a remake of the game Legend of Zelda.

### Summary ###

The repository is split into a game project and two tools. The tools extract resources from the original game and put them in a format usable by the game code.

The game is written in C++ and links with the Allegro library. The tools are written in C#.

Despite Allegro being a cross-platform library, all of the code is built with Visual Studio tools. Feel free to port all of this to other operating systems. Please let me know if you do.

The ExtractNsf project uses the Game Music Emu library.

### How do I get set up? ###

The subprojects build with Visual Studio 2010 and depend on the Allegro library (5.2) that you can get with Nuget. You’ll also need a copy of the original ROM and NSF for each game.

After you check out the project, you’ll have to define two variables/macros in you personal project settings: ALLEGRO_INC_ROOT and ALLEGRO_LIB_ROOT.

The remake is divided in two parts: the remade game in the Game folder, and the extractor in the Tools folder. The extractor pulls resources like graphics and data tables out of the original ROM and puts them into a format suitable for the remade game. Build and run the extractor before building and running the remade game.

The extractor takes some arguments. For example:

```
#!cmd

ExtractLoz <RomPath> <Function> -nsf <NsfFile> -out <OutputPath>
```

You can build all resources with the all function. Set OutputPath to the path where the remade game will go.

Once the resources are built, build and run the program in the bin folder.

### History ###

I've been interested in remaking the first Legend of Zelda since a family friend gave me a copy of Turbo C++ when I was in eighth grade.

There were many tries - each one more sophisticated than the last. But, at last I realized that the best chance I would have of remaking the game was to disassemble the original.

I started out with the help of [Computer Archeology](http://computerarcheology.com/NES/Zelda/) and [RomHacking.net](http://datacrystal.romhacking.net/wiki/The_Legend_of_Zelda) before I finally made a disassembler and other tools, and set out to disassemble the bulk of the game code myself.

The way I organized the disassembly was good enough for me, but I don't feel comfortable releasing my notes.

### Who do I talk to? ###

Repo owner: Aldo Nunez

If you would like to reach out to me, then please look me up at romhacking.net.