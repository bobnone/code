// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Game.h"
#include <algorithm>
#include "Renderer.h"
#include "Actor.h"
#include "SpriteComponent.h"
#include "MeshComponent.h"
#include "CameraActor.h"
#include "PlaneActor.h"

Game::Game(): pRenderer(nullptr), mIsRunning(true), mUpdatingActors(false)
{
}

bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	// Create the renderer
	pRenderer = new Renderer(this);
	if (!pRenderer->Initialize(1024.0f, 768.0f))
	{
		SDL_Log("Failed to initialize renderer");
		delete pRenderer;
		pRenderer = nullptr;
		return false;
	}
	LoadData();
	mTicksCount = SDL_GetTicks();
	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				mIsRunning = false;
				break;
		}
	}
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
	for (auto actor: mActors)
	{
		actor->ProcessInput(state);
	}
}

void Game::UpdateGame()
{
	// Compute delta time
	// Wait until 16ms has elapsed since last frame
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	mTicksCount = SDL_GetTicks();
// Update all actors:
	mUpdatingActors = true;
	for (auto actor: mActors)
	{
		actor->Update(deltaTime);
	}
	mUpdatingActors = false;
	// Move any pending actors to mActors
	for (auto pending: mPendingActors)
	{
		pending->ComputeWorldTransform();
		mActors.emplace_back(pending);
	}
	mPendingActors.clear();
	// Add any dead actors to a temp vector
	std::vector<Actor*> deadActors;
	for (auto actor: mActors)
	{
		if (actor->GetState() == Actor::EDead)
		{
			deadActors.emplace_back(actor);
		}
	}
	// Delete dead actors (which removes them from mActors)
	for (auto actor: deadActors)
	{
		delete actor;
	}
}

void Game::GenerateOutput()
{
	pRenderer->Draw();
}

void Game::LoadData()
{
	// Create actors
	Actor* a = new Actor(this);
	a->SetPosition(Vector3(200.0f, 75.0f, 0.0f));
	a->SetScale(100.0f);
	Quaternion q(Vector3::UnitY, -Math::PiOver2);
	q = Quaternion::Concatenate(q, Quaternion(Vector3::UnitZ, Math::Pi + Math::Pi / 4.0f));
	a->SetRotation(q);
	MeshComponent* mc = new MeshComponent(a, pRenderer->GetMesh("Assets/Cube.gpmesh"));
	a = new Actor(this);
	a->SetPosition(Vector3(200.0f, -75.0f, 0.0f));
	a->SetScale(3.0f);
	mc = new MeshComponent(a, pRenderer->GetMesh("Assets/Sphere.gpmesh"));
	// Setup floor
	const float start = -1250.0f;
	const float size = 250.0f;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			a = new PlaneActor(this);
			a->SetPosition(Vector3(start + i * size, start + j * size, -100.0f));
		}
	}
	// Left/right walls
	q = Quaternion(Vector3::UnitX, Math::PiOver2);
	for (int i = 0; i < 10; i++)
	{
		a = new PlaneActor(this);
		a->SetPosition(Vector3(start + i * size, start - size, 0.0f));
		a->SetRotation(q);
		a = new PlaneActor(this);
		a->SetPosition(Vector3(start + i * size, -start + size, 0.0f));
		a->SetRotation(q);
	}
	q = Quaternion::Concatenate(q, Quaternion(Vector3::UnitZ, Math::PiOver2));
	// Forward/back walls
	for (int i = 0; i < 10; i++)
	{
		a = new PlaneActor(this);
		a->SetPosition(Vector3(start - size, start + i * size, 0.0f));
		a->SetRotation(q);
		a = new PlaneActor(this);
		a->SetPosition(Vector3(-start + size, start + i * size, 0.0f));
		a->SetRotation(q);
	}
// Setup lights:
	// Directional Light
	pRenderer->SetAmbientLight(Vector3(0.2f, 0.2f, 0.2f));
	DirectionalLight& dir = pRenderer->GetDirectionalLight();
	dir.mDirection = Vector3(0.0f, -0.707f, -0.707f);
	dir.mDiffuseColor = Vector3(0.78f, 0.88f, 1.0f);
	dir.mSpecColor = Vector3(0.8f, 0.8f, 0.8f);
	// Point Lights
	std::vector<PointLight>* plvec = pRenderer->GetPointLights();
	// Point Light1
	PointLight pl1;
	pl1.mWorldPos = Vector3(0.0f, 0.0f, 0.0f);
	pl1.mDiffuseColor = Vector3(0.0f, 1.0f, 0.0f);
	pl1.mSpecColor = Vector3(0.0f, 1.0f, 0.0f);
	pl1.mInnerRadius = 0.0f;
	pl1.mOuterRadius = 500.0f;
	plvec->emplace_back(pl1);
	// Point Light2
	PointLight pl2;
	pl2.mWorldPos = Vector3(500.0f, 500.0f, 200.0f);
	pl2.mDiffuseColor = Vector3(0.0f, 0.0f, 1.0f);
	pl2.mSpecColor = Vector3(0.0f, 0.0f, 1.0f);
	pl2.mInnerRadius = 0.0f;
	pl2.mOuterRadius = 500.0f;
	plvec->emplace_back(pl2);
	PointLight pl3;
	pl3.mWorldPos = Vector3(0.0f, -200.0f, 0.0f);
	pl3.mDiffuseColor = Vector3(1.0f, 0.0f, 0.0f);
	pl3.mSpecColor = Vector3(1.0f, 0.0f, 0.0f);
	pl3.mInnerRadius = 0.0f;
	pl3.mOuterRadius = 500.0f;
	plvec->emplace_back(pl3);
	PointLight pl4;
	pl4.mWorldPos = Vector3(-1000.0f, 1000.0f, 0.0f);
	pl4.mDiffuseColor = Vector3(1.0f, 1.0f, 0.0f);
	pl4.mSpecColor = Vector3(1.0f, 1.0f, 0.0f);
	pl4.mInnerRadius = 0.0f;
	pl4.mOuterRadius = 500.0f;
	plvec->emplace_back(pl4);
	// Camera actor
	pCameraActor = new CameraActor(this);
	// UI elements
	a = new Actor(this);
	a->SetPosition(Vector3(-350.0f, -350.0f, 0.0f));
	SpriteComponent* sc = new SpriteComponent(a);
	sc->SetTexture(pRenderer->GetTexture("Assets/HealthBar.png"));
	a = new Actor(this);
	a->SetPosition(Vector3(375.0f, -275.0f, 0.0f));
	a->SetScale(0.75f);
	sc = new SpriteComponent(a);
	sc->SetTexture(pRenderer->GetTexture("Assets/Radar.png"));
}

void Game::UnloadData()
{
	// Delete actors
	// Because ~Actor calls RemoveActor, have to use a different style loop
	while (!mActors.empty())
	{
		delete mActors.back();
	}
	if (pRenderer)
	{
		pRenderer->UnloadData();
	}
}

void Game::Shutdown()
{
	UnloadData();
	if (pRenderer)
	{
		pRenderer->Shutdown();
	}
	SDL_Quit();
}

void Game::AddActor(Actor* actor)
{
	// If we're updating actors, need to add to pending
	if (mUpdatingActors)
	{
		mPendingActors.emplace_back(actor);
	}
	else
	{
		mActors.emplace_back(actor);
	}
}

void Game::RemoveActor(Actor* actor)
{
	// Is it in pending actors?
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end())
	{
		// Swap to end of vector and pop off (avoid erase copies)
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}
	// Is it in actors?
	iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end())
	{
		// Swap to end of vector and pop off (avoid erase copies)
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}
}