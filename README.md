# Welcome to BPV!

## What is BPV?
BPV stands for Basic Particle Viewer and is a Qt user interface used to... well, visualize particles!  
Particles can be used to represent a lot of different things in physics (electrons, atoms, balls, sand, even liquids or gases, etc. ).
When you start making your own program to simulate them it can be convenient to have an easy way to visualize them. Hence BPV!

## How do I compile the code?

The BPV project contains actually two binaries:

+ **dir2hcpp**: used to transform images (e.g. icons) into source code that can be compiled and included in the bpv binary. The code generated by dir2hcpp will be saved under ./app/include\_gen and ./app/source\_gen.
+ **bpv**: the main application (requires the files generated by dir2hcpp).

To compile the code, first you'll need to download and install the open source distribution of [Qt](http://www.qt.io/) and update your PATH environment variable with the bin directory containing qmake (and also the one containing mingw32-make.exe if you are using Windows). Then you have two options:

### Using a terminal

```
> qmake bpv.pro
> [mingw32-make, make] release
```
The bpv binary will be located under ./bin

### Using Qt Creator
+ Open Qt Creator.
+ Click on 'Open Project'.
+ Select bpv.pro.
+ Click on 'Configure Project'.
+ On the left hand side, click on 'Projects' and select the 'Release' build configuration.
+ On the left hand side, click on 'Edit'. Right click on the sub-directory dir2hcpp and select 'Build "dir2hcpp"'.
+ In the Menu, click on 'Build', then on 'Build Project "bpv"' (or simply Ctrl+B).

Here again the bpv binary will be located under ./bin.
To start the application from  Qt Creator, click on the 'Build' menu, then 'Run' (or just Ctrl+R).

## Input files
BPV can read two kinds of files: data files and geometry files.  
+ Data files are binary files containing all the simulation data (particles positions, particles type, bonds, etc.).
+ Geometry files are text files containing the static geometry of the environment in which the particles move (e.g. shape of a transistor in which electrons flow, box in which balls bounce, etc.)

### Data files (*.par)

Data files can be split in several sections: The first ones describe the particles (e.g. particle type, variables, bonds between particles, etc.) while the last one contains the simulation data itself (i.e. particle position and variables value at a given time).  
Here is the detailed structure of a data file:

+ **PAR**. Every data file must starts with the 3 bytes 'PAR' (i.e. 0x50 0x41 0x52).
```
  PAR (3 bytes)
```
+ **Types**. Particles can be grouped in different types. Each type has its own name, default radius, and default color which are used to display the particles (e.g. you can have one type to represent Hydrogen atoms in white, another one for Oxygen atoms in red, etc.)
```
  nbTypes (1 byte)
  name    type 1  (32 bytes)
  radius  type 1  (1 float)
  red     type 1  (1 byte)
  green   type 1  (1 byte)
  blue    type 1  (1 byte)
  name    type 2  (32 bytes)
  radius  type 2  (1 float)
  etc.
```
+ **Variables**. Particles have 3 coordinates, but can also have additional variables associated to them (e.g. energy, velocity, etc.). This section lets you name each axis and each variable.
```
  nbVariables (1 byte. Does not include the x, y, and z axis)
  name    x  axis    (32 bytes)
  name    y  axis    (32 bytes)
  name    z  axis    (32 bytes)
  name    variable 1 (32 bytes)
  name    variable 2 (32 bytes)
  etc.
```
+ **Particles**. This section contains the number of particles in the simulation and specifies the type of each particle.
```
  nbParticles (1 unsigned int)
  type of particle 1  (1 byte. 0 based: 0 for type 1, 1 for type 2, etc.)
  type of particle 2  (1 byte)
  etc.
```
+ **Bonds**. Bonds must be specified even if your simulation does not contain bonds (in which case set both nbBonds and bonds radius to 0 ).
```
  nbBonds       (1 unsigned int)
  bonds radius  (1 float)
  1st index of bond 1 (1 unsigned int. 0 based)
  2nd index of bond 1 (1 unsigned int)
  1st index of bond 2 (1 unsigned int)
  etc.
```
+ **Steps**. A simulation is made of successive steps corresponding to a given time and in which each particle has a given set of coordintes and variable values.
```
  nbSteps       (1 unsigned int)
  time step 1   (1 float. Expressed in seconds)
  x coordinate particle 1 step 1  (1 float)
  y coordinate particle 1 step 1  (1 float)
  z coordinate particle 1 step 1  (1 float)
  variable 1 particle 1 step 1    (1 float)
  variable 2 particle 1 step 1    (1 float)
  etc.
  x coordinate particle 2 step 1  (1 float)
  etc.
  time step 2   (1 float)
  etc.
```

### Geometry files (*.geo)
Geometry files are text files containing the geometry of the scene in which the particles move.  
The syntax used for these files was chosen such that they could be imported as-is in 3DSMax (Cf. MAXScript).  
Five types of primitives can be used (box, cylinder, sphere, line, and mesh). Each primitive is defined with one single line which starts with the name of the primitive, followed by a set of key:value pairs separated by a space character. A few of these keys are common to all primitives:
+ `name`: Name of the primitive. Value = string (e.g. `"my box"`).
+ `wirecolor`: Color used to draw the primitive. Value = `(color R G B A)` where `R`, `G`, `B`, and `A` have values between 0 and 255. The alpha value (`A`) is used to set the transparency (0: invisible, 255: fully opaque).
+ `transform`: A 4x4 transformation matrix `A` used to transform the primitive. Value = `(matrix3[a00,a10,a20][a01,a11,a21][a02,a12,a22][a03,a13,a23])` where `aij` are floats, `a30=a31=a32=0` and `a33=1`.

Here is the list of primitives with their specific keys:
+ **Box.** Starts with `box`.
  + `pos`: Position of the center of the bottom side of the box (not the center of the box...). This overwrites the translation component of the transformation matrix if any. Value = `[x,y,z]`. Default = `[0,0,0]`.
  + `width`: Box width (x axis). Value = float. Default = `0`.
  + `length`: Box length (y axis). Value = float. Default = `0`.
  + `height`: Box height (z axis). Value = float. Default = `0`.
  + `widthsegs`: Number of segments along the x axis. Value = integer. Default = `1`.
  + `lengthsegs`: Number of segments along the y axis. Value = integer. Default = `1`.
  + `heightsegs`: Number of segments along the z axis. Value = integer. Default = `1`.
+ **Cylinder.** Starts with `cylinder`
  + `pos`: Position of the center of the bottom of the cylinder (not the center of the cylinder...). This overwrites the translation component of the transformation matrix if any. Value = `[x,y,z]`. Default = `[0,0,0]`.
  + `radius`: Cylinder radius. Value = float. Default = `0`.
  + `height`: Cylinder height (z axis). Value = float. Default = `0`.
  + `heightsegs`: Number of segments along the z axis. Value = integer. Default = `1`.
  + `sides`: Number of sides. Value = integer. Default = `18`.
  + `capsegs`: Number of segments for the caps. Value = integer. Default = '1'.
+ **Sphere.** Starts with `sphere`
  + `pos`: Position of the center of the sphere. This overwrites the translation component of the transformation matrix if any. Value = `[x,y,z]`. Default = `[0,0,0]`.
  + `radius`: Sphere radius. Value = float. Default = `0`
  + `segs`: Number of 'segments' used to draw the sphere. Value = integer. Minimum value = `1`. Default = `8`.
+ **Lines.** Starts with `line`
  + `points`: List of points defining the lines. Value = `[x1,y1,z1][x2,y2,z2]...[xN,yN,zN]`.
  + `close`: Connect the first and last point if true. Value = bool (`yes` or `no`). Default = `no`.
+ **Mesh.** Starts with `mesh`
  + `vertices`: List of vertices in the mesh. Value = `#([x1,y1,z1],[x2,y2,z2],...,[xN,yN,zN])`.
  + `faces`: List of faces in the mesh. Value = `#([i11,i12,i13],[i21,i22,i23],...,[iM1,iM2,iM3])` where `i..` are integers and are the 1-based indices of the vertices given in the `vertices` array. Each set of 3 indices creates a triangle (order of vertices is important as only one side of the triangle is visible).
  
Examples:
+ `box wirecolor:(color 0 255 255 60) width:20 length:10 height:50 pos:[10,0,0] name:"box 1"`
+ `cylinder radius:2.5 height:20 wirecolor:(color 255 0 0 255) transform:(matrix3[0,0,1][0,1,0][1,0,0][0,0,0])`
+ `sphere radius:2.5 wirecolor:(color 255 255 0 60)`
+ `line wirecolor:(color 0 0 255 255) points:[0,0,0][20,0,0][20,5,0][0,5,0] close:yes`
+ `mesh wirecolor:(color 255 0 255 255) vertices:#([0,0,0],[10,0,0],[0,10,0],[0,0,10]) faces:#([1,3,2],[1,2,4],[1,4,3],[2,3,4])`

Finally, lines starting with two dashes (`--`) are ignored and can be used for comments.