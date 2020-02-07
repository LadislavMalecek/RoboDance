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

bool serialLog = false;

class Motor : public Servo {
  public:
    Motor(void) { _dir=1; }
    byte correction = 7;
    void go (int percentage) {
      writeMicroseconds((1500 - correction)+ _dir * percentage * 2);
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

String direction_to_string(byte direction){
  switch (direction){
  case DIRECTION_NORTH:
    return "North";
  case DIRECTION_EAST:
    return "East";
  case DIRECTION_SOUTH:
    return "South";
  case DIRECTION_WEST:
    return "West";
  default:
    return "UNKNOWN DIRECTION";
  }
}


// ----------------------------------------------------------------------------
// ------------------------- CHOREOGRAPHY PARSER ------------------------------
// ----------------------------------------------------------------------------
class ChoreographyParser {
  // direction 0 represents NORTH
  // direciton 1 represents WEST
  // direciton 2 represents SOUTH
  // direciton 3 represents EAST
  private:
    // unsigned int next_orientation = 0;
    // unsigned int next_possition_x = 0;
    // unsigned int next_possition_y = 0;

    // int next_time = 0;
    // bool preffer_column = true;

    // unsigned int current_orientation = 0;
    // unsigned int current_possition_x = 0;
    // unsigned int current_possition_y = 0;

    byte number_of_instructions = 0;

    byte instructions_direction_x[64];
    byte instructions_direction_y[64];
    int instructions_time[64];
    bool instructions_preffer_col[64];

    byte start_x, start_y = 0;
    byte start_orientation = 0;

    unsigned int current_string_possition = 0;

    String choreography = "";

    void skip_newline_chars(){
      while(choreography[current_string_possition] == '\n' || choreography[current_string_possition] == '\r'){
        current_string_possition++;
      }
    }

    // void copy_next_to_current(){
    //   current_orientation = next_orientation;
    //   current_possition_x = next_possition_x;
    //   current_possition_y = next_possition_y;
    // }

    void parse_first_line(String line){
      start_x = line[0] - 'a';
      start_y = line[1] - '1';

      switch(line[2]){
        case 'n':
        case 'N':
          start_orientation = DIRECTION_NORTH;
          // Serial.println("Starting possition dir: north");
          break;
        case 'e':
        case 'E':
          start_orientation = DIRECTION_EAST;
          // Serial.println("Starting possition dir: east");
          break;
        case 's':
        case 'S':
          start_orientation = DIRECTION_SOUTH;
          // Serial.println("Starting possition dir: south");
          break;
        case 'w':
        case 'W':
          start_orientation = DIRECTION_WEST;
          // Serial.println("Starting possition dir: west");
          break;
        default:
          // Serial.println("Invalid starting direction encountered.");
          break;
      }
      // Serial.println("Starting possition x: " + start_x);
      // Serial.println("Starting possition y: " + start_y);

      // start_x = next_orientation;
      // current_possition_x = next_possition_x;
      // current_possition_y = next_possition_y;
    }

    void parse_line(String line){
      // copy_next_to_current();
      // check if the first one is number
      if (isdigit(line[0]))
      {
        instructions_preffer_col[number_of_instructions] = false;
        instructions_direction_x[number_of_instructions] = line[1] - 'a';
        instructions_direction_y[number_of_instructions]  = line[0] - '1';
      }
      else
      {
        instructions_preffer_col[number_of_instructions] = true;
        instructions_direction_x[number_of_instructions]  = line[0] - 'a';
        instructions_direction_y[number_of_instructions]  = line[1] - '1';
      }

      if(serialLog){
        Serial.print("TIME RAW: '");
        Serial.print(line.substring(4, line.length()));
        Serial.print("', PARSED: ");
        Serial.println(line.substring(4, line.length()).toInt());
      }

      instructions_time[number_of_instructions] = line.substring(4, line.length()).toInt();
      number_of_instructions++;
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
      parse_first_line(next_line);
      next_line = get_next_line();
      while(next_line != ""){
        // Serial.println("Next line: " + next_line);
        parse_line(next_line);
        next_line = get_next_line();
      }

      if(serialLog){
        Serial.print("startO: ");
        Serial.println(start_orientation);
        Serial.print("startX: ");
        Serial.println(start_x);
        Serial.print("startY: ");
        Serial.println(start_y);
        for (byte i = 0; i < number_of_instructions; i++)
        {
          Serial.println();
          Serial.print("X: ");
          Serial.print(instructions_direction_x[i]);
          Serial.print(", Y: ");
          Serial.print(instructions_direction_y[i]);
          Serial.print(", T: ");
          Serial.print(instructions_time[i]);
          Serial.print(", P: ");
          Serial.print(instructions_preffer_col[i]);
        }
        Serial.println();
      }

    }

    byte get_start_x(){
      return start_x;
    }
    byte get_start_y(){
      return start_y;
    }
    byte get_start_orientation(){
      return start_orientation;
    }
    byte get_number_of_instructions(){
      return number_of_instructions;
    }

    byte get_instructions_x(byte i){
      return instructions_direction_x[i];
    }
    byte get_instructions_y(byte i){
      return instructions_direction_y[i];
    }
    bool get_instructions_pref_col(byte i){
      return instructions_preffer_col[i];
    }
    int get_instructions_time(byte i){
      return instructions_time[i];
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
    ChoreographyParser parser;
    byte current_parser_instruction = -1;
    byte current_direction = 0;
    byte current_x = 0;
    byte current_y = 0;

    byte next_x = 0;
    byte next_y = 0;
    int next_time = 0;
    byte next_direction = 0;
    bool next_x_first = true;
    
    bool move_to_next_parser_instruction(){
      if(serialLog){
        Serial.print("MOVING TO NEXT instruction: ");
      }

      if(current_parser_instruction == (parser.get_number_of_instructions() - 1)){
        if(serialLog){
          Serial.println("FINISHED");
        }
        return false;
      }
      current_parser_instruction++;
      if(serialLog){
        Serial.println(current_parser_instruction);
      }
      next_x = parser.get_instructions_x(current_parser_instruction);
      next_y = parser.get_instructions_y(current_parser_instruction);
      next_time = parser.get_instructions_time(current_parser_instruction);
      next_x_first = parser.get_instructions_pref_col(current_parser_instruction);
      if(serialLog){
        Serial.print("nextX: ");
        Serial.print(next_x);
        Serial.print(" , nextY: ");
        Serial.print(next_y);
        Serial.print(", nextTime: ");
        Serial.print(next_time);
        Serial.print(" xFirst: ");
        Serial.println(next_x_first);
      }
      return true;
    }

    void parse_string_choreography(String choreography){
      choreography.toLowerCase();
      parser = ChoreographyParser();
      parser.parse(choreography);

      current_x = parser.get_start_x();
      current_y = parser.get_start_y();
      current_direction = parser.get_start_orientation();
      current_parser_instruction = -1;
      move_to_next_parser_instruction();
    }

    void update_possition(){
      if(next_direction == DIRECTION_NORTH){
        current_y++;
      }
      if(next_direction == DIRECTION_SOUTH){
        current_y--;
      }
      if(next_direction == DIRECTION_WEST){
        current_x--;
      }
      if(next_direction == DIRECTION_EAST){
        current_x++;
      }
      current_direction = next_direction;
      if(serialLog){
        Serial.print("New possition: ");
        Serial.print(current_x);
        Serial.print(" ");
        Serial.println(current_y);
      }
    }

    bool is_finished_current_parser_instruction(){
      return current_x == next_x && current_y == next_y;
    }

    byte get_move_absolute_direction(){
      if(next_x_first){
        if(next_x != current_x){
          return get_move_x_absolute_direction();
        } else {
          return get_move_y_absolute_direction();
        }
      } else {
        if(next_y != current_y){
          return get_move_y_absolute_direction();
        } else {
          return get_move_x_absolute_direction();
        }
      }
    }

    byte get_move_x_absolute_direction(){
      if(current_x < next_x){
        return DIRECTION_EAST;
      } else {
        return DIRECTION_WEST;
      }
    }
    byte get_move_y_absolute_direction(){
      if(current_y < next_y){
        return DIRECTION_NORTH;
      } else {
        return DIRECTION_SOUTH;
      }
    }

    bool is_same_direction(byte dir1, byte dir2){
      return dir1 == dir2;
    }

    bool is_opposite_direction(byte dir1, byte dir2){
      return ((dir1 - dir2 + 4) % 4) == 2;
    }

    bool is_left_direction(byte dir1, byte dir2){
      return ((dir1 - dir2 + 4) % 4) == 1;
    }

    bool is_right_direction(byte dir1, byte dir2){
      return ((dir1 - dir2 + 4) % 4) == 3;
    }

    byte get_move_robot_local_direction(){
      next_direction = get_move_absolute_direction();
      if(serialLog){
        Serial.print("Current direction: ");
        Serial.print(direction_to_string(current_direction));
        Serial.print(", next direction: ");
        Serial.println(direction_to_string(next_direction));
      }

      if(is_same_direction(current_direction, next_direction)){
        return NAVIGATION_STRAIGHT;
      }

      if(is_opposite_direction(current_direction, next_direction)){
        return NAVIGATION_BACK;
      }

      if(is_left_direction(current_direction, next_direction)){
        return NAVIGATION_LEFT;
      }

      if(is_right_direction(current_direction, next_direction)){
        return NAVIGATION_RIGHT;
      }
    }
    
  public:

    void init_preload_choreography(){
      init_preload_choreography(0);
    }
    void init_preload_choreography(int i){
      // String choreography_0 = "B2S\nc2 t120\nd4 t0\nb5 t0\na2 t368\ne2 t452\n1c t0\ne1 t600";
      String choreography_0 = "A1N\n3a t30\nb2 t60\nc4 t90\nd3 t120\ne5 t180";
      parse_string_choreography(choreography_0);
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
    byte get_cross_direction(){
      // return parsed_instructions[current_step];
      return get_move_robot_local_direction();   
    }

    // returns the absolute timestamp in miliseconds
    // get the time at which we should leave the current cross
    int get_cross_leave_time(){
      return next_time;
    }
    
    // move navigation to next instruction
    // signal that the navigation on this cross was completed and update the orientation
    // return false if the whole sequence is completed
    bool move_to_next_instruction(){
      update_possition();
      if(is_finished_current_parser_instruction()){
        return move_to_next_parser_instruction();
      }
      return true;
    }
};


// ----------------------------------------------------------------------------
// ------------------------------- ROBOT LOGICS -------------------------------
// ----------------------------------------------------------------------------

Motor leftMotor, rightMotor;
Navigation navigation;

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


void printout_choreography_execute(){
  bool finished = false;
  Serial.println("Local instructions:");

  while(!finished){
    byte direction = navigation.get_cross_direction();
    int time = navigation.get_cross_leave_time();
    if(serialLog){
      switch (direction){
        case NAVIGATION_STRAIGHT:
          Serial.print("straight");
          break;
        case NAVIGATION_BACK:
          Serial.print("back");
          break;
        case NAVIGATION_LEFT:
          Serial.print("left");
          break;
        case NAVIGATION_RIGHT:
          Serial.print("right");
          break;
        default:
          Serial.println("ERROR");
          break;
      }
      Serial.print(", time: ");
      Serial.println(time);
    }
    finished = !navigation.move_to_next_instruction();
  }
}


void setup() {
  navigation = Navigation();
  navigation.init_preload_choreography();

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

// void turn_90_left() {
//   left_wheel_velocity = -25;
//   right_wheel_velocity = 25;
//   set_velocities();

//   // Rotate till the middle sensor sees black
//   bool turn_complete = false;
//   bool check = true;
//   while (!turn_complete) {
//     read_inputs();
//     if (middle_left == BLACK) {
//       delay(300);
//       stopCargo();
//       turn_complete = true;
//     }
//   }
// }

void turn_90_left() {
  left_wheel_velocity = -25;
  right_wheel_velocity = 25;
  set_velocities();

  rotate();
}

// void turn_90_right() {
//   left_wheel_velocity = 25;
//   right_wheel_velocity = -25;
//   set_velocities();

//   // Rotate till the middle sensor sees black
//   bool turn_complete = false;
//   bool check = true;
//   while (!turn_complete) {
//     read_inputs();
//     if (middle_right == BLACK) {
//       delay(300);
//       stopCargo();
//       turn_complete = true;
//     }
//   }
// }

void turn_90_right() {
  left_wheel_velocity = 25;
  right_wheel_velocity = -25;
  set_velocities();

  rotate();  
}

void rotate() {
  // Rotate till the middle sensor sees black

  digitalWrite(LED, LOW);
  // bool close_to_end = false;
  // while (!turn_complete) {
  //   read_inputs();
  //   if (!close_to_end) {
  //     if (middle == WHITE) {
  //       close_to_end = true;
  //     }
  //   } else if (middle == BLACK) {
  //     stopCargo();
  //     turn_complete = true;
  //   }
  // }
  delay(500);
  while (true) {
    read_inputs();
    if (middle == BLACK) {
      digitalWrite(LED, HIGH);
      stopCargo();
      break;
    }
  }
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
  if (button == 1) {
    start_dance_time = millis();
    state = STATE_NAVIGATION_ON_CROSSING;
  }
}

void control_go_to_next_crossing(){

  // Go straight for some time to be centered on the cross.
  if (border_left == BLACK || border_right == BLACK) {
    int previous_time = millis();
    bool cargo_centered = false;
    while (!cargo_centered) {
      straight();

      // Loop straight() for 250 milliseconds, then change state
      if (millis()-previous_time > 250) {
        state = STATE_NAVIGATION_ON_CROSSING;
        //we reached new crossing, ask the navigation for new instructions
        bool success = navigation.move_to_next_instruction();
        if(!success){
          state = STATE_FINAL;
          return;
        }
        cargo_centered = true;
      }
    }
  }
  
  if (middle_left == BLACK) {
    //turn_slightly_left();
    turnLeft();
  } else if (middle_right == BLACK) {
    //turn_slightly_right();
    turnRight();
  } else {
    straight();
  }
}

void control_navigation_on_crossing(unsigned long current_time){
  // int timestamp = navigation.get_cross_leave_time();
  // if (timestamp > current_time) {
  //   delay(timestamp - current_time);
  // }

  int instruction = navigation.get_cross_direction();

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
      control_go_to_next_crossing();
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
}