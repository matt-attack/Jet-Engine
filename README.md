
 Jet Engine
=============

An easy to use library to get you started with 3D graphics based in DirectX11, with OpenGl support to come.

Includes:
- Model loading and rendering (IQM, Obj and custom)
- Heightmap terrain rendering
- Directional cascading shadow maps
- Foliage rendering with impostor generation
- Basic geometry shader based particle systems
- Material system
- Live asset reloading
- Sound using OpenAL
- Math libraries
- Xbox controller and Joystick support
- Low level wrappers over graphics library concepts (vertex buffers, textures)
- OpenVR support and utilities


#Building


##Dependencies

Download the following dependencies into the same directory as this repo:

* [https://github.com/ValveSoftware/openvr.git](https://github.com/ValveSoftware/openvr.git)
* [https://github.com/Microsoft/DirectXTex.git](https://github.com/Microsoft/DirectXTex.git)
* [https://github.com/kcat/openal-soft.git](https://github.com/kcat/openal-soft.git)
* [https://github.com/matt-attack/netlibrary.git](https://github.com/matt-attack/netlibrary.git)

## Building

First download and build all of the dependencies listed above.

Then open the `BasicExample.sln` file in Visual Studio 2017 (lower versions may also be supported) and hit the play (build and run) button.

