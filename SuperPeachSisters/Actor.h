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
		virtual bool stationary() const { return false; } // If the Actor has functionality not involving moving (i.e. Blocks are stationary but Koopas are not)
		virtual bool friendly() const { return true; } // If the Actor has functionality that helps/does not harm the player (i.e. Blocks and Star Powers are friendly but Piranhas are not)
		virtual bool sentient() const { return true; } // If the Actor has functionality that involves any sort of action (effectively everything except for the Block and Pipe are sentient)
		virtual bool usable() const { return false; } // If the Actor has functionality that can be "used" by a player (i.e. Blocks, Flowers, Flags, etc. are usable)
		virtual bool player() const { return false; } // If the Actor is Peach
		virtual bool priority() const { return false; } // If the Actor is a fireball or goodie 
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
	int getHitPoints() const { return m_hitPoints; }
	int getJumping()  const { return (m_remainingJumpDistance > 0);  }
	bool getJumpPower() const { return m_jumpPower; }
	bool getShootPower() const { return m_shootPowerTicks != -1; }
	bool getStarPower() const { return m_starPowerTicks != 0; }
	bool getTempInvincibility() const { return m_tempInvincibilityTicks != 0; }
private:
	int m_hitPoints;
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
		virtual bool stationary() const { return true; }
		virtual bool sentient() const { return false; }
};

/*------------------------------------------------------------------------------------------------------------------------------*/
class Block : public Terrain
{
	public:
		Block(StudentWorld* world, int startX, int startY, int goodie);
		virtual void getBonked(const Actor& actor);
		virtual bool usable() const { return true; }
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
		virtual bool stationary() { return true; }
		virtual bool usable() { return true; }
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

#endif // ACTOR_H_
