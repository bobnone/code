// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Renderer.h"
#include "Texture.h"
#include "Mesh.h"
#include <algorithm>
#include "Shader.h"
#include "VertexArray.h"
#include "SpriteComponent.h"
#include "MeshComponent.h"
#include <GL/glew.h>

Renderer::Renderer(Game* game): pGame(game), pCurrentShader(nullptr)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(float screenWidth, float screenHeight)
{
	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;
	// Set OpenGL attributes
	// Use the core OpenGL profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	// Specify version 3.3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	// Request a color buffer with 8-bits per RGBA channel
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	// Enable double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// Force OpenGL to use hardware acceleration
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	pWindow = SDL_CreateWindow("Game Programming in C++ (Chapter 6)", 100, 100, static_cast<int>(mScreenWidth), static_cast<int>(mScreenHeight), SDL_WINDOW_OPENGL);
	if (!pWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}
	// Create an OpenGL context
	mContext = SDL_GL_CreateContext(pWindow);
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		SDL_Log("Failed to initialize GLEW.");
		return false;
	}
	// On some platforms, GLEW will emit a benign error code, so clear it
	glGetError();
	// Make sure we can create/compile shaders
	if (!LoadShaders())
	{
		SDL_Log("Failed to load shaders.");
		return false;
	}
	// Create quad for drawing sprites
	CreateSpriteVerts();
	return true;
}

void Renderer::Shutdown()
{
	delete pSpriteVerts;
	pSpriteVerts = nullptr;
	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(pWindow);
}

void Renderer::UnloadData()
{
	// Destroy textures
	for (auto i : mTextures)
	{
		i.second->Unload();
		delete i.second;
	}
	mTextures.clear();
	// Destroy meshes
	for (auto i : mMeshes)
	{
		i.second->Unload();
		delete i.second;
	}
	mMeshes.clear();
	// Destroy shaders
	for (auto i : mShaders)
	{
		i.second->Unload();
		delete i.second;
	}
	mShaders.clear();
}

void Renderer::Draw()
{
	// Set the clear color to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Draw mesh components
	// Enable depth buffering/disable alpha blend
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	// Set the mesh shader active
	SetCurrentShader("Mesh");
	// Update view-projection matrix
	pCurrentShader->SetMatrixUniform("uViewProj", mView * mProjection);
	// Update lighting uniforms
	SetLightUniforms(pCurrentShader);
	// Loop through each mesh-shader
	for (auto mclist: mMeshComps)
	{
		SetCurrentShader(mclist.first);
		// Loop through each mesh component using the current shader
		for (MeshComponent* mc: mclist.second)
		{
			mc->Draw(pCurrentShader);
		}
	}
	// Draw all sprite components
	// Disable depth buffering
	glDisable(GL_DEPTH_TEST);
	// Enable alpha blending on the color buffer
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	// Set shader/VAO as active
	SetCurrentShader("Sprite");
	pSpriteVerts->SetActive();
	for (auto sprite: mSprites)
	{
		sprite->Draw(pCurrentShader);
	}
	// Swap the buffers
	SDL_GL_SwapWindow(pWindow);
}

void Renderer::AddSprite(SpriteComponent* sprite)
{
	// Find the insertion point in the sorted vector
	// (The first element with a higher draw order than me)
	int myDrawOrder = sprite->GetDrawOrder();
	auto iter = mSprites.begin();
	for (; iter != mSprites.end(); ++iter)
	{
		if (myDrawOrder < (*iter)->GetDrawOrder())
		{
			break;
		}
	}
	// Inserts element before position of iterator
	mSprites.insert(iter, sprite);
}

void Renderer::RemoveSprite(SpriteComponent* sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	mSprites.erase(iter);
}

void Renderer::AddMeshComp(MeshComponent* mesh)
{
	// Is the shader in the map?
	auto iter = mMeshComps.find(mesh->GetShaderName());
	if (iter != mMeshComps.end()) // Yes
	{
		// Add the mesh component to the list (vector)
		iter->second.emplace_back(mesh);
	}
	else // No
	{
		// Create a new vector containing the mesh component
		std::vector<MeshComponent*> mcvec = {mesh};
		// Add the pair<shader name, mesh component vector> to the map
		mMeshComps.emplace(mesh->GetShaderName(), mcvec);
	}
}

void Renderer::RemoveMeshComp(MeshComponent* mesh)
{
	// Retrive the mesh component vector for the specific shader
	std::vector<MeshComponent*> mcvec = mMeshComps.at(mesh->GetShaderName());
	// Loop through the vector until you find what you are looking for
	auto iter = std::find(mcvec.begin(), mcvec.end(), mesh);
	// remove the mesh componment from the list (vector)
	mcvec.erase(iter);
}

Texture* Renderer::GetTexture(const std::string& fileName)
{
	Texture* tex = nullptr;
	auto iter = mTextures.find(fileName);
	if (iter != mTextures.end())
	{
		tex = iter->second;
	}
	else
	{
		tex = new Texture();
		if (tex->Load(fileName))
		{
			mTextures.emplace(fileName, tex);
		}
		else
		{
			delete tex;
			tex = nullptr;
		}
	}
	return tex;
}

Mesh* Renderer::GetMesh(const std::string & fileName)
{
	Mesh* m = nullptr;
	auto iter = mMeshes.find(fileName);
	if (iter != mMeshes.end())
	{
		m = iter->second;
	}
	else
	{
		m = new Mesh();
		if (m->Load(fileName, this))
		{
			mMeshes.emplace(fileName, m);
		}
		else
		{
			delete m;
			m = nullptr;
		}
	}
	return m;
}

bool Renderer::SetCurrentShader(const std::string& fileName)
{
	auto iter = mShaders.find(fileName);
	if (iter != mShaders.end())
	{
		Shader* sh = iter->second;
		pCurrentShader = sh;
		sh->SetActive();
		return true;
	}
	return false;
}

bool Renderer::LoadShader(const std::string& name, const std::string& vertFile, const std::string& fragFile)
{
	// Create a new shader
	Shader* sh = new Shader();
	if (sh->Load(vertFile, fragFile))
	{
		mShaders.emplace(name, sh);
		pCurrentShader = sh;
		sh->SetActive();
		return true;
	}
	// Loading failed, so delete the Shader
	delete sh;
	sh = nullptr;
	return false;
}

bool Renderer::LoadShaders()
{
	// Create sprite shader
	if(!LoadShader("Sprite", "Shaders/Sprite.vert", "Shaders/Sprite.frag"))
	{
		return false;
	}
	// Set the view-projection matrix
	Matrix4 viewProj = Matrix4::CreateSimpleViewProj(mScreenWidth, mScreenHeight);
	pCurrentShader->SetMatrixUniform("uViewProj", viewProj);
	// Create basic mesh shader
	if (!LoadShader("Mesh", "Shaders/Phong.vert", "Shaders/Phong.frag"))
	{
		return false;
	}
	// Set the view-projection matrix
	mView = Matrix4::CreateLookAt(Vector3::Zero, Vector3::UnitX, Vector3::UnitZ);
	mProjection = Matrix4::CreatePerspectiveFOV(Math::ToRadians(70.0f), mScreenWidth, mScreenHeight, 25.0f, 10000.0f);
	pCurrentShader->SetMatrixUniform("uViewProj", mView * mProjection);
	return true;
}

void Renderer::CreateSpriteVerts()
{
	float vertices[] = {
		-0.5f, 0.5f, 0.f, 0.f, 0.f, 0.0f, 0.f, 0.f, // top left
		0.5f, 0.5f, 0.f, 0.f, 0.f, 0.0f, 1.f, 0.f, // top right
		0.5f,-0.5f, 0.f, 0.f, 0.f, 0.0f, 1.f, 1.f, // bottom right
		-0.5f,-0.5f, 0.f, 0.f, 0.f, 0.0f, 0.f, 1.f  // bottom left
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};
	pSpriteVerts = new VertexArray(vertices, 4, indices, 6);
}

void Renderer::SetLightUniforms(Shader* shader)
{
	// Camera position is from inverted view
	Matrix4 invView = mView;
	invView.Invert();
	shader->SetVectorUniform("uCameraPos", invView.GetTranslation());
	// Ambient light
	shader->SetVectorUniform("uAmbientLight", mAmbientLight);
	// Directional light
	shader->SetVectorUniform("uDirLight.mDirection", mDirLight.mDirection);
	shader->SetVectorUniform("uDirLight.mDiffuseColor", mDirLight.mDiffuseColor);
	shader->SetVectorUniform("uDirLight.mSpecColor", mDirLight.mSpecColor);
	// Point lights
	for (int i = 0; i < mPointLights.size(); i++)
	{
		std::string intstring = std::to_string(i);
		shader->SetVectorUniform(((std::string)"uPointLights[" + intstring + "].mWorldPos").c_str(), mPointLights[i].mWorldPos);
		shader->SetVectorUniform(((std::string)"uPointLights[" + intstring + "].mDiffuseColor").c_str(), mPointLights[i].mDiffuseColor);
		shader->SetVectorUniform(((std::string)"uPointLights[" + intstring + "].mSpecColor").c_str(), mPointLights[i].mSpecColor);
		shader->SetFloatUniform(((std::string)"uPointLights[" + intstring + "].mInnerRadius").c_str(), mPointLights[i].mInnerRadius);
		shader->SetFloatUniform(((std::string)"uPointLights[" + intstring + "].mOuterRadius").c_str(), mPointLights[i].mOuterRadius);
	}
	// Pass the size of the array to the shader
	shader->SetIntUniform("uNumPointLights", mPointLights.size());
}