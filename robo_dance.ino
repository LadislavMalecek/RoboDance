#include <Servo.h>

#define STATE_WAITING_START 0
#define STATE_GO_TO_NEXT_CROSSING 1
#define STATE_NAVIGATION_ON_CROSSING 2
#define STATE_FINAL 3
#define STATE_NORMAL_OPERATION 4


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
      writeMicroseconds(1500 + _dir * percentage * 2);
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

#define DIRECTION_NORTH 0
#define DIRECTION_EAST 1
#define DIRECTION_SOUTH 2
#define DIRECTION_WEST 3


// ----------------------------------------------------------------------------
// ------------------------- CHOREOGRAPHY PARSER ------------------------------
// ----------------------------------------------------------------------------
class ChoreographyParser {
  // direction 0 represents NORTH
  // direciton 1 represents WEST
  // direciton 2 represents SOUTH
  // direciton 3 represents EAST
  private:
    unsigned int next_orientation = 0;
    unsigned int next_possition_x = 0;
    unsigned int next_possition_y = 0;

    unsigned int current_orientation = 0;
    unsigned int current_possition_x = 0;
    unsigned int current_possition_y = 0;

    byte instructions_direction[200];
    unsigned int instructions_time[200];

    unsigned int current_string_possition = 0;

    String choreography = "";

    void skip_newline_chars(){
      while(choreography[current_string_possition] == '\n' || choreography[current_string_possition] == '\r'){
        current_string_possition++;
      }
    }

    void parse_first_line(String line){
      next_possition_x = line[0] - 'a';
      next_possition_y = line[1] - '1';

      switch(line[3]){
        case 'n':
          next_orientation = DIRECTION_NORTH;
          Serial.println("Starting possition dir: north");
          break;
        case 'e':
          next_orientation = DIRECTION_EAST;
          Serial.println("Starting possition dir: east");
          break;
        case 's':
          next_orientation = DIRECTION_SOUTH;
          Serial.println("Starting possition dir: south");
          break;
        case 'w':
          next_orientation = DIRECTION_WEST;
          Serial.println("Starting possition dir: west");
          break;
        default:
          Serial.println("Invalid starting direction encountered.");
          break;
      }
      Serial.println("Starting possition x: " + next_possition_x);
      Serial.println("Starting possition y: " + next_possition_y);

      current_orientation = next_orientation;
      current_possition_x = next_possition_x;
      current_possition_y = next_possition_y;
    }

    void parse_line(String line){

    }

    String get_next_line(){
      int start_possition = current_string_possition;
      int end_possition = 0;
      
      // Serial.println("Before cycle");
      // Serial.println(choreography.length());
      
      while(choreography.length() - 1 >= current_string_possition){

        // Serial.println("X");
        // Serial.println("Current char: " + choreography[current_string_possition]);
        // parse with \n \r, skip the trailing ones
        if(choreography[current_string_possition] == '\n' || choreography[current_string_possition] == '\r'){
          end_possition = current_string_possition - 1;
          skip_newline_chars();
          break;
        }
        current_string_possition++;
      }
      // if end of the file
      if(end_possition == 0){
        end_possition = current_string_possition;
      }
      if(end_possition == start_possition){
        return "";
      }
      return choreography.substring(start_possition, end_possition + 1);
    }


  public:
    void parse(String choreography_string){
      // Serial.println("entered parse");
      choreography = choreography_string;
      // Serial.println("before string create");

      String next_line = String("_");
      // Serial.println("before get_next_line");
      next_line = get_next_line();
      while(next_line != ""){
        Serial.println("Next line: " + next_line);
        next_line = get_next_line();
      }
    }
};

// ----------------------------------------------------------------------------
// ----------------------------- NAVIGATION -----------------------------------
// ----------------------------------------------------------------------------

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
    int current_direction = 0;
    int current_navigation_instruction = 0;
    
    int parsed_timesteps[100];
    byte parsed_instructions[100];
    byte current_step = 0;
    byte number_of_steps = 0;

    

    void parse_string_choreography(String choreography){
      Serial.println("entering parse_string_choreography");
      choreography.toLowerCase();
      Serial.println("creating parser");
      ChoreographyParser parser = ChoreographyParser();
      parser.parse(choreography);
    }

    
    
  public:

    // not implemented yet, just call it once at the start of the run
    void init_preload_choreography(){
      init_preload_choreography(0);
    }
    // not implemented yet, call this if you want to initialize a specific choreography
    void init_preload_choreography(int i){
      // String choreography_0 = "A1N\nc2 t120\nd4 t0\nb5 t0\na2 t368\ne2 t452\n1c t0\ne1 t600";
      // Serial.println("preload choreography with:");
      // Serial.println(choreography_0);
      // parse_string_choreography(choreography_0);

      parsed_timesteps[0] = 0;
      parsed_timesteps[1] = 10000;
      parsed_timesteps[2] = 20000;
      parsed_timesteps[3] = 30000;
      parsed_timesteps[4] = 40000;

      parsed_instructions[0] = NAVIGATION_LEFT;
      parsed_instructions[1] = NAVIGATION_STRAIGHT;
      parsed_instructions[2] = NAVIGATION_STRAIGHT;
      parsed_instructions[3] = NAVIGATION_BACK;
      parsed_instructions[4] = NAVIGATION_RIGHT;

      number_of_steps = 5;

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
      return parsed_instructions[current_step];
      // return LEFT;      
    }

    // returns the absolute timestamp in miliseconds
    // get the time at which we should leave the current cross
    int get_cross_leave_time(){
      return parsed_timesteps[current_step];
      // return 10202; // this is something more than 10 seconds
    }
    
    // move navigation to next instruction
    // signal that the navigation on this cross was completed and update the orientation
    bool move_to_next_instruction(){
      if(current_step < number_of_steps){
        current_step++;
      }
    }
};


// ----------------------------------------------------------------------------
// ------------------------------- ROBOT LOGICS -------------------------------
// ----------------------------------------------------------------------------

Motor leftMotor, rightMotor;
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

int state = STATE_WAITING_START;

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
  // Rotate till the middle sensor sees black
  bool turn_complete = false;
  bool close_to_end = false;
  while (!turn_complete) {
    turnLeft();
    if (middle_right == BLACK) {
      close_to_end = true;
    }
    if (close_to_end && middle == BLACK) {
      turn_complete = true;
    }
  }
  stopCargo();
}

void turn_90_right() {
  // Rotate till the middle sensor sees black
  bool turn_complete = false;
  bool close_to_end = false;
  while (!turn_complete) {
    turnRight();
    if (middle_left == BLACK) {
      close_to_end = true;
    }
    if (close_to_end && middle == BLACK) {
      turn_complete = true;
    }
  }
  stopCargo();
}

void turn_back() {
  turn_90_left();
  turn_90_left();
  stopCargo();
}


// ----------------------------------------------------------------------------
// --------------------------- NORMAL OPERATION -------------------------------
// ----------------------------------------------------------------------------

void control_waiting_start(){
  stopCargo();
  // if BUTTON == LOW
  if (button == 0) {
    state = STATE_GO_TO_NEXT_CROSSING;
    start_dance_time = millis();
  }
}

void control_go_to_next_crossing(unsigned long current_time){

  // Go straight for some time to be centered on the cross.
  if (border_left == BLACK || border_right == BLACK) {
    int previous_time = current_time;
    bool cargo_centered = false;
    while (!cargo_centered) {
      straight();

      // Loop straight() for 500 milliseconds, then change state
      if (current_time-previous_time > 500) {
        state = STATE_NAVIGATION_ON_CROSSING;
        cargo_centered = true;
      }
    }
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
  state = STATE_GO_TO_NEXT_CROSSING;
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