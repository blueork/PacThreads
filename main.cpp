// Current Task
// Multi-Thread the PacMan
#include <iostream>
#include <SFML/Graphics.hpp>
#include <pthread.h>

#define s std
#define height 31
#define width 28

// Declare all the Global Variables here for the all the threads to access
// the underlying 2D Maze
sf::RenderWindow window;
sf::Texture pacManTex;
sf::Sprite pacManSprite;

int direction = 0;
pthread_mutex_t lock1, lock2, waitForPacMan, waitForInput;

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

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  int localDirection;

  while(true) {

    pthread_mutex_lock(&waitForInput);
    pthread_mutex_lock(&lock1);      
        
        localDirection = direction;
    
    pthread_mutex_unlock(&lock1);

    pthread_mutex_lock(&lock2);
    if(localDirection) {
      
      sf::Vector2f currPos = pacManSprite.getPosition();
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
    pthread_mutex_unlock(&waitForPacMan);
    pthread_mutex_unlock(&lock2);
    
  }
}

// the userInterface thread
// display the grid
// rendering the graphics
// responsible for taking input


void userInterface() {

  pthread_mutex_init(&lock1, NULL);
  pthread_mutex_init(&lock2, NULL);
  pthread_mutex_init(&waitForPacMan, NULL);
  pthread_mutex_init(&waitForInput, NULL); 
   
  pthread_mutex_lock(&waitForPacMan);
  pthread_mutex_lock(&waitForInput);

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_t pacManThread;
  
  for(int i = 0; i < 1; ++i)
    pthread_create(&pacManThread, &attr, pacMan, NULL);



  window.create(sf::VideoMode(448, 496), "Pac-Man");

  // Rectangle Blocks to make up the wall
  sf::RectangleShape rectangle;
  rectangle.setSize(sf::Vector2f (16, 16));
  rectangle.setOutlineThickness(0);
  rectangle.setFillColor(sf::Color::Blue);

  sf::Clock delay;
  int localDirection = 0;
  // to check for user inputs
  sf::Event event;
  bool check = false;
  while(window.isOpen()) {

    while(window.pollEvent(event)) {
      if(event.type == sf::Event::Closed) {
        window.close(); 
        s::cout<<"check1\n";
        check = true;
      }
      if(event.type == sf::Event::KeyPressed) {
        if(event.key.code == sf::Keyboard::W) {
          //delta_y = -1;
          // s::cout<<"W\n";
          localDirection = 1;
        }
        else if(event.key.code == sf::Keyboard::S) {
          // delta_y = 1;
          // s::cout<<"S\n";
          localDirection = 2;
        }
        else if(event.key.code == sf::Keyboard::A) {
          //delta_x = -1;
          // s::cout<<"A\n";
          localDirection = 3;
        }
        else if(event.key.code == sf::Keyboard::D) {
          //delta_x = 1;
          // s::cout<<"D\n";
          localDirection = 4;
        }
      }
    }

    if(check)
      s::cout<<"Check 2\n";

    pthread_mutex_lock(&lock1);   
      direction = localDirection;
    pthread_mutex_unlock(&waitForInput);
    pthread_mutex_unlock(&lock1);

    
    
    //pacMan(direction);

    pthread_mutex_lock(&waitForPacMan);    
    pthread_mutex_lock(&lock2);

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
    // render everything
    window.display();

    pthread_mutex_unlock(&lock2);

    // add delay for the effect of FrameRate
    delay.restart();
    while(delay.getElapsedTime().asSeconds() < 0.1) {}

    
    if(check)
      s::cout<<"Check 3\n";

  }
s::cout<<"Hello\n";
s::cout<<"World\n";
for(int i = 0; i < 1; ++i)
  pthread_cancel(pacManThread);
    if(check)
      s::cout<<"Check 4\n";
  
  // cancel all the remaining threads to stop the process


  pthread_exit(NULL);
}


int main() {

  //std::cout<<"Hello World"<<std::endl;
  
  // load the texture for PacMan
  pacManTex.loadFromFile("./Resources/Images/player.png");
  pacManSprite.setTexture(pacManTex);
  // assign the PacMan sprite the correct texture
  pacManSprite.setTextureRect(sf::IntRect(16,0,16,16));
  pacManSprite.setPosition(16, 16);

  pacManSprite.setPosition(16,16);
  userInterface();


  return 0;
}
