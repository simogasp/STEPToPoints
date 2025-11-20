# STEPToPoints

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/86fe3d55b6934d8fa95d6e03bed9f103)](https://app.codacy.com/gh/simogasp/STEPToPoints?utm_source=github.com&utm_medium=referral&utm_content=simogasp/STEPToPoints&utm_campaign=Badge_Grade)
[![CI-Build](https://github.com/simogasp/STEPToPoints/actions/workflows/build.yml/badge.svg)](https://github.com/simogasp/STEPToPoints/actions/workflows/build.yml)

## Description

The program STEPToPoints is a command line utility to generate point clouds out of solids contained in STEP files.
The supported output file format is xyz (vertex positions and normal vectors).
A popular viewer for the supported file format is MeshLab (<https://www.meshlab.net>).
STEPToPoints is based on OpenCASCADE (<https://www.opencascade.com>).
The program uses cxxops (<https://github.com/jarro2783/cxxopts>) for parsing the command line.

## Requirements

* CMake installation (<https://cmake.org>)
* OpenCASCADE installation (<https://old.opencascade.com/content/latest-release>, download needs registration)

For OpenCASCADE if it is not found it will be fetched and built via CMake FetchContent.
For that, you need the following additional libs installed:

* tcl-dev
* tk-dev

On Linux you can install these packages via the package manager, e.g. on ubuntu:

```bash
sudo apt-get install tcl-dev tk-dev
```

## Usage

Listing the contents (solids) of a STEP file:

```bash
STEPToPoints -c -i <step file>
```

Generating point clouds for selected solids of the file:

```bash
STEPToPoints -i <step file> -o <output file> -g <sampling distance> -s <solid1>,<solid2>,<...>`
```

Following the help text from the command line:

```bash
STEPToPoints.exe
STEP to point cloud conversion by regular sampling
Usage:
  STEPToPoints [OPTION...]

  -i, --in arg        Input file
  -o, --out arg       Output file (.obj, .ply, .xyz)
  -c, --content       List content (solids)
  -s, --select arg    Select solids by name or index (comma seperated list,
                      index starts with 1)
  -g, --sampling arg  Sampling distance
  -b, --binary        Write binary file (only for .ply files)
  -h, --help          Print usage
```

> [!NOTE]
> As output file, xyz, obj, and ply files are supported.

## Examples

Examples are from the `examples` directory.

### Example Basic Shapes

| Solids |
| :--- |
| ![Image Solids-Basic-Shapes](examples/basic_shapes/solids.png) |

| Point cloud with normal vectors |
| :--- |
| `STEPToPoints.exe -i basic_shapes.stp -o out.xyz -g 0.5` |
| ![Image Point-Cloud-Basic-Shapes](examples/basic_shapes/point_cloud.png) |

## Remarks

This code has been tested with an OpenCASCADE 7.5.0 prebuilt binary (`opencascade-7.5.0-vc14-64.exe`) on Windows, as well as OpenCASCADE system packages on openSUSE Linux. With changes in the configuration section in the `CMakeLists.txt` file the build should also work with other OpenCASCADE versions.
