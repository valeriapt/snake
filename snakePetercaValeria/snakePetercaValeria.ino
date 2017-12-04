#include "LedControl.h"
#include <LiquidCrystal.h>

LedControl matrice = LedControl(12, 11, 10, 1);
LiquidCrystal lcd(9, 6, 5, 4, 3, A4);

const int buttonPinStart = 7;
const int buttonPinSelectEasy = 8;
const int buttonPinSelectHard = A5;
#define Y_PIN A2
#define X_PIN A0

int debugTime = 100;
int snakeLength = 3;
int speed = 300;
bool eat = true;
bool gameOver = false;

//coordonatele de inceput ale sarpelui
int x = 3, y = 4;
// coordonate mancare
int foodX;
int foodY;
// directie joystick/sarpe
const short up     = 0;
const short right  = 1;
const short down   = 2;
const short left   = 3;
int direction = -1;
int openButtonStatus = LOW;
int difficultyStatus = 0;

// pt matrice
//mat
bool mat[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 1, 0, 0, 0, 0}, 
    {0, 0, 0, 1, 0, 0, 0, 0}, 
    {0, 0, 0, 1, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}
};
bool happyFace[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 1, 1, 0, 0, 1, 1, 0}, 
    {0, 1, 1, 0, 0, 1, 1, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 1, 0, 0, 0, 0, 1, 0}, 
    {0, 0, 1, 1, 1, 1, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}
};
bool sadFace[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 1, 1, 0, 0, 1, 1, 0}, 
    {0, 1, 1, 0, 0, 1, 1, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 1, 1, 1, 1, 0, 0}, 
    {0, 1, 0, 0, 0, 0, 1, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}
};

bool difficulty[8][8] = {
    {0, 0, 1, 0, 0, 1, 1, 0}, 
    {0, 0, 1, 0, 0, 0, 1, 0}, 
    {0, 0, 1, 0, 0, 1, 1, 0}, 
    {0, 0, 1, 0, 0, 1, 0, 0}, 
    {0, 0, 1, 0, 0, 1, 1, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}
};
// pentru a tine minte cat timp sa apara fiecare celula din sarpe
long age[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 2, 0, 0, 0, 0}, 
    {0, 0, 0, 3, 0, 0, 0, 0}, 
    {0, 0, 0, 4, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}
};
// retine unde se afla mancarea
bool foodMap[8][8];


// reincep functia
void(* restart) (void) = 0;


void setup() {
  unsigned long  time = millis();
  while(time < 30000 && openButtonStatus == LOW){   
    time=millis();
    openButtonStatus = digitalRead(buttonPinStart);
  }
  if(openButtonStatus == HIGH){
     initialization();
     chooseDifficulty();
     updateMap();
     waitMove();
     generateFood();
     updateMap();
     updateLCD();     
  } 
}

void loop() {
  // ma reasigur ca intru in joc doar dupa ce a apasat pe butonul de start jucatorul
  if(openButtonStatus == HIGH){
    // jocul se considera castigat daca sarpele a ajuns la lungimea 20( scorul 17)
    if(gameOver == false && snakeLength <= 20){
      continueGame();
    }
    else if(gameOver == true){
      lostGame();
      
     // int time=millis()+10000;
     // while(millis()<time){}
     // restartGame();   
    }
    else{
       winGame();
    }
  }
}

/*
void restartGame(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start again?");
  Serial.print("da");
  int firstTime=millis();
  int time=millis() + 50000;
  openButtonStatus == LOW;
  while(millis() < time && openButtonStatus == LOW){
    openButtonStatus = digitalRead(buttonPinStart);
  }
  if(openButtonStatus == HIGH)
      restart();
}
*/


// pe matrice afisez:"1","2"(cele 2 optiuni)
// 1-> poate trece prin pereti
// 2-> nu poate
// jucatorul alege apasand pe unul dintre cele 2 butoane mici
// are 30 de secunde timp in care sa aleaga

void chooseDifficulty(){
  matrice.clearDisplay(0);
  for(int row = 0; row < 8; row++) {
     for(int col = 0; col < 8; col++){
        matrice.setLed(0, row, col, difficulty[row][col]);   
     }
  }
  long timestamp = millis() + 30000;
  while(millis() < timestamp){
    int readBtn;
    readBtn = digitalRead(buttonPinSelectEasy);
    if(readBtn == HIGH){
      difficultyStatus=1;
      break;
    }
    readBtn = digitalRead(buttonPinSelectHard);
    if(readBtn == HIGH){
      difficultyStatus=2;
      break;
    }
   }
}

void continueGame(){
  generateFood();
  scanJoistyck();
  calculateSnake();
  updateMap(); 
  updateLCD();
  updateSpeed();
}
// afisez o fata trista si pe LCD scriu un mesaj urmat de scor
void lostGame(){
  matrice.clearDisplay(0);
  for(int row = 0; row < 8; row++){
    for(int col = 0; col < 8; col++) {
       matrice.setLed(0, row, col, sadFace[row][col]);   
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over!!  ");
  lcd.setCursor(0, 1);
  lcd.print("Your score is:");
  lcd.print(snakeLength-3);
}

// afisez o fata fericita si pe LCD scriu un mesaj 
void winGame(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Well done!  ");
  lcd.setCursor(0, 1);
  lcd.print("You won!!! ");
  matrice.clearDisplay(0);
  for(int row = 0; row < 8; row++) {
    for(int col = 0; col < 8; col++) {
      matrice.setLed(0, row, col, happyFace[row][col]);   
    }
  }
}
   // in timpul jocului, pe ecran va aparea scorul la momentul actual 
   // am pus snakeLength-3 pt ca la inceputul jocului sarpele are lungimea 3, nu porneste de la 0
void updateLCD(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Your score is:");
  lcd.setCursor(0, 1);
  lcd.print(snakeLength-3);
}
// pentru a nu fi prea usor, viteza se mareste in timpul jocului
void updateSpeed(){
  if(snakeLength == 8)speed = 200;
  if(snakeLength == 13)speed = 100;
}

void initialization() {
  pinMode(buttonPinStart, INPUT);
  pinMode(buttonPinSelectEasy, INPUT);
  pinMode(buttonPinSelectHard, INPUT);
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  Serial.begin(9600);
  matrice.shutdown(0, false);         
  matrice.setIntensity(0, 8); 
  matrice.clearDisplay(0);
  lcd.begin(16, 2);  
  lcd.print("You can start!");   
}

// afisez sarpele si mancarea( punctul pe care trebuie sa il manance)
void updateMap(){
  for(int row = 0; row < 8; row++) {
     for(int col = 0; col < 8; col++) {
        if(foodMap[row][col]){
           matrice.setLed(0, row, col, foodMap[row][col]);
        }
        else{
           matrice.setLed(0, row, col, mat[row][col]);
        }
     }
  }    
}

// while-> pentru a citi joystycul
// citesc valorile X_PIN, Y_PIN cat timp nu am miscat joystickul
void waitMove(){
  while(direction == -1){
     analogRead(Y_PIN) < 200 ? direction = up    : 0;
     analogRead(Y_PIN) > 800 ? direction = up    : 0; 
     analogRead(X_PIN) < 200 ? direction = left  : 0;
     analogRead(X_PIN) > 800 ? direction = right : 0;  
   }
   // generez o noua casuta pe care sa pun mancare
  foodX = random(8);
  foodY = random(8);
}

// daca a mancat deja, verific daca casutele alese pt noua mancare sunt libere. Daca nu sunt, generez alta casuta pt mancare
void generateFood(){
  
  if(eat){
    while(mat[foodY][foodX] == 1){
      foodX = random(8);
      foodY = random(8);
    }
    //daca nu a mancat, marchez casuta pt a putea fi afisata
    //si resetez statusul eat
    mat[foodY][foodX] = 1; 
    foodMap[foodY][foodX] = 1;
    eat = false;  
  }
}

void scanJoistyck(){
  // tin minte in ce directie a mers data trecuta
  // pentru a nu face un pas de 180 de grade
  int previousDirection = direction;
  long timestamp = millis() + speed;
  // timpul in care jucatorul tebuie sa miste joysticul( sarpele)
  while(millis() < timestamp){
    analogRead(Y_PIN) < 200 ? direction = up    : 0;
    analogRead(Y_PIN) > 800 ? direction = down  : 0;
    analogRead(X_PIN) < 200 ? direction = left  : 0;
    analogRead(X_PIN) > 800 ? direction = right : 0;
       
       // pentru a nu face un pas de 180 de grade 
       // in caz de s-a ales asa, se va considera ca urmatoarea miscare a sarpelui e un pas inainte
    direction + 2 == previousDirection ? direction = previousDirection : 0;
    direction - 2 == previousDirection ? direction = previousDirection : 0;
    // clipire a mancarii (pentru a se deosebi de sarpe)
    if(timestamp - millis() < 1)
      matrice.setLed(0, foodY, foodX, 1);
    else if(timestamp - millis() < speed / 4)
             matrice.setLed(0, foodY, foodX, 0);
         else if(timestamp - millis() < speed / 2)
             matrice.setLed(0, foodY, foodX, 1);
              else if(timestamp - millis() < 3 * speed / 4)
             matrice.setLed(0, foodY, foodX, 0);
  }
}
// mutam sarpele in functie de directia calculata la functia scanJoystick 
void calculateSnake(){
  switch (direction) {
    case up:
        y--;
        // pentru cazul in care sarpele trece prin perete
        fixWall();
        mat[y][x] = 1;
        age[y][x]++;
        break;
        
    case right:
        x++;
        fixWall();
        mat[y][x] = 1;
        age[y][x]++;
        break;
    
    case down:
        y++;
        fixWall();
        mat[y][x] = 1;
        age[y][x]++;
        break;
    
    case left:
        x--;
        fixWall();
        mat[y][x] = 1;
        age[y][x]++;
        break;
   }
   detectBody();
   handleFood();
   updateAge();
}

void fixWall(){
  // sarpele trece prin perete numai daca e pe nivelul de dificultate 1
  if(difficultyStatus==1){  
    x < 0 ? x += 8 : 0;
    x > 7 ? x -= 8 : 0;
    y < 0 ? y += 8 : 0;
    y > 7 ? y -= 8 : 0;
  }
  else{ 
    if(x < 0 || x > 7 || y > 7 || y < 0) gameOver=true; 
  }
}

void set(int x, int y, bool state){
  mat[y][x] = state;
  age[y][x]++;
}
// verific daca casuta pe care a vrut sa se deplaseze face parte din corpul sarpelui
// daca e 1 e capul sau mancarea
void detectBody(){
  if(age[y][x] > 3){
    gameOver = true;
  }
}
// verifica daca sarpele a ajuns pe mancare
void handleFood(){
  if(foodMap[y][x]){
    foodMap[y][x] = 0;
    if(gameOver == false) snakeLength++;
    eat = true;
  }
}

//modifica lungimea sarpelui 
void updateAge(){
  for(int row = 0; row < 8; row++) {
    for(int col = 0; col < 8; col++) {
      // daca age[row][col]>mai mare decat lungimea sarpelui atunci sarpele a trecut de acel patratel
      if(age[row][col] > snakeLength || age[row][col] == 0){
        age[row][col] = 0;
         mat[row][col] = 0;
       }
       // daca age[row][col]!=1,-1 atunci e sarpe, si ii creste nr 
       if(age[row][col] != 0 && age[row][col] != -1){
          age[row][col]++;
       }   
    }
  }
}




