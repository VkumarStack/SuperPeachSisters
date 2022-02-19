#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, int dir, int depth, double size) : GraphObject(imageID, startX, startY, dir, depth, size)
{
	m_studentWorld = world;
	m_isAlive = true;
}

bool Actor::operator<(const Actor& other) const
{
	// Interactable (fireballs, goodies) Actors should have higher priority since their actions can change more behavior
	// i.e. The action of a fireball hitting an enemy should be performed first since beyond so the enemy can no longer 
	// attack Peach if interacting with her 
	if (priority() && !other.priority())
		return true;
	else if (other.priority() && !priority())
		return false;

	// All non-interactable Actors (meaning those that are NOT goodies, fire flowers, or peach) are intended to stay in a fixed vertical 
	// position, so comparison can be done in terms of their vertical position (and there is no need to worry about having to rearrange 
	// since vertical position will stay the same)
	if (getY() < other.getY())
		return true;
	else
		return false;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

Peach::Peach(StudentWorld* world, int startX, int startY) : Actor(world, IID_PEACH, startX, startY, 0)
{
	m_remainingJumpDistance = 0;
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

	getStudentWorld()->bonkAt(getX(), getY(), *this);

	if (getJumping())
	{
		if (getStudentWorld()->isBlockingAt(getX(), getY() + 4))
		{
			m_remainingJumpDistance = 0;
			getStudentWorld()->bonkAt(getX(), getY() + 4, *this);
		}
		else
		{
			moveTo(getX(), getY() + 4);
			m_remainingJumpDistance--;
		}
	}
	else
	{
		if (!(getStudentWorld()->isBlockingAt(getX(), getY()) || getStudentWorld()->isBlockingAt(getX(), getY() - 3)))
			moveTo(getX(), getY() - 4);
	}

	int key = -1;
	if (getStudentWorld()->getKey(key))
	{
		switch (key)
		{
		case KEY_PRESS_LEFT:
			setDirection(180);
			if (!getStudentWorld()->isBlockingAt(getX() - 4, getY()))
				moveTo(getX() - 4, getY());
			else
				getStudentWorld()->bonkAt(getX() - 4, getY(), *this);
			break;
		case KEY_PRESS_RIGHT:
			setDirection(0);
			if (!getStudentWorld()->isBlockingAt(getX() + 4, getY()))
				moveTo(getX() + 4, getY());
			else
				getStudentWorld()->bonkAt(getX() + 4, getY(), *this);
			break;
		case KEY_PRESS_UP:
			if (getStudentWorld()->isBlockingAt(getX(), getY() - 1))
			{
				if (getJumpPower())
					m_remainingJumpDistance = 12;
				else
					m_remainingJumpDistance = 8;
				getStudentWorld()->playSound(SOUND_PLAYER_JUMP);
			}
			break;
		case KEY_PRESS_SPACE:
			if (m_shootPowerTicks == 0)
			{
				getStudentWorld()->playSound(SOUND_PLAYER_FIRE);
				m_shootPowerTicks = 8;
				// TO DO
				// getStudentWorld()->addActor(new PeachFireball(...))
			}
			break;
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
Block::Block(StudentWorld* world, int startX, int startY, int goodie) : Terrain(world, IID_BLOCK, startX, startY)
{
	m_containsGoodie = goodie;
}

void Block::getBonked(const Actor& actor)
{
	if (actor.player())
	{
		if (m_containsGoodie == 0)
			getStudentWorld()->playSound(SOUND_PLAYER_BONK);
		else
		{
			getStudentWorld()->playSound(SOUND_POWERUP_APPEARS);
			switch (m_containsGoodie)
			{
				case 1:
					getStudentWorld()->addActor(new Mushroom(getStudentWorld(), getX(), getY() + 8));
					break;
				case 2:
					getStudentWorld()->addActor(new Flower(getStudentWorld(), getX(), getY() + 8));
					break;
				case 3:
					getStudentWorld()->addActor(new Star(getStudentWorld(), getX(), getY() + 8));
					break;
			}
			m_containsGoodie = 0;
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
void Flag::doSomething()
{
	if (alive())
	{
		if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, false))
		{
			getStudentWorld()->increaseScore(1000);
			setDead();
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Mario::doSomething()
{
	if (alive())
	{
		if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, false))
		{
			getStudentWorld()->increaseScore(1000);
			setDead();
			getStudentWorld()->setFinalLevel();
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Powerup::doSomething()
{
	if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, false))
	{
		getStudentWorld()->increaseScore(score());
		getStudentWorld()->givePowerup(m_powerup);
		setDead();
		getStudentWorld()->playSound(SOUND_PLAYER_POWERUP);
		return;
	}

	if (!(getStudentWorld()->isBlockingAt(getX(), getY()) || getStudentWorld()->isBlockingAt(getX(), getY() - 1)))
		moveTo(getX(), getY() - 2);
	
	if (getDirection() == 180)
	{
		if (!getStudentWorld()->isBlockingAt(getX() - 2, getY()))
			moveTo(getX() - 2, getY());
		else
			setDirection(0);
	}
	else
	{
		if (!getStudentWorld()->isBlockingAt(getX() + 2, getY()))
			moveTo(getX() + 2, getY());
		else
			setDirection(180);
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Projectile::doSomething()
{
	if (friendly())
	{

	}
}
