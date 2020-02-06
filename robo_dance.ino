#include <Servo.h>

#define STATE_WAITING_START 0
#define STATE_GO_TO_NEXT_CROSSING 1
#define STATE_NAVIGATION_ON_CROSSING 2
#define STATE_FINAL 3

#define LEFT 12
#define RIGHT 13
#define MIDDLE 5
#define MIDDLE_LEFT 4
#define MIDDLE_RIGHT 6
#define BORDER_LEFT 3
#define BORDER_RIGHT 7
#define BUTTON 2
#define LED 11

#define BLACK 0
#define WHITE 1


class Motor : public Servo {
  public:
    Motor(void) { _dir=1; }
    void go (int percentage) {
      writeMicroseconds(1500+_dir*percentage*2);
    }
    void setDirection(bool there) {
      if(there)
        _dir=1;
      else
        _dir=-1;
    }
    private:
    int _dir;
};



#define NAVIGATION_LEFT 0
#define NAVIGATION_STRAIGHT 1
#define NAVIGATION_RIGHT 2
#define NAVIGATION_BACK 3

// Navigation part of the robot
// it translates the global instructions to local robot-centric ones
// instead of having absolute coordinates it creates a simple series of local ones
// on every step it returns the current local direction that the robot should take
// left, right, straight or back
// then it returns the timestamp of the current action
//
// so first call the init method
// then get the current instruction using the get_current_navigation and get_current_time
// you will have:
// direction
// time
// rotate in the direction and leave the cross at the time
class Navigation {
  private:
    // direction 0 represents NORTH
    // direciton 1 represents WEST
    // direciton 2 represents SOUTH
    // direciton 3 represents EAST
    
    int current_direction = 0;
    int current_navigation_instruction = 0;
    
    int parsed_timesteps[100];
    byte parsed_instructions[100];

    void parse_string_choreography(String choreography){ }

    
    
  public:

    // not implemented yet, just call it once at the start of the run
    void init_preload_choreography(){
      return;
    }
    // not implemented yet, call this if you want to initialize a specific choreography
    void init_preload_choreography(int i){
      return;
    }
    // returns the total number of available preloaded choreographies
    int number_of_preloaded_choreographies(){
      return 1;
    }
    void init_load_choreography_from_serial(String choreography){
      return;
    }


    // ------------------------------------------------------------------------
    // Main navigation methods
    // ------------------------------------------------------------------------
    
    // gets one of LEFT, RIGHT, STRAIGHT, BACK
    // what to do on the current cross
    int get_cross_direction(){
      return LEFT;      
    }

    // returns the absolute timestamp in miliseconds
    // get the time at which we should leave the current cross
    int get_cross_leave_time(){
      return 10202; // this is something more than 10 seconds
    }
    
    // move navigation to next instruction
    // signal that the navigation on this cross was completed and update the orientation
    move_to_next_instruction(){
      return;
    }
}

motor leftMotor, rightMotor;
Navigation nav;

unsigned long current_time = 0;
unsigned long start_dance_time = 0;

int middle = 0;
int middle_left = 0;
int middle_right = 0;
int border_left = 0;
int border_right = 0;
int button = 0;

int left_wheel_velocity = 0;
int right_wheel_velocity = 0;

int lastButtonState = LOW;
int ledState = HIGH;

int state = STATE_NORMAL_OPERATION;

void read_inputs() {
  middle = digitalRead(MIDDLE);
  middle_left = digitalRead(MIDDLE_LEFT);
  middle_right = digitalRead(MIDDLE_RIGHT);
  border_left = digitalRead(BORDER_LEFT);
  border_right = digitalRead(BORDER_RIGHT);
  button = !digitalRead(BUTTON);
}

void setup() {
  leftMotor.attach(LEFT, 500, 2500);
  leftMotor.setDirection(true);
  rightMotor.attach(RIGHT, 500, 2500);
  rightMotor.setDirection(false);
  pinMode(MIDDLE, INPUT);
  pinMode(MIDDLE_LEFT, INPUT);
  pinMode(MIDDLE_RIGHT, INPUT);
  pinMode(BORDER_LEFT, INPUT);
  pinMode(BORDER_RIGHT, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}


void stopCargo() {
  left_wheel_velocity = 0;
  right_wheel_velocity = 0;
  set_velocities();
}

void straight() {
  left_wheel_velocity = 50;
  right_wheel_velocity = 50;
  set_velocities();
}

void turnLeft() {
  left_wheel_velocity = 0;
  right_wheel_velocity = 25;
  set_velocities();
}

void turnRight() {
  left_wheel_velocity = 25;
  right_wheel_velocity = 0;
  set_velocities();
}

void turnLeftBack() {
  left_wheel_velocity = 0;
  right_wheel_velocity = -25;
  set_velocities();
}

void turnRightBack() {
  left_wheel_velocity = -25;
  right_wheel_velocity = 0;
  set_velocities();
}

void turn_slightly_left() {
  left_wheel_velocity = max(0, left_wheel_velocity - 5);
  right_wheel_velocity = min(100, right_wheel_velocity + 5);
  set_velocities();
}

void turn_slightly_right() {
  left_wheel_velocity = min(0, left_wheel_velocity + 5);
  right_wheel_velocity = max(100, right_wheel_velocity - 5);
  set_velocities();
}

void set_velocities() {
  leftMotor.go(left_wheel_velocity);
  rightMotor.go(right_wheel_velocity);
}

void turn_90_left() {
  // alternative - rotate till the middle sensor sees black
  bool was_white = false;
  turnLeft();
  while (true) {

  }

  turnLeft();
  // TODO - proper delay
  stopCargo();
}

void turn_90_right() {
  turnRight();
  // TODO - proper delay
  stopCargo();
}

void turn_back() {
  turnLeft();
  // TODO - proper delay
  stopCargo();
}





// ----------------------------------------------------------------------------
// --------------------------- NORMAL OPERATION -------------------------------
// ----------------------------------------------------------------------------

void control_waiting_start(){
  stopCargo();
  state = STATE_GO_TO_NEXT_CROSSING;
  start_dance_time = millis();
}

void control_go_to_next_crossing(unsigned long current_time){
  if (border_left == BLACK || border_right == BLACK) {
    // Go straight for some time to be centered on the cross
    
    state = control_navigation_on_crossing;
  }
  
  if (middle_left == BLACK) {
    turn_slightly_left();
  } else if (middle_right == BLACK) {
    turn_slightly_right();
  } else {
    straight();
  }
}

void control_navigation_on_crossing(unsigned long current_time){
  int timestamp = nav.get_cross_leave_time();
  if (timestamp > current_time) {
    delay(timestamp - current_time);
  }

  int instruction = nav.get_cross_direction();
  switch(instruction) {
    case NAVIGATION_LEFT:
      turn_90_left();
      break;
    case NAVIGATION_RIGHT:
      turn_90_right();
      break;
    case NAVIGATION_STRAIGHT:
      break;
    case NAVIGATION_BACK:
      turn_back();
      break;
  }
  state = control_go_to_next_crossing;
}

void control_final() {
  stopCargo();
}


void control(unsigned long current_time) {
  switch(state){
    case STATE_WAITING_START:
      control_waiting_start();
      return;
    case STATE_GO_TO_NEXT_CROSSING:
      control_go_to_next_crossing(current_time);
      return;
    case STATE_NAVIGATION_ON_CROSSING:
      control_navigation_on_crossing(current_time);
      return;
    case STATE_FINAL:
      control_final();
      return;
  }
}


void loop() {
  read_inputs();
  current_time = millis()-start_dance_time;
  control(current_time);
  delay(10);
}