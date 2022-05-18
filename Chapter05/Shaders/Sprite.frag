// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Tex coord input from vertex shader
in vec2 fragTexCoord;
// Vertex color
in vec3 vertexColor;
// This corresponds to the output color to the color buffer
out vec4 outColor;
// This is used for the texture sampling
uniform sampler2D uTexture;

void main()
{
	// Sample color from texture
	float x = (texture(uTexture, fragTexCoord).x * vertexColor.x)/2;
	float y = (texture(uTexture, fragTexCoord).y * vertexColor.y)/2;
	float z = (texture(uTexture, fragTexCoord).z * vertexColor.z)/2;
	outColor = vec4(x, y, z, texture(uTexture, fragTexCoord).w);
}