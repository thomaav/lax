#!/bin/bash

glslangValidator -V basic.vert -o basic.vert.spv
glslangValidator -V basic.frag -o basic.frag.spv
glslangValidator -V skybox.vert -o skybox.vert.spv
glslangValidator -V skybox.frag -o skybox.frag.spv
glslangValidator -V grid.vert -o grid.vert.spv
glslangValidator -V grid.frag -o grid.frag.spv
glslangValidator -V plane.vert -o plane.vert.spv
glslangValidator -V plane.frag -o plane.frag.spv
