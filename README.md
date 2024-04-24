<div align="center">
  <p>
      <img src="logo/trinex.svg" width="500" height="500" />
  </p>
</div>

# Trinex Engine
**Trinex Engine** is a simple game engine targeting platforms such as Windows, Linux and Android. This project was developed with the aim of studying the work of modern game engines and studying the graphics api such as OpenGL and Vulkan.

## Requirements
To use Trinex Engine, you need to have the following software installed:
- CMake
- G++
- Make

## Getting Started
Follow these steps to get started with Trinex Engine:
1. **Install CMake, G++, and Make**: Ensure you have CMake, G++, and Make installed on your system. These tools are required to build the project.
2. **Clone the repository**: Clone the Trinex Engine repository to your local machine using Git:
```bash
git clone https://github.com/Programier/Trinex-Engine.git
```
3. **Create a build directory**: Navigate to the root directory of the cloned repository and create a `build` directory. Open a terminal in the `build` directory.
``` bash
mkdir build
cd build
```
4. **Generate build files**: Run CMake to generate the necessary build files. Execute the following command in the `build` directory:
```bash
cmake ..
```
5. **Build the project**: Once CMake has generated the build files, use Make to build the project. You can specify the number of threads to use for the build process by replacing `<num threads>` with the desired number.
```bash
make install -j<num threads>
```
6. **Find the built engine**: After the build process completes, the built engine will be located in the `build/Game/` directory.
Now you're ready to explore the built engine and start developing your own projects with Trinex Engine!

## Current Status
- Not ready for any use

## Support
If you have any questions, issues, or suggestions regarding the development of Trinex Engine, please create a new Issue in this repository or contact us directly.

## License

Trinex Engine is distributed under the [MIT License](LICENSE).

