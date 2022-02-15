#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
Actor::Actor(int imageID, int startX, int startY, int startDirection, StudentWorld* world, int dir = 0, int depth = 0, double size = 1.0) : GraphObject(imageID, startX, startY, dir, depth, size)
{
	m_studentWorld = world;
	m_isAlive = true;
}

bool Actor::operator<(const Actor& other) const
{
	// Interactable (fireballs, goodies) Actors should have higher priority since their actions can change more behavior
	// i.e. The action of a fireball hitting an enemy should be performed first since beyond so the enemy can no longer 
	// attack Peach if interacting with her 
	if (usable() && !other.usable())
		return true;
	else if (other.usable() && !usable())
		return false;

	// All non-interactable Actors (meaning those that are NOT goodies, fire flowers, or peach) are intended to stay in a fixed vertical 
	// position, so comparison can be done in terms of their vertical position (and there is no need to worry about having to rearrange 
	// since vertical position will stay the same)
	if (getY() < other.getY())
		return true;
	else if (getY() > other.getY())
		return false; 
}
/*------------------------------------------------------------------------------------------------------------------------------*/
Block::Block(int startX, int startY, StudentWorld* world, int goodie) : Actor(IID_BLOCK, startX, startY, 0, world, 2, 1)
{
	m_containsGoodie = goodie;
}

void Block::getBonked(const Actor& actor) 
{
	if (actor.player())
	{
		if (m_containsGoodie == 0)
			getStudentWorld()->playSound(SOUND_PLAYER_BONK);
		// TODO IMPLEMENT POWERUPS
	}
}



/*------------------------------------------------------------------------------------------------------------------------------*/
Peach::Peach(int startX, int startY, StudentWorld* world) : Actor(IID_PEACH, startX, startY, 0, world)
{
	m_hitPoints = 1;
	m_jumpPower = false;
	m_shootPowerTicks = -1;
	m_starPowerTicks = 0;
	m_tempInvincibilityTicks = 0;
}	

void Peach::doSomething()
{
	if (!alive())
		return;
	if (getStarPower())
		m_starPowerTicks--;
	if (getTempInvincibility())
		m_tempInvincibilityTicks--;
	if (m_shootPowerTicks > 0)
		m_shootPowerTicks--;
	
	int key = -1;
	if (getStudentWorld()->getKey(key))
	{
		switch (key)
		{
			case KEY_PRESS_LEFT:
				setDirection(180);
				Actor* actor = nullptr;
				if (!isBlockingObjectAt(getX() - 4, getY()))
					moveTo(getX() - 4, getY());
				else if (actor != nullptr)
				{
					actor->getBonked(*this);
				}
				break;
			case KEY_PRESS_RIGHT:
				setDirection(0);
				Actor* actor = nullptr;
				if (!isBlockingObjectAt(getX() + 4, getY()))
					moveTo(getX() + 4, getY());
				else if (actor != nullptr)
				{
					actor->getBonked(*this);
				}
				break;
			case KEY_PRESS_UP:
				break;
			case KEY_PRESS_SPACE:
				break;
		}
	}
}