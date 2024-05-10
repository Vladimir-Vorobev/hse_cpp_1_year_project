GPT setup for Windows:
1. Install CMake and MSBuild
2. Install a release (not debug) version of libtorch to include/cpp_gpt
3. If you installed GPU version of libtorch, install the corresponding CUDA version on your computer and cuDNN library
4. Add to PATH C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\<VERSION>\bin\Hostx86\x64, C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin, C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\<VERSION>\bin, C:\Program Files\NVIDIA\CUDNN\<VERSION>\bin
5. In case of "CMake error: No CUDA toolset found", copy everything from: C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\<VERSION>\extras\visual_studio_integration\MSBuildExtensions to: C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Microsoft\VC\v160\BuildCustomizations
6. In case of "CMake error: Failed to find nvToolsExt", use this: https://discuss.pytorch.org/t/failed-to-find-nvtoolsext/179635/4
