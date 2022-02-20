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
    int i;
    for (i = 0; i < m_actors.size(); i++)
    {
        if (m_actors[i]->alive())
        {
            m_actors[i]->doSomething();
            if (!m_peach->alive())
            {
                playSound(SOUND_PLAYER_DIE);
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
            if (m_actors[i]->goalpost() && !m_actors[i]->alive())
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
    
    vector<Actor*>::iterator it;
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

    ostringstream oss;
    oss << "Lives: " << getLives() << "  Level: " << getLevel() << "  Points: " << getScore();
    if (m_peach->getStarPower())
        oss << " StarPower!";
    if (m_peach->getShootPower())
        oss << " ShootPower!";
    if (m_peach->getJumpPower())
        oss << " JumpPower!";
    setGameStatText(oss.str());
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
    int i = 0;

    // Use binary search based on vertical position to find the actor blocking 
    i = actorBinarySearch(y, m_actors, i, m_actors.size() - m_numSpecialActors);
    if (i == -1) // No vertical matches 
        return false;

    // At this point, it points to a position in actor that overlaps VERTICALLY; since Actors is sorted vertically, all that needs to be checked is the horizontally overlapping of all 
    // Actors relative to it (because it merely found the vertical matches with the binary search it is still possible that there could be vertical matches to the left or right)
    int j = i;
    // Check for left overlapping
    while (j >= 0 && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (m_actors[j]->terrain())
                return true;
        }
        j--;
    }
    // Check for right overlapping
    j = i + 1;
    while (j < m_actors.size() - m_numSpecialActors && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (m_actors[j]->terrain())
                return true;
        }
        j++;
    }
    // No overlapping found
    return false;
}


bool StudentWorld::isDamageableAt(double x, double y) const
{
    bool lower;
    int i = 0;

    // Use binary search based on vertical position to find the actor blocking 
    i = actorBinarySearch(y, m_actors, i, m_actors.size() - m_numSpecialActors);
    if (i == -1) // No vertical matches 
        return false;

    // At this point, it points to a position in actor that overlaps VERTICALLY; since Actors is sorted vertically, all that needs to be checked is the horizontally overlapping of all 
    // Actors relative to it (because it merely found the vertical matches with the binary search it is still possible that there could be vertical matches to the left or right)
    int j = i;
    // Check for left overlapping
    while (j >= 0 && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (!m_actors[j]->friendly() && !m_actors[j]->projectile() && m_actors[j]->alive())
                return true;
        }
        j--;
    }
    // Check for right overlapping
    j = i + 1;
    while (j < m_actors.size() - m_numSpecialActors && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (!m_actors[j]->friendly() && !m_actors[j]->projectile() && m_actors[j]->alive())
                return true;
        }
        j++;
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
    bool bonked = false;
    bool lower;
    int i;
    // Iterate through prioritized Actors (fireballs, goodies, etc.)
    for (i = m_actors.size() - m_numSpecialActors; i < m_actors.size(); i++)
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[i]->getX(), m_actors[i]->getX() + SPRITE_WIDTH - 1, lower) && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[i]->getY(), m_actors[i]->getY() + SPRITE_HEIGHT - 1, lower))
        {
            if (m_actors[i] != &actor)
            {
                m_actors[i]->getBonked(actor);
                bonked = true;
            }
        }
    }

    // Use binary search based on vertical position to find the actor blocking 
    i = actorBinarySearch(y, m_actors, 0, m_actors.size() - m_numSpecialActors);
    if (i == -1) // No vertical matches 
        return false;

    // At this point, it points to a position in actor that overlaps VERTICALLY; since Actors is sorted vertically, all that needs to be checked is the horizontally overlapping of all 
    // Actors relative to it (because it merely found the vertical matches with the binary search it is still possible that there could be vertical matches to the left or right)
   int j = i;
    // Check for left overlapping
    while (j >= 0 && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (m_actors[j] != &actor)
            {
                m_actors[j]->getBonked(actor);
                bonked = true;
            }
        }
        j--;
    }
    // Check for right overlapping
    j = i + 1;
    while (j < m_actors.size() - m_numSpecialActors && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (m_actors[j] != &actor)
            {
                m_actors[j]->getBonked(actor);
                bonked = true;
            }
        }
        j++;
    }
    // No overlapping found
    return bonked;
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

int StudentWorld::actorBinarySearch(double y, const vector<Actor*>& actors, int start, int end) const
{
    if (start <= end)
    {
        bool lower;
        int mid = start + (end - start) / 2;
        if (overlap(y, y + SPRITE_HEIGHT - 1, m_actors[mid]->getY(), m_actors[mid]->getY() + SPRITE_HEIGHT - 1, lower))
            return mid;
        else if (lower)
            return actorBinarySearch(y, actors, start, mid - 1);
        else
            return actorBinarySearch(y, actors, mid + 1, end);
    }
    return -1;
}