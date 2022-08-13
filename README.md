# CSD2150-MT
#### CSD2150 MTEngine for Windows.</br>

This is a mesh viewer demo showcasing my Vulkan graphics engine: MTEngine.</br>
Created during Year 2 Trimester 2 of my studies at DigiPen Institute of Technology Singapore.</br>
Requires the user to have a graphics driver supporting Vulkan API 1.2 or above.</br>

To view the demo without compiling, download the MTDemo from the releases page.</br>

## To compile a version for yourself:</br>
1. Clone the repository.</br>
2. Run getDependencies.bat from the Tools directory (This might take awhile) OR get prebuilt dependencies from the release section of this repository (structure: solutionDir\dependencies).</br>
3. Open the solution in Visual Studio and compile for your desired architecture.</br>

## getDependencies requires:</br>
- powershell (I'm assuming anyone building this project has it since it's targeted for Windows)</br>
- git</br>
- Visual Studio 16 2019 (you can change this in buildAssimp.bat to use future versions under SET GENERATOR = your version here)</br>

## Project requires:</br>
- A version of Visual Studio supporting C++20</br>
- Vulkan SDK 1.2.198.1 or above installed

#### important folders for the project
- Assets folder (for the meshes, textures, shaders used)</br>
- prop-pages folder (QOL so no need to manually copy DLLs and such)</br>
- dependencies folder (if the script fails, I am providing the dependencies used for my compilation as part of the release)</br>
