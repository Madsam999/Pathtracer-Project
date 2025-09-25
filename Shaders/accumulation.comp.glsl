#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D accumulationBuffer;
layout(rgba32f, binding = 1) uniform image2D outImage;

uniform int frameCount;

void main() {

}
