#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include <vector>
#include <string>
using namespace std;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp;
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
	void addActor(Actor* actor);
	bool isBlockingAt(double x, double y) const;
	bool isDamageableAt(double x, double y) const;
	bool isPlayerAt(double x, double y, const Actor& actor, bool bonk);
	void getPlayerLocation(double& x, double& y) const;
	bool bonkAt(double x, double y, const Actor& actor);
	void givePowerup(int powerup);
	void setFinalLevel() { m_finalLevel = true; }


private:
	Peach* m_peach;
	vector<Actor*> m_actors;
	int m_numSpecialActors;
	bool m_finalLevel;

	bool overlap(double start1, double end1, double start2, double end2, bool& lower) const;
	int actorBinarySearch(double y, const vector<Actor*>& actors, int start, int end) const;

};

#endif // STUDENTWORLD_H_