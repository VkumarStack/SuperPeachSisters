#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <algorithm>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    m_actors = vector<Actor*>();
    m_numSpecialActors = 0;
}

int StudentWorld::init()
{
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    // This code is here merely to allow the game to build, run, and terminate after you hit enter.
    // Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
    decLives();
    return GWSTATUS_PLAYER_DIED;
}

void StudentWorld::cleanUp()
{
}

void StudentWorld::addActor(Actor* actor)
{
    if (actor->priority())
        m_numSpecialActors++;

    if (m_actors.size() == 0)
        m_actors.push_back(actor);

    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if (*actor < *(*it))
        {
            m_actors.insert(it, actor);
            return;
        }
    }
    m_actors.insert(it, actor);
}

bool StudentWorld::isBlockingAt(double x, double y, Actor*& actor) const
{
    bool lower;
    vector<Actor*>::const_iterator it;
    it = m_actors.begin();
    // Iterate through prioritized Actors (fireballs, goodies, etc.)
    for (int i = 0; i < m_numSpecialActors; i++)
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, (*it)->getX(), (*it)->getX() + SPRITE_WIDTH - 1, lower) && overlap(y, y + SPRITE_HEIGHT - 1, (*it)->getY(), (*it)->getY() + SPRITE_HEIGHT - 1, lower))
        {
            actor = *it;
            return true; 
        }
        it++;
    }

    // Use binary search based on vertical position to find the actor blocking 
    it = actorBinarySearch(y, m_actors, it, m_actors.end());
    if (it == m_actors.end()) // No vertical matches 
        return false;
    // At this point, it points to a position in actor that overlaps VERTICALLY; since Actors is sorted vertically, all that needs to be checked is the horizontally overlapping of all 
    // Actors relative to it (because it merely found the vertical matches with the binary search it is still possible that there could be vertical matches to the left or right)
    vector<Actor*>::const_iterator horizontalIt = it; 
    // Check for left overlapping
    while (horizontalIt >= m_actors.begin() && overlap(y, y + SPRITE_HEIGHT - 1, (*horizontalIt)->getY(), (*horizontalIt)->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, (*horizontalIt)->getX(), (*horizontalIt)->getX() + SPRITE_WIDTH - 1, lower))
        {
            actor = *horizontalIt;
            return true;
        }
        horizontalIt--;
    }
    // Check for right overlapping
    horizontalIt = it + 1;
    while (horizontalIt < m_actors.end() && overlap(y, y + SPRITE_HEIGHT - 1, (*horizontalIt)->getY(), (*horizontalIt)->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, (*horizontalIt)->getX(), (*horizontalIt)->getX() + SPRITE_WIDTH - 1, lower))
        {
            actor = *horizontalIt;
            return true;
        }
        horizontalIt++;
    }
    // No overlapping found
    return false;
}

bool StudentWorld::overlap(double start1, double end1, double start2, double end2, bool& lower /*if (start1, end1) is lower than (start2, end2)*/) const
{
    if ((end1 >= start2) && (end2 >= start1))
        return true;

    if (end1 < start2)
        lower = true;
    else
        lower = false;
    return false;
}

vector<Actor*>::const_iterator StudentWorld::actorBinarySearch(double y, vector<Actor*> actors, vector<Actor*>::const_iterator start, vector<Actor*>::const_iterator end) const
{
    if (start >= end)
    {
        bool lower;
        vector<Actor*>::const_iterator mid = start + (end - start) / 2; 
        if (overlap(y, y + SPRITE_HEIGHT - 1, (*mid)->getY(), (*mid)->getY() + SPRITE_HEIGHT - 1, lower))
            return mid;
        else if (lower)
            return actorBinarySearch(y, actors, start, mid - 1);
        else
            return actorBinarySearch(y, actors, mid + 1, end);
    }
    vector<Actor*>::const_iterator ret;
    ret = actors.end();
    return ret;
}