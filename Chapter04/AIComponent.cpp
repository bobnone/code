// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "AIComponent.h"
#include "Actor.h"
#include "AIState.h"
#include <SDL/SDL_log.h>

AIComponent::AIComponent(class Actor* owner): Component(owner), pCurrentState(nullptr)
{
}

void AIComponent::Update(float deltaTime)
{
	if (pCurrentState)
	{
		pCurrentState->Update(deltaTime);
	}
}

void AIComponent::ChangeState(const std::string& name)
{
	if (pCurrentState)
	{
		// Check if we even need to change states
		if (pCurrentState->GetName() == name)
		{
			return;
		}
		// First exit the current state
		pCurrentState->OnExit();
	}
	// Try to find the new state from the map
	auto iter = mStateMap.find(name);
	if (iter != mStateMap.end())
	{
		pCurrentState = iter->second;
		// We're entering the new state
		pCurrentState->OnEnter();
	}
	else
	{
		SDL_Log("Could not find AIState %s in state map", name.c_str());
		pCurrentState = nullptr;
	}
}

void AIComponent::RegisterState(AIState* state)
{
	mStateMap.emplace(state->GetName(), state);
}