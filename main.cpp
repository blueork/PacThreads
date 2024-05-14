// Current Task
// Overhaul the game
// Step 1
// Create a GameEngine Thread
// It shall be responsible for creating the PacMan thread and managing it
// Step 2 
// Render the graphics in the GameEngine Thread
// Step 3
// Reduce the number of mutexes used
#include <iostream>
#include <SFML/Graphics.hpp>
#include <pthread.h>
#include <atomic>


#define s std
#define height 31
#define width 28


// Declare all the Global Variables here for the all the threads to access
std::atomic<bool> exit_thread_flag{false};
sf::RenderWindow window;
sf::Texture pacManTex;
sf::Sprite pacManSprite;
pthread_mutex_t waitForPacMan, waitForInput, waitForGameEngine, waitForDraw, waitForRender;
int direction = 0;
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


// the pacMan thread
// responsible for everything pacMan related
// shall move the PacMan with respect to Walls 
void* pacMan(void* anything) {

  int localDirection;

  while(!exit_thread_flag) {

    // wait fot the UI Thread to Update the value of direction 
    // with respect to the input
    pthread_mutex_lock(&waitForInput);
        
    localDirection = direction;
    if(localDirection) {
      // get the current position of the pac man
      sf::Vector2f currPos = pacManSprite.getPosition();
      // translate the coordinates with respect to
      // the size of each cell in the set grid
      int xPos = (currPos.x/16), yPos = (currPos.y/16);
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
    
    }
    // signal the game engine thread that it can proceed further
    pthread_mutex_unlock(&waitForPacMan);
  }
  pthread_exit(NULL);
}


void* gameEngine(void* anything) {

  pthread_mutex_init(&waitForInput, NULL);
  pthread_mutex_lock(&waitForInput);

  pthread_mutex_init(&waitForPacMan, NULL);
  pthread_mutex_lock(&waitForPacMan);
  
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

  while(!exit_thread_flag) {

    // wait for the PacMan thread to execute
    pthread_mutex_lock(&waitForPacMan);

    // signal the ui thread that it can draw the final sprites on to the screen
    pthread_mutex_unlock(&waitForGameEngine);
    
    // wait for the ui thread to draw each sprite on to the screen
    pthread_mutex_lock(&waitForDraw);
    
    // render everything on to the screen
    window.display();
    
    // signal the ui thread that everything has been rendered
    pthread_mutex_unlock(&waitForRender);

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

  // Rectangle Blocks to make up the wall
  sf::RectangleShape rectangle;
  rectangle.setSize(sf::Vector2f (16, 16));
  rectangle.setOutlineThickness(0);
  rectangle.setFillColor(sf::Color::Blue);

  // used for adding delay in the game to make it playable
  sf::Clock delay;
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

    direction = localDirection;
    // signal the PacMan thread that the update value
    // has been written into the direction variable
    pthread_mutex_unlock(&waitForInput);
    
    // waiting for the gameEngine to coordinate in-between the 
    // PacMan and the Ghosts
    pthread_mutex_lock(&waitForGameEngine);    

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

    // ensures that the game engine can now render, after 
    // the relevant sprites have been draw on the window
    pthread_mutex_unlock(&waitForDraw);
    // ensures that the game engine has rendered everything 
    // before continuing further execution
    pthread_mutex_lock(&waitForRender);

    // add delay for the effect of FrameRate
    delay.restart();
    while(delay.getElapsedTime().asSeconds() < 0.1) {}
  }

}

// signal functions to destroy all the mutexes used within the entire process
void destroyMutexes() {
  pthread_mutex_destroy(&waitForGameEngine);
  pthread_mutex_destroy(&waitForDraw);
  pthread_mutex_destroy(&waitForInput);
  pthread_mutex_destroy(&waitForPacMan);
  pthread_mutex_destroy(&waitForRender);
}

int main() {

  //std::cout<<"Hello World"<<std::endl;
  
  // load the texture for PacMan
  pacManTex.loadFromFile("./Resources/Images/player.png");
  pacManSprite.setTexture(pacManTex);
  // assign the PacMan sprite the correct texture
  pacManSprite.setTextureRect(sf::IntRect(16,0,16,16));
  pacManSprite.setPosition(16, 16);
  // set the initial starting position
  pacManSprite.setPosition(16,16);
  
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
