#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
Actor::Actor(int imageID, int startX, int startY, int startDirection, StudentWorld* world) : GraphObject(imageID, startX, startY)
{
	m_studentWorld = world;
	isAlive = true;
}

bool Actor::operator<(const Actor& other) const
{
	// Interactable (fireballs, goodies) Actors should have higher priority since their actions can change more behavior
	// i.e. The action of a fireball hitting an enemy should be performed first since beyond so the enemy can no longer 
	// attack Peach if interacting with her 
	if (isInteractable() && !other.isInteractable())
		return true;
	else if (other.isInteractable() && !isInteractable())
		return false;

	// All non-interactable Actors (meaning those that are NOT goodies, fire flowers, or peach) are intended to stay in a fixed vertical 
	// position, so comparison can be done in terms of their vertical position (and there is no need to worry about having to rearrange 
	// since vertical position will stay the same)
	if (getY() < other.getY())
		return true;
	else
		return false; 
}