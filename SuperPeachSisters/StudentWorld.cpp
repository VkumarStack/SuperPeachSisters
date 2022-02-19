#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <iostream>
#include <sstream>
#include <iomanip>
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
    m_finalLevel = false;
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

int StudentWorld::init()
{
    int level = getLevel();
    ostringstream oss;
    oss.fill('0');
    oss << "level" << setw(2) << level << ".txt";

    Level lev(assetPath());
    string level_file = oss.str();
    Level::LoadResult result = lev.loadLevel(level_file);
    if (result == Level::load_success)
    {
        Level::GridEntry ge;
        int dir;
        for (int w = 0; w < GRID_WIDTH; w++)
        {
            for (int c = 0; c < GRID_HEIGHT; c++)
            {
                ge = lev.getContentsOf(w, c);
                switch (ge)
                {
                case Level::peach:
                    m_peach = new Peach(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT);
                    break;
                case Level::mushroom_goodie_block:
                    addActor(new Block(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 1));
                    break;
                case Level::flower_goodie_block:
                    addActor(new Block(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 2));
                    break;
                case Level::star_goodie_block:
                    addActor(new Block(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 3));
                    break;
                case Level::block:
                    addActor(new Block(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 0));
                    break;
                case Level::pipe:
                    addActor(new Pipe(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT));
                    break;
                case Level::flag:
                    addActor(new Flag(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT));
                    break;
                case Level::mario:
                    addActor(new Mario(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT));
                    break;
                case Level::goomba:
                    dir = rand() % 2;
                    if (dir == 0)
                        addActor(new Goomba(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 0));
                    else
                        addActor(new Goomba(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 180));
                    break;
                case Level::koopa:
                    dir = rand() % 2;
                    if (dir == 0)
                        addActor(new Koopa(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 0));
                    else
                        addActor(new Koopa(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 180));
                    break;
                case Level::piranha:
                    dir = rand() % 2;
                    if (dir == 0)
                        addActor(new Piranha(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 0));
                    else
                        addActor(new Piranha(this, w * SPRITE_WIDTH, c * SPRITE_HEIGHT, 180));
                    break;
                }
            }
        }
    }
    else
    {
        exit(1);
    }

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    // This code is here merely to allow the game to build, run, and terminate after you hit enter.
    // Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
    //decLives();
    //return GWSTATUS_PLAYER_DIED;
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->alive())
        {
            (*it)->doSomething();
            if (!m_peach->alive())
            {
                playSound(SOUND_PLAYER_DIE);
                return GWSTATUS_PLAYER_DIED;
            }
            if ((*it)->goalpost() && !(*it)->alive())
            {
                if (m_finalLevel)
                {
                    playSound(SOUND_GAME_OVER);
                    return GWSTATUS_PLAYER_WON;
                }
                playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
    }
    m_peach->doSomething();
    
    it = m_actors.begin();
    while (it != m_actors.end())
    {
        if (!(*it)->alive())
        {
            if ((*it)->priority())
                m_numSpecialActors--;
            delete* it;
            it = m_actors.erase(it);
        }
        else
            it++;
    }
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    vector<Actor*>::iterator it;
    it = m_actors.begin();
    while (it != m_actors.end())
    {
        delete* it;
        it = m_actors.erase(it);
    }
    delete m_peach;
}

void StudentWorld::addActor(Actor* actor)
{
    if (actor->priority())
        m_numSpecialActors++;

    if (m_actors.size() == 0)
    {
        m_actors.push_back(actor);
        return;
    }

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

bool StudentWorld::isBlockingAt(double x, double y) const
{
    bool lower;
    vector<Actor*>::const_iterator it;
    it = m_actors.begin() + m_numSpecialActors;

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
            if ((*horizontalIt)->terrain())
                return true;
        }
        if (horizontalIt == m_actors.begin())
            break;
        horizontalIt--;
    }
    // Check for right overlapping
    horizontalIt = it + 1;
    while (horizontalIt < m_actors.end() && overlap(y, y + SPRITE_HEIGHT - 1, (*horizontalIt)->getY(), (*horizontalIt)->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, (*horizontalIt)->getX(), (*horizontalIt)->getX() + SPRITE_WIDTH - 1, lower))
        {
            if ((*horizontalIt)->terrain())
                return true;
        }
        horizontalIt++;
    }
    // No overlapping found
    return false;
}

bool StudentWorld::isDamageableAt(double x, double y) const
{
    bool lower;
    vector<Actor*>::const_iterator it;
    it = m_actors.begin() + m_numSpecialActors;

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
            if (!(*horizontalIt)->friendly() && !(*horizontalIt)->projectile())
                return true;
        }
        if (horizontalIt == m_actors.begin())
            break;
        horizontalIt--;
    }
    // Check for right overlapping
    horizontalIt = it + 1;
    while (horizontalIt < m_actors.end() && overlap(y, y + SPRITE_HEIGHT - 1, (*horizontalIt)->getY(), (*horizontalIt)->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, (*horizontalIt)->getX(), (*horizontalIt)->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (!(*horizontalIt)->friendly() && !(*horizontalIt)->projectile())
                return true;
        }
        horizontalIt++;
    }
    // No overlapping found
    return false;
}


bool StudentWorld::isPlayerAt(double x, double y, const Actor& actor, bool bonk)
{
    bool lower;
    if (overlap(x, x + SPRITE_WIDTH - 1, m_peach->getX(), m_peach->getX() + SPRITE_WIDTH - 1, lower) && overlap(y, y + SPRITE_HEIGHT - 1, m_peach->getY(), m_peach->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (bonk)
            m_peach->getBonked(actor);
        return true;
    }
    return false;
}

void StudentWorld::getPlayerLocation(double& x, double& y) const
{
    x = m_peach->getX();
    y = m_peach->getY();
}

bool StudentWorld::bonkAt(double x, double y, const Actor& actor) 
{
    bool lower;
    vector<Actor*>::const_iterator it;
    it = m_actors.begin();
    // Iterate through prioritized Actors (fireballs, goodies, etc.)
    for (int i = 0; i < m_numSpecialActors; i++)
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, (*it)->getX(), (*it)->getX() + SPRITE_WIDTH - 1, lower) && overlap(y, y + SPRITE_HEIGHT - 1, (*it)->getY(), (*it)->getY() + SPRITE_HEIGHT - 1, lower))
        {
            if ((*it) != &actor)
            {
                (*it)->getBonked(actor);
                return true;
            }
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
            if ((*horizontalIt) != &actor)
            {
                (*horizontalIt)->getBonked(actor);
                return true;
            }
        }
        if (horizontalIt == m_actors.begin())
            break;
        horizontalIt--;
    }
    // Check for right overlapping
    horizontalIt = it + 1;
    while (horizontalIt < m_actors.end() && overlap(y, y + SPRITE_HEIGHT - 1, (*horizontalIt)->getY(), (*horizontalIt)->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, (*horizontalIt)->getX(), (*horizontalIt)->getX() + SPRITE_WIDTH - 1, lower))
        {
            if ((*horizontalIt) != &actor)
            {
                (*horizontalIt)->getBonked(actor);
                return true;
            }
        }
        horizontalIt++;
    }
    // No overlapping found
    return false;
}

void StudentWorld::givePowerup(int powerup)
{
    switch (powerup)
    {
        case 1:
            m_peach->giveJumpPower();
            break;
        case 2:
            m_peach->giveShootPower();
            break;
        case 3:
            m_peach->giveStarPower();
            break;
    }
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

vector<Actor*>::const_iterator StudentWorld::actorBinarySearch(double y, const vector<Actor*>& actors, vector<Actor*>::const_iterator start, vector<Actor*>::const_iterator end) const
{
    if (start <= end)
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