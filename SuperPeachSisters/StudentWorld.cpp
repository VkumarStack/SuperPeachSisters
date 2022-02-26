#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
    : GameWorld(assetPath)
{
    // Initialize member variables 
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
        // Populate m_actors with appropriate actors from level file 
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
        return GWSTATUS_LEVEL_ERROR;
    }

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    // Using an int counter rather than an iterator since doSomething() can, in some instances, add result in new actors being added to m_actors,
    // which may invalidate the iterator. Since all potentially new actors that spawn can either be Shells, Fireballs, or Goodies, though, they 
    // are guaranteed to be at the end of m_actors due to how the < operator is defined, so no adjustment of i must be made in that case 
    int i;
    for (i = 0; i < m_actors.size(); i++)
    {
        if (m_actors[i]->alive())
        {
            m_actors[i]->doSomething();
            if (!m_peach->alive()) // If Peach died during the actor's doSomething(), play the appropriate sound and decrement the player's lives 
            {
                playSound(SOUND_PLAYER_DIE);
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
            if (m_actors[i]->goalpost() && !m_actors[i]->alive()) // If a Flag or Mario were reached, it would be set dead, implying that the level is finished 
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
    m_peach->doSomething(); // Allow Peach to do something 
    
    // Erase all dead Actors from vector 
    vector<Actor*>::iterator it;
    it = m_actors.begin();
    while (it != m_actors.end())
    {
        if (!(*it)->alive())
        {
            if ((*it)->priority()) // If the Actor was a priority actor, make sure to decrement m_numSpecialActors counter 
                m_numSpecialActors--; 
            delete* it;
            it = m_actors.erase(it);
        }
        else
            it++;
    }

    // Display appropriate text 
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
    // Delete all actors and Peach 
    vector<Actor*>::iterator it;
    it = m_actors.begin();
    while (it != m_actors.end())
    {
        delete* it;
        it = m_actors.erase(it);
    }
    delete m_peach;
    m_numSpecialActors = 0;
}

void StudentWorld::addActor(Actor* actor)
{
    if (actor->priority()) // If the actor being added is a priority actor, make sure to increment the priority actor counter 
        m_numSpecialActors++;

    if (m_actors.size() == 0) // Size zero, just push_back 
    {
        m_actors.push_back(actor);
        return;
    }

    // Insert such that m_actors is in ascending order as defined by < 
    vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if (*actor < *(*it))
        {
            m_actors.insert(it, actor);
            return;
        }
    }
    m_actors.insert(it, actor); // In case of no insertion occurring in loop - probably redundant though
}

// If there is a blockable object overlapping at a given (x, y) coordinate 
bool StudentWorld::isBlockingAt(double x, double y) const
{
    return blockOrDamageAt(x, y, true);
}

// If there is a damageable object overlapping at a given (x, y) coordinate 
bool StudentWorld::isDamageableAt(double x, double y) const
{
    return blockOrDamageAt(x, y, false);
}

// Determine if Peach is overlapping at a given (x, y,) coordinate; pass in true for the last parameter to also bonk Peach if she is indeed overlapping 
bool StudentWorld::isPlayerAt(double x, double y, const Actor& actor, bool bonk)
{
    bool lower;
    if (overlap(x, x + SPRITE_WIDTH - 1, m_peach->getX(), m_peach->getX() + SPRITE_WIDTH - 1, lower) && overlap(y, y + SPRITE_HEIGHT - 1, m_peach->getY(), m_peach->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (bonk)
            m_peach->bonk(actor);
        return true;
    }
    return false;
}

void StudentWorld::getPlayerLocation(double& x, double& y) const
{
    x = m_peach->getX();
    y = m_peach->getY();
}

// Bonk an object at a given location 
bool StudentWorld::bonkAt(double x, double y, const Actor& actor) 
{
    bool bonked = false;
    bool lower;
    int i;
    // Iterate through prioritized Actors (fireballs, goodies, etc.)
    // There are m_numSpecialActors prioritized actors, and they are guaranteed to all be at the end of the Vector, so start looping from 
    // m_actors.size() - m_numSpecialActors and go to the end 
    for (i = m_actors.size() - m_numSpecialActors; i < m_actors.size(); i++)
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[i]->getX(), m_actors[i]->getX() + SPRITE_WIDTH - 1, lower) && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[i]->getY(), m_actors[i]->getY() + SPRITE_HEIGHT - 1, lower))
        {
            if (m_actors[i] != &actor) // Check to ensure that the actor doing the bonking is not bonking itself (no self bonking)
            {
                m_actors[i]->bonk(actor);
                bonked = true;
            }
        }
    }

    // Use binary search based on vertical position to search for remaining actors at the given vertical position  
    i = actorBinarySearch(y, m_actors, 0, m_actors.size() - m_numSpecialActors); // The end index if m_actors.size() - m_numSpecialActors since binary search cannot be performed with specialActors (hence why they were iterated through manually)
    if (i == -1) // No vertical matches 
        return false;

    // At this point, i points to a position in actor that overlaps VERTICALLY; since Actors is sorted vertically, all that needs to be checked is the horizontally overlapping of all 
    // Actors relative to it (because it merely found the vertical matches with the binary search it is still possible that there could be vertical matches to the left or right of i)
    int j = i;
    // Check for left overlapping
    while (j >= 0 && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (m_actors[j] != &actor)
            {
                m_actors[j]->bonk(actor);
                bonked = true;
            }
        }
        j--;
    }
    // Check for right overlapping
    j = i + 1; // i + 1 since i was already checked 
    while (j < m_actors.size() - m_numSpecialActors && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (m_actors[j] != &actor)
            {
                m_actors[j]->bonk(actor);
                bonked = true;
            }
        }
        j++;
    }
    
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

bool StudentWorld::overlap(double start1, double end1, double start2, double end2, bool& lower /*if (start1, end1) is lower than (start2, end2) - to be used for binary search*/) const
{
    // Overlapping occurs if both ends of each coordinate are greater than the respective starts of the other coordinates 
    if ((end1 >= start2) && (end2 >= start1))
        return true;
    // No overlapping occured if this is reached, but still needs to check if (start1, end1) is relatively lower than (start2, end2)
    if (end1 < start2)
        lower = true;
    else
        lower = false;
    return false;
}

// Standard binary search algorithm, using the overlap() method to determine which half to search 
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

// Same algorithm as bonkAt(), however special actors are not iterated through since they (Goodies, Fireballs, Shells) are guaranteed to never 
// block or be damageable
bool StudentWorld::blockOrDamageAt(double x, double y, bool block) const
{
    bool lower;
    int i = 0;

    // Use binary search based on vertical position to find the actor blocking 
    i = actorBinarySearch(y, m_actors, i, m_actors.size() - m_numSpecialActors);
    if (i == -1) // No vertical matches 
        return false;

    // At this point, i points to a position in actor that overlaps VERTICALLY; since Actors is sorted vertically, all that needs to be checked is the horizontally overlapping of all 
    // Actors relative to it (because it merely found the vertical matches with the binary search it is still possible that there could be vertical matches to the left or right of i)
    int j = i;
    // Check for left overlapping
    while (j >= 0 && overlap(y, y + SPRITE_HEIGHT - 1, m_actors[j]->getY(), m_actors[j]->getY() + SPRITE_HEIGHT - 1, lower))
    {
        if (overlap(x, x + SPRITE_WIDTH - 1, m_actors[j]->getX(), m_actors[j]->getX() + SPRITE_WIDTH - 1, lower))
        {
            if (block)
            {
                if (m_actors[j]->terrain())
                    return true;
            }
            else
            {
                if (!m_actors[j]->friendly() && !m_actors[j]->projectile() && m_actors[j]->alive())
                    return true;
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
            if (block)
            {
                if (m_actors[j]->terrain())
                    return true;
            }
            else
            {
                if (!m_actors[j]->friendly() && !m_actors[j]->projectile() && m_actors[j]->alive())
                    return true;
            }
        }
        j++;
    }
    // No overlapping found
    return false;
}