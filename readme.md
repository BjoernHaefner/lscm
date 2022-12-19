# lscm

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

This code uses [geogram](https://github.com/BrunoLevy/geogram) to compute least squares conformal maps based on this [paper](https://members.loria.fr/Bruno.Levy/papers/LSCM_SIGGRAPH_2002.pdf). 

## 1. Requirements and Setup

This code has been tested under Ubuntu 20.04.5 LTS (Focal Fossa) with an Intel(R) Xeon(R) CPU E5-2637 v3 @ 3.50GHz, 31Gb of RAM

### 1.1. Install geogram
To successfully install [geogram](https://github.com/BrunoLevy/geogram), I also had to run the following commands before installation:

```bash
$ sudo apt install cmake build-essential libx11-dev libxcb1-dev libxau-dev libxdmcp-dev
$ sudo apt install libxrandr-dev
$ sudo apt install xorg-dev
```

Follow the installation guide for [geogram](https://github.com/BrunoLevy/geogram), e.g. for Linux [here](https://github.com/BrunoLevy/geogram/wiki/compiling_Linux#quick-compilation-guide). I also had to run `$ sudo make install`

### 1.2. Setup of lscm
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make -j 8
```
### 1.5 Download example data set
Run
```bash
$ bash ../data/download.sh
```
to download the cow data set to the data folder:
1) `cow.obj` of <1Mb

## 2. Usage

`.ply` and `.obj` file-reading is supported

```bash
$ ./lscm in_mesh_file out_mesh_file out_tex_map out_normal_map
```

Example with the provided data set:

```bash
$ ./lscm ../data/cow.obj ../output/cow_out.obj ../output/cow_tex.png ../output/cow_normals.png
```

Probably you don't see much when opening the `.obj`-file, but you can edit the output `.mtl`-file from

```
newmtl Material_0
map_Kd cow_tex.png
```

to

```
newmtl Material_0
map_Kd cow_normals.png
```

Then you should see a the cow-mesh with the normals as its texture. Note the visible seems, these have to be fixed manually or with a different tool, e.g. you can inpaint the texture atlas.
