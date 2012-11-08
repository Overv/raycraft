# raycraft

There have been many Minecraft clones already, but this one is a little different. The goal of this project is to be a proof of concept implementation of a voxel building game that uses ray tracing instead of rasterization for rendering.

<img src="http://i.imgur.com/CE0Jd.png" alt="" />

The sample program included shows off some of the features of the ray tracer.

## Requirements

- Linux or Windows
- Hardware support for `OpenGL 3.3`

## Implementation details

Although OpenCL is around these days, I found it more convenient to implement the ray tracer in a fragment shader and have it run over every pixel on the screen by drawing a full screen quad. That is because GLSL is a language much more suited for graphics programming in general.

The world is currently uploaded as a 3D texture with a single 32-bit channel for every block. Although 8 bits is more than enough to contain the data for any block, it makes dealing with memory alignment easier right now. It is unlikely that this will change any time soon.

The ray tracer finds the first coordinate inside the world as reached when following the direction of the ray and then continues stepping through the world until a non-empty block is found. It steps by doing a ray/cube intersection on the block its currently in and continues from the intersection point. This guarantees a maximum amount of iterations per pixel less than `sizeX+sizeY+sizeZ` where these are the dimensions of the world.

The largest difference between this rendering approach and Minecraft's rasterization approach is that the concept of building chunks doesn't exist. Modifying the world is as simple as a single `glTexSubImage3D` call. It is of course still preferable to not have parts of the world in memory that are too far away besides the implementation defined 3D texture dimensions limits.

## Performance

The performance is *reasonable*, support for larger worlds will require the ray tracer to be optimized better and there are rendering artefacts that need to be fixed.

## Todo

* Fixing rendering artefacts, especially on larger worlds
* Optimization

## License

MIT

The source code comes bundled with [gl3w](https://github.com/skaslev/gl3w) and [GLM](http://glm.g-truc.net/), which are licensed as public domain and MIT respectively.