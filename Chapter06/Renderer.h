// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL/SDL.h>
#include "Math.h"

struct DirectionalLight
{
	// Direction of light
	Vector3 mDirection;
	// Diffuse color
	Vector3 mDiffuseColor;
	// Specular color
	Vector3 mSpecColor;
};

struct PointLight
{
	// Position of light
	Vector3 mWorldPos;
	// Diffuse color
	Vector3 mDiffuseColor;
	// Specular color
	Vector3 mSpecColor;
	// Radius of the light
	float mInnerRadius;
	float mOuterRadius;
};

class Renderer
{
public:
	Renderer(class Game* game);
	~Renderer();
	bool Initialize(float screenWidth, float screenHeight);
	void Shutdown();
	void UnloadData();
	void Draw();
	void AddSprite(class SpriteComponent* sprite);
	void RemoveSprite(class SpriteComponent* sprite);
	void AddMeshComp(class MeshComponent* mesh);
	void RemoveMeshComp(class MeshComponent* mesh);
	class Texture* GetTexture(const std::string& fileName);
	class Mesh* GetMesh(const std::string& fileName);
	bool SetCurrentShader(const std::string& fileName);
	void SetViewMatrix(const Matrix4& view) { mView = view; }
	void SetAmbientLight(const Vector3& ambient) { mAmbientLight = ambient; }
	DirectionalLight& GetDirectionalLight() { return mDirLight; }
	std::vector<PointLight>* GetPointLights() { return &mPointLights; }
	float GetScreenWidth() const { return mScreenWidth; }
	float GetScreenHeight() const { return mScreenHeight; }
private:
	// Creates a new shader and sets it as the current shader
	bool LoadShader(const std::string& name, const std::string& vertFile, const std::string& fragFile);
	// Loads all shaders
	bool LoadShaders();
	void CreateSpriteVerts();
	void SetLightUniforms(class Shader* shader);
	// Map of textures loaded
	std::unordered_map<std::string, class Texture*> mTextures;
	// Map of meshes loaded
	std::unordered_map<std::string, class Mesh*> mMeshes;
	// Map of shaders loaded
	std::unordered_map<std::string, class Shader*> mShaders;
	// Map linking mesh components to specific shaders
	std::unordered_map<std::string, std::vector<class MeshComponent*>> mMeshComps;
	// All the sprite components drawn
	std::vector<class SpriteComponent*> mSprites;
	// Game
	class Game* pGame;
	// Current shader
	class Shader* pCurrentShader;
	// Sprite vertex array
	class VertexArray* pSpriteVerts;
	// View/projection for 3D shaders
	Matrix4 mView;
	Matrix4 mProjection;
	// Width/height of screen
	float mScreenWidth;
	float mScreenHeight;
	// Lighting data
	Vector3 mAmbientLight;
	DirectionalLight mDirLight;
	std::vector<PointLight> mPointLights = {};
	// Window
	SDL_Window* pWindow;
	// OpenGL context
	SDL_GLContext mContext;
};