#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor : public GraphObject
{
	public:
		Actor(int imageID, int startX, int startY, int startDirection, StudentWorld* world);
		bool operator<(const Actor& other) const; // Define < operator for use in STL data structures 
		void setDead() { isAlive = false; }
		virtual bool canWalk() const { return true; }
		virtual bool isInteractable() const { return false; }
		virtual void bonk() const = 0;
	private:
		StudentWorld* m_studentWorld; // Access other actors contained in the StudentWorld object
		bool isAlive;
};

#endif // ACTOR_H_
