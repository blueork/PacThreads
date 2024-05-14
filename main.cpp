// Current Task
// Step 1:
// Add Boosts for the Ghosts
// Step 2:
// Solve Scenario 4

#include <iostream>
#include <SFML/Graphics.hpp>
#include <pthread.h>
#include <atomic>
#include <time.h>     
#include <semaphore.h>

#define s std
#define height 31
#define width 28
#define cellSize 16   // each block on the game grid corresponds to a 16*16 cell 

const int numOfPowerPellets = 4;
const int numOfGhosts = 4;

// Declare all the Global Variables here for the all the threads to access
std::atomic<bool> exit_thread_flag{false};
sf::RenderWindow window;
sf::RectangleShape rectangle;
sf::Texture pacManTex, pacManDeathTex, powerPelletTex, ghostTex[numOfGhosts], blueGhosts;
sf::Sprite pacManSprite,pacManDeathSprite, powerPelletSprite[numOfPowerPellets], ghostSprite[numOfGhosts];
pthread_mutex_t waitForPacMan, waitForInput, waitForGameEngine, waitForDraw, waitForRender;
pthread_mutex_t waitForGhost[numOfGhosts], waitForGameEngine1[numOfGhosts], powerPellet;
pthread_mutex_t consumeBoost;

int direction = 0, lives = 3, currentPowerPellets = 0, numOfBoosts = 2;
bool powerUp = false;
int readCount = 0;            // keeps track of the number of ghosts reading the maze at a time
sem_t ghostMutex, mazeAccess; // semaphores used to address the Reader/Writer 
                              // scenario in the context of the PacMan and the Ghosts

int powerPelletLoc[numOfPowerPellets][2] = { {16,16},
                                             {26*16,16},
                                             {16,29*16},
                                             {26*16,29*16} };

int ghostStartingLoc[numOfGhosts][2] = {{10*16,12*16},
                                        {10*16,18*16},
                                        {17*16,12*16},
                                        {17*16,18*16} };                                             
// boolean variables related to te functionality of the ghosts
// appearing as blue and white when the pacman eats a power up
bool initialState = true, blueGhostOnly = true;

// the underlying 2D Maze
int maze[height][width] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1},
  {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1},
  {1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1},
  {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
  {1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1},
  {1,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
  {1,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1} };  


// variable and class for testing ghostThreadCleanupHandler
// int cleanup_pop_arg = 0;
// class info {
// public:
//   int i;
// };

class info1 {
public:
  int ghostNum;
  bool hasBoost = false; 
};

// function called when the ghost thread is cancelled
void ghostThreadCleanupHandler(void* arg) {
  info1* temp = (info1*)arg;
  if(temp != NULL) {
    if(temp->hasBoost == true) {
      pthread_mutex_lock(&consumeBoost);
        s::cout<<"In the Cleanup Handler of the "<<temp->ghostNum<<" Ghost\n";
        s::cout<<"Freeing up the Boost\n";
        ++numOfBoosts;
      pthread_mutex_unlock(&consumeBoost);
    }
    //s::cout<<"Hello World";
    delete temp;
    temp = NULL;
  }
  //s::cout<<temp->i;
  //s::cout<<"In Ghost Thread Cleanup Handler\n";
  
  
  //delete temp;
  //temp = NULL;
  //s::cout<<((*info)arg)->i;
  return;
}



void ghostMvmt() {

}

void* ghost(void* anything) {

  // info1* temp1 = (info1*)anything;
  // int ghostNum = temp1->ghostNum;
  //delete temp1;
  //emp1 = NULL; 
  //info* infoBlock = new info;
  //infoBlock->i = 10;

  // set the function to call when the ghost thread is cancelledS
  pthread_cleanup_push(ghostThreadCleanupHandler, anything);

  info1* temp1 = (info1*)anything;
  int ghostNum = temp1->ghostNum;

  // set the relevant cancel states for the thread
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  // set the initial starting position
  ghostSprite[ghostNum].setPosition(ghostStartingLoc[ghostNum][0],ghostStartingLoc[ghostNum][1]);
  //int key = 1, exitPermit = 1, currentState = 0;
  //bool withInHouse = true;
  // int initialDirection[3][2] = { {1, 0},
  //                                {0, -1},
  //                                {0, -1} };

  //bool temp = false;
  int numOfMoves = 0, direction = 0, xPos, yPos, ghostSpriteFrame = 0;
  sf::Vector2f currPos;
  // variables used for the purpose of display blue and white ghosts when the pacman
  // eats the power up
  bool check = false, hasBoost = false;
  int blueGhostSpriteFrame = 0;



  while(!exit_thread_flag) {

    
    // if(withInHouse) {

    //     if(key && exitPermit) {

    //       ghostSprite1.move(initialDirection[currentState][0]*16, initialDirection[currentState][1]*16);
    //       currentState = (currentState + 1)%3;
    //       if(currentState == 0)
    //         withInHouse = false;
    //     }
    //     else 
    //       s::cout<<"Lack of Key and Exit Permit\n";

    // }
    // else {

    if(!hasBoost) {
      pthread_mutex_lock(&consumeBoost);
      if(numOfBoosts > 0) {
        hasBoost = true;
        --numOfBoosts;
        temp1->hasBoost = 1;
        s::cout<<"Ghost "<<temp1->ghostNum<<" has acquired the boost\n";
      }
      pthread_mutex_unlock(&consumeBoost);
    }

    // basic Movement Generator for the Ghost
    if(numOfMoves == 0) {
      numOfMoves = (rand()%5) + 1;
      direction = (rand()%4) + 1;        
    }
    --numOfMoves;

    // get the current position of the ghost
    currPos = ghostSprite[ghostNum].getPosition();
    // translate the coordinates with respect to the set cell size in the gird
    xPos = (currPos.x/16); 
    yPos = (currPos.y/16);
      
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
    
      if(maze[yPos-1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, -16);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }  
    
      else if(maze[yPos+1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, 16);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }
    
      else if(maze[yPos][xPos-1] != 1) {
        ghostSprite[ghostNum].move(-16, 0);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(32 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }
    
      else if(maze[yPos][xPos+1] != 1) {
        ghostSprite[ghostNum].move(16, 0);
        
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;      
        } 
      }
    
    }
    
    else if(direction == 2) {
    
      if(maze[yPos+1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, 16);
        
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }        
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }
    
      else if(maze[yPos-1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, -16);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2; 
        }
      }
    
      else if(maze[yPos][xPos-1] != 1) {
        ghostSprite[ghostNum].move(-16, 0);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(32 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }
    
      else if(maze[yPos][xPos+1] != 1) {
        ghostSprite[ghostNum].move(16, 0);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
        }
        if(initialState) { 
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }
    
    }

    else if(direction == 3) {

      if(maze[yPos][xPos-1] != 1) {
        ghostSprite[ghostNum].move(-16, 0);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(32 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }

      else if(maze[yPos+1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, 16);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) { 
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }

      else if(maze[yPos-1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, -16);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }

      else if(maze[yPos][xPos+1] != 1) {
        ghostSprite[ghostNum].move(16, 0);       
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }     
    
    }

    else if(direction == 4) {

      if(maze[yPos][xPos+1] != 1) {
        ghostSprite[ghostNum].move(16, 0);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(0 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }

      else if(maze[yPos+1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, 16);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(96 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }

      else if(maze[yPos-1][xPos] != 1) {
        ghostSprite[ghostNum].move(0, -16);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
        }
        if(initialState) {
          ghostSprite[ghostNum].setTextureRect(sf::IntRect(64 + 16*ghostSpriteFrame,0,16,16));
          ghostSpriteFrame = (ghostSpriteFrame+1)%2;
        }
      }

      else if(maze[yPos][xPos-1] != 1) {
        ghostSprite[ghostNum].move(-16, 0);
        if(hasBoost == true) {
          currPos = ghostSprite[ghostNum].getPosition();
          xPos = (currPos.x/16); 
          yPos = (currPos.y/16);
          if(maze[yPos][xPos-1] != 1)
            ghostSprite[ghostNum].move(-16, 0);
          else if(maze[yPos+1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, 16);
          else if(maze[yPos-1][xPos] != 1) 
            ghostSprite[ghostNum].move(0, -16);
          else if(maze[yPos][xPos+1] != 1)
            ghostSprite[ghostNum].move(16, 0);
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
    pthread_mutex_unlock(&waitForGhost[ghostNum]);
    pthread_mutex_lock(&waitForGameEngine1[ghostNum]);
  }
  if(hasBoost == true) {
    pthread_mutex_lock(&consumeBoost);
    hasBoost = false;
    ++numOfBoosts;
    s::cout<<"Game is closing down. Ghost "<<ghostNum<<" is freeing up the boost accordingly\n";
    pthread_mutex_unlock(&consumeBoost);
  }

  if(temp1 != NULL) {      
    delete temp1;
    temp1 = NULL;
    //s::cout<<"Free up DMA";   
  }
  // int temp = 0;
  // DO NOT REMOVE THE BELOW COUT STATEMENT
  // IT DOES NOT PRINT ANYTHING HOWEVER IT SOMEHOW PREVENTS A MASSIVE ERROR
  // IT SOMEHOW ENSURES THAT THE CLEAN UP ROUTINE RUNS PROPERLY
  s::cout<<"Here\n"; 
  // pthread_cleanup_pop(cleanup_pop_arg);
  pthread_cleanup_pop(0);
  s::cout<<"Do we reach here in case of pop";
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
  pacManSprite.setPosition(32,16);
  // set the initial sprite
  pacManSprite.setTextureRect(sf::IntRect(16,16,16,16));

  int localDirection, xPos, yPos, spriteFrame = 0;
  sf::Vector2f currPos;
  // To keep track of the time for the effect of the Power Pellet
  sf::Clock powerUpClock;

  while(!exit_thread_flag) {

    // wait for the UI Thread to Update the value of direction 
    // with respect to the input
    pthread_mutex_lock(&waitForInput);
        
    localDirection = direction;
    if(localDirection) {
      // get the current position of the pac man
      currPos = pacManSprite.getPosition();
      // translate the coordinates with respect to
      // the size of each cell in the set grid
      xPos = (currPos.x/16); 
      yPos = (currPos.y/16);
      
      sem_wait(&mazeAccess);
      if(direction == 1) {
        if(maze[yPos-1][xPos] != 1) {
          pacManSprite.move(0, -16);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,16,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
      }
      else if(direction == 2) {
        if(maze[yPos+1][xPos] != 1) {
          pacManSprite.move(0, 16);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,48,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
      }
      else if(direction == 3) {
        if(maze[yPos][xPos-1] != 1) {
          pacManSprite.move(-16, 0);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,32,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
      }
      else if(direction == 4) {
        if(maze[yPos][xPos+1] != 1) {
          pacManSprite.move(16, 0);
          pacManSprite.setTextureRect(sf::IntRect(16*spriteFrame,0,16,16));
          spriteFrame = ((spriteFrame+1)%5)+1;
        }
      }
      sem_post(&mazeAccess);

    }

    if(!powerUp) {
      pthread_mutex_lock(&powerPellet);
      if(currentPowerPellets > 0) {
        for(int i = 0; i < numOfPowerPellets; ++i) {
          
          if(powerPelletSprite[i].getPosition() == pacManSprite.getPosition()) {
            
            // reset the power pellet position after consuming it
            powerPelletSprite[i].setPosition(-10, -10);
            s::cout<<"Power Pellet Acquired\n";        
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
        s::cout<<"Lost The Power Up\n";
        powerUp = false;
      }
    }

    // signal the game engine thread that it can proceed further
    pthread_mutex_unlock(&waitForPacMan);
  }
  pthread_exit(NULL);
}


void collisionAnimation() {

  pacManDeathSprite.setPosition(pacManSprite.getPosition());

  sf::Clock delay;
  
  for(int i = 0; i < 12; ++i) {
    window.clear(sf::Color::Black);
    // Display the grid
    for(int i = 0; i < height; ++i) {
      for(int j = 0; j < width; ++j) {
        if(maze[i][j] == 1) {
          rectangle.setPosition(16*j,16*i);
          window.draw(rectangle); 
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
    while(delay.getElapsedTime().asSeconds() < 0.1) {};
  }
  // reset the relevant values
  direction = -1;
  pacManSprite.setPosition(-10,-10);
  for(int i = 0; i < numOfGhosts; ++i)
    ghostSprite[i].setPosition(-10,-10);
}

void* gameEngine(void* anything) {

  //int ghostCount = 1;
  
  sem_init(&ghostMutex, 0, 1);
  sem_init(&mazeAccess, 0, 1);

  pthread_mutex_init(&waitForPacMan, NULL);
  pthread_mutex_lock(&waitForPacMan);
  
  for(int i = 0; i < numOfGhosts; ++i) {
    pthread_mutex_init(&waitForGhost[i], NULL);
    pthread_mutex_lock(&waitForGhost[i]);
  }
  for(int i = 0; i < numOfGhosts; ++i) {
    pthread_mutex_init(&waitForGameEngine1[i], NULL);
    pthread_mutex_lock(&waitForGameEngine1[i]);
  }

  pthread_mutex_init(&powerPellet, NULL);
  pthread_mutex_init(&consumeBoost, NULL);

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
  sf::Vector2f pacManPos, ghostPos, temp(-10,-10);

  bool restart = false, ghostDead = false, blueGhostSprite = false;
  int powerPelletPos = 0;     // points to the first power pellet that has to be created 
  int ghostToReAnimate = -1;

  while(!exit_thread_flag) {

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
    pthread_mutex_lock(&waitForPacMan);

    // wait for the Ghost Threads to Execute
    for(int i = 0; i < numOfGhosts; ++i)
      pthread_mutex_lock(&waitForGhost[i]);

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

    // check for collision in-between PacMan and Ghost
    pacManPos = pacManSprite.getPosition();
    for(int i = 0; i < numOfGhosts; ++i) {
      if(pacManPos == ghostSprite[i].getPosition()) {
        
        // destroy the Ghost Threads
        pthread_cancel(ghostThread[i]);
        ghostDead = true;
        ghostToReAnimate = i;
        // if the pacman has a power up enabled then only destroy the ghost
        if(powerUp)
          break;

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
        
        if(lives == 0) 
          exit_thread_flag = true;
        restart = true;
        break;
      }
    }

    // signal the ui thread that it can draw the final sprites on to the screen
    pthread_mutex_unlock(&waitForGameEngine);
    
    // wait for the ui thread to draw each sprite on to the screen
    pthread_mutex_lock(&waitForDraw);
    
    // render everything on to the screen
    window.display();
    
    // add the effect of delay
    delayClock.restart();
    while(delayClock.getElapsedTime().asSeconds() < 0.1) {};
    
    // signal the ui thread that everything has been rendered
    pthread_mutex_unlock(&waitForRender);
    // signal the Ghost Thread to continue execution
    for(int i = 0; i < numOfGhosts; ++i)
      pthread_mutex_unlock(&waitForGameEngine1[i]);
  }

  // destroy thread attributes
  pthread_attr_destroy(&attr);
  
  // destroy the mutexes initialized within the Game Engine Thread
  pthread_mutex_destroy(&waitForInput);
  pthread_mutex_destroy(&waitForPacMan);
  for(int i = 0; i < numOfGhosts; ++i) {
    pthread_mutex_destroy(&waitForGhost[i]);
    pthread_mutex_destroy(&waitForGameEngine1[i]); 
  }
  pthread_mutex_destroy(&powerPellet);
  pthread_mutex_destroy(&consumeBoost);

  // 
  

  // destroy the semaphores initialized within the Game Engine Thread
  sem_destroy(&ghostMutex);
  sem_destroy(&mazeAccess);

  s::cout<<"Checking if the Game Engine Perfroms all the execution\n";
  // exit the thread
  pthread_exit(NULL);
}



// the userInterface thread
// display the sprites
// responsible for taking input
void userInterface() {

  // opens up the window to display everything
  window.create(sf::VideoMode(448, 496), "Pac-Man");

  // 0 -> Up, 1 -> Down, 2 -> Left, 3 -> Right
  // Value set in the UI Thread and Accessed by the PacMan Thread
  int localDirection = 0;
  // to check for user inputs
  sf::Event event;

  while(window.isOpen()) {

    while(window.pollEvent(event)) {
      if(event.type == sf::Event::Closed) {
        window.close();
        pthread_mutex_unlock(&waitForInput);
        pthread_mutex_unlock(&waitForDraw);
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
      }
    }

    if(direction == -1) 
      direction = localDirection = 0;
    else
      direction = localDirection;
    // signal the PacMan thread that the update value
    // has been written into the direction variable
    pthread_mutex_unlock(&waitForInput);
    
    // waiting for the gameEngine to coordinate in-between the 
    // PacMan and the Ghosts
    pthread_mutex_lock(&waitForGameEngine);    

    if(exit_thread_flag) {
      window.close();
      pthread_mutex_unlock(&waitForInput);
      pthread_mutex_unlock(&waitForDraw);
      break;
    }
    // clear the screen
    window.clear(sf::Color::Black);
    // Display the grid
    for(int i = 0; i < height; ++i) {
      for(int j = 0; j < width; ++j) {
        if(maze[i][j] == 1) {
          rectangle.setPosition(16*j,16*i);
          window.draw(rectangle); 
        }
        // else if(maze[i][j] == 2) {
        //   coinSprite.setPosition(16*j, 16*i);
        //   window.draw(coinSprite);
        // }
      }
    }
    // draw everything else on the screen
    window.draw(pacManSprite);
    for(int i = 0; i < numOfGhosts; ++i)
      window.draw(ghostSprite[i]);
    for(int i = 0; i < numOfPowerPellets; ++i)
      window.draw(powerPelletSprite[i]);
    // ensures that the game engine can now render, after 
    // the relevant sprites have been draw on the window
    pthread_mutex_unlock(&waitForDraw);
    // ensures that the game engine has rendered everything 
    // before continuing further execution
    pthread_mutex_lock(&waitForRender);
  }

}


// destroy all the mutexes used within the entire process
void destroyMutexes() {
  pthread_mutex_destroy(&waitForGameEngine);
  pthread_mutex_destroy(&waitForDraw);
  pthread_mutex_destroy(&waitForInput);
  pthread_mutex_destroy(&waitForRender);
}


// load the textures
// and initialize the sprites accordingly
void loadSprites() {
  // load the texture for PacMan
  pacManTex.loadFromFile("./Resources/Images/Pacman16.png");
  pacManSprite.setTexture(pacManTex);
  // assign the PacMan sprite the correct texture
  pacManSprite.setTextureRect(sf::IntRect(16,0,16,16));

  // load the texture for the Death Animation of the PacMan
  pacManDeathTex.loadFromFile("./Resources/Images/PacmanDeath16.png");
  pacManDeathSprite.setTexture(pacManDeathTex);

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
  }
  blueGhosts.loadFromFile("./Resources/Images/blueGhosts.png");

  // load the texture for the power pellets
  powerPelletTex.loadFromFile("./Resources/Images/Map16.png");
  for(int i = 0; i < numOfPowerPellets; ++i) {
    powerPelletSprite[i].setTexture(powerPelletTex);
    // assign the Power Pellet Sprite the correct Texture
    powerPelletSprite[i].setTextureRect(sf::IntRect(16,16,16,16));
    powerPelletSprite[i].setPosition(-10, -10);
  }

  // initialize the rectangle blocks that make up the wall
  rectangle.setSize(sf::Vector2f (16, 16));
  rectangle.setOutlineThickness(0);
  rectangle.setFillColor(sf::Color::Blue);
}

int main() {

  //std::cout<<"Hello World"<<std::endl;
  
  // seed the random number generator
  srand(time(0));


  loadSprites();

  pthread_mutex_init(&waitForInput, NULL);
  pthread_mutex_lock(&waitForInput);
  
  pthread_mutex_init(&waitForGameEngine, NULL);
  pthread_mutex_lock(&waitForGameEngine);
  
  pthread_mutex_init(&waitForDraw, NULL);
  pthread_mutex_lock(&waitForDraw);

  pthread_mutex_init(&waitForRender, NULL);
  pthread_mutex_lock(&waitForRender);
  
  int rc;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  // create thread for the game engine
  pthread_t gameEngineThread;
  rc = pthread_create(&gameEngineThread, &attr, gameEngine, NULL);
  if(rc) {
    s::cout<<"Error unable to create thread. Exiting\n";
    exit(-1);
  }
  userInterface();
  
  // destroy thread attributes
  pthread_attr_destroy(&attr);
  destroyMutexes();

  return 0;
}
