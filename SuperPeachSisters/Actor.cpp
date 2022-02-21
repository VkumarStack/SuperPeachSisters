#include "Actor.h"
#include "StudentWorld.h"
#include <cmath>

Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, int dir, int depth, double size) : GraphObject(imageID, startX, startY, dir, depth, size)
{
	m_studentWorld = world;
	m_isAlive = true; // All Actors start out alive
}

// The purpose of defining the < operator in this way is to ensure that all Actors that are limited to moving only horizontally (and therefore not moving vertically)
// are prioritized towards being first in a container and all Actors that are able to move vertically (i.e. Shells, Fireballs, Goodies) are prioritized towards being 
// last in a container. This way, a binary search can be performed for all non-vertically moving actors based on their vertical position (since this stays constant)
bool Actor::operator<(const Actor& other) const
{
	// Priority actors, recall, are those that can move vertically - which are either projectiles or powerups 
	if (priority() && !other.priority())
		return false;
	else if (other.priority() && !priority())
		return true;

	// All non-priority Actors (meaning those that are NOT Goodies, Fireballs, etc.) are intended to stay in a fixed vertical 
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

	// Update any necessary ticks 
	if (getStarPower())
		m_starPowerTicks--;
	if (getTempInvincibility())
		m_tempInvincibilityTicks--;
	if (m_shootPowerTicks > 0)
		m_shootPowerTicks--;

	// Bonk whatever Peach is at 
	getStudentWorld()->bonkAt(getX(), getY(), *this);

	if (getJumping())
	{
		if (getStudentWorld()->isBlockingAt(getX(), getY() + 4)) // Check for blockable object in jump path 
		{
			m_remainingJumpDistance = 0;
			getStudentWorld()->bonkAt(getX(), getY() + 4, *this); // If so, bonk the object 
		}
		else // Otherwise, continue jump
		{
			moveTo(getX(), getY() + 4); 
			m_remainingJumpDistance--;
		}
	}
	else // Not jumping 
	{
		if (!(getStudentWorld()->isBlockingAt(getX(), getY()) || getStudentWorld()->isBlockingAt(getX(), getY() - 3))) // Check if no object blocking fall
			moveTo(getX(), getY() - 4);
	}

	int key = -1;
	if (getStudentWorld()->getKey(key))
	{
		switch (key)
		{
		case KEY_PRESS_LEFT:
			setDirection(180);
			if (!getStudentWorld()->isBlockingAt(getX() - 4, getY())) // Nothing blocking movement left 
				moveTo(getX() - 4, getY());
			else
				getStudentWorld()->bonkAt(getX() - 4, getY(), *this); // Otherwise bonk object blocking 
			break;
		case KEY_PRESS_RIGHT:
			setDirection(0);
			if (!getStudentWorld()->isBlockingAt(getX() + 4, getY())) // Nothing blocking movement right 
				moveTo(getX() + 4, getY());
			else
				getStudentWorld()->bonkAt(getX() + 4, getY(), *this); // Otherwise bonk object blocking 
			break;
		case KEY_PRESS_UP:
			if (getStudentWorld()->isBlockingAt(getX(), getY() - 1)) // Can only jump if blockable object beneath 
			{
				if (getJumpPower())
					m_remainingJumpDistance = 12;
				else
					m_remainingJumpDistance = 8;
				getStudentWorld()->playSound(SOUND_PLAYER_JUMP);
			}
			break;
		case KEY_PRESS_SPACE:
			if (m_shootPowerTicks == 0) // Able to shoot 
			{
				getStudentWorld()->playSound(SOUND_PLAYER_FIRE);
				m_shootPowerTicks = 8;
				int x;
				if (getDirection() == 0)
					x = getX() + 4;
				else
					x = getX() - 4;
				getStudentWorld()->addActor(new PeachFireball(getStudentWorld(), x, getY(), getDirection())); // Spawn Fireball in direction 
			}
			break;
		}
	}
}

void Peach::getBonked(const Actor& actor)
{
	// Any damage done to Peach occurs if she is being bonked by a non-friendly actor and if Peach does not have any sort of invincibility 
	if (!actor.friendly() && !getStarPower() && !getTempInvincibility())
	{
		m_tempInvincibilityTicks = 10;
		if (getHitPoints() == 2) // If Peach has 2 hitpoints and was hit, disable her powerups 
		{
			m_shootPowerTicks = -1;
			m_jumpPower = false;
			getStudentWorld()->playSound(SOUND_PLAYER_HURT);
		}
		else // If peach otherwise has 1 hitpoint, she is dead
		{
			setDead();
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
void Block::getBonked(const Actor& actor)
{
	// A Block can only get bonked if it by Peach 
	if (actor.player())
	{
		if (m_containsGoodie == 0) // No Goodie in the block, so just play the sound 
			getStudentWorld()->playSound(SOUND_PLAYER_BONK);
		else // Goodie in the block 
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
void Goalpost::doSomething()
{
	if (alive())
	{
		if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, false))
		{
			getStudentWorld()->increaseScore(1000);
			setDead();
			reachAction(); // In the case of Mario, this will tell StudentWorld that the end of the final level has been reached 
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/
void Mario::reachAction()
{
	getStudentWorld()->setFinalLevel();
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Powerup::doSomething()
{
	if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, false)) // Check for contact with Player
	{
		getStudentWorld()->increaseScore(score()); // Give appropriate Powerup score 
		getStudentWorld()->givePowerup(m_powerup); // Give appropriate Powerup to Peach via StudentWorld as an intermediary 
		setDead(); // The Powerup is now dead since it has been used 
		getStudentWorld()->playSound(SOUND_PLAYER_POWERUP);
		return;
	}

	// Powerup movement:
	if (!(getStudentWorld()->isBlockingAt(getX(), getY()) || getStudentWorld()->isBlockingAt(getX(), getY() - 1))) // Check if it can fall 
		moveTo(getX(), getY() - 2);
	
	if (getDirection() == 180)  // Movement left 
	{
		if (!getStudentWorld()->isBlockingAt(getX() - 2, getY()))
			moveTo(getX() - 2, getY());
		else // If blocking, switch directions 
			setDirection(0);
	}
	else // Movement right 
	{
		if (!getStudentWorld()->isBlockingAt(getX() + 2, getY()))
			moveTo(getX() + 2, getY());
		else // If blocking, switch directions 
			setDirection(180);
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Projectile::doSomething()
{
	// What a Projectile tries to damage differs depending on whether it is friendly or not 
	if (friendly()) // A friendly projectile will attempt to damage enemies 
	{
		if (getStudentWorld()->isDamageableAt(getX(), getY()))
		{
			getStudentWorld()->bonkAt(getX(), getY(), *this);
			setDead();
			return;
		}
	}
	else // A non-friendly projectile will attempt to damage Peach 
	{
		if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, true)) // Bonks player if they are at that position 
		{
			setDead();
			return;
		}
	}

	// Projectile movement:
	if (!(getStudentWorld()->isBlockingAt(getX(), getY()) || getStudentWorld()->isBlockingAt(getX(), getY() - 1))) // If it can fall 
		moveTo(getX(), getY() - 2);

	if (getDirection() == 180) // Movement left 
	{
		if (!getStudentWorld()->isBlockingAt(getX() - 2, getY()))
			moveTo(getX() - 2, getY());
		else // If blocking, kill projectile 
			setDead();
	}
	else
	{
		if (!getStudentWorld()->isBlockingAt(getX() + 2, getY())) // Movement right 
			moveTo(getX() + 2, getY());
		else // If blocking, kill projectile 
			setDead();
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Enemy::getBonked(const Actor& actor)
{
	// An enemy will get bonked (die) if it contacts a projectile from a FRIENDLY source
	// Note that if Peach has a Star power, she is considered to be a projectile for its duration, so she will bonk an enemy in the same way as a 
	// PeachFireball would 
	if (actor.projectile() && actor.friendly())
	{
		if (actor.player())
		{
			getStudentWorld()->playSound(SOUND_PLAYER_KICK);
		}
		getStudentWorld()->increaseScore(100);
		setDead();
		deathAction(); // A Koopa has a special death action of spawning a shell 
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Enemy::doSomething()
{
	if (alive())
	{
		if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, true)) // If the Player is blocking an Enemy, they do nothing 
			return;

		int mvt;
		int partialBlock; // Check if the Enemy may even partially walk off an edge 
		if (getDirection() == 0)
		{
			mvt = getX() + 1;
			partialBlock = getX() + SPRITE_WIDTH;
		}
		else
		{
			mvt = getX() - 1;
			partialBlock = getX() - SPRITE_WIDTH;
		}

		if (getStudentWorld()->isBlockingAt(mvt, getY())) // Check if anything blocking movement in direction 
		{
			if (getDirection() == 0) // If so, turn around 
				setDirection(180);
			else
				setDirection(0);
		}
		else if (!getStudentWorld()->isBlockingAt(partialBlock, getY() - 1)) // Check if ground below in target direction to be moved 
		{
			if (getDirection() == 0) // If so, turn around 
				setDirection(180);
			else
				setDirection(0);
		}

		// Move if nothing blocking 
		if (getDirection() == 180)
		{
			if (!getStudentWorld()->isBlockingAt(getX() - 1, getY()))
				moveTo(getX() - 1, getY());
		}
		else
		{
			if (!getStudentWorld()->isBlockingAt(getX() + 1, getY()))
				moveTo(getX() + 1, getY());
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------------------*/
void Koopa::deathAction()
{
	getStudentWorld()->addActor(new Shell(getStudentWorld(), getX(), getY(), getDirection()));
}

/*------------------------------------------------------------------------------------------------------------------------------*/
// Override doSomething() since a Piranha's functionality differs from a Goomba and Koopa 
void Piranha::doSomething()
{
	if (alive())
	{
		increaseAnimationNumber();
		if (getStudentWorld()->isPlayerAt(getX(), getY(), *this, true))
			return;

		double peachX = -1;
		double peachY = -1;
		getStudentWorld()->getPlayerLocation(peachX, peachY);

		if (peachY != -1 && (1.5 * SPRITE_HEIGHT >= abs(peachY - getY()))) // Check if Peach is within 1.5 * SPRITE_HEIGHT
		{
			if (peachX != -1 && (peachX < getX())) // Adjust direction of Piranha to face Peach 
				setDirection(180);
			else if (peachX != -1)
				setDirection(0);
		}
		else
			return;
		
		if (firingDelay())
		{
			m_firingDelay--;
			return;
		}

		if (peachX != -1 && (SPRITE_WIDTH * 8 > abs(peachX - getX()))) // Check if Peach is within firing distance; if so, fire 
		{
			getStudentWorld()->addActor(new PiranhaFireball(getStudentWorld(), getX(), getY(), getDirection()));
			getStudentWorld()->playSound(SOUND_PIRANHA_FIRE);
			m_firingDelay = 40;
		}
	}
}