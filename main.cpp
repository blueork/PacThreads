// Current Task
// Step 1:
// Add Mechanism to detect collision between the Ghost and the PacMan
// Step 2:
// Add Lives
// Step 3:
// PacMan goes to its initial postion and so does the ghost
// Step 4:
// Achieve step 3 by destroying and re-creating ghost threads
// Step 5:
// Add Animation of Bomb Blast

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

// Declare all the Global Variables here for the all the threads to access
std::atomic<bool> exit_thread_flag{false};
sf::RenderWindow window;
sf::RectangleShape rectangle;
sf::Texture pacManTex, pacManDeathTex, ghostTex;
sf::Sprite pacManSprite,pacManDeathSprite, ghostSprite1;
pthread_mutex_t waitForPacMan, waitForInput, waitForGameEngine, waitForDraw, waitForRender;
pthread_mutex_t waitForGhost[1], waitForGameEngine1[1];
int direction = 0;
int lives = 3;
int readCount = 0;            // keeps track of the number of ghosts reading the maze at a time
sem_t ghostMutex, mazeAccess; // semaphores used to address the Reader/Writer 
                              // scenario in the context of the PacMan and the Ghosts


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
  {1,1,1,1,1,0,0,1,0,0,0,1,3,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1},
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


void* ghost(void* anything) {

  // set the relevant cancel states for the thread
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  // set the initial starting position
  ghostSprite1.setPosition(160, 224);
  //int key = 1, exitPermit = 1, currentState = 0;
  //bool withInHouse = true;
  // int initialDirection[3][2] = { {1, 0},
  //                                {0, -1},
  //                                {0, -1} };

  //bool temp = false;
  int numOfMoves = 0, direction = 0, xPos, yPos;
  sf::Vector2f currPos;


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
      
    // basic Movement Generator for the Ghost
    if(numOfMoves == 0) {
      numOfMoves = (rand()%5) + 1;
      direction = (rand()%4) + 1;        
    }
    --numOfMoves;

    // get the current position of the ghost
    currPos = ghostSprite1.getPosition();
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
      
    if(direction == 1) {
      if(maze[yPos-1][xPos] != 1)
        ghostSprite1.move(0, -16);
      else if(maze[yPos+1][xPos] != 1)
        ghostSprite1.move(0, 16);
      else if(maze[yPos][xPos-1] != 1)
        ghostSprite1.move(-16, 0);
      else if(maze[yPos][xPos+1] != 1)
        ghostSprite1.move(16, 0);
    }
    else if(direction == 2) {
      if(maze[yPos+1][xPos] != 1)
        ghostSprite1.move(0, 16);
      else if(maze[yPos-1][xPos] != 1)
        ghostSprite1.move(0, -16);
      else if(maze[yPos][xPos-1] != 1)
        ghostSprite1.move(-16, 0);
      else if(maze[yPos][xPos+1] != 1)
        ghostSprite1.move(16, 0);
    }
    else if(direction == 3) {
      if(maze[yPos][xPos-1] != 1)
        ghostSprite1.move(-16, 0);
      else if(maze[yPos+1][xPos] != 1)
        ghostSprite1.move(0, 16);
      else if(maze[yPos-1][xPos] != 1)
        ghostSprite1.move(0, -16);
      else if(maze[yPos][xPos+1] != 1)
        ghostSprite1.move(16, 0);            
    }
    else if(direction == 4) {
      if(maze[yPos][xPos+1] != 1)
        ghostSprite1.move(16, 0);
      else if(maze[yPos+1][xPos] != 1)
        ghostSprite1.move(0, 16);
      else if(maze[yPos-1][xPos] != 1)
        ghostSprite1.move(0, -16);
      else if(maze[yPos][xPos-1] != 1)
        ghostSprite1.move(-16, 0);
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
      // }
    //}
    pthread_mutex_unlock(&waitForGhost[0]);
    pthread_mutex_lock(&waitForGameEngine1[0]);
  }

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
  pacManSprite.setPosition(16,16);

  int localDirection, xPos, yPos;
  sf::Vector2f currPos;

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
        if(maze[yPos-1][xPos] != 1)
          pacManSprite.move(0, -16);
      }
      else if(direction == 2) {
        if(maze[yPos+1][xPos] != 1)
          pacManSprite.move(0, 16);
      }
      else if(direction == 3) {
        if(maze[yPos][xPos-1] != 1)
          pacManSprite.move(-16, 0);
      }
      else if(direction == 4) {
        if(maze[yPos][xPos+1] != 1)
          pacManSprite.move(16, 0);
      }
      sem_post(&mazeAccess);

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
    // add delay to add the effect fof frame-rates 
    delay.restart();
    while(delay.getElapsedTime().asSeconds() < 0.1) {};
  }
  // reset the relevant values
  direction = -1;
  pacManSprite.setPosition(-10,-10);
  ghostSprite1.setPosition(-10,-10);
}

void* gameEngine(void* anything) {

  int ghostCount = 1;

  pthread_mutex_init(&waitForInput, NULL);
  pthread_mutex_lock(&waitForInput);

  pthread_mutex_init(&waitForPacMan, NULL);
  pthread_mutex_lock(&waitForPacMan);
  
  pthread_mutex_init(&waitForGhost[0], NULL);
  pthread_mutex_lock(&waitForGhost[0]);

  pthread_mutex_init(&waitForGameEngine1[0], NULL);
  pthread_mutex_lock(&waitForGameEngine1[0]);

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

  // create the thread for ghost
  pthread_t ghostThread1;
  rc = pthread_create(&ghostThread1, &attr, ghost, NULL);
  if(rc) {
    s::cout<<"Error unable to create Ghost thread. Exiting\n";
    exit(-1);
  }

  // clock for adding delay
  sf::Clock delayClock;
  sf::Vector2f pacManPos, ghostPos;

  bool restart = false;
  while(!exit_thread_flag) {

    // recreate the PacMan and the Ghost Threads after collision
    if(restart) {
      rc = pthread_create(&pacManThread, &attr, pacMan, NULL);
      if(rc) {
        s::cout<<"Error unable to create PacMan thread. Exiting\n";
        exit(-1);
      }
      rc = pthread_create(&ghostThread1, &attr, ghost, NULL);
      if(rc) {
        s::cout<<"Error unable to create Ghost thread. Exiting\n";
        exit(-1);
      }
      restart = false;
    }
    // wait for the PacMan thread to execute
    pthread_mutex_lock(&waitForPacMan);

    // wait for the Ghost Threads to Execute
    for(int i = 0; i < ghostCount; ++i)
      pthread_mutex_lock(&waitForGhost[i]);

    // check for collision in-between PacMan and Ghost
    pacManPos = pacManSprite.getPosition();
    for(int i = 0; i < ghostCount; ++i) {
      if(pacManPos == ghostSprite1.getPosition()) {
        // destroy the Ghost Threads
        pthread_cancel(ghostThread1);
        // destroy the PacMan Threads
        pthread_cancel(pacManThread);
        //s::cout<<"Collision Check\n";
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
    pthread_mutex_unlock(&waitForGameEngine1[0]);
  }
  // destroy thread attributes
  pthread_attr_destroy(&attr);

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
    window.draw(ghostSprite1);
    // ensures that the game engine can now render, after 
    // the relevant sprites have been draw on the window
    pthread_mutex_unlock(&waitForDraw);
    // ensures that the game engine has rendered everything 
    // before continuing further execution
    pthread_mutex_lock(&waitForRender);
  }

}

// destroy all the semaphores used within the entire process
void destroySema() {
  sem_destroy(&ghostMutex);
  sem_destroy(&mazeAccess);
}

// destroy all the mutexes used within the entire process
void destroyMutexes() {
  pthread_mutex_destroy(&waitForGameEngine);
  pthread_mutex_destroy(&waitForDraw);
  pthread_mutex_destroy(&waitForInput);
  pthread_mutex_destroy(&waitForPacMan);
  pthread_mutex_destroy(&waitForRender);
}

int main() {

  //std::cout<<"Hello World"<<std::endl;
  
  // seed the random number generator
  srand(time(0));

  // load the texture for PacMan
  pacManTex.loadFromFile("./Resources/Images/player.png");
  pacManSprite.setTexture(pacManTex);
  // assign the PacMan sprite the correct texture
  pacManSprite.setTextureRect(sf::IntRect(16,0,16,16));
  
  // load the texture for the Death Animation of the PacMan
  pacManDeathTex.loadFromFile("./Resources/Images/PacmanDeath16.png");
  pacManDeathSprite.setTexture(pacManDeathTex);

  // load the texture for Ghost
  ghostTex.loadFromFile("./Resources/Images/blinky.png");
  ghostSprite1.setTexture(ghostTex);
  // assign the Ghost Sprite the correct texture
  ghostSprite1.setTextureRect(sf::IntRect(0,0,16,16));
  
  // initialize the rectangle blocks that make up the wall
  rectangle.setSize(sf::Vector2f (16, 16));
  rectangle.setOutlineThickness(0);
  rectangle.setFillColor(sf::Color::Blue);
  
  pthread_mutex_init(&waitForGameEngine, NULL);
  pthread_mutex_lock(&waitForGameEngine);
  
  pthread_mutex_init(&waitForDraw, NULL);
  pthread_mutex_lock(&waitForDraw);

  pthread_mutex_init(&waitForRender, NULL);
  pthread_mutex_lock(&waitForRender);

  sem_init(&ghostMutex, 0, 1);
  sem_init(&mazeAccess, 0, 1);
  
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
  destroySema();

  return 0;
}
