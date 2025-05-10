#!/bin/bash

glslangValidator -V basic.vert -o basic.vert.spv
glslangValidator -V basic.frag -o basic.frag.spv
glslangValidator -V skybox.vert -o skybox.vert.spv
glslangValidator -V skybox.frag -o skybox.frag.spv
