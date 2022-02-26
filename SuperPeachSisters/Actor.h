#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

class Actor : public GraphObject
{
	public:
		Actor(StudentWorld* world, int imageID, int startX, int startY, int dir = 0, int depth = 0, double size = 1.0);
		bool operator<(const Actor& other) const; // Define < operator for use in STL data structures 

		// Pure virtual methods to be overriden with functionality 
		virtual void doSomething() = 0;
		virtual void bonk(const Actor& actor) = 0; // bonk takes in an Actor in order to determine how exactly the bonking should occur 
		// i.e. a Block needs to know that the actor bonking it is Peach. Since this Actor reference is const, however, it cannot modify the actor passed in

		// Identifiers to be overriden in order to distinguish between different actors 
		virtual bool player() const { return false; }  
		virtual bool terrain() const { return false; } 
		virtual bool goalpost() const { return false; } 
		virtual bool projectile() const { return false; }
		virtual bool friendly() const { return true; } 
		virtual bool powerup() const { return false; }

		// An Actor is considered to be priority if it is able to move vertically - which include all goodies and projectiles since they 
		// can fall; this distinguishment is for the purpose of sorting Actors in an STL container 
		bool priority() const { return (projectile() || powerup()); }
		
		bool alive() const { return m_isAlive; }
	
	protected:
		// Methods only to be used within derived classes 
		StudentWorld* getStudentWorld() const { return m_studentWorld; }
		void setDead() { m_isAlive = false; }
	
	private:
		StudentWorld* m_studentWorld; // Access StudentWorld methods
		bool m_isAlive;
};

/*----------------------------------------------------------------------------------------------------------------------------*/

class Peach : public Actor
{
	public:
		Peach(StudentWorld* world, int startX, int startY);
		virtual void doSomething();
		virtual void bonk(const Actor& actor);

		virtual bool player() const { return true; }
		// Peach will be treated as a projectile that can damage enemies if she has a star 
		virtual bool projectile() const { return getStarPower(); }
		
		// Instead of storing the hitpoints of Peach as a data member, the presence of any powerups can be checked since having a Jump or Shoot 
		// powerup implies 2 hitpoints and all other cases imply 1 hitpoint 
		int getHitPoints() const 
		{
			if (getJumpPower() || getShootPower())
				return 2;
			return 1;
		}

		bool getJumpPower() const { return m_jumpPower; }
		bool getShootPower() const { return m_shootPowerTicks != -1; } // Peach has Shoot Power if there are 0 or more ticks remaining to shoot 
		bool getStarPower() const { return m_starPowerTicks != 0; } // Peach has Star Power if there are more than 0 ticks remaining 
		
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

		// Helper methods 
		int getJumping()  const { return (m_remainingJumpDistance > 0); }
		bool getTempInvincibility() const { return m_tempInvincibilityTicks != 0; }
};

/*------------------------------------------------------------------------------------------------------------------------------*/
// Class to hold functionality shared by both a Block and Pipe 
class Terrain : public Actor
{
	public:
		Terrain(StudentWorld* world, int imageID, int startX, int startY) : Actor(world, imageID, startX, startY, 0, 2, 1) {}
		virtual void doSomething() {} // Both a Block and Pipe do nothing 
		virtual void bonk(const Actor& actor) = 0;
		virtual bool terrain() const { return true; }
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Block : public Terrain
{
	public:
		Block(StudentWorld* world, int startX, int startY, int goodie) : Terrain(world, IID_BLOCK, startX, startY) { m_containsGoodie = goodie; }
		virtual void bonk(const Actor& actor);
	
	private:
		int m_containsGoodie; // 0 = No Goodie, 1 = Mushroom, 2 = Flower, 3 = Star 
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Pipe : public Terrain
{
	public:
		Pipe(StudentWorld* world, int startX, int startY) : Terrain(world, IID_PIPE, startX, startY) {}
		virtual void bonk(const Actor& actor) {}
};

/*------------------------------------------------------------------------------------------------------------------------------*/
// Class to hold functionality shared by both a Flag and Mario
class Goalpost : public Actor
{
	public:
		Goalpost(StudentWorld* world, int imageID, int startX, int startY) : Actor(world, imageID, startX, startY, 0, 1, 1) {}
		// Both a Flag and Mario share essentially the same doSomething(); any minor difference is accounted for in reachAction()
		virtual void doSomething();
		virtual void bonk(const Actor& actor) {}
		virtual bool goalpost() const { return true; }
	
	protected:
		// doSomething() for both a Flag and Mario are virtually the same EXCEPT for distinguishing whether to move to the next level (Flag)
		// or end the game (Mario). reachAction() is to be overriden and called by the shared doSomething() method to account for this 
		// minor difference in an otherwise similar action
		virtual void reachAction() = 0;
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Flag : public Goalpost
{
	public:
		Flag(StudentWorld* world, int startX, int startY) : Goalpost(world, IID_FLAG, startX, startY) {}
	
	protected:
		virtual void reachAction() {} 
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Mario : public Goalpost
{
	public:
		Mario(StudentWorld* world, int startX, int startY) : Goalpost(world, IID_MARIO, startX, startY) {}
	
	protected:
		virtual void reachAction();
};

/*------------------------------------------------------------------------------------------------------------------------------*/
// Class to hold functionality shared between Mushroom, Flower, and Star 
class Powerup : public Actor
{
	public:
		Powerup(StudentWorld* world, int imageID, int startX, int startY) : Actor(world, imageID, startX, startY, 0, 1, 1) {}
		// All powerups share the same doSomething() functionality 
		virtual void doSomething();
		virtual void bonk(const Actor& actor) {}
		virtual bool powerup() const { return true; }
	
	protected:
		// Powerups differ in the score that they give 
		virtual int score() const = 0;
		virtual int typePowerUp() const = 0; // Identify exactly which Powerup it is; 1 = Mushroom, 2 = Flower, 3 = Star
	
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Mushroom : public Powerup
{
	public:
		Mushroom(StudentWorld* world, int startX, int startY) : Powerup(world, IID_MUSHROOM, startX, startY) {}
	
	protected:	
		virtual int score() const { return 75; }
		virtual int typePowerUp() const { return 1; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Flower : public Powerup
{
	public:
		Flower(StudentWorld* world, int startX, int startY) : Powerup(world, IID_FLOWER, startX, startY) {}
	
	protected:
		virtual int score() const { return 50; }
		virtual int typePowerUp() const { return 2; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Star : public Powerup
{
	public:
		Star(StudentWorld* world, int startX, int startY) : Powerup(world, IID_STAR, startX, startY) {}
	
	protected:
		virtual int score() const { return 100; }
		virtual int typePowerUp() const { return 3; }
};
/*------------------------------------------------------------------------------------------------------------------------------*/
// Class to hold functionality shared between PiranhaFireball, PeachFireball, and Shell
class Projectile : public Actor
{
	public:
		Projectile(StudentWorld* world, int imageID, int startX, int startY, int direction) : Actor(world, imageID, startX, startY, direction, 1, 1) {}
		// All projectiles share the same doSomething() functionality 
		virtual void doSomething();
		virtual void bonk(const Actor& actor) {}
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
// Class to hold functionality shared between Goomba, Koopa, and Piranha
class Enemy : public Actor
{
	public:
		Enemy(StudentWorld* world, int imageID, int startX, int startY, int direction) : Actor(world, imageID, startX, startY, direction, 1, 0) {}
		// Goomba and Koopa essentially share the same doSomething() functionality, though Piranha does not. The base doSomething() method is for 
		// Goomba and Koopa, but will overriden for Piranha's unique functionality 
		virtual void doSomething();
		// Goomba, Koopa, and Piranha all share the same functionality for getting bonked 
		virtual void bonk(const Actor& actor);
		virtual bool friendly() const { return false; }
	
	protected:
		// Any special actions upon death (Koopa spawning a shell) to be defined in base classes 
		virtual void deathAction() = 0;
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Goomba : public Enemy
{
	public:
		Goomba(StudentWorld* world, int startX, int startY, int direction) : Enemy(world, IID_GOOMBA, startX, startY, direction) {}
	
	protected:	
		virtual void deathAction() {}
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Koopa : public Enemy
{
	public:
		Koopa(StudentWorld* world, int startX, int startY, int direction) : Enemy(world, IID_KOOPA, startX, startY, direction) {}
	
	protected:
		virtual void deathAction();
};
/*------------------------------------------------------------------------------------------------------------------------------*/
class Piranha : public Enemy
{
	public:
		Piranha(StudentWorld* world, int startX, int startY, int direction) : Enemy(world, IID_PIRANHA, startX, startY, direction) { m_firingDelay = 0; }
		// Overriden doSomething() since unlike a Goomba and Koopa, a Piranha does not move but rather stays still and shoots 
		virtual void doSomething();
	
	protected:
		virtual void deathAction() {}

	private:
		int m_firingDelay;
		bool firingDelay() { return (m_firingDelay > 0); }
};
#endif // ACTOR_H_
