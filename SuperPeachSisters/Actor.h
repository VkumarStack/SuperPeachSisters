#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor : public GraphObject
{
public:
	Actor(StudentWorld* world, int imageID, int startX, int startY, int dir = 0, int depth = 0, double size = 1.0);
	bool operator<(const Actor& other) const; // Define < operator for use in STL data structures 

	virtual void doSomething() = 0;
	virtual void getBonked(const Actor& actor) = 0;

	void setDead() { m_isAlive = false; }

	StudentWorld* getStudentWorld() const { return m_studentWorld; }
	virtual bool player() const { return false; } // Peach identifier 
	virtual bool terrain() const { return false; } // Block/Pipe identifier 
	virtual bool goalpost() const { return false; } // Flag/Mario identifier
	virtual bool projectile() const { return false; } // Fireball identifier (also to be used if Peach has star power)
	virtual bool friendly() const { return true; } // Identifier for Actors that can damage Peach
	virtual bool powerup(int& powerupType) const { return false; } // Goodies 
	bool priority() const  // If the Actor is a projectile or goodie 
	{ 
		int x;
		return (projectile() || powerup(x));
	}
	bool alive() const { return m_isAlive; }
private:
	StudentWorld* m_studentWorld; // Access other actors contained in the StudentWorld object
	bool m_isAlive;
};

/*----------------------------------------------------------------------------------------------------------------------------*/

class Peach : public Actor
{
public:
	Peach(StudentWorld* world, int startX, int startY);
	virtual void doSomething();
	virtual void getBonked(const Actor& actor) {}

	virtual bool player() const { return true; }
	int getHitPoints() const 
	{
		if (getJumpPower() || getShootPower())
			return 2;
		return 1;
	}
	int getJumping()  const { return (m_remainingJumpDistance > 0); }
	bool getJumpPower() const { return m_jumpPower; }
	bool getShootPower() const { return m_shootPowerTicks != -1; }
	bool getStarPower() const { return m_starPowerTicks != 0; }
	bool getTempInvincibility() const { return m_tempInvincibilityTicks != 0; }
	void giveJumpPower() { m_jumpPower = true; }
	void giveShootPower() 
	{
		if (!getShootPower())
			m_shootPowerTicks = 8;
	}
	void giveStarPower()
	{
		if (!getStarPower())
			m_starPowerTicks = 150;
	}
private:
	int m_remainingJumpDistance;
	bool m_jumpPower;
	int m_shootPowerTicks;
	int m_starPowerTicks;
	int m_tempInvincibilityTicks;
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Terrain : public Actor
{
public:
	Terrain(StudentWorld* world, int imageID, int startX, int startY) : Actor(world, imageID, startX, startY, 0, 2, 1) {}
	virtual void doSomething() {}
	virtual void getBonked(const Actor& actor) = 0;
	virtual bool terrain() const { return true; }
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Block : public Terrain
{
public:
	Block(StudentWorld* world, int startX, int startY, int goodie);
	virtual void getBonked(const Actor& actor);
private:
	int m_containsGoodie; // 0 = No Goodie, 1 = Mushroom, 2 = Flower, 3 = Star 
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Pipe : public Terrain
{
public:
	Pipe(StudentWorld* world, int startX, int startY) : Terrain(world, IID_PIPE, startX, startY) {}
	virtual void getBonked(const Actor& actor) {}
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Goalpost : public Actor
{
public:
	Goalpost(StudentWorld* world, int imageID, int startX, int startY) : Actor(world, imageID, startX, startY, 0, 1, 1) {}
	virtual void doSomething() = 0;
	virtual void getBonked(const Actor& actor) {}
	virtual bool goalpost() const { return true; }
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Flag : public Goalpost
{
public:
	Flag(StudentWorld* world, int startX, int startY) : Goalpost(world, IID_FLAG, startX, startY) {}
	virtual void doSomething();
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Mario : public Goalpost
{
public:
	Mario(StudentWorld* world, int startX, int startY) : Goalpost(world, IID_MARIO, startX, startY) {}
	virtual void doSomething();
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Powerup : public Actor
{
	public:
		Powerup(StudentWorld* world, int imageID, int startX, int startY, int powerup) : Actor(world, imageID, startX, startY, 0, 1, 1) { m_powerup = powerup; }
		virtual void doSomething();
		virtual void getBonked(const Actor& actor) {}
		virtual int score() const = 0;
		virtual bool powerup(int& powerupType) const 
		{ 
			powerupType = m_powerup;
			return true; 
		}
	private:
		int m_powerup;
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Mushroom : public Powerup
{
	public:
		Mushroom(StudentWorld* world, int startX, int startY) : Powerup(world, IID_MUSHROOM, startX, startY, 1) {}
		virtual int score() const { return 75; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Flower : public Powerup
{
	public:
		Flower(StudentWorld* world, int startX, int startY) : Powerup(world, IID_FLOWER, startX, startY, 2) {}
		virtual int score() const { return 50; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Star : public Powerup
{
	public:
		Star(StudentWorld* world, int startX, int startY) : Powerup(world, IID_STAR, startX, startY, 3) {}
		virtual int score() const { return 100; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Projectile : public Actor
{
	public:
		Projectile(StudentWorld* world, int imageID, int startX, int startY, int direction) : Actor(world, imageID, startX, startY, direction, 1, 1) {}
		virtual void doSomething();
		virtual void getBonked(const Actor& actor) {}
		virtual bool projectile() const { return true; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class PiranhaFireball : public Projectile
{
	public:
		PiranhaFireball(StudentWorld* world, int startX, int startY, int direction) : Projectile(world, IID_PIRANHA_FIRE, startX, startY, direction) {}
		virtual bool friendly() const { return false; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class PeachFireball : public Projectile
{
	public:
		PeachFireball(StudentWorld* world, int startX, int startY, int direction) : Projectile(world, IID_PEACH_FIRE, startX, startY, direction) {}
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Shell : public Projectile
{
	public:
		Shell(StudentWorld* world, int startX, int startY, int direction) : Projectile(world, IID_SHELL, startX, startY, direction) {}
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Enemy : public Actor
{
	public:
		Enemy(StudentWorld* world, int imageID, int startX, int startY, int direction) : Actor(world, imageID, startX, startY, direction, 1, 0) {}
		virtual void doSomething() = 0;
		virtual void getBonked(const Actor& actor) = 0;
		virtual bool friendly() const { return false; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Goomba : public Enemy
{
	public:
		Goomba(StudentWorld* world, int startX, int startY, int direction) : Enemy(world, IID_GOOMBA, startX, startY, direction) {}
		virtual void doSomething();
		virtual void getBonked(const Actor& actor) {}
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Koopa : public Enemy
{
	public:
		Koopa(StudentWorld* world, int startX, int startY, int direction) : Enemy(world, IID_KOOPA, startX, startY, direction) {}
		virtual void doSomething();
		virtual void getBonked(const Actor& actor) {}
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Piranha : public Enemy
{
	public:
		Piranha(StudentWorld* world, int startX, int startY, int direction) : Enemy(world, IID_PIRANHA, startX, startY, direction) { m_firingDelay = 40; }
		virtual void doSomething();
		virtual void getBonked(const Actor& actor) {}
		bool firingDelay() { return (m_firingDelay > 0); }
	private:
		int m_firingDelay;
};
#endif // ACTOR_H_
