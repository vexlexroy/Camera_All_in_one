# awCameraLocalization
This project is master thesis task that localizes object in 3D space using multiple 3D cameras.

Prerequisites:
windows 11 (maybe can work on older versions as well)

msvc compiler - for compiling project (available in system path)

vcpkg - for downloading all needed libraries (available in system path)

cmake - for creating compilation files

Build steps:
1. git clone <this repository>
2. vcpkg install (installes all necessery libraries in current folder, if done succesfully folder vcpkg_installed should be visible)
3. cmake --preset=release  (create build files)
4. cd build/release
5. msbuild diplomski.sln /p:Configuration=Release /p:Platform=x64 (build project) (in cmd for VS)
6. .\Release\diplomski.exe (run project, if not working and missing some .dll files copy/paste all dlls from vcpkg_installed/x64-windows/bin)



