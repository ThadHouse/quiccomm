#pragma once

#include <stdint.h>

typedef uint8_t COMM_Bool;

struct COMM_ControlWord {
  uint8_t test : 1;
  uint8_t autonomous : 1;
  uint8_t enabled : 1;
  uint8_t dsConnected : 1;
  uint8_t fmsConnected : 1;
  uint8_t reserved : 2;
  uint8_t eStop : 1;
};

struct COMM_RequestWord {
  uint8_t unknown : 2;
  uint8_t restartcode : 1;
  uint8_t restartrio : 1;
  uint8_t unknown2 : 4;
};

struct COMM_StatusWord {
  uint8_t test : 1;
  uint8_t autonomous : 1;
  uint8_t enabled : 1;
  uint8_t progStart : 1;
  uint8_t brownout : 1;
  uint8_t reserved : 2;
  uint8_t estop : 1;
};

struct COMM_TraceWord {
  uint8_t disabled : 1;
  uint8_t teleop : 1;
  uint8_t autonomous : 1;
  uint8_t test : 1;
  uint8_t rio : 1;
  uint8_t usercode : 1;
  uint8_t reserved : 2;
};

struct COMM_ModeRequestWord {
  uint8_t datetime : 1;
  uint8_t disabled : 1;
  uint8_t reserved : 6;
};

enum COMM_AllianceStation {
  COMM_Alliance_Red1,
  COMM_Alliance_Red2,
  COMM_Alliance_Red3,
  COMM_Alliance_Blue1,
  COMM_Alliance_Blue2,
  COMM_Alliance_Blue3
};

#define COMM_MAX_JOYSTICKS 12
#define COMM_MAX_JOYSTICK_AXES 12
#define COMM_MAX_JOYSTICK_POVS 12
#define COMM_MAX_JOYSTICK_BUTTONS 64

struct COMM_JoystickData {
  uint8_t joystickNumber;
  COMM_Bool isConnected;
  uint8_t axesCount;
  uint8_t povsCount;
  uint8_t buttonsCount;
  int16_t povs[COMM_MAX_JOYSTICK_POVS];
  float axes[COMM_MAX_JOYSTICK_AXES];

  uint64_t buttons;
};

struct COMM_JoystickHIDRumble {
  uint32_t hidOutputs;
  float leftRumble;
  float rightRumble;
  uint8_t joystickNumber;
  COMM_Bool isConnected;
};

enum COMM_MatchType {
  COMM_MatchType_None,
  COMM_MatchType_Practice,
  COMM_MatchType_Qualification,
  COMM_MatchType_Elimination,
  COMM_MatchType_Test
};

struct COMM_MatchInfo {
  char matchName[256];
  COMM_MatchType matchType;
  uint16_t matchNumber;
  uint8_t replayNumber;
};

struct COMM_JoystickDescriptor {
  uint8_t joystickNumber;
  COMM_Bool isConnected;
  COMM_Bool isXbox;
  uint8_t type;
  uint8_t nameLength;
  char name[256];
  uint8_t axesCount;
  uint8_t axesTypes[COMM_MAX_JOYSTICK_AXES];
  uint8_t buttonCount;
  uint8_t povCount;
};
