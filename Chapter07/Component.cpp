// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Component.h"
#include "Actor.h"

Component::Component(Actor* owner, int updateOrder): pOwner(owner), mUpdateOrder(updateOrder)
{
	// Add to actor's vector of components
	pOwner->AddComponent(this);
}

Component::~Component()
{
	pOwner->RemoveComponent(this);
}

void Component::Update(float deltaTime)
{
}