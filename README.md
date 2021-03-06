# fractals
Fractal implementations

This project is divided into two sub projects:
- console-fractals
- vulkan-fractals

## Build

The console-fractals project is platform independant, but the vulkan project depends windows, currently.
Just run the top-level `CMakeLists.txt` file.

Tip: `cmake --build . ` builds the cmake project without explicitly using the build system.

Additional packages you have to install:

### Fedora

```shell
sudo dnf groupinstall "X Software Development"
sudo dnf install libXinerama-devel
```

### Ubuntu

```shell
sudo apt-get install xorg-dev libglu1-mesa-dev libxinerama-dev
```


## console-fractals

Contained in the `console-fractals` directory.

Prints a fractal, either Mandelbrot or Julia to the console.
The image is pretty large, therefore it's recommended to set
a small font size, like 4 or 5 points.

Hit enter to see the fractal with in incremented iteration count.

To tune the fractal you see, you have to go into the source code, namely `main.cpp`.

## vulkan-fractals

Contained in the `vulkan-fractals` directory.

Real-time and interactive rendering of the Mandelbrot fractal.

## Devel

### Vulkan crashes - first aid

- Enable more validation layers (`VK_LAYER_LUNARG_api_dump`)
- Recreate objects that depends on other objects, which has been recreated (e.g. Swapchain recreation)
- Check alignment (reasonable data in Renderdoc mesh output)

