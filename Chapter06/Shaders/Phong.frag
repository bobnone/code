// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Inputs from vertex shader
// Tex coord
in vec2 fragTexCoord;
// Normal (in world space)
in vec3 fragNormal;
// Position (in world space)
in vec3 fragWorldPos;
// This corresponds to the output color to the color buffer
out vec4 outColor;
// This is used for the texture sampling
uniform sampler2D uTexture;
// Create a struct for directional light
struct DirectionalLight
{
	// Direction of light
	vec3 mDirection;
	// Diffuse color
	vec3 mDiffuseColor;
	// Specular color
	vec3 mSpecColor;
};
// Create a struct for point light
struct PointLight
{
	// Position of light
	vec3 mWorldPos;
	// Diffuse color
	vec3 mDiffuseColor;
	// Specular color
	vec3 mSpecColor;
	// Radius of the light
	float mInnerRadius;
	float mOuterRadius;
};
// Uniforms for lighting
// Camera position (in world space)
uniform vec3 uCameraPos;
// Specular power for this surface
uniform float uSpecPower;
// Ambient light level
uniform vec3 uAmbientLight;
// Directional Light
uniform DirectionalLight uDirLight;
// Point Lights
uniform PointLight[4] uPointLights;
// Size of PointLight array
uniform int uNumPointLights;

void main()
{
	vec4 output = texture(uTexture, fragTexCoord);
	// Surface normal
	vec3 N = normalize(fragNormal);
	// Vector from surface to camera
	vec3 V = normalize(uCameraPos - fragWorldPos);
// Directional Light:
	// Vector from surface to light
	vec3 D_L = normalize(-uDirLight.mDirection);
	// Reflection of -L about N
	vec3 D_R = normalize(reflect(-D_L, N));
	// Compute phong reflection
	vec3 D_Phong = uAmbientLight;
	float D_NdotL = dot(N, D_L);
	if (D_NdotL > 0)
	{
		vec3 D_Diffuse = uDirLight.mDiffuseColor * D_NdotL;
		vec3 D_Specular = uDirLight.mSpecColor * pow(max(0.0, dot(D_R, V)), uSpecPower);
		D_Phong += D_Diffuse + D_Specular;
	}
	output *= vec4(D_Phong, 1.0f);
// Point Lights:
	for(int i = 0; i < uNumPointLights; i++)
	{
		// Vector from surface to light
		vec3 P_L = normalize(uPointLights[i].mWorldPos - fragWorldPos);
		// Reflection of -L about N
		vec3 P_R = normalize(reflect(-P_L, N));
		// Compute phong reflection
		vec3 P_Phong = vec3(1.0f, 1.0f, 1.0f);
		float P_NdotL = dot(N, P_L);
		if (P_NdotL > 0)
		{
			// Get the distance between the light and the world pos
			float dist = distance(uPointLights[i].mWorldPos, fragWorldPos);
			// Use smoothstep to compute value in range [0,1] between inner/outer radius
			float intensity = smoothstep(uPointLights[i].mInnerRadius, uPointLights[i].mOuterRadius, dist);
			vec3 P_Diffuse = mix(uPointLights[i].mDiffuseColor, vec3(0.0, 0.0, 0.0), intensity) * P_NdotL;
			vec3 P_Specular = mix(uPointLights[i].mSpecColor, vec3(0.0, 0.0, 0.0), intensity) * pow(max(0.0, dot(P_R, V)), uSpecPower);
			P_Phong += P_Diffuse + P_Specular;
			output *= vec4(P_Phong, 1.0f);
		}
	}
	// Final color is texture color times phong light (alpha = 1)
    outColor = output;
}