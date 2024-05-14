// Current Task
// Add Sound effects

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <pthread.h>
#include <atomic>
#include <time.h>     
#include <semaphore.h>

#define s std
#define height 27
#define width 28
#define cellSize 24   // each block on the game grid corresponds to a 16*16 cell 

const int numOfPowerPellets = 4;
const int numOfGhosts = 4;
const int coins = 7;

// Declare all the Global Variables here for the all the threads to access
std::atomic<bool> exit_thread_flag{false}, gameWon{false}, gameOver{false}, pauseGame{false};

sf::RenderWindow window;
sf::RectangleShape rectangle;

sf::Texture pacManTex, pacManDeathTex, powerPelletTex, ghostTex[numOfGhosts], blueGhosts, coinTex;
sf::Texture mainMenuTex, rectTex;

sf::Sprite pacManSprite,pacManDeathSprite, powerPelletSprite[numOfPowerPellets], ghostSprite[numOfGhosts];
sf::Sprite pacManLifeSprite, mainMenuSprite, ghostHouseSprite, coinSprite;

sf::SoundBuffer pacManDeathSoundBuffer, pacManEatGhostSoundBuffer, pacManChompSoundBuffer, pacManStartSoundBuffer;

//pthread_mutex_t waitForRender, waitForPacMan, waitForGameEngine, waitForInput, waitForDraw;
// pthread_mutex_t waitForGhost[numOfGhosts], waitForGameEngine1[numOfGhosts],
pthread_mutex_t powerPellet, consumeBoost, permitCheck, pthread_mutex, checkGhostHouseGateAccess;
sem_t waitForInput, waitForPacMan, waitForGameEngine, waitForDraw, waitForRender;
sem_t waitForGhost[numOfGhosts], waitForGameEngine1[numOfGhosts];

int numOfPermits, numOfKeys, direction, lives, currentPowerPellets, numOfBoosts, coinsPickedUp, currScore;
bool powerUp = false, ghostHouseGateAccess = true;
int readCount = 0;      // keeps track of the number of ghosts reading the maze at a time

// boolean variables related to te functionality of the ghosts
// appearing as blue and white when the pacman eats a power up
bool initialState = true, blueGhostOnly = true;

sem_t ghostMutex, mazeAccess; // semaphores used to address the Reader/Writer 
                              // scenario in the context of the PacMan and the Ghosts

int powerPelletLoc[numOfPowerPellets][2] = { {cellSize,cellSize},
                                             {26*cellSize,cellSize},
                                             {cellSize,25*cellSize},
                                             {26*cellSize,25*cellSize} };

int ghostStartingLoc[numOfGhosts][2] = {{12*cellSize,12*cellSize},
                                        {12*cellSize,14*cellSize},
                                        {15*cellSize,12*cellSize},
                                        {15*cellSize,14*cellSize} };                                             

// the underlying 2D Maze
int maze[height][width] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1},
  {1,0,0,0,0,0,0,1,0,0,0,3,3,3,3,3,3,0,0,0,1,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,1,0,0,0,1,2,2,2,2,1,0,0,0,1,0,0,0,0,0,0,1},
  {1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1},
  {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1},
  {1,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,1,4,4,4,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
  {1,0,0,1,1,0,0,1,1,1,4,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,4,4,4,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1} };  

// class to hold info about a ghost, used primarily for 
// passing data onto the cleanup handler of a ghost
class info1 {
public:
  int ghostNum;
  bool hasBoost = false, hasKey = false, hasPermit = false, hasGhostHouseGateAccess = false;
};

// function called when the ghost thread is cancelled
void ghostThreadCleanupHandler(void* arg) {
  info1* infoBlock = (info1*)arg;
  
  if(infoBlock != NULL) {
    // if the cancelled ghost has an active boost then release it
    if(infoBlock->hasBoost == true) {
      pthread_mutex_lock(&consumeBoost);
      s::cout<<"Ghost "<<infoBlock->ghostNum<<" has dropped the boost after dieing\n";
        //s::cout<<"In the Cleanup Handler of the "<<infoBlock->ghostNum<<" Ghost\n";
        //s::cout<<"Freeing up the Boost\n";
        ++numOfBoosts;
      pthread_mutex_unlock(&consumeBoost);
    }
    // if the cancelled ghost has a permit or a key or both then release them accordingly
    if(infoBlock->hasPermit || infoBlock->hasKey) {
      pthread_mutex_lock(&permitCheck);
      if(infoBlock->hasPermit) 
        ++numOfPermits;
      if(infoBlock->hasKey) 
        ++numOfKeys;
      s::cout<<"Ghost "<<infoBlock->ghostNum<<" has dropped permit and key after dieing\n";
      pthread_mutex_unlock(&permitCheck);
    }
    // if the cancelled ghost has access to the ghost house gate
    if(infoBlock->hasGhostHouseGateAccess) {
      pthread_mutex_lock(&checkGhostHouseGateAccess);
      ghostHouseGateAccess = true;
      s::cout<<"Ghost "<<infoBlock->ghostNum<<" has give up the access to ghost house gate\n";
      pthread_mutex_unlock(&checkGhostHouseGateAccess);
    }
    delete infoBlock;
    infoBlock = NULL;
  }

  return;
}

// ghost thread function
void* ghost(void* anything) {

  // set the function to call when the ghost thread is cancelledS
  pthread_cleanup_push(ghostThreadCleanupHandler, anything);

  // set the relevant cancel states for the thread
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  info1* temp1 = (info1*)anything;
  int ghostNum = temp1->ghostNum;  
  
  // set the initial starting position
  ghostSprite[ghostNum].setPosition(ghostStartingLoc[ghostNum][0],ghostStartingLoc[ghostNum][1]);
  
  int numOfMoves = 0, direction = 0, xPos, yPos, ghostSpriteFrame = 0;
  sf::Vector2f currPos;

  // variables used for the purpose of displaying blue
  // and white ghosts when the pacman eats the power up
  bool check = false, hasBoost = false, withinHouse = true, haveKey = false, havePermit = false;
  bool haveGhostHouseGateAccess = false;
  int blueGhostSpriteFrame = 0;
  
  sf::Clock startDelay, permitDelay;

  while(!exit_thread_flag) {
    
    if(pauseGame)
      continue;

    if(startDelay.getElapsedTime().asSeconds() > 3) {
      
      // if the ghost is within the house and does not have a permit or key or both
      if(withinHouse == true && (havePermit == false || haveKey == false || haveGhostHouseGateAccess == false)) {
        if(havePermit == false || haveKey == false) {
          pthread_mutex_lock(&permitCheck);
          // check if there any available permits
          if(numOfPermits > 0) {
            // first check if the current ghost needs a permit
            if(havePermit == false) {
              --numOfPermits;
              havePermit = true;
              temp1->hasPermit = true;
              s::cout<<"Ghost "<<temp1->ghostNum<<" has picked up a permit\n";
            }
            // then check if the current ghost needs a key
            // if(numOfKeys > 0 && haveKey == false) {
            //   --numOfKeys;
            //   haveKey = true;
            //   temp1->hasKey = true;
            //   s::cout<<"Ghost "<<temp1->ghostNum<<" has picked up a key\n";
            // }  
          }
          if(havePermit == true && numOfKeys > 0 && haveKey == false) {
              --numOfKeys;
              haveKey = true;
              temp1->hasKey = true;
              s::cout<<"Ghost "<<temp1->ghostNum<<" has picked up a key\n";
          }
          pthread_mutex_unlock(&permitCheck);
        }
        if(havePermit == true && haveKey == true && haveGhostHouseGateAccess == false ) {
          pthread_mutex_lock(&checkGhostHouseGateAccess);
          if(ghostHouseGateAccess == true) {
            haveGhostHouseGateAccess = true;
            ghostHouseGateAccess = false;
            s::cout<<"Ghost "<<temp1->ghostNum<<" has picked up the gate access";
            temp1->hasGhostHouseGateAccess = true;
          }
          pthread_mutex_unlock(&checkGhostHouseGateAccess);
        }
      }
      // ghost relinquishes the permit and the key once it is out of the house
      else if(withinHouse == false && havePermit && haveKey && permitDelay.getElapsedTime().asSeconds() > 3) {
        pthread_mutex_lock(&permitCheck);
          havePermit = haveKey = false;
          ++numOfPermits;
          ++numOfKeys;
          temp1->hasPermit = temp1->hasKey = false;
          s::cout<<"Ghost "<<temp1->ghostNum<<" has given up the permit and key\n";
        pthread_mutex_unlock(&permitCheck);
      }

      // give up the ghost house gate access once out of the house
      if(withinHouse == false && haveGhostHouseGateAccess == true) {
        pthread_mutex_lock(&checkGhostHouseGateAccess);
        ghostHouseGateAccess = true;
        haveGhostHouseGateAccess = false;
        s::cout<<"Ghost "<<temp1->ghostNum<<" has given up the ghost house key\n";
        temp1->hasGhostHouseGateAccess = false;
        pthread_mutex_unlock(&checkGhostHouseGateAccess);
      }

      // check if any boost is available 
      if(!hasBoost && !withinHouse) {
        pthread_mutex_lock(&consumeBoost);
        if(numOfBoosts > 0) {
          hasBoost = true;
          --numOfBoosts;
          temp1->hasBoost = 1;
          s::cout<<"Ghost "<<temp1->ghostNum<<" has acquired the boost\n";
        }
        pthread_mutex_unlock(&consumeBoost);
      }

      // if(withinHouse && havePermit && haveKey && haveGhostHouseGateAccess) {
        
      //   ghostSprite[ghostNum].move(0, -cellSize);

      //   continue;
      // }

      // basic Movement Generator for the Ghost
      if(numOfMoves == 0) {
        if(withinHouse && haveGhostHouseGateAccess == false) {
          numOfMoves = 1;
          direction = -1;
        }
        else {
          numOfMoves = (rand()%5) + 1;
          if(withinHouse && haveGhostHouseGateAccess)
            direction = 1;
          else
            direction = (rand()%4) + 1;        
        }
      }
      --numOfMoves;
      if(numOfMoves > 0 && hasBoost == true)
        --numOfMoves;

      // get the current position of the ghost
      currPos = ghostSprite[ghostNum].getPosition();
      // translate the coordinates with respect to the set cell size in the gird
      xPos = (currPos.x/cellSize); 
      yPos = (currPos.y/cellSize);
        
      sem_wait(&ghostMutex);
      // increment the number of ghosts accessing the maze for read
      ++readCount;
      // ensure that if a Ghost has accessed the maze
      // the PacMan has to wait then
      if(readCount == 1)
        sem_wait(&mazeAccess);
      sem_post(&ghostMutex);
        
      // set the relevant textures for the Ghost Sprite
      if(initialState == false && check == false) {
        ghostSprite[ghostNum].setTexture(blueGhosts);
        check = true;
      }
      else if(initialState == true && check == true) {
        ghostSprite[ghostNum].setTexture(ghostTex[ghostNum]);
        check = false;
      }
      // set the relevant sprites for the Ghost Sprite
      if(initialState == false) {
        ghostSprite[ghostNum].setTextureRect(sf::IntRect(blueGhostSpriteFrame*16,0,16,16));
        if(blueGhostOnly) 
          blueGhostSpriteFrame = (blueGhostSpriteFrame+1)%2;
        else
          blueGhostSpriteFrame = (blueGhostSpriteFrame+1)%4;
      }
      
      if(direction == 1) {
      
        if(maze[yPos-1][xPos] != 1 && (maze[yPos-1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))   ) {
          ghostSprite[ghostNum].move(0, -cellSize);
          if(maze[yPos-1][xPos] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          } 
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }  
      
        else if(maze[yPos+1][xPos] != 1 && (maze[yPos+1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess)) ) {
          ghostSprite[ghostNum].move(0, cellSize);
          if(maze[yPos+1][xPos] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }          
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }
      
        else if(maze[yPos][xPos-1] != 1 && (maze[yPos][xPos-1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(-cellSize, 0);
          if(maze[yPos][xPos-1] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(32 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }
      
        else if(maze[yPos][xPos+1] != 1 && (maze[yPos][xPos+1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(cellSize, 0);
          if(maze[yPos][xPos+1] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;      
          } 
        }
      
      }
      
      else if(direction == 2) {
      
        if(maze[yPos+1][xPos] != 1 && (maze[yPos+1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(0, cellSize);
          if(maze[yPos+1][xPos] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }        
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }
      
        else if(maze[yPos-1][xPos] != 1 && (maze[yPos-1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(0, -cellSize);
          if(maze[yPos-1][xPos] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2; 
          }
        }
      
        else if(maze[yPos][xPos-1] != 1 && (maze[yPos][xPos-1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(-cellSize, 0);
          if(maze[yPos][xPos-1] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(32 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }
      
        else if(maze[yPos][xPos+1] != 1 && (maze[yPos][xPos+1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(cellSize, 0);
          if(maze[yPos][xPos+1] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
          }
          if(initialState) { 
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }
      
      }

      else if(direction == 3) {

        if(maze[yPos][xPos-1] != 1 && (maze[yPos][xPos-1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(-cellSize, 0);
          if(maze[yPos][xPos-1] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(32 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }

        else if(maze[yPos+1][xPos] != 1 && (maze[yPos+1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(0, cellSize);
          if(maze[yPos+1][xPos] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) { 
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }

        else if(maze[yPos-1][xPos] != 1 && (maze[yPos-1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(0, -cellSize);
          if(maze[yPos-1][xPos] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }            
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }

        else if(maze[yPos][xPos+1] != 1 && (maze[yPos][xPos+1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(cellSize, 0);       
          if(maze[yPos][xPos+1] == 3 && withinHouse) {
            withinHouse = true;
            permitDelay.restart();
          }
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }     
      
      }

      else if(direction == 4) {

        if(maze[yPos][xPos+1] != 1 && (maze[yPos][xPos+1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(cellSize, 0);
          if(maze[yPos][xPos+1] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }

        else if(maze[yPos+1][xPos] != 1 && (maze[yPos+1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(0, cellSize);
          if(maze[yPos+1][xPos] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }

        else if(maze[yPos-1][xPos] != 1 && (maze[yPos-1][xPos] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(0, -cellSize);
          if(maze[yPos-1][xPos] == 3 && withinHouse)
            withinHouse = false;
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }

        else if(maze[yPos][xPos-1] != 1 && (maze[yPos][xPos-1] != 2 || (havePermit && haveKey && withinHouse && haveGhostHouseGateAccess))) {
          ghostSprite[ghostNum].move(-cellSize, 0);
          if(maze[yPos][xPos-1] == 3 && withinHouse) {
            withinHouse = false;
            permitDelay.restart();
          }
          if(hasBoost == true) {
            currPos = ghostSprite[ghostNum].getPosition();
            xPos = (currPos.x/cellSize); 
            yPos = (currPos.y/cellSize);
            if(maze[yPos][xPos-1] != 1)
              ghostSprite[ghostNum].move(-cellSize, 0);
            else if(maze[yPos+1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, cellSize);
            else if(maze[yPos-1][xPos] != 1) 
              ghostSprite[ghostNum].move(0, -cellSize);
            else if(maze[yPos][xPos+1] != 1)
              ghostSprite[ghostNum].move(cellSize, 0);
          }
          if(initialState) {
            ghostSprite[ghostNum].setTextureRect(sf::IntRect(32 + 16*ghostSpriteFrame,0,16,16));
            ghostSpriteFrame = (ghostSpriteFrame+1)%2;
          }
        }
      
      }

      sem_wait(&ghostMutex);
      // decrement the number of ghosts accessing the maze for read purposes
      --readCount;
      // ensure that the PacMan can only then access the maze 
      // if there is no active ghost currently accessing the maze
      if(readCount == 0)
        sem_post(&mazeAccess);
      sem_post(&ghostMutex);
        
        // if(!temp) {
        //   s::cout<<"Out of House\n";
        //   temp = true;
        //  }
      //}
    }
    sem_post(&waitForGhost[ghostNum]);
    sem_wait(&waitForGameEngine1[ghostNum]);
  
  }
  
  if(hasBoost == true) {
    pthread_mutex_lock(&consumeBoost);
    hasBoost = false;
    ++numOfBoosts;
    s::cout<<"Game is closing down. Ghost "<<ghostNum<<" is freeing up the boost accordingly\n";
    pthread_mutex_unlock(&consumeBoost);
  }

  if(havePermit || haveKey){
    pthread_mutex_lock(&permitCheck);
    if(havePermit) {
      havePermit = false;
      ++numOfPermits;
      if(temp1 != NULL)
        temp1->hasPermit = false;
    }
    if(haveKey) {
      haveKey = false;
      ++numOfKeys;
      if(temp1 != NULL)
        temp1->hasKey = false;
    }
    pthread_mutex_unlock(&permitCheck);
  }


  if(temp1 != NULL) {      
    delete temp1;
    temp1 = NULL;
  }

  // DO NOT ANY OF THE COMMENTS BELOW
  // DO NOT CHANGE ANYTHING BELOW. THINGS MAY END UP BREAKING
  // int temp = 0;
  // DO NOT REMOVE THE BELOW COUT STATEMENT
  // IT DOES NOT PRINT ANYTHING HOWEVER IT SOMEHOW PREVENTS A MASSIVE ERROR
  // IT SOMEHOW ENSURES THAT THE CLEAN UP ROUTINE RUNS PROPERLY
  // s::cout<<"Here\n"; 
  // pthread_cleanup_pop(cleanup_pop_arg);
  // pthread_cleanup_pop(1);
  pthread_cleanup_pop(0);
  // s::cout<<"Do we reach here in case of pop";
  pthread_exit(NULL);
}

// the pacMan thread
// responsible for everything pacMan related
// shall move the PacMan with respect to Walls 
void* pacMan(void* anything) {

  // set the relevant cancel states for the thread
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  // set the initial starting position
  pacManSprite.setPosition(cellSize*13,cellSize*16);
  // set the initial sprite
  pacManSprite.setTextureRect(sf::IntRect(16,16,16,16));

  int localDirection, xPos, yPos, spriteFrame = 0;
  
  sf::Vector2f currPos;
  
  sf::Sound pacManChompSound;
  pacManChompSound.setBuffer(pacManChompSoundBuffer);
  bool soundOn = false;
  int iter = 0;

  // To keep track of the time for the effect of the Power Pellet
  sf::Clock powerUpClock, initialDelayClock;

  while(!exit_thread_flag) {

    // if(initialDelayClock.getElapsedTime().asSeconds() < 2 && !exit_thread_flag) 
    //   continue;
    
    if(pauseGame)
      continue;

    // wait for the UI Thread to Update the value of direction 
    // with respect to the input
    sem_wait(&waitForInput);
        
    localDirection = direction;
    if(localDirection) {
      // get the current position of the pac man
      currPos = pacManSprite.getPosition();
      // translate the coordinates with respect to
      // the size of each cell in the set grid
      xPos = (currPos.x/cellSize); 
      yPos = (currPos.y/cellSize);
      
      sem_wait(&mazeAccess);
      if(direction == 1) {
        if(maze[yPos-1][xPos] != 1 && maze[yPos-1][xPos] != 2) {
          ++iter;
          if(soundOn == false || iter >= 5) {
            pacManChompSound.play(); 
            iter = 0;
            soundOn = true;
          }
          if(maze[yPos-1][xPos] == 4) {
            ++coinsPickedUp;
            maze[yPos-1][xPos] = 0;
            currScore += 10;
          }
          pacManSprite.move(0, -cellSize);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,16,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
        else {
          pacManChompSound.stop();
          iter = 0;
          soundOn = false;
        }
      }
      else if(direction == 2) {
        if(maze[yPos+1][xPos] != 1 && maze[yPos+1][xPos] != 2 ) {
          ++iter;
          if(soundOn == false || iter >= 5) {
            pacManChompSound.play();
            iter = 0;
            soundOn = true;
          }
          if(maze[yPos+1][xPos] == 4) {
            ++coinsPickedUp;
            maze[yPos+1][xPos] = 0;
            currScore += 10;
          }
          pacManSprite.move(0, cellSize);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,48,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
        else {
          pacManChompSound.stop();
          iter = 0;
          soundOn = false;
        }
      }
      else if(direction == 3) {
        if(maze[yPos][xPos-1] != 1 && maze[yPos][xPos-1] != 2 ) {
          ++iter;
          if(soundOn == false || iter >= 5) {
            pacManChompSound.play();
            soundOn = true;
            iter = 0;
          }
          if(maze[yPos][xPos-1] == 4) {
            ++coinsPickedUp;
            maze[yPos][xPos-1] = 0;
            
          }          
          pacManSprite.move(-cellSize, 0);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,32,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
        else {
          pacManChompSound.stop();
          iter = 0;
          soundOn = false;
        }
      }
      else if(direction == 4) {
        if(maze[yPos][xPos+1] != 1 && maze[yPos][xPos+1] != 2) {
          ++iter;
          if(soundOn == false || iter >= 5) {
            pacManChompSound.play();
            iter = 0;
            soundOn = true;
          }
          if(maze[yPos][xPos+1] == 4) {
            ++coinsPickedUp;
            maze[yPos][xPos+1] = 0;
            currScore += 10;
          }          
          pacManSprite.move(cellSize, 0);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,0,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
        else {
          pacManChompSound.stop();
          iter = 0;
          soundOn = false;
        }
      }
      else 
        pacManChompSound.stop();
      sem_post(&mazeAccess);

    }

    if(!powerUp) {
      pthread_mutex_lock(&powerPellet);
      if(currentPowerPellets > 0) {
        for(int i = 0; i < numOfPowerPellets; ++i) {
          
          if(powerPelletSprite[i].getPosition() == pacManSprite.getPosition()) {
            
            // reset the power pellet position after consuming it
            powerPelletSprite[i].setPosition(-20, -20);
            // s::cout<<"Power Pellet Acquired\n";        
            // consume the power pellet
            powerUp = true;
            --currentPowerPellets;
            powerUpClock.restart();
            
            break;          
          }
        }

      }      
      pthread_mutex_unlock(&powerPellet);
    }
    else {
      if(powerUpClock.getElapsedTime().asSeconds() > 10) {
        // s::cout<<"Lost The Power Up\n";
        powerUp = false;
      }
    }

    // signal the game engine thread that it can proceed further
    sem_post(&waitForPacMan);
  }
  pacManChompSound.stop();

  pthread_exit(NULL);
}

void collisionAnimation() {

  sf::Sound pacManDeathSound;
  pacManDeathSound.setBuffer(pacManDeathSoundBuffer);
  pacManDeathSound.play();

  pacManDeathSprite.setPosition(pacManSprite.getPosition());

  sf::Clock delay;
  
  for(int i = 0; i < 12; ++i) {
    window.clear(sf::Color::Black);
    // Display the grid
    for(int i = 0; i < height; ++i) {
      for(int j = 0; j < width; ++j) {
        if(maze[i][j] == 1) {
          rectangle.setPosition(cellSize*j,cellSize*i);
          window.draw(rectangle); 
        }
        else if(maze[i][j] == 2) {
          ghostHouseSprite.setPosition(cellSize*j, cellSize*i);
          window.draw(ghostHouseSprite);
        }
        // else if(maze[i][j] == 2) {
        //   coinSprite.setPosition(16*j, 16*i);
        //   window.draw(coinSprite);
        // }
      }
    }
    pacManDeathSprite.setTextureRect(sf::IntRect(i*16,0,16,16));
    window.draw(pacManDeathSprite);
    window.display();
    // add delay to add the effect of frame-rates 
    delay.restart();
    while(delay.getElapsedTime().asSeconds() < 0.123) {};
  }
   
  // reset the relevant values
  direction = -1;
  pacManSprite.setPosition(-20,-20);
  for(int i = 0; i < numOfGhosts; ++i)
    ghostSprite[i].setPosition(-20,-0);

  pacManDeathSound.stop();

  return;
}

void* gameEngine(void* anything) {

  //int ghostCount = 1;
  
  sem_init(&ghostMutex, 0, 1);
  sem_init(&mazeAccess, 0, 1);

  sem_init(&waitForPacMan, 0, 1);
  sem_wait(&waitForPacMan);
  
  for(int i = 0; i < numOfGhosts; ++i) {
    sem_init(&waitForGhost[i], 0, 1);
    sem_wait(&waitForGhost[i]);
  //}
  //for(int i = 0; i < numOfGhosts; ++i) {
    sem_init(&waitForGameEngine1[i], 0, 1);
    sem_wait(&waitForGameEngine1[i]);
  }

  pthread_mutex_init(&powerPellet, NULL);
  pthread_mutex_init(&consumeBoost, NULL);
  pthread_mutex_init(&permitCheck, NULL);
  pthread_mutex_init(&checkGhostHouseGateAccess, NULL);

  // initialize thread attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  int rc;
  // create the thread for PacMan
  pthread_t pacManThread;
  rc = pthread_create(&pacManThread, &attr, pacMan, NULL);
  if(rc) {
    s::cout<<"Error unable to create PacMan thread. Exiting\n";
    exit(-1);
  }

  // create the threads for the ghosts
  pthread_t ghostThread[numOfGhosts];
  for(int i = 0; i < numOfGhosts; ++i) {
    info1* temp = new info1;  
    temp->ghostNum = i;    
    rc = pthread_create(&ghostThread[i], &attr, ghost, temp);
    if(rc) {
      s::cout<<"Error unable to create Ghost thread. Exiting\n";
      exit(-1);
    }
    temp = NULL;
  }

  // clocks for adding delay and generating power pellets
  sf::Clock delayClock, powerPelletGeneratorTimer, blueGhostTimer1, blueGhostTimer2;
  sf::Vector2f pacManPos, ghostPos, temp(-20,-20);

  sf::Sound pacManEatGhostSound, pacManStartSound;
  pacManEatGhostSound.setBuffer(pacManEatGhostSoundBuffer);
  pacManStartSound.setBuffer(pacManStartSoundBuffer);

  bool restart = false, ghostDead = false, blueGhostSprite = false;
  int powerPelletPos = 0;     // points to the first power pellet that has to be created 
  int ghostToReAnimate = -1;

  pacManStartSound.play();

  while(!exit_thread_flag) {

    if(pauseGame)
      continue;

    // generate power pellets at set intervals
    if(powerPelletGeneratorTimer.getElapsedTime().asSeconds() > 6) {
      pthread_mutex_lock(&powerPellet);
      // if more power pellets can be produced
      if(currentPowerPellets < numOfPowerPellets) {
        // since the pacman can eat the power pellets in any order
        // hence we have to linearly check for the first free power pellet slot
        for(int i = 0; i < numOfPowerPellets; ++i) {
          // use the (-10, -10) position of a power pellet sprite as a 
          // reference to check whether it can be produced or not
          if(powerPelletSprite[powerPelletPos].getPosition() == temp) {
            // set the corresponding position with respect to main maze grid
            powerPelletSprite[powerPelletPos].setPosition(powerPelletLoc[powerPelletPos][0], powerPelletLoc[powerPelletPos][1]);
            powerPelletPos = (powerPelletPos+1)%numOfPowerPellets;
            // reset the timer for next power pellet generation
            powerPelletGeneratorTimer.restart();
            ++currentPowerPellets;
            break;
          }
          else
            powerPelletPos = (powerPelletPos+1)%numOfPowerPellets;
        }
      }
      powerPelletGeneratorTimer.restart();
      pthread_mutex_unlock(&powerPellet);
    }

    // recreate the PacMan and the Ghost Threads after collision
    if(restart) {
      rc = pthread_create(&pacManThread, &attr, pacMan, NULL);
      if(rc) {
        s::cout<<"Error unable to create PacMan thread. Exiting\n";
        exit(-1);
      }
      info1* temp;  
      for(int i = 0; i < numOfGhosts; ++i) {
        temp = new info1;
        temp->ghostNum = i;
        rc = pthread_create(&ghostThread[i], &attr, ghost, temp);
        if(rc) {
          s::cout<<"Error unable to create Ghost thread. Exiting\n";
          exit(-1);
        }
        temp = NULL;
      }
      restart = false;
      ghostDead = false;
      ghostToReAnimate = -1;
    } 
    else if(ghostDead) {
      if(ghostToReAnimate != -1 && ghostToReAnimate < numOfGhosts) {
        info1* temp = new info1;  
        temp->ghostNum = ghostToReAnimate;
        rc = pthread_create(&ghostThread[ghostToReAnimate], &attr, ghost, temp);
        if(rc) {
          s::cout<<"Error unable to create Ghost thread. Exiting\n";
          exit(-1);
        }
        temp = NULL;
      }
      ghostToReAnimate = -1;
      ghostDead = false;
    }
    
    // wait for the PacMan thread to execute
    sem_wait(&waitForPacMan);

    // wait for the Ghost Threads to Execute
    for(int i = 0; i < numOfGhosts; ++i)
      sem_wait(&waitForGhost[i]);

    // conditionals enabling to create the animation of blue and white ghosts 
    // when the pacman picks up a power up
    // this conditional occurs when the pacman pick ups a power up
    // the ghosts become blue
    if(powerUp && initialState) {
      initialState = false;
      blueGhostTimer1.restart();
    }
    // this conditional runs 5 secs after the pacman has eaten a power pellet
    // allows to show blue and white ghosts
    else if(powerUp && blueGhostTimer1.getElapsedTime().asSeconds() > 5 && !initialState && blueGhostOnly) {
      blueGhostTimer2.restart();
      blueGhostOnly = false;
    }
    // this conditional runs 10 secs after the pacman has eaten a power pellet
    // also the ghost to go back to its original sprite
    else if(blueGhostTimer2.getElapsedTime().asSeconds() > 5 && !initialState && !blueGhostOnly) {
      initialState = true;
      blueGhostOnly = true;
    }

    if(coinsPickedUp == coins) {
      s::cout<<"You have won the game\n";
      gameWon = true;
      exit_thread_flag = true;
      restart = true;
    }

    // check for collision in-between PacMan and Ghost
    pacManPos = pacManSprite.getPosition();
    for(int i = 0; i < numOfGhosts && !gameWon; ++i) {
      if( (pacManPos == ghostSprite[i].getPosition()) || pacManSprite.getGlobalBounds().intersects(ghostSprite[i].getGlobalBounds())) {
        
        // destroy the Ghost Threads
        pthread_cancel(ghostThread[i]);
        ghostDead = true;
        ghostToReAnimate = i;
        // if the pacman has a power up enabled then only destroy the ghost
        if(powerUp) {
          pacManEatGhostSound.play();
          break;
        }
        // destroy the PacMan Thread
        pthread_cancel(pacManThread);

        // destroy the rest of the Ghost Threads as well
        for(int j = 0; j < numOfGhosts; ++j) {
          if(j == i)
            continue;
          else {
            pthread_cancel(ghostThread[j]);
          }
        }

        --lives;
        
        collisionAnimation();
        
        // exit all the threads if no lives left        
        if(lives == 0) {
          exit_thread_flag = true;
          gameOver = true;
        }
        restart = true;
        break;
      }
    }

    // signal the ui thread that it can draw the final sprites on to the screen
    sem_post(&waitForGameEngine);
    
    // wait for the ui thread to draw each sprite on to the screen
    sem_wait(&waitForDraw);
    
    // render everything on to the screen
    window.display();
    
    // add the effect of delay
    delayClock.restart();
    while(delayClock.getElapsedTime().asSeconds() < 0.11) {};
    
    // signal the ui thread that everything has been rendered
    sem_post(&waitForRender);
    
    // signal the Ghost Thread to continue execution
    for(int i = 0; i < numOfGhosts; ++i)
      sem_post(&waitForGameEngine1[i]);
  }

  pacManEatGhostSound.stop();
  pacManStartSound.stop();
  
  // destroy thread attributes
  pthread_attr_destroy(&attr);
  
  // destroy the mutexes initialized within the Game Engine Thread
  for(int i = 0; i < numOfGhosts; ++i) {
    sem_destroy(&waitForGhost[i]);
    sem_destroy(&waitForGameEngine1[i]); 
  }
  pthread_mutex_destroy(&powerPellet);
  pthread_mutex_destroy(&consumeBoost);
  pthread_mutex_destroy(&permitCheck);
  pthread_mutex_destroy(&checkGhostHouseGateAccess);

  // destroy the semaphores initialized within the Game Engine Thread
  sem_destroy(&waitForPacMan);
  sem_destroy(&ghostMutex);
  sem_destroy(&mazeAccess);

  // join with the exited ghost threads ensures that all threads exit properly
  for(int i = 0; i < numOfGhosts; ++i)
    pthread_join(ghostThread[i], NULL);

  // join with the exited pacman thread
  pthread_join(pacManThread, NULL);

  // s::cout<<"Checking if the Game Engine Performs all the execution\n";
  // exit the thread
  pthread_exit(NULL);
}

// load the textures
// and initialize the sprites accordingly
void loadSprites() {

  // load the texture for PacMan
  pacManTex.loadFromFile("./Resources/Images/Pacman16.png");
  pacManSprite.setTexture(pacManTex);
  // assign the PacMan sprite the correct texture
  pacManSprite.setTextureRect(sf::IntRect(16,0,16,16));
  pacManSprite.setScale(1.5, 1.5);

  // load the texture for the Death Animation of the PacMan
  pacManDeathTex.loadFromFile("./Resources/Images/PacmanDeath16.png");
  pacManDeathSprite.setTexture(pacManDeathTex);
  pacManDeathSprite.setScale(1.5, 1.5);

  // 
  pacManLifeSprite.setTexture(pacManTex);
  // assign the sprite the correct texture
  pacManLifeSprite.setTextureRect(sf::IntRect(32,32,16,16));
  pacManLifeSprite.setScale(1.5, 1.5);
  
  // load the texture for the Ghosts
  ghostTex[0].loadFromFile("./Resources/Images/blinky.png");
  ghostTex[1].loadFromFile("./Resources/Images/clyde.png");
  ghostTex[2].loadFromFile("./Resources/Images/inky.png");
  ghostTex[3].loadFromFile("./Resources/Images/pinky.png");
  
  // set the sprites of the Ghosts accordingly
  for(int i = 0; i < numOfGhosts; ++i) {
    ghostSprite[i].setTexture(ghostTex[i]);
    // assign the Ghost Sprite the correct texture
    ghostSprite[i].setTextureRect(sf::IntRect(0,0,16,16));
    ghostSprite[i].setScale(1.5,1.5);
  }
  blueGhosts.loadFromFile("./Resources/Images/blueGhosts.png");

  // load the texture for the power pellets
  powerPelletTex.loadFromFile("./Resources/Images/Map16.png");
  for(int i = 0; i < numOfPowerPellets; ++i) {
    powerPelletSprite[i].setTexture(powerPelletTex);
    // assign the Power Pellet Sprite the correct Texture
    powerPelletSprite[i].setTextureRect(sf::IntRect(16,16,16,16));
    powerPelletSprite[i].setPosition(-20, -20);
    powerPelletSprite[i].setScale(1.5, 1.5);
  }

  // load the texture for the coins
  coinSprite.setTexture(powerPelletTex);
  // assign the correct texture
  coinSprite.setTextureRect(sf::IntRect(0,16,16,16));
  coinSprite.setScale(1.5, 1.5);

  // sprite to be displayed at the door of the ghostHouse;
  ghostHouseSprite.setTexture(powerPelletTex);
  ghostHouseSprite.setTextureRect(sf::IntRect(32,16,16,16));
  ghostHouseSprite.setScale(1.5, 1.5);

  // initialize the rectangle blocks that make up the wall
  rectangle.setSize(sf::Vector2f (24, 24));
  rectangle.setOutlineThickness(0);
  rectangle.setFillColor(sf::Color::Blue);
  // give the walls a texture  
  rectTex.loadFromFile("./Resources/Images/19f75f2bf6e184ef14e6f5c3a5b1aa47.jpg");
  rectangle.setTexture(&rectTex);

  mainMenuTex.loadFromFile("./Resources/Images/peakpx2.jpg");
  mainMenuSprite.setTexture(mainMenuTex);
  mainMenuSprite.setScale(sf::Vector2f(0.68,0.72));
}

void loadSounds() {
  pacManDeathSoundBuffer.loadFromFile("./Resources/Sounds/pacman_death.wav");
  pacManEatGhostSoundBuffer.loadFromFile("./Resources/Sounds/pacman_eatghost.wav");
  pacManChompSoundBuffer.loadFromFile("./Resources/Sounds/pacman_chomp.wav");
  pacManStartSoundBuffer.loadFromFile("./Resources/Sounds/pacman_beginning.wav");
}

void initDefault() {
  
  exit_thread_flag = false;
  
  numOfPermits = numOfKeys = 2;
  direction = 0;
  lives = 3;
  currentPowerPellets = 0;
  numOfBoosts = 2;
  powerUp = false;
  readCount = 0;
  coinsPickedUp = currScore = 0;
  
  for(int i = 0; i < numOfPowerPellets; ++i)
    powerPelletSprite[i].setPosition(-20, -20);
  
  initialState = true;
  blueGhostOnly = true;

}

void mainUserInterface() {

  bool displayMainMenu = true, gameStart = false;
  
  loadSprites();
  loadSounds();

  sf::Text mainMenuOptions[3], gameWonOptions[4], gameOverOptions[4], scoreDisplay;
  sf::Font pacFont;
  pacFont.loadFromFile("./Resources/Fonts/PacfontGood-yYye.ttf");
  
  scoreDisplay.setFont(pacFont);
  scoreDisplay.setCharacterSize(20);
  scoreDisplay.setFillColor(sf::Color::White);
  scoreDisplay.setPosition((width-4)*cellSize, (height+1)*cellSize);

  for(int i = 0; i < 3; ++i) {
    mainMenuOptions[i].setFont(pacFont);
    mainMenuOptions[i].setCharacterSize(45);
    mainMenuOptions[i].setFillColor(sf::Color::Red);
    
    gameWonOptions[i+1].setFont(pacFont);
    gameWonOptions[i+1].setCharacterSize(45);
    gameWonOptions[i+1].setFillColor(sf::Color::Red);
  
    gameOverOptions[i+1].setFont(pacFont);
    gameOverOptions[i+1].setCharacterSize(45);
    gameOverOptions[i+1].setFillColor(sf::Color::Red);
  }

  gameWonOptions[0].setFont(pacFont);
  gameWonOptions[0].setCharacterSize(55);
  gameWonOptions[0].setFillColor(sf::Color::Red);
  
  mainMenuOptions[0].setString("Start New Game");
  mainMenuOptions[0].setPosition(110, 350);
  mainMenuOptions[1].setString("Instructions");
  mainMenuOptions[1].setPosition(160, 350 + 80*1);
  mainMenuOptions[2].setString("Exit");
  mainMenuOptions[2].setPosition(270, 350 + 80*2);
  
  gameWonOptions[0].setString("You have Won !!");
  gameWonOptions[0].setPosition(110, 200); 
  gameWonOptions[1].setString("Play Again");
  gameWonOptions[1].setPosition(160, 350);
  gameWonOptions[2].setString("Go Back to Main Menu");
  gameWonOptions[2].setPosition(110, 350 + 80*1);
  gameWonOptions[3].setString("Exit");
  gameWonOptions[3].setPosition(270, 350 + 80*2);

  gameOverOptions[0].setString("You have Lost !!");
  gameOverOptions[0].setPosition(110, 200); 
  gameOverOptions[1].setString("Play Again");
  gameOverOptions[1].setPosition(160, 350);
  gameOverOptions[2].setString("Go Back to Main Menu");
  gameOverOptions[2].setPosition(110, 350 + 80*1);
  gameOverOptions[3].setString("Exit");
  gameOverOptions[3].setPosition(270, 350 + 80*2);

  // for(int i = 0; i < 3; ++i) {
    
    
  //   gameWonOptions[i].setPosition(140, 200 + 50*i);
  // }

  window.create(sf::VideoMode(width*cellSize, (height+3)*cellSize), "Pac-Man");
  
  while(window.isOpen()){

    // loop to display the main menu and relevant functionality
    while(displayMainMenu) {
      sf::Event event;

      window.clear(sf::Color::Black);
  
      window.draw(mainMenuSprite);
      for(int i = 0; i < 3; ++i)
        window.draw(mainMenuOptions[i]);
      
      window.display();

      while(window.pollEvent(event)) {
        if(event.type == sf::Event::Closed) {
          displayMainMenu = false;
          window.close();
        }
        if(event.type== sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
          
          if(mainMenuOptions[0].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            displayMainMenu = false;
            gameStart = true;
          }
          else if(mainMenuOptions[1].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            s::cout<<"Nothing Here\n";
          }
          else if(mainMenuOptions[2].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            displayMainMenu = false;
            window.close();
          }
        }
        // if(menu.type == sf::Event::KeyPressed) {
        //   if(menu.key.code == sf::Keyboard::Num1) {
        //     displayMainMenu = false;
        //     gameStart = true;
        //     //s::cout<<"1 Pressed\n";
        //   }
        //   else if(menu.key.code == sf::Keyboard::Num2) {
        //     s::cout<<"2 Pressed\n";
        //   }
        //   else if(menu.key.code == sf::Keyboard::Num3) { 
        //     s::cout<<"3 Pressed\nExiting the Game\n";
            
            
        //     displayMainMenu = false;
        //     window.close();
        //   }
        // }      
      }
    }

    if(gameStart) {

      pauseGame = false;
      bool resetGame = false;
      // initialize the global variables with the default values
      initDefault();

      // initialize semaphores and mutexes and lock them according to game logic
      sem_init(&waitForInput, 0, 1);
      sem_wait(&waitForInput);
      
      sem_init(&waitForGameEngine, 0, 1);
      sem_wait(&waitForGameEngine);
      
      sem_init(&waitForDraw, 0, 1);
      sem_wait(&waitForDraw);

      sem_init(&waitForRender, 0, 1);
      sem_wait(&waitForRender);

      int rc;
     
      // initialize thread attributes
      pthread_attr_t attr;
      pthread_attr_init(&attr);
     
      // create thread for the game engine
      pthread_t gameEngineThread;
      rc = pthread_create(&gameEngineThread, &attr, gameEngine, NULL);
      if(rc) {
        s::cout<<"Error unable to create thread. Exiting\n";
        exit(-1);
      }

      // 0 -> Up, 1 -> Down, 2 -> Left, 3 -> Right
      // Value set in the UI Thread and Accessed by the PacMan Thread
      int localDirection = 0;
      // to check for user inputs
      sf::Event event;

      // this while loop below acts as the user interface thread
      while(!exit_thread_flag) {
        
        while(window.pollEvent(event)) {
          if(event.type == sf::Event::Closed) {
            window.close();
            sem_post(&waitForInput);
            sem_post(&waitForDraw);
            gameStart = false;
            exit_thread_flag = true; 
          }
          if(event.type == sf::Event::KeyPressed) {
            if(event.key.code == sf::Keyboard::W) {
              localDirection = 1;
            }
            else if(event.key.code == sf::Keyboard::S) {
              localDirection = 2;
            }
            else if(event.key.code == sf::Keyboard::A) {
              localDirection = 3;
            }
            else if(event.key.code == sf::Keyboard::D) {
              localDirection = 4;
            }
            else if(event.key.code == sf::Keyboard::P) {
              if(pauseGame)
                pauseGame = false;
              else
                pauseGame = true;
            }
            else if(event.key.code == sf::Keyboard::R) {
              sem_post(&waitForInput);
              sem_post(&waitForDraw);
              exit_thread_flag = true;
              gameStart = true;
              resetGame = true;
            }
          }
        }

        if(resetGame == true)
          break;

        if(pauseGame)
          continue;
        
        if(direction == -1) 
          direction = localDirection = 0;
        else
          direction = localDirection;

        // signal the PacMan thread that the update value
        // has been written into the direction variable
        sem_post(&waitForInput);
        
        // waiting for the gameEngine to coordinate in-between the 
        // PacMan and the Ghosts
        sem_wait(&waitForGameEngine);    

        if(exit_thread_flag) {
          //window.close();
          //displayMainMenu = true;
          gameStart = false;
          sem_post(&waitForInput);
          sem_post(&waitForDraw);
          break;
        }

        // clear the screen
        window.clear(sf::Color::Black);
        // Display the grid
        for(int i = 0; i < height; ++i) {
          for(int j = 0; j < width; ++j) {
            if(maze[i][j] == 1) {
              rectangle.setPosition(cellSize*j, cellSize*i);
              window.draw(rectangle); 
            }
            else if(maze[i][j] == 2) {
              ghostHouseSprite.setPosition(cellSize*j, cellSize*i);
              window.draw(ghostHouseSprite);
            }
            else if(maze[i][j] == 4) {
              coinSprite.setPosition(cellSize*j, cellSize*i);
              window.draw(coinSprite);
            }
          }
        }

        // draw everything else on the screen
        window.draw(pacManSprite);
        for(int i = 0; i < numOfGhosts; ++i)
          window.draw(ghostSprite[i]);
        for(int i = 0; i < numOfPowerPellets; ++i)
          window.draw(powerPelletSprite[i]);
        for(int i = 0; i < lives-1; ++i) {
          pacManLifeSprite.setPosition(20 + (i*30), (height+1)*cellSize );
          window.draw(pacManLifeSprite);
        }
        scoreDisplay.setString(std::to_string(currScore));
        
        window.draw(scoreDisplay);

        // ensures that the game engine can now render, after 
        // the relevant sprites have been draw on the window
        sem_post(&waitForDraw);
        
        // ensures that the game engine has rendered everything 
        // before continuing further execution
        sem_wait(&waitForRender);
      }

      pthread_join(gameEngineThread, NULL);

      // destroy thread attributes
      pthread_attr_destroy(&attr);

      // destroy the semaphores initialized above      
      sem_destroy(&waitForInput);
      sem_destroy(&waitForGameEngine);
      sem_destroy(&waitForDraw);
      sem_destroy(&waitForRender);

    }

    // screen to display when the user has won the game
    // Provide the Relevant Functionality
    while(gameWon) {

      window.clear(sf::Color::Black);

      //window.draw(mainMenuSprite);
      for(int i = 0; i < 4; ++i) 
        window.draw(gameWonOptions[i]);  

      window.display();

      sf::Event event;
      while(window.pollEvent(event)) {
        if(event.type == sf::Event::Closed) {
          window.close();
          gameWon = displayMainMenu = gameStart = false;         
        }        
        if(event.type== sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
          // Restart  
          if(gameWonOptions[1].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            gameWon = false;
            gameStart = true;
          }
          // Go back to main menu
          else if(gameWonOptions[2].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            gameWon = false;
            displayMainMenu = true;
          }
          // exit the game
          else if(gameWonOptions[3].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            gameWon = false;
            window.close();
          }
        }
        // if(event.type == sf::Event::KeyPressed) {           
        //   // Restart
        //   if(event.key.code == sf::Keyboard::Num1) {
        //     gameWon = displayMainMenu = false;
        //     gameStart = true;
        //   }
        //   // go back to main menu
        //   else if(event.key.code == sf::Keyboard::Num2) {
        //     displayMainMenu = true;
        //     gameWon = gameStart = false;
        //   }
        //   // exit
        //   else if(event.key.code == sf::Keyboard::Num3) {
        //     window.close();
        //     gameWon = gameStart = displayMainMenu = false;
          
        //   }
        // }
      }
    }
    // screen to display when the pacman has lost of all its lives
    while(gameOver) {

      window.clear(sf::Color::Black);

      //window.draw(mainMenuSprite);
      for(int i = 0; i < 4; ++i) 
        window.draw(gameOverOptions[i]);  

      window.display();

      sf::Event event;
      while(window.pollEvent(event)) {
        if(event.type == sf::Event::Closed) {
          window.close();
          displayMainMenu = gameStart = gameWon = gameOver = false;         
        }
        if(event.type== sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
          // Restart  
          if(gameOverOptions[1].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            gameOver = false;
            gameStart = true;
          }
          // Go back to main menu
          else if(gameOverOptions[2].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            gameOver = false;
            displayMainMenu = true;
          }
          // exit the game
          else if(gameOverOptions[3].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
            gameOver = false;
            window.close();
          }
        }
        // if(event.type == sf::Event::KeyPressed) {           
        //   // Restart
        //   if(event.key.code == sf::Keyboard::Num1) {
        //     displayMainMenu = gameWon = gameOver = false;
        //     gameStart = true;
        //   }
        //   // go back to main menu
        //   else if(event.key.code == sf::Keyboard::Num2) {
        //     displayMainMenu = true;
        //     gameStart = gameWon = gameOver = false;
        //   }
        //   // exit
        //   else if(event.key.code == sf::Keyboard::Num3) {
        //     window.close();
        //     displayMainMenu = gameStart = gameWon = gameOver = false;
        //   }
        // }
      }
    }


  }
  
}


int main() {

  //std::cout<<"Hello World"<<std::endl;
  
  // seed the random number generator
  srand(time(0));

  mainUserInterface();
  
 // pthread_exit(NULL);
  
  return 0;
}
