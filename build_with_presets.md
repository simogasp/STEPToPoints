# Building with Presets and VCPKG

How to build the project using CMake presets and VCPKG for dependency management.

1. **Install VCPKG**: If you haven't already, clone the VCPKG repository and bootstrap it.

    ```bash
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ```

2. set the environment variable for vcpkg. Assuming you are still in the vcpkg directory:

    ```bash
    export VCPKG_ROOT=`pwd`
    ```

3. go back to the project directory and configure the project using CMake presets:

   ```bash
    cd ..
    cmake --preset release-preset
    cmake --preset debug-preset
    ```

    This will create the build directories `build/<preset-name>`.

4. **Build the project**: You can now build the project using the configured presets.

    ```bash
    cmake --build --preset release-preset
    cmake --build --preset debug-preset
    ```

Note that the dependencies will be installed in a common directory `vcpkg_installed` in the project root (check `VCPKG_INSTALLED_DIR` in the presets) so that it will only build the dependencies in release/debug once.
