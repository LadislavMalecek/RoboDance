#include <Servo.h>

bool serialLog = false;
bool dummyNavigation = false;

class Motor : public Servo {
  private:
    int _dir;
    byte _correction;

  public:
    void go (int percentage) {
      writeMicroseconds((1500 - _correction ) + (_dir * percentage * 2));
    }

    void set_direction(bool there) {
      if(there){
        _dir=1;
      } else {
        _dir=-1;
      }
    }

    void set_correction(byte correction){
      _correction = correction;
    }
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

String navigation_to_string(byte navigation){
  switch (navigation){
  case NAVIGATION_STRAIGHT:
    return "straight";
  case NAVIGATION_BACK:
    return "back";
  case NAVIGATION_LEFT:
    return "left";
  case NAVIGATION_RIGHT:
    return "right";
  default:
    return "UNKNOWN NAVIGATION";
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
      char current = choreography[current_string_possition];
      while(current == '\n' || current == '\r' || current == '\t' || current == ',' || current == ';'){
        current_string_possition++;
        current = choreography[current_string_possition]; 
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
        char current = choreography[current_string_possition];
        if(current == '\n' || current == '\r' || current == '\t'|| current == ',' || current == ';'){
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
      String choreography_0 = "B2S,c2 t100,d2 t200,b5 t300,e2 t400,d2 t500,c2 t0,b2 t600";
      // String choreography_0 = "A1N\n3a t30\nb2 t60\nc4 t90\nd3 t120\ne5 t180";
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
      if(dummyNavigation){
        return NAVIGATION_BACK;
      }
      // return parsed_instructions[current_step];
      return get_move_robot_local_direction();   
    }

    // returns the absolute timestamp in miliseconds
    // get the time at which we should leave the current cross
    int get_cross_leave_time(){
      if(dummyNavigation){
        return 0;
      }
      return next_time;
    }
    
    // move navigation to next instruction
    // signal that the navigation on this cross was completed and update the orientation
    // return false if the whole sequence is completed
    bool move_to_next_instruction(){
      if(dummyNavigation){
        return true;
      }
      update_possition();
      if(is_finished_current_parser_instruction()){
        return move_to_next_parser_instruction();
      }
      return true;
    }
};





// ----------------------------------------------------------------------------
// ----------------------------------- ROBOT ----------------------------------
// ----------------------------------------------------------------------------

#define LEFT 0
#define RIGHT 1

#define BLACK 0
#define WHITE 1

class Robot {
  private:
    Motor _left_motor; 
    Motor _right_motor;

    byte _middle = 0;
    byte _middle_left = 0;
    byte _middle_right = 0;
    byte _border_left = 0;
    byte _border_right = 0;

    byte _button_state = 0;
    byte _led_state = HIGH;

    byte _pin_middle_sensor = 5;
    byte _pin_middle_left_sensor = 4;
    byte _pin_middle_right_sensor = 6;
    byte _pin_border_left_sensor = 3;
    byte _pin_border_right_sensor = 7;
    byte _pin_led = 11;
    byte _pin_button = 2;
    byte _pin_left_motor = 12;
    byte _pin_right_motor = 13;
    
    void read_inputs() {
      _middle = digitalRead(_pin_middle_sensor);
      _middle_left = digitalRead(_pin_middle_left_sensor);
      _middle_right = digitalRead(_pin_middle_right_sensor);
      _border_left = digitalRead(_pin_border_left_sensor);
      _border_right = digitalRead(_pin_border_right_sensor);
      _button_state = digitalRead(_pin_button);
    }

    void set_velocities(int left, int right) {
      _left_motor.go(left);
      _right_motor.go(right);
    }

    void set_led_state(){
      digitalWrite(_pin_led, _led_state);
    }
    

  public:
    void initialize(){
      _left_motor = Motor();
      _left_motor.attach(_pin_left_motor, 500, 2500);
      _left_motor.set_direction(true);

      _right_motor = Motor();
      _right_motor.attach(_pin_right_motor, 500, 2500);
      _right_motor.set_direction(false);
      
      pinMode(_pin_middle_sensor, INPUT);
      pinMode(_pin_middle_left_sensor, INPUT);
      pinMode(_pin_middle_right_sensor, INPUT);
      pinMode(_pin_border_left_sensor, INPUT);
      pinMode(_pin_border_right_sensor, INPUT);
      pinMode(_pin_button, INPUT_PULLUP);
      pinMode(_pin_led, OUTPUT);
    }

    void refresh_inputs(){
      read_inputs();
    }


    void led_on(){
      _led_state = HIGH;
      set_led_state();
    }
    void led_off(){
      _led_state = LOW;
      set_led_state();
    }

    byte led_state(){
      return _led_state;
    }
    bool is_button_pressed(){
      return _button_state == 0;
    }


    bool is_border_left_black(){ return _border_left == BLACK; }
    bool is_border_left_white(){ return _border_left == WHITE; }

    bool is_border_right_black(){ return _border_right == BLACK; }
    bool is_border_right_white(){ return _border_right == WHITE; }

    bool is_middle_left_black(){ return _middle_left == BLACK; }
    bool is_middle_left_white(){ return _middle_left == WHITE; }

    bool is_middle_right_black(){ return _middle_right == BLACK; }
    bool is_middle_right_white(){ return _middle_right == WHITE; }

    bool is_middle_black(){ return _middle == BLACK; }
    bool is_middle_white(){ return _middle == WHITE; }

    void move_stop(){ set_velocities(0, 0); }
    void move_straight(){ set_velocities(50, 50); }

    void move_left(){ set_velocities(0, 25); }
    void move_right(){ set_velocities(25, 0); }

    void move_slightly_left(){ set_velocities(20, 50); }
    void move_slightly_right(){ set_velocities(50, 20); }

    void move_rotate_on_spot_left(){ set_velocities(-25, 25); }
    void move_rotate_on_spot_right(){ set_velocities(25, -25); }
};



// ----------------------------------------------------------------------------
// ------------------------------- MAIN PROGRAM -------------------------------
// ----------------------------------------------------------------------------


#define STATE_WAITING_START 0
#define STATE_GO_TO_NEXT_CROSSING 1
#define STATE_NAVIGATION_ON_CROSSING 2
#define STATE_FINAL 3
#define STATE_NORMAL_OPERATION 4


Navigation navigation;
Robot robot;

unsigned long start_dance_time = 0;
bool turn_already_seen_border_black = false;
byte turn_around_counter = 0;

byte cross_border_side = LEFT;
int state = STATE_WAITING_START;

// void printout_choreography_execute(){
//   bool finished = false;
//   Serial.println("Local instructions:");

//   while(!finished){
//     byte direction = navigation.get_cross_direction();
//     int time = navigation.get_cross_leave_time();
//     if(serialLog){
//       switch (direction){
//         case NAVIGATION_STRAIGHT:
//           Serial.print("straight");
//           break;
//         case NAVIGATION_BACK:
//           Serial.print("back");
//           break;
//         case NAVIGATION_LEFT:
//           Serial.print("left");
//           break;
//         case NAVIGATION_RIGHT:
//           Serial.print("right");
//           break;
//         default:
//           Serial.println("ERROR");
//           break;
//       }
//       Serial.print(", time: ");
//       Serial.println(time);
//     }
//     finished = !navigation.move_to_next_instruction();
//   }
// }


void setup() {
  navigation.init_preload_choreography();
  robot.initialize();
}

void control_waiting_start(){
  robot.move_stop();
  // if BUTTON == LOW
  if (robot.is_button_pressed()) {
    start_dance_time = millis();
    state = STATE_NAVIGATION_ON_CROSSING;
  }
}

void control_go_to_next_crossing(unsigned int current_time){
  int timestamp = navigation.get_cross_leave_time();
  // delay if the navigation say so
  if (timestamp > current_time) {
    return;
  }
  // Go straight for some time to be centered on the cross.
  // did we encountered a cross?
  if(robot.is_border_left_black() && robot.is_middle_left_black()){
    control_go_to_next_crossing_border_met(LEFT);
    return;
  }
  if(robot.is_border_right_black() && robot.is_middle_right_black()){
    control_go_to_next_crossing_border_met(RIGHT);
    return;
  }
  
  // normal movement
  if (robot.is_middle_left_black()) {
    robot.move_slightly_left();
    return;
  }
  if (robot.is_middle_right_black()) {
    robot.move_slightly_right();
    return;
  }

  robot.move_straight();
}

void control_go_to_next_crossing_border_met(byte which_side_border_encountered){
  cross_border_side = which_side_border_encountered;
  
  // move a little forward to center the robot above the crossing
  robot.move_straight();
  delay(250);
  robot.move_stop();
  
  //we reached new crossing, ask the navigation for new instructions
  bool success = navigation.move_to_next_instruction();

  turn_already_seen_border_black = false;
  turn_around_counter = 0;
  state = STATE_NAVIGATION_ON_CROSSING;
  
  if(!success){
    // we reached the end of the navigation instructions
    state = STATE_FINAL;
    return;
  }
}

void control_navigation_on_crossing(){
  byte instruction = navigation.get_cross_direction();
  bool finished = false;
  switch(instruction) {
    case NAVIGATION_LEFT:
      finished = control_navigation_on_crossing_turn(LEFT);
      break;
    case NAVIGATION_RIGHT:
      finished = control_navigation_on_crossing_turn(RIGHT);
      break;
    case NAVIGATION_STRAIGHT:
      finished = true;
      break;
    case NAVIGATION_BACK:
      finished = control_navigation_on_crossing_turn_around(cross_border_side);
      break;
  }
  if(finished){
    state = STATE_GO_TO_NEXT_CROSSING;
  }
}

// returns true if finished
bool control_navigation_on_crossing_turn(byte direction) {
  // Rotate till the middle sensor sees black  
  if(robot.is_middle_black() && turn_already_seen_border_black) {
    robot.move_stop();
    return true;
  }
  
  if(!turn_already_seen_border_black && (robot.is_border_left_black() || robot.is_border_right_black())){
    turn_already_seen_border_black = true;
  }

  if(direction == LEFT){
    robot.move_rotate_on_spot_left();
    return false;
  }
  if(direction == RIGHT){
    robot.move_rotate_on_spot_right();
    return false;
  }
}

bool control_navigation_on_crossing_turn_around(byte direction){
  if(turn_around_counter >= 2){
    robot.led_off();
    return true;
  }
  bool one_turn_finished = control_navigation_on_crossing_turn(direction);
  if(one_turn_finished){
    robot.led_on();
    turn_around_counter++;
    turn_already_seen_border_black = false;
  }
  return false;
}


void control_final() {
  robot.move_stop();
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
      control_navigation_on_crossing();
      return;
    case STATE_FINAL:
      control_final();
      return;
  }
}


void loop() {
  robot.refresh_inputs();
  unsigned int current_time = millis() - start_dance_time;
  control(current_time);
  // the operation speed of the robot 1000 / 10 per second
  delay(10);
}