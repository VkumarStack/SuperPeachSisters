#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include <vector>
#include <string>
using namespace std;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp;
class Actor;
class StudentWorld : public GameWorld
{
public:
  StudentWorld(std::string assetPath);
  virtual int init();
  virtual int move();
  virtual void cleanUp();
  void addActor(Actor* actor);
  bool isBlockingAt (double x, double y, Actor*& actor) const;
  

private:
	bool overlap (double start1, double end1, double start2, double end2, bool& lower) const;
	vector<Actor*>::const_iterator actorBinarySearch(double y, vector<Actor*> actors, vector<Actor*>::const_iterator start, vector<Actor*>::const_iterator end) const;
	vector<Actor*> m_actors;
	int m_numSpecialActors;
	
};

#endif // STUDENTWORLD_H_
