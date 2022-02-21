#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include <vector>
#include <string>
using namespace std;

class Actor;
class Peach;

class StudentWorld : public GameWorld
{
	public:
		StudentWorld(std::string assetPath);
		~StudentWorld();
		virtual int init();
		virtual int move();
		virtual void cleanUp();
		void addActor(Actor* actor); // Insert Actor into appropriate position in actors Vector 
		bool isBlockingAt(double x, double y) const; // Check if an Actor capable of blocking is at the specified position 
		bool isDamageableAt(double x, double y) const; // Check if an Actor capable of being damaged (NOT PEACH) is at the specified position 
		bool isPlayerAt(double x, double y, const Actor& actor, bool bonk); // Check if Peach is at the specified position; pass true to the final parameter to also bonk Peach
		bool bonkAt(double x, double y, const Actor& actor); // Bonk Actor at given position (NOT PEACH)
		void getPlayerLocation(double& x, double& y) const; //  Get Location of Peach 
		void givePowerup(int powerup); // Give Peach a Powerup
		void setFinalLevel() { m_finalLevel = true; } // Indicate that the current level is the final level

	private:
		Peach* m_peach;
		vector<Actor*> m_actors; // Using a vector since access will be more common than insertion
		int m_numSpecialActors; // Number of prioritized actors that are at the end of the m_actors vector 
		bool m_finalLevel;

		bool overlap(double start1, double end1, double start2, double end2, bool& lower) const; // Overlap auxiliary method 
		int actorBinarySearch(double y, const vector<Actor*>& actors, int start, int end) const;
		bool blockOrDamageAt(double x, double y, bool block) const;

};

#endif // STUDENTWORLD_H_