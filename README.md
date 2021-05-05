# Marching cubes

Lab project for the advanced 3D modeling course in [MIRI](https://masters.fib.upc.edu/masters/miri-computer-graphics-and-virtual-reality), at UPC. Implementation of the [_Marching cubes_](http://fab.cba.mit.edu/classes/S62.12/docs/Lorensen_marching_cubes.pdf) algorithm (built on Qt).


## Animation demo
https://user-images.githubusercontent.com/44426596/117112911-0d96a280-ad8a-11eb-9854-c7fcfe69d433.mp4


## How to set-up
Use any UNIX terminal to navigate to the [Mesh Viewer](MeshViewer_73156e6) directory, create a new _build_ directory and run the _makefile_:
```
>> cd MeshViewer_73156e6
>> mkdir build
>> make -j
```
Finally, run the actual application:
```
>> ./MeshViewer
```
  
## Application use
### Compute isosurface
Isosurface computation can be achieved by right-clicking inside the program window and selecting *Compute Volume Isosurface*. Then, the explorer will open so that an actual volume file can be chosen.

### Volume file format
The required volume format is a text file (_.txt_) consisting of _N<sup>3</sup> + 1_ lines, each line containing a number, so that the first line contains _N_ (the size of the voxelization, which is the same in all three dimensions), and the following lines contain the values of the scalar field sorted so that the value for voxel (_i_; _j_; _k_) is at line _iN<sup>2</sup> + jN + k + 1_ (lines numbered starting at zero, which is the line containing _N_).  
  
Sample volume files can be found in the [_Data_](Data/) folder.

### Isovalue
The attached marching cubes implementation will set the isovalue to the minimum value in the volume by default.  
This can be changed by passing the desired isovalue as input argument, for instance:

```
>> ./MeshViewer -2
```

### Rendering animation
When the *Animate* button is pressed, the program will increase the isovalue progressivelly, storing each output as separate images which can be found in the [_img_](MeshViewer_73156e6/img) folder. Afterwards, a video can be built using any external software. In case of *ffmpeg*:

```
>> ffmpeg -framerate 24 -i %d.png -pix_fmt yuv420p ../out.mp4
```
  
## Dependencies
The external libraries being used are already included in the repository, so no further integration step is needed:
- [OpenMesh](https://github.com/Lawrencemm/openmesh)
- [GLM](https://github.com/g-truc/glm)
