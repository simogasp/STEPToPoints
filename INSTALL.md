# Building with vcpkg

Assume that we are creating a directory `step2points` in which we will clone the repository and build the project.

```bash
mkdir step2points
cd step2points

# let's clone vcpkg first
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
# this set where vcpkg is located
export VCPKG_ROOT=$(pwd)

# go back to the step2points dir
cd ..
# clone this repository
git clone --branch dev/modernize https://github.com/simogasp/STEPToPoints.git
cd STEPToPoints
# create a build directory
mkdir build
cd build
# configure the project and automatically install the dependencies via vcpkg
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..

# this will take a while as it will download and build all the necessary dependencies
# now we can build the project
cmake --build . --config Release
```
