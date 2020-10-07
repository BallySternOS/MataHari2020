/**************************************************************************
       This file is part of MataHari2020.

    I, Dick Hamill, the author of this program disclaim all copyright
    in order to make this program freely available in perpetuity to
    anyone who would like to use it. Dick Hamill, 6/1/2020

    MataHari2020 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MataHari2020 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
*/

#include "BallySternOS.h"
#include "MataHari2020.h"
#include "SelfTestAndAudit.h"
#include <EEPROM.h>

// The defines for sound can be used separately or in combination
//#define USE_WAV_TRIGGER
//#define USE_WAV_TRIGGER_1p3
#define USE_CHIMES


#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
#include <wavTrigger.h>
wavTrigger wTrig;             // Our WAV Trigger object
#endif

#define MATAHARI2020_MAJOR_VERSION  2020
#define MATAHARI2020_MINOR_VERSION  1
#define DEBUG_MESSAGES  0


// This constant defines how much gap is inserted between chime hits
// during melodies and sound effects
// 40 - quick chimes
// 50 - normal (default)
// 75 - slow chimes
#define CHIME_SPACING_CONSTANT    50

/*********************************************************************

    Game specific code

*********************************************************************/

// MachineState
//  0 - Attract Mode
//  negative - self-test modes
//  positive - game play
char MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_UNVALIDATED     3
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 99
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110

#define MACHINE_STATE_ADJUST_FREEPLAY           -16
#define MACHINE_STATE_ADJUST_BALL_SAVE          -17
#define MACHINE_STATE_ADJUST_MUSIC_LEVEL        -18
#define MACHINE_STATE_ADJUST_TOURNAMENT_SCORING -19
#define MACHINE_STATE_ADJUST_REBOOT             -20
#define MACHINE_STATE_ADJUST_SKILL_SHOT_AWARD   -21
#define MACHINE_STATE_ADJUST_TILT_WARNING       -23
#define MACHINE_STATE_ADJUST_AWARD_OVERRIDE     -24
#define MACHINE_STATE_ADJUST_BALLS_OVERRIDE     -25
#define MACHINE_STATE_ADJUST_SCROLLING_SCORES   -27
#define MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD   -28
#define MACHINE_STATE_ADJUST_SPECIAL_AWARD      -29
#define MACHINE_STATE_ADJUST_PLAYFIELD_VALID    -31
#define MACHINE_STATE_ADJUST_WIZARD_DURATION    -32
#define MACHINE_STATE_ADJUST_WIZARD_REWARD      -33
#define MACHINE_STATE_ADJUST_DIM_LEVEL          -34
#define MACHINE_STATE_ADJUST_DONE               -36

#define GAME_MODE_SKILL_SHOT            0
#define GAME_MODE_QUALIFY_SELECT        1
#define GAME_MODE_SELECT_MODE           2
#define GAME_MODE_AB_LANES              3
#define GAME_MODE_LEFT_DROP_TARGETS     4
#define GAME_MODE_RIGHT_DROP_TARGETS    5
#define GAME_MODE_POP_BUMPERS           6
#define GAME_MODE_SLINGS_AND_LANES      7
#define GAME_MODE_WIZARD                8

#define MODE_STATUS_BIT_AB_LANES          0x01
#define MODE_STATUS_BIT_LEFT_DROPS        0x02
#define MODE_STATUS_BIT_RIGHT_DROPS       0x04
#define MODE_STATUS_BIT_POP_BUMPERS       0x08
#define MODE_STATUS_BIT_SLINGS_AND_LANES  0x10


#define EEPROM_BALL_SAVE_BYTE           100
#define EEPROM_FREE_PLAY_BYTE           101
#define EEPROM_MUSIC_LEVEL_BYTE         102
#define EEPROM_SKILL_SHOT_BYTE          103
#define EEPROM_TILT_WARNING_BYTE        104
#define EEPROM_AWARD_OVERRIDE_BYTE      105
#define EEPROM_BALLS_OVERRIDE_BYTE      106
#define EEPROM_TOURNAMENT_SCORING_BYTE  107
#define EEPROM_SCROLLING_SCORES_BYTE    110
#define EEPROM_PLAYFIELD_VALID_BYTE     111
#define EEPROM_WIZARD_DURATION_BYTE     112
#define EEPROM_DIM_LEVEL_BYTE           113
#define EEPROM_EXTRA_BALL_SCORE_BYTE    140
#define EEPROM_SPECIAL_SCORE_BYTE       144
#define EEPROM_WIZARD_REWARD_BYTE       152

#define SOUND_EFFECT_NONE               0
#define SOUND_EFFECT_BONUS_COUNT        2
#define SOUND_EFFECT_INLANE_UNLIT       3
#define SOUND_EFFECT_OUTLANE_UNLIT      4
#define SOUND_EFFECT_INLANE_LIT         5
#define SOUND_EFFECT_OUTLANE_LIT        6
#define SOUND_EFFECT_BUMPER_HIT         7
#define SOUND_EFFECT_DROP_TARGET        9
#define SOUND_EFFECT_ADD_CREDIT         10
#define SOUND_EFFECT_ADD_PLAYER         11
#define SOUND_EFFECT_PLAYER_UP          15
#define SOUND_EFFECT_BALL_OVER          19
#define SOUND_EFFECT_GAME_OVER          20
#define SOUND_EFFECT_2X_BONUS_COUNT     21
#define SOUND_EFFECT_3X_BONUS_COUNT     22
#define SOUND_EFFECT_EXTRA_BALL         23
#define SOUND_EFFECT_MACHINE_START      24
#define SOUND_EFFECT_SKILL_SHOT         25
#define SOUND_EFFECT_TILT_WARNING       28
#define SOUND_EFFECT_WIZARD_SCORE       29
#define SOUND_EFFECT_MATCH_SPIN         30
#define SOUND_EFFECT_WIZARD_TIMER       31
#define SOUND_EFFECT_SLING_SHOT         34
#define SOUND_EFFECT_10PT_SWITCH        36
#define SOUND_EFFECT_BACKGROUND_1       37
#define SOUND_EFFECT_BACKGROUND_2       38
#define SOUND_EFFECT_BACKGROUND_3       39
#define SOUND_EFFECT_BACKGROUND_WIZ     40
#include "MataHari2020Chimes.h"

#define SKILL_SHOT_DURATION 15
#define MAX_DISPLAY_BONUS     69
#define TILT_WARNING_DEBOUNCE_TIME 1000

#define MODE_LENGTH_IN_SECONDS          30
#define AB_TIME_TO_QUALIFY_MODE         10
#define NUM_ORBITS_IN_AB_GOAL           5
#define NUM_POP_BUMPERS_HIT_GOAL        20
#define NUM_LEFT_TARGETS_GOAL           12
#define NUM_RIGHT_TARGETS_GOAL          8
#define NUM_SLINGS_AND_INLANES          15


/*********************************************************************

    Machine state and options

*********************************************************************/
unsigned long HighScore = 0;
unsigned long AwardScores[3];
byte Credits = 0;
boolean FreePlayMode = false;
byte MusicLevel = 2;
byte BallSaveNumSeconds = 0;
unsigned long ExtraBallValue = 0;
unsigned long SpecialValue = 0;
unsigned long CurrentTime = 0;
byte MaximumCredits = 5;
byte BallsPerGame = 3;
boolean CreditDisplay = false;
byte DimLevel = 2;
byte ScoreAwardReplay = 0;
boolean HighScoreReplay = false;
boolean MatchFeature = false;
byte SpecialLightAward = 0;
boolean BonusCountdown1000Steps = false;
boolean MaximumNumber4Players = false;
boolean TournamentScoring = false;
boolean ResetScoresToClearVersion = false;
boolean ScrollingScores = true;
unsigned long WizardSwitchReward = 50000;
byte WizardModeTimeLimit = 30;
byte dipBank0, dipBank1, dipBank2, dipBank3;


/*********************************************************************

    Game State

*********************************************************************/
byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
unsigned long CurrentScores[4];
byte Bonus;
byte BonusX;
byte ABLaneState;
byte ModeCompletionStatus[4];
byte PopBumperGoal[4];
byte ABLaneGoal[4];
byte SlingsAndLanesGoal[4];
byte LeftTargetGoal[4];
byte RightTargetGoal[4];
unsigned long LastAHit = 0;
unsigned long LastBHit = 0;
unsigned long LastPopBumperHit = 0;
boolean DropTargetsScoreSpecial = false;
byte LeftOutlane;
byte RightOutlane;
boolean SamePlayerShootsAgain = false;

boolean BallSaveUsed = false;
unsigned long BallFirstSwitchHitTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long WizardModeStartTime = 0;

boolean CurrentlyShowingBallSave = false;
byte PlayfieldValidation = 0;
byte MaxTiltWarnings = 2;
byte NumTiltWarnings = 0;
unsigned long LastTiltWarningTime = 0;

boolean SkillShotRunning = false;
byte SkillShotAwardsLevel = 0;

byte GameMode = GAME_MODE_SKILL_SHOT;
byte ProspectiveGameMode = GAME_MODE_AB_LANES;
byte PopBumperPhase = 0;
unsigned long GameModeStartTime = 0;
// GameModeEndTime is zero until the mode is started
unsigned long GameModeEndTime = 0;
unsigned long LastModeShotTime = 0;


void GetDIPSwitches() {
  dipBank0 = BSOS_GetDipSwitches(0);
  dipBank1 = BSOS_GetDipSwitches(1);
  dipBank2 = BSOS_GetDipSwitches(2);
  dipBank3 = BSOS_GetDipSwitches(3);
}

void DecodeDIPSwitchParameters() {

}

void ReadStoredParameters() {
  HighScore = BSOS_ReadULFromEEProm(BSOS_HIGHSCORE_EEPROM_START_BYTE, 10000);
  Credits = BSOS_ReadByteFromEEProm(BSOS_CREDITS_EEPROM_BYTE);
  if (Credits > MaximumCredits) Credits = MaximumCredits;

  ReadSetting(EEPROM_FREE_PLAY_BYTE, 0);
  FreePlayMode = (EEPROM.read(EEPROM_FREE_PLAY_BYTE)) ? true : false;

  BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, 16);
  if (BallSaveNumSeconds > 21) BallSaveNumSeconds = 16;

  MusicLevel = ReadSetting(EEPROM_MUSIC_LEVEL_BYTE, 2);
#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (MusicLevel > 5) MusicLevel = 5;
#else
  if (MusicLevel > 3) MusicLevel = 3;
#endif

  TournamentScoring = (ReadSetting(EEPROM_TOURNAMENT_SCORING_BYTE, 0)) ? true : false;

  MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, 2);
  if (MaxTiltWarnings > 2) MaxTiltWarnings = 2;

  byte awardOverride = ReadSetting(EEPROM_AWARD_OVERRIDE_BYTE, 99);
  if (awardOverride != 99) {
    ScoreAwardReplay = awardOverride;
  }

  byte ballsOverride = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  if (ballsOverride == 3 || ballsOverride == 5) {
    BallsPerGame = ballsOverride;
  } else {
    if (ballsOverride != 99) EEPROM.write(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  }

  ScrollingScores = (ReadSetting(EEPROM_SCROLLING_SCORES_BYTE, 1)) ? true : false;

  ExtraBallValue = BSOS_ReadULFromEEProm(EEPROM_EXTRA_BALL_SCORE_BYTE);
  if (ExtraBallValue % 1000 || ExtraBallValue > 100000) ExtraBallValue = 20000;

  SpecialValue = BSOS_ReadULFromEEProm(EEPROM_SPECIAL_SCORE_BYTE);
  if (SpecialValue % 1000 || SpecialValue > 100000) SpecialValue = 40000;

  PlayfieldValidation = ReadSetting(EEPROM_PLAYFIELD_VALID_BYTE, 1);
  if (PlayfieldValidation > 3) PlayfieldValidation = 1;

  WizardModeTimeLimit = ReadSetting(EEPROM_WIZARD_DURATION_BYTE, 30);
  if (WizardModeTimeLimit > 60) WizardModeTimeLimit = 30;

  WizardSwitchReward = BSOS_ReadULFromEEProm(EEPROM_WIZARD_REWARD_BYTE);
  if (WizardSwitchReward % 5000 || WizardSwitchReward > 100000 || WizardSwitchReward == 0) WizardSwitchReward = 50000;

  DimLevel = ReadSetting(EEPROM_DIM_LEVEL_BYTE, 2);
  if (DimLevel < 2 || DimLevel > 3) DimLevel = 2;
  BSOS_SetDimDivisor(1, DimLevel);

  AwardScores[0] = BSOS_ReadULFromEEProm(BSOS_AWARD_SCORE_1_EEPROM_START_BYTE);
  AwardScores[1] = BSOS_ReadULFromEEProm(BSOS_AWARD_SCORE_2_EEPROM_START_BYTE);
  AwardScores[2] = BSOS_ReadULFromEEProm(BSOS_AWARD_SCORE_3_EEPROM_START_BYTE);

}


void setup() {
  if (DEBUG_MESSAGES) {
    Serial.begin(115200);
  }

  // Tell the OS about game-specific lights and switches
  BSOS_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, TriggeredSwitches);

  // Set up the chips and interrupts
  BSOS_InitializeMPU();
  BSOS_DisableSolenoidStack();
  BSOS_SetDisableFlippers(true);

  // Use dip switches to set up game variables
  GetDIPSwitches();
  DecodeDIPSwitchParameters();

  // Read parameters from EEProm
  ReadStoredParameters();

  CurrentScores[0] = MATAHARI2020_MAJOR_VERSION;
  CurrentScores[1] = MATAHARI2020_MINOR_VERSION;
  CurrentScores[2] = BALLY_STERN_OS_MAJOR_VERSION;
  CurrentScores[3] = BALLY_STERN_OS_MINOR_VERSION;
  ResetScoresToClearVersion = true;

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  // WAV Trigger startup at 57600
  wTrig.start();
  delay(10);

  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);
#endif

  CurrentTime = millis();
  PlaySoundEffect(SOUND_EFFECT_MACHINE_START);
  BSOS_PushToSolenoidStack(SOL_SAUCER, 5, true);
  
}

byte ReadSetting(byte setting, byte defaultValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}

////////////////////////////////////////////////////////////////////////////
//
//  Lamp Management functions
//
////////////////////////////////////////////////////////////////////////////
void SetPlayerLamps(byte numPlayers, byte playerOffset = 0, int flashPeriod = 0) {
  // For Mata Hari, the "Player Up" lights are all +4 of the "Player" lights
  // so this function covers both sets of lights. Putting a 4 in playerOffset
  // will turn on/off the player up lights.
  for (int count = 0; count < 4; count++) {
    BSOS_SetLampState(PLAYER_1 + playerOffset + count, (numPlayers == (count + 1)) ? 1 : 0, 0, flashPeriod);
  }
}


void ShowBonusOnTree(byte bonus, byte dim=0) {
  if (bonus>MAX_DISPLAY_BONUS) bonus = MAX_DISPLAY_BONUS;

  if (bonus>=60) {
    BSOS_SetLampState(BONUS_10, 1, dim, 250);
    bonus -= 20;
  } else if ( ((bonus/10)%2)==1 ) {
    BSOS_SetLampState(BONUS_10, 1, dim);
    bonus -= 10;
  } else {
    BSOS_SetLampState(BONUS_10, 0, dim, 250);
  }

  if (bonus>=40) {
    BSOS_SetLampState(BONUS_20, 1, dim, 250);
    bonus -= 40;
  } else if (bonus>=20) {
    BSOS_SetLampState(BONUS_20, 1, dim);
    bonus -= 20;   
  } else {
    BSOS_SetLampState(BONUS_20, 0, dim);
  }
 
  for (byte count=0; count<9; count++) {
    if (count==(bonus-1)) BSOS_SetLampState(BONUS_1+count, 1, dim);
    else BSOS_SetLampState(BONUS_1+count, 0, dim);
  }

}


void ShowBonusLights(byte mode, byte prospectiveMode, byte bonus) {
  if (mode==GAME_MODE_QUALIFY_SELECT) {
    unsigned long mostRecentHit = LastAHit;
    if (LastBHit>mostRecentHit) mostRecentHit = LastBHit;
    if ((LastAHit || LastBHit) && ((CurrentTime-mostRecentHit)/1000)<AB_TIME_TO_QUALIFY_MODE) {
      byte numLights = 9 - (9*(CurrentTime-mostRecentHit)/1000)/AB_TIME_TO_QUALIFY_MODE;
      for (byte count=0; count<9; count++) BSOS_SetLampState(BONUS_1+count, (count<numLights)?1:0, 1);
    } else {
      ShowBonusOnTree(bonus);
    }
  } else if (mode==GAME_MODE_SELECT_MODE) {
    if ((prospectiveMode==GAME_MODE_POP_BUMPERS || prospectiveMode==GAME_MODE_SLINGS_AND_LANES) && (((CurrentTime-GameModeStartTime)/2000)%2)==0) {
      byte lightPhase=100;
      if (prospectiveMode==GAME_MODE_POP_BUMPERS) {
        lightPhase = ((CurrentTime-GameModeStartTime)/100)%9;
        for (byte count=0; count<9; count++) BSOS_SetLampState(BONUS_1+count, ((count==lightPhase)||(count==(lightPhase-1)))?1:0, (count==lightPhase)?0:1);
      } else if (prospectiveMode==GAME_MODE_SLINGS_AND_LANES) {
        lightPhase = 8-((CurrentTime-GameModeStartTime)/100)%9;
        for (byte count=0; count<9; count++) BSOS_SetLampState(BONUS_1+count, ((count==lightPhase)||(count==(lightPhase+1)))?1:0, (count==lightPhase)?0:1);
      }
      BSOS_SetLampState(BONUS_10, 0);
      BSOS_SetLampState(BONUS_20, 0);
    } else {
      ShowBonusOnTree(bonus);
    }
  } else if (mode>=GAME_MODE_AB_LANES && mode<=GAME_MODE_SLINGS_AND_LANES) {
    // Show time remaining in mode
    byte displayPhase = ((CurrentTime-GameModeStartTime)/2000)%2;

    // For the moment, always show time countdown during a mode    
    if (displayPhase==0 || 1) {
      byte scaledTimeLeft = 0;
      if (GameModeEndTime>CurrentTime) {
        scaledTimeLeft = 9 - ((CurrentTime-GameModeStartTime)*9)/(GameModeEndTime-GameModeStartTime);
        for (byte count=0; count<9; count++) BSOS_SetLampState(BONUS_1+count, (count<scaledTimeLeft)?1:0, 1, (scaledTimeLeft<3)?100:0);
      } else if (GameModeEndTime==0) {
        for (byte count=0; count<9; count++) BSOS_SetLampState(BONUS_1+count, 1, 1, 250);
      } else {
        // This shouldn't happen (mode should end after CurrentTime>=GameModeEndTime
      }
      BSOS_SetLampState(BONUS_10, 0);
      BSOS_SetLampState(BONUS_20, 0);

    } else {
      // If we're in the other part of the display phase, show the bonus
      ShowBonusOnTree(bonus);
    }
  }
}


void ShowBonusXLights(byte mode, byte prospectiveMode, byte bonusX, unsigned long lastSlingAndLaneHit) {
  if (mode==GAME_MODE_SELECT_MODE) {  
    if (prospectiveMode==GAME_MODE_SLINGS_AND_LANES) {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/300)%2;
      BSOS_SetLampState(BONUS_2X, (lightPhase==0)?1:0, 1);
      BSOS_SetLampState(BONUS_3X, (lightPhase==0)?1:0, 1);
      BSOS_SetLampState(BONUS_5X, (lightPhase==1)?1:0, 1);
    } else {
      BSOS_SetLampState(BONUS_2X, (bonusX==2)?1:0);
      BSOS_SetLampState(BONUS_3X, (bonusX==3)?1:0);
      BSOS_SetLampState(BONUS_5X, (bonusX==5)?1:0);
    }
  } else if (mode==GAME_MODE_SLINGS_AND_LANES) {
    if ((CurrentTime-lastSlingAndLaneHit)<500) {
      BSOS_SetLampState(BONUS_2X, (bonusX==2)?1:0);
      BSOS_SetLampState(BONUS_3X, (bonusX==3)?1:0);
      BSOS_SetLampState(BONUS_5X, (bonusX==5)?1:0);
    } else {
      BSOS_SetLampState(BONUS_2X, (bonusX==2)?1:0, 1);
      BSOS_SetLampState(BONUS_3X, (bonusX==3)?1:0, 1);
      BSOS_SetLampState(BONUS_5X, (bonusX==5)?1:0, 1);
    }

  } else {
    BSOS_SetLampState(BONUS_2X, (bonusX==2)?1:0);
    BSOS_SetLampState(BONUS_3X, (bonusX==3)?1:0);
    BSOS_SetLampState(BONUS_5X, (bonusX==5)?1:0);
  }
 
}


void ShowOutlanes(byte mode, byte prospectiveMode, bool leftOutlaneLit, bool rightOutlaneLit, unsigned long lastSlingAndLaneHit) {
  if (mode==GAME_MODE_SELECT_MODE) {  
    if (prospectiveMode==GAME_MODE_SLINGS_AND_LANES) {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/300)%2;
      BSOS_SetLampState(LEFT_OUTLANE_50, (lightPhase==1)?1:0, 1);
      BSOS_SetLampState(RIGHT_OUTLANE_50, (lightPhase==0)?1:0, 1);
    } else {
      BSOS_SetLampState(LEFT_OUTLANE_50, (leftOutlaneLit)?1:0, 1);
      BSOS_SetLampState(RIGHT_OUTLANE_50, (rightOutlaneLit)?1:0, 1);
    }
  } else if (mode==GAME_MODE_SLINGS_AND_LANES) {
    if ((CurrentTime-lastSlingAndLaneHit)<500) {
      BSOS_SetLampState(LEFT_OUTLANE_50, 1);
      BSOS_SetLampState(RIGHT_OUTLANE_50, 1);
    } else {
      BSOS_SetLampState(LEFT_OUTLANE_50, (leftOutlaneLit)?1:0, 1);
      BSOS_SetLampState(RIGHT_OUTLANE_50, (rightOutlaneLit)?1:0, 1);
    }

  } else {
    BSOS_SetLampState(LEFT_OUTLANE_50, (leftOutlaneLit)?1:0);
    BSOS_SetLampState(RIGHT_OUTLANE_50, (rightOutlaneLit)?1:0);
  }
   
}

unsigned long LastShowSaucerLamps = 0;
void ShowSaucerLamps(byte mode) {

//  if ((CurrentTime-LastShowSaucerLamps)<250) return;
  LastShowSaucerLamps = CurrentTime;
  
  if (mode==GAME_MODE_SKILL_SHOT) {
    byte lightPhase = ((CurrentTime-GameModeStartTime)/250)%24;
    if (lightPhase>14) {
      lightPhase-=15;
      BSOS_SetLampState(BONUS_2X_POTENTIAL, (lightPhase%3)==0);      
      BSOS_SetLampState(BONUS_3X_POTENTIAL, (lightPhase%3)==1);      
      BSOS_SetLampState(BONUS_5X_POTENTIAL, (lightPhase%3)==2);      
    } else {
      BSOS_SetLampState(BONUS_2X_POTENTIAL, 0);
      BSOS_SetLampState(BONUS_3X_POTENTIAL, 0);
      BSOS_SetLampState(BONUS_5X_POTENTIAL, 0);
    }
  } else {
  }
}


void ShowPopBumperLamps(byte mode, byte prospectiveMode, byte popStatus, unsigned long lastTimePopBumperHit) {
  if (mode==GAME_MODE_SELECT_MODE) {
    byte lightPhase = ((CurrentTime-GameModeStartTime)/200)%2;
    if (prospectiveMode==GAME_MODE_POP_BUMPERS) {
      BSOS_SetLampState(POP_BUMPER_1, lightPhase%2);
      BSOS_SetLampState(POP_BUMPER_2, (lightPhase%2)?0:1);
    }
  } else if (mode==GAME_MODE_POP_BUMPERS) {
    if ((CurrentTime-lastTimePopBumperHit)<1000) {
      BSOS_SetLampState(POP_BUMPER_1, 1, 0, 100);
      BSOS_SetLampState(POP_BUMPER_2, 1, 0, 100);
    } else {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/400)%2;
      BSOS_SetLampState(POP_BUMPER_1, 1, lightPhase);
      BSOS_SetLampState(POP_BUMPER_2, 1, (lightPhase)?false:true);
    }
  } else {
    if (popStatus) {
      
    }
  }
}

void ShowABLamps(byte mode, byte prospectiveMode, byte abStatus) {
  bool showABStatus = false;
  
  if (mode==GAME_MODE_SKILL_SHOT) {
    byte lightPhase = ((CurrentTime-GameModeStartTime)/250)%24;
    if (lightPhase<8) {
      BSOS_SetLampState(A_LANE, 1, lightPhase%2);
      BSOS_SetLampState(B_LANE, 0);
    } else if (lightPhase<16) {
      BSOS_SetLampState(A_LANE, 0);
      BSOS_SetLampState(B_LANE, 1, lightPhase%2);
    } else {
      BSOS_SetLampState(A_LANE, 0);
      BSOS_SetLampState(B_LANE, 0);      
    }
  } else if (mode==GAME_MODE_QUALIFY_SELECT) {
    unsigned long mostRecentHit = LastAHit;
    if (LastBHit>mostRecentHit) mostRecentHit = LastBHit;
    if ((LastAHit || LastBHit) && ((CurrentTime-mostRecentHit)/1000)<AB_TIME_TO_QUALIFY_MODE) {  
      BSOS_SetLampState(A_LANE, 1, 1, (mostRecentHit==LastBHit)?100:0);
      BSOS_SetLampState(B_LANE, 1, 1, (mostRecentHit==LastAHit)?100:0);
    } else {      
      showABStatus = true;
    }
  } else if (mode==GAME_MODE_SELECT_MODE) {
    if (prospectiveMode==GAME_MODE_AB_LANES) {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/500)%2;
      BSOS_SetLampState(A_LANE, lightPhase%2);
      BSOS_SetLampState(B_LANE, (lightPhase%2)?0:1);
    } else {
      showABStatus = true;
    }
  } else if (mode==GAME_MODE_AB_LANES) {
    if (LastModeShotTime && (CurrentTime-LastModeShotTime)<1000) {
      BSOS_SetLampState(A_LANE, 1, 0, 200);
      BSOS_SetLampState(B_LANE, 1, 0, 200);
    } else {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/250)%2;
      BSOS_SetLampState(A_LANE, lightPhase%2);
      BSOS_SetLampState(B_LANE, (lightPhase%2)?0:1);
    }
  } else {
    showABStatus = true;
  }

  if (showABStatus) {
    byte aStatus = abStatus & 0x0F;
    byte bStatus = (abStatus & 0xF0)>>4;
    if (aStatus==bStatus) {
      byte lightPhase = ((CurrentTime)/250)%4;
      BSOS_SetLampState(A_LANE, (lightPhase)?1:0, (lightPhase%2)?1:0);
      BSOS_SetLampState(B_LANE, (lightPhase)?1:0, (lightPhase%2)?1:0);
    } else {
      BSOS_SetLampState(A_LANE, 1, (aStatus>bStatus)?1:0, (aStatus<bStatus)?100:0);
      BSOS_SetLampState(B_LANE, 1, (aStatus<bStatus)?1:0, (aStatus>bStatus)?100:0);
    }
  }
}

byte ProspectiveModeShown = 0;
unsigned long LastABReportTime = 0;

void ShowABRewardLamps(byte mode, byte prospectiveMode, byte abStatus) {

  byte modeShown = mode;
  
  if (mode==GAME_MODE_SKILL_SHOT) {
    for (int count=0; count<7; count++) BSOS_SetLampState(AB_SCORES_1000+count, 0);
  } else {
    
    if (mode==GAME_MODE_SELECT_MODE) {
      // The first two seconds, we'll show lights to point to the drop targets
      if (prospectiveMode==GAME_MODE_LEFT_DROP_TARGETS || prospectiveMode==GAME_MODE_RIGHT_DROP_TARGETS || prospectiveMode==GAME_MODE_AB_LANES) {
        if ((((CurrentTime-GameModeStartTime)/2000)%2)==0) {
          modeShown=prospectiveMode;        
        }        
      }
    }
    
    if (modeShown==GAME_MODE_LEFT_DROP_TARGETS) {
      byte phase = ((CurrentTime-GameModeStartTime)/100)%10;
      if (phase<7) {
        for (int count=0; count<7; count++) BSOS_SetLampState(AB_SCORES_1000+count, (count==(6-phase))?1:0);
      } else {
        for (int count=1; count<7; count++) BSOS_SetLampState(AB_SCORES_1000+count, 0);
        BSOS_SetLampState(AB_SCORES_1000, (phase%2)?0:1);
      }
    } else if (modeShown==GAME_MODE_RIGHT_DROP_TARGETS) {
      byte phase = ((CurrentTime-GameModeStartTime)/100)%10;
      if (phase<7) {
        for (int count=0; count<7; count++) BSOS_SetLampState(AB_SCORES_1000+count, (count==(phase))?1:0);
      } else {
        for (int count=0; count<6; count++) BSOS_SetLampState(AB_SCORES_1000+count, 0);
        BSOS_SetLampState(AB_SCORES_SPECIAL, (phase%2)?0:1);
      }
    } else {
      byte abWillScore = (abStatus&0x0F);
      byte bStatus = (abStatus&0xF0)>>4;
      if (bStatus<abWillScore) abWillScore = bStatus;
      
      // Show the current state of the AB reward
      for (int count=0; count<7; count++) {
        BSOS_SetLampState(AB_SCORES_1000+count, (count==abWillScore)?1:0, 0);  
      }
    }
    
  }
  
}


byte BallSaveShown = false;
byte BallSaveHurryUp = false;

void ShowSamePlayerLamps(boolean samePlayerShootsAgain) {
  // Check to see if we're in ball-save time
  unsigned long timeLeftOnBallSave = ((CurrentTime-BallFirstSwitchHitTime)/1000);
  if (  !BallSaveUsed && timeLeftOnBallSave<((unsigned long)BallSaveNumSeconds) ) {
    if (timeLeftOnBallSave<3 && !BallSaveHurryUp) {
      // If we're near the end of ball save
      BSOS_SetLampState(SHOOT_AGAIN, 1, 0, 100);
      BallSaveHurryUp = true;
    } else if (!BallSaveShown) {
      BSOS_SetLampState(SHOOT_AGAIN, 1, 0, 500);
      BallSaveShown = true;
      BallSaveHurryUp = false;
    }
  } else if (BallFirstSwitchHitTime!=0) {
    BSOS_SetLampState(SAME_PLAYER_SHOOTS_AGAIN, samePlayerShootsAgain);
    BSOS_SetLampState(SHOOT_AGAIN, samePlayerShootsAgain);
  }
}




////////////////////////////////////////////////////////////////////////////
//
//  Display Management functions
//
////////////////////////////////////////////////////////////////////////////
unsigned long LastTimeScoreChanged = 0;
unsigned long LastTimeOverrideAnimated = 0;
unsigned long LastFlashOrDash = 0;
unsigned long ScoreOverrideValue[4]= {0, 0, 0, 0};
byte ScoreOverrideStatus = 0;
byte LastScrollPhase = 0;

byte MagnitudeOfScore(unsigned long score) {
  if (score == 0) return 0;

  byte retval = 0;
  while (score > 0) {
    score = score / 10;
    retval += 1;
  }
  return retval;
}

void OverrideScoreDisplay(byte displayNum, unsigned long value, boolean animate) {
  if (displayNum>3) return;
  ScoreOverrideStatus |= (0x10<<displayNum);
  if (animate) ScoreOverrideStatus |= (0x01<<displayNum);
  else ScoreOverrideStatus &= ~(0x01<<displayNum);
  ScoreOverrideValue[displayNum] = value;
}

byte GetDisplayMask(byte numDigits) {
  byte displayMask = 0;
  for (byte digitCount=0; digitCount<numDigits; digitCount++) {
    displayMask |= (0x20>>digitCount);
  }  
  return displayMask;
}


void ShowPlayerScores(byte displayToUpdate, boolean flashCurrent, boolean dashCurrent, unsigned long allScoresShowValue=0) {

  if (displayToUpdate==0xFF) ScoreOverrideStatus = 0;

  byte displayMask = 0x3F;
  unsigned long displayScore = 0;
  unsigned long overrideAnimationSeed = CurrentTime/250;
  byte scrollPhaseChanged = false;

  byte scrollPhase = ((CurrentTime-LastTimeScoreChanged)/250)%16;
  if (scrollPhase!=LastScrollPhase) {
    LastScrollPhase = scrollPhase;
    scrollPhaseChanged = true;
  }

  for (byte scoreCount=0; scoreCount<4; scoreCount++) {
    // If this display is currently being overriden, then we should update it
    if (allScoresShowValue==0 && (ScoreOverrideStatus & (0x10<<scoreCount))) {
      displayScore = ScoreOverrideValue[scoreCount];
      byte numDigits = MagnitudeOfScore(displayScore);
      if (numDigits==0) numDigits = 1;
      if (numDigits<5 && (ScoreOverrideStatus & (0x01<<scoreCount))) {
        if (overrideAnimationSeed!=LastTimeOverrideAnimated) {
          LastTimeOverrideAnimated = overrideAnimationSeed;
          byte shiftDigits = (overrideAnimationSeed)%((7-numDigits)+(5-numDigits));
          if (shiftDigits>=(7-numDigits)) shiftDigits = (6-numDigits)*2-shiftDigits;
          byte digitCount;
          displayMask = GetDisplayMask(numDigits);
          for (digitCount=0; digitCount<shiftDigits; digitCount++) {
            displayScore *= 10;
            displayMask = displayMask>>1;
          }
          BSOS_SetDisplayBlank(scoreCount, 0x00);
          BSOS_SetDisplay(scoreCount, displayScore, false);
          BSOS_SetDisplayBlank(scoreCount, displayMask);
        }
      } else {
        BSOS_SetDisplay(scoreCount, displayScore, true);
      }
      
    } else {
      // No override, update scores designated by displayToUpdate
      if (allScoresShowValue==0) displayScore = CurrentScores[scoreCount];
      else displayScore = allScoresShowValue;
      
      if (displayToUpdate==0xFF || displayToUpdate==scoreCount || displayScore>999999) {

        // Don't show this score if it's not a current player score (even if it's scrollable)
        if (displayToUpdate==0xFF && (scoreCount>=CurrentNumPlayers&&CurrentNumPlayers!=0) && allScoresShowValue==0) {
          BSOS_SetDisplayBlank(scoreCount, 0x00);
          continue;
        }

        if (displayScore>999999) {
          // Score needs to be scrolled
          if ((CurrentTime-LastTimeScoreChanged)<5000) {
            BSOS_SetDisplay(scoreCount, displayScore%1000000, true);  
          } else {

            // Scores are scrolled 10 digits and then we wait for 6
            if (scrollPhase<11 && scrollPhaseChanged) {
              byte numDigits = MagnitudeOfScore(displayScore);
              
              // Figure out top part of score
              if (scrollPhase<6) {
                displayMask = 0x3F;
                for (byte scrollCount=0; scrollCount<scrollPhase; scrollCount++) {
                  displayScore = (displayScore % 1000000) * 10;
                  displayMask = displayMask >> 1;
                }
              } else {
                displayScore = 0; 
                displayMask = 0x00;
              }

              // Add in lower part of score
              if ((numDigits+scrollPhase)>10) {
                byte numDigitsNeeded = (numDigits+scrollPhase)-10;
                unsigned long tempScore = CurrentScores[scoreCount];
                for (byte scrollCount=0; scrollCount<(numDigits-numDigitsNeeded); scrollCount++) {
                  tempScore /= 10;
                }
                displayMask |= GetDisplayMask(MagnitudeOfScore(tempScore));
                displayScore += tempScore;
              }
              BSOS_SetDisplayBlank(scoreCount, displayMask);
              BSOS_SetDisplay(scoreCount, displayScore);
            }
          }          
        } else {
          if (flashCurrent) {
            unsigned long flashSeed = CurrentTime/250;
            if (flashSeed != LastFlashOrDash) {
              LastFlashOrDash = flashSeed;
              if (((CurrentTime/250)%2)==0) BSOS_SetDisplayBlank(scoreCount, 0x00);
              else BSOS_SetDisplay(scoreCount, displayScore, true, 2);
            }
          } else if (dashCurrent) {
            unsigned long dashSeed = CurrentTime/50;
            if (dashSeed != LastFlashOrDash) {
              LastFlashOrDash = dashSeed;
              byte dashPhase = (CurrentTime/60)%36;
              byte numDigits = MagnitudeOfScore(displayScore);
              if (dashPhase<12) { 
                displayMask = GetDisplayMask((numDigits==0)?2:numDigits);
                if (dashPhase<7) {
                  for (byte maskCount=0; maskCount<dashPhase; maskCount++) {
                    displayMask &= ~(0x01<<maskCount);
                  }
                } else {
                  for (byte maskCount=12; maskCount>dashPhase; maskCount--) {
                    displayMask &= ~(0x20>>(maskCount-dashPhase-1));
                  }
                }
                BSOS_SetDisplay(scoreCount, displayScore);
                BSOS_SetDisplayBlank(scoreCount, displayMask);
              } else {
                BSOS_SetDisplay(scoreCount, displayScore, true, 2);
              }
            }
          } else {
            BSOS_SetDisplay(scoreCount, displayScore, true, 2);          
          }
        }
      } // End if this display should be updated
    } // End on non-overridden
  } // End loop on scores
  
}


////////////////////////////////////////////////////////////////////////////
//
//  Machine State Helper functions
//
////////////////////////////////////////////////////////////////////////////

boolean AddPlayer(boolean resetNumPlayers = false) {

  if (Credits < 1 && !FreePlayMode) return false;
  if (CurrentNumPlayers >= 4 || (CurrentNumPlayers >= 2 && !MaximumNumber4Players)) return false;

  if (resetNumPlayers) CurrentNumPlayers = 0;
  CurrentNumPlayers += 1;
  BSOS_SetDisplay(CurrentNumPlayers - 1, 0, true, 2);
//  BSOS_SetDisplayBlank(CurrentNumPlayers - 1, 0x30);

  if (!FreePlayMode) {
    Credits -= 1;
    BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, Credits);
    BSOS_SetDisplayCredits(Credits);
    BSOS_SetCoinLockout(false);
  }
  PlaySoundEffect(SOUND_EFFECT_ADD_PLAYER);
  SetPlayerLamps(CurrentNumPlayers);

  BSOS_WriteULToEEProm(BSOS_TOTAL_PLAYS_EEPROM_START_BYTE, BSOS_ReadULFromEEProm(BSOS_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}

void AddCoinToAudit(byte switchHit) {

  unsigned short coinAuditStartByte = 0;

  switch (switchHit) {
    case SW_COIN_3: coinAuditStartByte = BSOS_CHUTE_3_COINS_START_BYTE; break;
    case SW_COIN_2: coinAuditStartByte = BSOS_CHUTE_2_COINS_START_BYTE; break;
    case SW_COIN_1: coinAuditStartByte = BSOS_CHUTE_1_COINS_START_BYTE; break;
  }

  if (coinAuditStartByte) {
    BSOS_WriteULToEEProm(coinAuditStartByte, BSOS_ReadULFromEEProm(coinAuditStartByte) + 1);
  }

}

boolean GetLeftOutlane(byte playerNum) {
  return (LeftOutlane&(1<<playerNum))?true:false;
}

void SetLeftOutlane(byte playerNum) {
  LeftOutlane |= (1<<playerNum);
}

boolean GetRightOutlane(byte playerNum) {
  return (RightOutlane&(1<<playerNum))?true:false;
}

void SetRightOutlane(byte playerNum) {
  RightOutlane |= (1<<playerNum);
}

#define ADJ_TYPE_LIST                 1
#define ADJ_TYPE_MIN_MAX              2
#define ADJ_TYPE_MIN_MAX_DEFAULT      3
#define ADJ_TYPE_SCORE                4
#define ADJ_TYPE_SCORE_WITH_DEFAULT   5
#define ADJ_TYPE_SCORE_NO_DEFAULT     6
byte AdjustmentType = 0;
byte NumAdjustmentValues = 0;
byte AdjustmentValues[8];
unsigned long AdjustmentScore;
byte *CurrentAdjustmentByte = NULL;
unsigned long *CurrentAdjustmentUL = NULL;
byte CurrentAdjustmentStorageByte = 0;
byte TempValue = 0;


int RunSelfTest(int curState, boolean curStateChanged) {
  int returnState = curState;
  CurrentNumPlayers = 0;

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (curStateChanged) {
    // Send a stop-all command and reset the sample-rate offset, in case we have
    //  reset while the WAV Trigger was already playing.
    wTrig.stopAllTracks();
    wTrig.samplerateOffset(0);
  }
#endif

  // Any state that's greater than CHUTE_3 is handled by the Base Self-test code
  // Any that's less, is machine specific, so we handle it here.
  if (curState >= MACHINE_STATE_TEST_CHUTE_3_COINS) {
    returnState = RunBaseSelfTest(returnState, curStateChanged, CurrentTime, SW_CREDIT_RESET, SW_SLAM);
  } else {
    byte curSwitch = BSOS_PullFirstFromSwitchStack();

    if (curSwitch == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      SetLastSelfTestChangedTime(CurrentTime);
      returnState -= 1;
    }

    if (curSwitch == SW_SLAM) {
      returnState = MACHINE_STATE_ATTRACT;
    }

    if (curStateChanged) {
      for (int count = 0; count < 4; count++) {
        BSOS_SetDisplay(count, 0);
        BSOS_SetDisplayBlank(count, 0x00);
      }
      BSOS_SetDisplayCredits(abs(curState) - 4.);
      BSOS_SetDisplayBallInPlay(0, false);
      CurrentAdjustmentByte = NULL;
      CurrentAdjustmentUL = NULL;
      CurrentAdjustmentStorageByte = 0;

      AdjustmentType = ADJ_TYPE_MIN_MAX;
      AdjustmentValues[0] = 0;
      AdjustmentValues[1] = 1;
      TempValue = 0;

      switch (curState) {
        case MACHINE_STATE_ADJUST_FREEPLAY:
          CurrentAdjustmentByte = (byte *)&FreePlayMode;
          CurrentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALL_SAVE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 5;
          AdjustmentValues[1] = 6;
          AdjustmentValues[2] = 11;
          AdjustmentValues[3] = 16;
          AdjustmentValues[4] = 21;
          CurrentAdjustmentByte = &BallSaveNumSeconds;
          CurrentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_MUSIC_LEVEL:
          AdjustmentType = ADJ_TYPE_MIN_MAX_DEFAULT;
#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
          AdjustmentValues[1] = 5;
#else
          AdjustmentValues[1] = 3;
#endif
          CurrentAdjustmentByte = &MusicLevel;
          CurrentAdjustmentStorageByte = EEPROM_MUSIC_LEVEL_BYTE;
          break;
        case MACHINE_STATE_ADJUST_TOURNAMENT_SCORING:
          CurrentAdjustmentByte = (byte *)&TournamentScoring;
          CurrentAdjustmentStorageByte = EEPROM_TOURNAMENT_SCORING_BYTE;
          break;
        case MACHINE_STATE_ADJUST_REBOOT:
          for (byte count = 0; count < 4; count++) {
            BSOS_SetDisplay(count, 8007, true);
          }
          CurrentAdjustmentByte = 0;
          break;
        case MACHINE_STATE_ADJUST_SKILL_SHOT_AWARD:
          CurrentAdjustmentByte = (byte *)&SkillShotAwardsLevel;
          CurrentAdjustmentStorageByte = EEPROM_SKILL_SHOT_BYTE;
          break;
        case MACHINE_STATE_ADJUST_TILT_WARNING:
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &MaxTiltWarnings;
          CurrentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        case MACHINE_STATE_ADJUST_AWARD_OVERRIDE:
          AdjustmentType = ADJ_TYPE_MIN_MAX_DEFAULT;
          AdjustmentValues[1] = 7;
          CurrentAdjustmentByte = &ScoreAwardReplay;
          CurrentAdjustmentStorageByte = EEPROM_AWARD_OVERRIDE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALLS_OVERRIDE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 3;
          AdjustmentValues[0] = 3;
          AdjustmentValues[1] = 5;
          AdjustmentValues[2] = 99;
          CurrentAdjustmentByte = &BallsPerGame;
          CurrentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SCROLLING_SCORES:
          CurrentAdjustmentByte = (byte *)&ScrollingScores;
          CurrentAdjustmentStorageByte = EEPROM_SCROLLING_SCORES_BYTE;
          break;

        case MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &ExtraBallValue;
          CurrentAdjustmentStorageByte = EEPROM_EXTRA_BALL_SCORE_BYTE;
          break;

        case MACHINE_STATE_ADJUST_SPECIAL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &SpecialValue;
          CurrentAdjustmentStorageByte = EEPROM_SPECIAL_SCORE_BYTE;
          break;

        case MACHINE_STATE_ADJUST_PLAYFIELD_VALID:
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &PlayfieldValidation;
          CurrentAdjustmentStorageByte = EEPROM_PLAYFIELD_VALID_BYTE;
          break;

        case MACHINE_STATE_ADJUST_WIZARD_DURATION:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 5;
          AdjustmentValues[1] = 15;
          AdjustmentValues[2] = 30;
          AdjustmentValues[3] = 45;
          AdjustmentValues[4] = 60;
          CurrentAdjustmentByte = &WizardModeTimeLimit;
          CurrentAdjustmentStorageByte = EEPROM_WIZARD_DURATION_BYTE;
          break;

        case MACHINE_STATE_ADJUST_WIZARD_REWARD:
          AdjustmentType = ADJ_TYPE_SCORE_NO_DEFAULT;
          CurrentAdjustmentUL = &WizardSwitchReward;
          CurrentAdjustmentStorageByte = EEPROM_WIZARD_REWARD_BYTE;
          break;

        case MACHINE_STATE_ADJUST_DIM_LEVEL:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 2;
          AdjustmentValues[0] = 2;
          AdjustmentValues[1] = 3;
          CurrentAdjustmentByte = &DimLevel;
          CurrentAdjustmentStorageByte = EEPROM_DIM_LEVEL_BYTE;
          for (int count = 0; count < 10; count++) BSOS_SetLampState(BONUS_1 + count, 1, 1);
          break;

        case MACHINE_STATE_ADJUST_DONE:
          returnState = MACHINE_STATE_ATTRACT;
          break;
      }

    }

    // Change value, if the switch is hit
    if (curSwitch == SW_CREDIT_RESET) {

      if (CurrentAdjustmentByte && (AdjustmentType == ADJ_TYPE_MIN_MAX || AdjustmentType == ADJ_TYPE_MIN_MAX_DEFAULT)) {
        byte curVal = *CurrentAdjustmentByte;
        curVal += 1;
        if (curVal > AdjustmentValues[1]) {
          if (AdjustmentType == ADJ_TYPE_MIN_MAX) curVal = AdjustmentValues[0];
          else {
            if (curVal > 99) curVal = AdjustmentValues[0];
            else curVal = 99;
          }
        }
        *CurrentAdjustmentByte = curVal;
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, curVal);
      } else if (CurrentAdjustmentByte && AdjustmentType == ADJ_TYPE_LIST) {
        byte valCount = 0;
        byte curVal = *CurrentAdjustmentByte;
        byte newIndex = 0;
        for (valCount = 0; valCount < (NumAdjustmentValues - 1); valCount++) {
          if (curVal == AdjustmentValues[valCount]) newIndex = valCount + 1;
        }
        *CurrentAdjustmentByte = AdjustmentValues[newIndex];
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, AdjustmentValues[newIndex]);
      } else if (CurrentAdjustmentUL && (AdjustmentType == ADJ_TYPE_SCORE_WITH_DEFAULT || AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT)) {
        unsigned long curVal = *CurrentAdjustmentUL;
        curVal += 5000;
        if (curVal > 100000) curVal = 0;
        if (AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT && curVal == 0) curVal = 5000;
        *CurrentAdjustmentUL = curVal;
        if (CurrentAdjustmentStorageByte) BSOS_WriteULToEEProm(CurrentAdjustmentStorageByte, curVal);
      }

      if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
        BSOS_SetDimDivisor(1, DimLevel);
      }
      if (curState == MACHINE_STATE_ADJUST_REBOOT) {
        returnState = MACHINE_STATE_ATTRACT;
      }
    }

    // Show current value
    if (CurrentAdjustmentByte != NULL) {
      BSOS_SetDisplay(0, (unsigned long)(*CurrentAdjustmentByte), true);
    } else if (CurrentAdjustmentUL != NULL) {
      BSOS_SetDisplay(0, (*CurrentAdjustmentUL), true);
    }

  }

  if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
    for (int count = 0; count < 10; count++) BSOS_SetLampState(BONUS_1 + count, 1, (CurrentTime / 1000) % 2);
  }

  if (returnState == MACHINE_STATE_ATTRACT) {
    // If any variables have been set to non-override (99), return
    // them to dip switch settings
    // Balls Per Game, Player Loses On Ties, Novelty Scoring, Award Score
    DecodeDIPSwitchParameters();
    ReadStoredParameters();
  }

  return returnState;
}




////////////////////////////////////////////////////////////////////////////
//
//  Audio Output functions
//
////////////////////////////////////////////////////////////////////////////

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
byte CurrentBackgroundSong = SOUND_EFFECT_NONE;
#endif


void PlayBackgroundSong(byte songNum) {

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (MusicLevel > 4) {
    if (CurrentBackgroundSong != songNum) {
      if (CurrentBackgroundSong != SOUND_EFFECT_NONE) wTrig.trackStop(CurrentBackgroundSong);
      if (songNum != SOUND_EFFECT_NONE) {
#ifdef USE_WAV_TRIGGER_1p3
        wTrig.trackPlayPoly(songNum, true);
#else
        wTrig.trackPlayPoly(songNum);
#endif
        wTrig.trackLoop(songNum, true);
      }
      CurrentBackgroundSong = songNum;
    }
  }
#else
  byte test = songNum;
  songNum = test;
#endif

}

unsigned long NextSoundEffectTime = 0;

void PlaySoundEffect(byte soundEffectNum) {

  if (MusicLevel == 0) return;

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (MusicLevel > 3) {

#ifndef USE_WAV_TRIGGER_1p3
    if (  soundEffectNum == SOUND_EFFECT_BUMPER_HIT || soundEffectNum == SOUND_EFFECT_ROLLOVER ||
          soundEffectNum == SOUND_EFFECT_10PT_SWITCH || SOUND_EFFECT_SPINNER_HIGH ||
          SOUND_EFFECT_SPINNER_LOW ) wTrig.trackStop(soundEffectNum);
#endif
    wTrig.trackPlayPoly(soundEffectNum);
  }
#endif

#ifdef USE_CHIMES
  // If the user selects electronic sounds, don't do chimes
  if (MusicLevel > 3) return;

  // Music level 3 = allow melodies to overlap
  if (CurrentTime > NextSoundEffectTime || MusicLevel == 3) {
    NextSoundEffectTime = CurrentTime;
  } else if ( (NextSoundEffectTime - CurrentTime) > 2000 ) {
    // if we already have two seconds of sound effects
    // lined up, simply return
    return;
  }
  int count = 0;

  unsigned long soundGapUL = (unsigned long)CHIME_SPACING_CONSTANT;
  byte longestGap = 0;

  // Look for chimes that need to be added based on the current sound effect
  int arrayCount;
  int arraySize;
  ChimeEntry *chimeArray;
  
  for (arrayCount=0; arrayCount<MusicLevel*2; arrayCount++) {
    switch (arrayCount) {
      case 0: chimeArray = MataHariSFXLowPriorityLevel1; arraySize = sizeof(MataHariSFXLowPriorityLevel1)/sizeof(ChimeEntry); break;
      case 1: chimeArray = MataHariSFXHighPriorityLevel1; arraySize = sizeof(MataHariSFXHighPriorityLevel1)/sizeof(ChimeEntry); break;
      case 2: chimeArray = MataHariSFXLowPriorityLevel2; arraySize = sizeof(MataHariSFXLowPriorityLevel2)/sizeof(ChimeEntry); break;
      case 3: chimeArray = MataHariSFXHighPriorityLevel2; arraySize = sizeof(MataHariSFXHighPriorityLevel2)/sizeof(ChimeEntry); break;
      default: chimeArray = NULL;
    }
    if (chimeArray) {
      for (count=0; count<arraySize; count++) {
        longestGap = 0;
        bool solenoidOverride = (count%2)?true:false;
        if (chimeArray[count].SoundEffectNum==soundEffectNum) {
          BSOS_PushToTimedSolenoidStack(chimeArray[count].SolNumber, 3, NextSoundEffectTime + soundGapUL*((unsigned long)chimeArray[count].TimeOffset), solenoidOverride);
          if (chimeArray[count].TimeOffset > longestGap) longestGap = chimeArray[count].TimeOffset;
        }
        NextSoundEffectTime = CurrentTime + ((unsigned long)longestGap)*soundGapUL;
      }
    }
  }

#endif

}


void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
    BSOS_SetDisplayCredits(Credits);
    BSOS_SetCoinLockout(false);
  } else {
    BSOS_SetDisplayCredits(Credits);
    BSOS_SetCoinLockout(true);
  }

}



////////////////////////////////////////////////////////////////////////////
//
//  Attract Mode
//
////////////////////////////////////////////////////////////////////////////

byte AttractSweepLights = 1;
unsigned long AttractLastSweepTime = 0;
unsigned long AttractLastLadderTime = 0;
byte AttractLastLadderBonus = 0;
unsigned long AttractLastStarTime = 0;
byte AttractLastHeadMode = 255;
byte AttractLastPlayfieldMode = 255;
byte InAttractMode = false;

int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {
    BSOS_DisableSolenoidStack();
    BSOS_TurnOffAllLamps();
    BSOS_SetDisableFlippers(true);
    if (DEBUG_MESSAGES) {
      Serial.write("Entering Attract Mode\n\r");
    }

    AttractLastHeadMode = 0;
    AttractLastPlayfieldMode = 0;
  }

  // Alternate displays between high score and blank
  if ((CurrentTime / 6000) % 2 == 0) {

    if (AttractLastHeadMode != 1) {
      BSOS_SetLampState(HIGH_SCORE_TO_DATE, 1, 0, 250);
      BSOS_SetLampState(GAME_OVER, 0);
      SetPlayerLamps(0);

      BSOS_SetDisplayCredits(Credits, true);
      BSOS_SetDisplayBallInPlay(0, true);
    }
    AttractLastHeadMode = 1;
    
    if (CurrentTime > 30000) ShowPlayerScores(0xFF, false, false, HighScore);
    else ShowPlayerScores(0xFF, false, false);

  } else {
    if (AttractLastHeadMode != 2) {
      if (ResetScoresToClearVersion == true && CurrentTime > 30000) {
        for (int count = 0; count < 4; count++) {
          CurrentScores[count] = 0;
        }
        CurrentNumPlayers = 0;
        ResetScoresToClearVersion = false;
      }
      BSOS_SetLampState(HIGH_SCORE_TO_DATE, 0);
      BSOS_SetLampState(GAME_OVER, 1);
      BSOS_SetDisplayCredits(Credits, true);
      BSOS_SetDisplayBallInPlay(0, true);
    }
    ShowPlayerScores(0xFF, false, false);
    
    SetPlayerLamps(((CurrentTime / 250) % 4) + 1);
    AttractLastHeadMode = 2;
  }

  if ((CurrentTime / 10000) % 3 < 2) {
    if (AttractLastPlayfieldMode != 1) {
      BSOS_TurnOffAllLamps();
    }

    AttractLastPlayfieldMode = 1;
  } else {
    if (AttractLastPlayfieldMode != 2) {
      BSOS_TurnOffAllLamps();
      AttractLastLadderBonus = 0;
    }

    AttractLastPlayfieldMode = 2;
  }

  byte switchHit;
  while ( (switchHit = BSOS_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    if (switchHit == SW_CREDIT_RESET) {
      if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
    }
    if (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3) {
      AddCoinToAudit(switchHit);
      AddCredit(true, 1);
    }
    if (switchHit == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      returnState = MACHINE_STATE_TEST_LIGHTS;
      SetLastSelfTestChangedTime(CurrentTime);
    }
  }

  return returnState;
}





////////////////////////////////////////////////////////////////////////////
//
//  Game Play functions
//
////////////////////////////////////////////////////////////////////////////

int InitGamePlay() {

  if (DEBUG_MESSAGES) {
    Serial.write("Starting game\n\r");
  }

  // The start button has been hit only once to get
  // us into this mode, so we assume a 1-player game
  // at the moment
  BSOS_EnableSolenoidStack();
  BSOS_SetCoinLockout((Credits >= MaximumCredits) ? true : false);
  BSOS_TurnOffAllLamps();

  // Turn back on all lamps that are needed
  SetPlayerLamps(1);

  // When we go back to attract mode, there will be no need to reset scores
  ResetScoresToClearVersion = false;
  SamePlayerShootsAgain = false;

  // Reset displays & game state variables
  for (int count = 0; count < 4; count++) {
    CurrentScores[count] = 0;

    // Initialize game-specific variables
    ABLaneState = 0x11;
    PopBumperGoal[count] = NUM_POP_BUMPERS_HIT_GOAL;
    ABLaneGoal[count] = NUM_ORBITS_IN_AB_GOAL;
    SlingsAndLanesGoal[count] = NUM_SLINGS_AND_INLANES;
    LeftTargetGoal[count] = NUM_LEFT_TARGETS_GOAL;
    RightTargetGoal[count] = NUM_RIGHT_TARGETS_GOAL;
    ModeCompletionStatus[count] = 0;
  }

  CurrentBallInPlay = 1;
  CurrentNumPlayers = 1;
  ShowPlayerScores(0xFF, false, false);
  CurrentPlayer = 0;
  LeftOutlane = 0;
  RightOutlane = 0;
  LastBHit = 0; 
  LastAHit = 0;
  LastPopBumperHit = 0;
  ScoreOverrideStatus = 0;

  if (BSOS_ReadSingleSwitchState(SW_SAUCER)) {
    BSOS_PushToSolenoidStack(SOL_SAUCER, 5, true);
  }

  return MACHINE_STATE_INIT_NEW_BALL;
}


int InitNewBall(bool curStateChanged, byte playerNum, int ballNum) {

  // If we're coming into this mode for the first time
  // then we have to do everything to set up the new ball
  if (curStateChanged) {
    SamePlayerShootsAgain = false;
    BallFirstSwitchHitTime = 0;
    DropTargetsScoreSpecial = false;

    BSOS_SetDisableFlippers(false);
    BSOS_EnableSolenoidStack();
    BSOS_SetDisplayCredits(Credits, true);
    SetPlayerLamps(playerNum + 1, 4);

    BSOS_SetDisplayBallInPlay(ballNum);
    BSOS_SetLampState(BALL_IN_PLAY, 1);
    BSOS_SetLampState(TILT, 0);

    if (BallSaveNumSeconds > 0) {
      BSOS_SetLampState(SHOOT_AGAIN, 1, 0, 500);
    }

    Bonus = 0;
    BonusX = 1;
    BallSaveUsed = false;
    SkillShotRunning = true;
    BallTimeInTrough = 0;
    NumTiltWarnings = 0;
    LastTiltWarningTime = 0;
    PopBumperPhase = 0;

    // Initialize game-specific start-of-ball lights & variables
    GameMode = GAME_MODE_SKILL_SHOT;
    GameModeStartTime = CurrentTime;
    GameModeEndTime = 0;
    LastModeShotTime = 0;
    BallSaveShown = false;
    ProspectiveModeShown = 0;
    BSOS_SetLampState(SAME_PLAYER_SHOOTS_AGAIN, SamePlayerShootsAgain);
    BSOS_SetLampState(SHOOT_AGAIN, SamePlayerShootsAgain);
    BSOS_SetLampState(LAST_TARGET_SCORES_SPECIAL, 0);
    
    // Start appropriate mode music

    if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
      BSOS_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100);
    }

    // Reset drop targets
    BSOS_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 20);
    BSOS_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 150);
  }

  // We should only consider the ball initialized when
  // the ball is no longer triggering the SW_OUTHOLE
  if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    return MACHINE_STATE_INIT_NEW_BALL;
  } else {
    return MACHINE_STATE_NORMAL_GAMEPLAY;
  }

}


void AddToBonus(byte bonusAddition) {
  Bonus += bonusAddition;
  if (Bonus > MAX_DISPLAY_BONUS) Bonus = MAX_DISPLAY_BONUS;
}


int LastReportedValue = 0;
boolean PlayerUpLightBlinking = false;
unsigned long LastTimeSlingOrLaneHit = 0;

// This function manages all timers, flags, and lights
int NormalGamePlay() {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  if (GameMode==GAME_MODE_WIZARD) {
    WizardMode();
  }

  // If the playfield hasn't been validated yet, flash score and player up num
  if (BallFirstSwitchHitTime == 0) {
    if (!PlayerUpLightBlinking) {
      SetPlayerLamps((CurrentPlayer + 1), 4, 250);
      PlayerUpLightBlinking = true;
    }
  } else {
    if (PlayerUpLightBlinking) {
      SetPlayerLamps((CurrentPlayer + 1), 4);
      PlayerUpLightBlinking = false;
    }
  }


  // Update game mode 
  switch (GameMode) {
    case GAME_MODE_SKILL_SHOT:
      if (BallFirstSwitchHitTime!=0) {
        // Something has been hit, so we shouldn't be in skill shot anymore
        GameMode = GAME_MODE_QUALIFY_SELECT;
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        if (DEBUG_MESSAGES) {
          Serial.write("Exit skill shot - Changing to Qualify Select\n\r");
        }
      }
    break;
    case GAME_MODE_QUALIFY_SELECT:
      // To get from qualify to select, the player has to hit both A&B within 10 seconds
      if (LastAHit && LastBHit && ((CurrentTime-LastAHit)/1000)<AB_TIME_TO_QUALIFY_MODE && ((CurrentTime-LastBHit)/1000)<AB_TIME_TO_QUALIFY_MODE) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        GameMode = GAME_MODE_SELECT_MODE;
        if (DEBUG_MESSAGES) {
          Serial.write("Exit qualify - entering into select mode\n\r");
        }
      }
    break;
    case GAME_MODE_SELECT_MODE:
      if (GameModeEndTime==0) {
        // This mode doesn't have an end
        GameModeEndTime = CurrentTime;
        ProspectiveGameMode = GetNextUnfinishedMode(GAME_MODE_AB_LANES-1);
        if (DEBUG_MESSAGES) {
          Serial.write("In select mode - setting prospective mode to GAME_MODE_AB_LANES\n");
        }
      }
    break;
    case GAME_MODE_AB_LANES:
      if (GameModeEndTime==0) {
        OverrideScoreDisplay(CurrentPlayer, ABLaneGoal[CurrentPlayer], true);
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;
      }
      if (CurrentTime>GameModeEndTime || !ABLaneGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;
        if (ABLaneGoal[CurrentPlayer]<=(NUM_ORBITS_IN_AB_GOAL/2)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_AB_LANES;
        }
      }
    break;
    case GAME_MODE_LEFT_DROP_TARGETS:
      if (GameModeEndTime==0) {
        OverrideScoreDisplay(CurrentPlayer, LeftTargetGoal[CurrentPlayer], true);
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;        
        BSOS_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 100);
      }
      if (CurrentTime>GameModeEndTime || !LeftTargetGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;
        if (LeftTargetGoal[CurrentPlayer]<(NUM_LEFT_TARGETS_GOAL/2)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_LEFT_DROPS;
        }
      }
    break;
    case GAME_MODE_RIGHT_DROP_TARGETS:
      if (GameModeEndTime==0) {
        OverrideScoreDisplay(CurrentPlayer, RightTargetGoal[CurrentPlayer], true);
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;
        BSOS_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 100);
      }
      if (CurrentTime>GameModeEndTime || !RightTargetGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;
        if (LeftTargetGoal[CurrentPlayer]<(NUM_RIGHT_TARGETS_GOAL-4)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_RIGHT_DROPS;
        }
      }
    break;
    case GAME_MODE_POP_BUMPERS:
      if (GameModeEndTime==0) {
        OverrideScoreDisplay(CurrentPlayer, PopBumperGoal[CurrentPlayer], true);
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;
      }
      if (CurrentTime>GameModeEndTime || !PopBumperGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;
        if (PopBumperGoal[CurrentPlayer]==0) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_POP_BUMPERS;
        }
      }
    break;
    case GAME_MODE_SLINGS_AND_LANES:
      if (GameModeEndTime==0) {
        OverrideScoreDisplay(CurrentPlayer, SlingsAndLanesGoal[CurrentPlayer], true);
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;
      }
      if (CurrentTime>GameModeEndTime || !SlingsAndLanesGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;
        if (SlingsAndLanesGoal[CurrentPlayer]<(NUM_SLINGS_AND_INLANES/2)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_SLINGS_AND_LANES;
        }
      }
    break;
    case GAME_MODE_WIZARD:
      if (GameModeEndTime==0) {
        // First time we're in this mode
      }
    break;
  }

  // Show all the appropriate lamps

  ShowABLamps(GameMode, ProspectiveGameMode, ABLaneState);
  ShowSamePlayerLamps(SamePlayerShootsAgain);
  ShowBonusLights(GameMode, ProspectiveGameMode, Bonus);
  ShowBonusXLights(GameMode, ProspectiveGameMode, BonusX, LastTimeSlingOrLaneHit);
  ShowOutlanes(GameMode, ProspectiveGameMode, GetLeftOutlane(CurrentPlayer), GetRightOutlane(CurrentPlayer), LastTimeSlingOrLaneHit);
  ShowSaucerLamps(GameMode);
  ShowABRewardLamps(GameMode, ProspectiveGameMode, ABLaneState);
  ShowPopBumperLamps(GameMode, ProspectiveGameMode, 0, LastPopBumperHit);

  // Check to see if ball is in the outhole
  if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    if (BallTimeInTrough == 0) {
      BallTimeInTrough = CurrentTime;
    } else {
      // Make sure the ball stays on the sensor for at least
      // 0.5 seconds to be sure that it's not bouncing
      if ((CurrentTime - BallTimeInTrough) > 500) {

        if (BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings) {
          // Nothing hit yet, so return the ball to the player
          BSOS_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {
          // if we haven't used the ball save, and we're under the time limit, then save the ball
          if (  !BallSaveUsed &&
                ((CurrentTime - BallFirstSwitchHitTime) / 1000) < ((unsigned long)BallSaveNumSeconds) ) {

            BSOS_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100);
            if (BallFirstSwitchHitTime > 0) {
              BallSaveUsed = true;
              BSOS_SetLampState(SHOOT_AGAIN, 0);
            }
            BallTimeInTrough = CurrentTime;

            returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
          } else {
            ShowPlayerScores(0xFF, false, false);
            PlayBackgroundSong(SOUND_EFFECT_NONE);
            returnState = MACHINE_STATE_COUNTDOWN_BONUS;
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }


  // Check for Wizard Mode
  /*
    if (  WizardModeTimeLimit!=0 &&
          BumperHits[CurrentPlayer]>=PopBumperGoal &&
          BonusX==6 &&
          StarGoalComplete[CurrentPlayer] ) {
      WizardModeStartTime = CurrentTime;
      returnState = MACHINE_STATE_WIZARD_MODE;
      PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_WIZ);
    }
  */

  return returnState;
}



unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;

int CountdownBonus(boolean curStateChanged) {

  // If this is the first time through the countdown loop
  if (curStateChanged) {
    BSOS_SetLampState(BALL_IN_PLAY, 1, 0, 250);

    CountdownStartTime = CurrentTime;
    ShowBonusOnTree(Bonus);

    LastCountdownReportTime = CountdownStartTime;
    BonusCountDownEndTime = 0xFFFFFFFF;
  }

  if ((CurrentTime - LastCountdownReportTime) > 100) {

    if (Bonus > 0) {

      // Only give sound & score if this isn't a tilt
      if (NumTiltWarnings <= MaxTiltWarnings) {
        if (BonusX == 1) {
          PlaySoundEffect(SOUND_EFFECT_BONUS_COUNT);
          CurrentScores[CurrentPlayer] += 1000;
        } else if (BonusX == 2) {
          PlaySoundEffect(SOUND_EFFECT_2X_BONUS_COUNT);
          CurrentScores[CurrentPlayer] += 2000;
        } else if (BonusX > 2) {
          PlaySoundEffect(SOUND_EFFECT_3X_BONUS_COUNT);
          CurrentScores[CurrentPlayer] += 3000;
        }
      }

      Bonus -= 1;
      ShowBonusOnTree(Bonus);
    } else if (BonusCountDownEndTime == 0xFFFFFFFF) {
      PlaySoundEffect(SOUND_EFFECT_BALL_OVER);
      BSOS_SetLampState(BONUS_1, 0);
      BonusCountDownEndTime = CurrentTime + 1000;
    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime > BonusCountDownEndTime) {

    // Reset any lights & variables of goals that weren't completed

    BonusCountDownEndTime = 0xFFFFFFFF;
    return MACHINE_STATE_BALL_OVER;
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}



void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    if (CurrentScores[count] > highestScore) highestScore = CurrentScores[count];
    highScorePlayerNum = count;
  }

  if (highestScore > HighScore) {
    HighScore = highestScore;
    if (HighScoreReplay) {
      AddCredit(false, 3);
      BSOS_WriteULToEEProm(BSOS_TOTAL_REPLAYS_EEPROM_START_BYTE, BSOS_ReadULFromEEProm(BSOS_TOTAL_REPLAYS_EEPROM_START_BYTE) + 3);
    }
    BSOS_WriteULToEEProm(BSOS_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    BSOS_WriteULToEEProm(BSOS_TOTAL_HISCORE_BEATEN_START_BYTE, BSOS_ReadULFromEEProm(BSOS_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < 4; count++) {
      if (count == highScorePlayerNum) {
        BSOS_SetDisplay(count, CurrentScores[count], true, 2);
      } else {
        BSOS_SetDisplayBlank(count, 0x00);
      }
    }

    BSOS_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
    BSOS_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 300, true);
    BSOS_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 600, true);
  }
}




unsigned long LastWizardTimeReported = 0;
int WizardMode() {
  int returnMode = GAME_MODE_WIZARD;

  // Show timer & play sound for Wizard Mode
  if ((CurrentTime - LastWizardTimeReported) > 250) {
    unsigned long numSeconds = 1 + (unsigned long)WizardModeTimeLimit - ((CurrentTime - WizardModeStartTime) / 1000);
    byte offset = ((CurrentTime - WizardModeStartTime) / 250) % 8;
    if (offset > 4) offset = 8 - offset;
    unsigned long shift10 = 1;
    for (int powCount = 0; powCount < offset; powCount++) {
      shift10 *= 10;
    }

    if (  (numSeconds > 20 && (offset == 0)) ||
          (numSeconds > 10 && numSeconds <= 20 && (offset == 0 || offset == 4)) ||
          (numSeconds <= 10 && (offset % 2) == 0) ) {
      PlaySoundEffect(SOUND_EFFECT_WIZARD_TIMER);
    }

    for (int count = 0; count < 4; count++) {
      if (count != CurrentPlayer) {
        BSOS_SetDisplay(count, numSeconds * shift10);
        if (numSeconds < 10) BSOS_SetDisplayBlank(count, 0x20 >> offset);
        else BSOS_SetDisplayBlank(count, 0x30 >> offset);
      }
    }
    LastWizardTimeReported = CurrentTime;
  }

  if ((CurrentTime - WizardModeStartTime) > (1000 * (unsigned long)WizardModeTimeLimit)) {

    // Wizard mode is complete - return variables & displays to normal

    for (int count = 0; count < 4; count++) {
      BSOS_SetDisplay(count, CurrentScores[count]);
      if (count < CurrentNumPlayers) {
        if (CurrentScores[count] > 0) BSOS_SetDisplay(count, CurrentScores[count], true, 2);
        else BSOS_SetDisplayBlank(count, 0x30);
      }
      else BSOS_SetDisplayBlank(count, 0x00);
    }

    PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_1);
    returnMode = GAME_MODE_SELECT_MODE;
  }

  return returnMode;
}

unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 1500;
    MatchDigit = random(0, 10);
    NumMatchSpins = 0;
    BSOS_SetLampState(MATCH, 1, 0);
    BSOS_SetDisableFlippers();
    ScoreMatches = 0;
    BSOS_SetLampState(BALL_IN_PLAY, 0);
  }

  if (NumMatchSpins < 40) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      PlaySoundEffect(SOUND_EFFECT_MATCH_SPIN);
      BSOS_SetDisplayBallInPlay((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      BSOS_SetLampState(MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        BSOS_SetLampState(MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddCredit(false, 1);
        BSOS_WriteULToEEProm(BSOS_TOTAL_REPLAYS_EEPROM_START_BYTE, BSOS_ReadULFromEEProm(BSOS_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
        BSOS_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
        MatchDelay += 1000;
        NumMatchSpins += 1;
        BSOS_SetLampState(MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < 4; count++) {
    if ((ScoreMatches >> count) & 0x01) {
      // If this score matches, we're going to flash the last two digits
      if ( (CurrentTime / 200) % 2 ) {
        BSOS_SetDisplayBlank(count, BSOS_GetDisplayBlank(count) & 0x0F);
      } else {
        BSOS_SetDisplayBlank(count, BSOS_GetDisplayBlank(count) | 0x30);
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}


byte GetNextUnfinishedMode(byte startMode) {
  byte nextMode;
  if (ModeCompletionStatus[CurrentPlayer]==0x1F) return GAME_MODE_WIZARD;

  for (nextMode=startMode+1; nextMode!=startMode; nextMode++) {
    if (nextMode>GAME_MODE_SLINGS_AND_LANES) nextMode = GAME_MODE_AB_LANES;

    byte bitCheck = (0x01<<(nextMode-GAME_MODE_AB_LANES));
    if (!(ModeCompletionStatus[CurrentPlayer] & bitCheck)) break;
  }
  if (nextMode==startMode) return GAME_MODE_WIZARD;

  return nextMode;
}

boolean CheckIfLeftDropTargetsDown() {
  return (  BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_1) &&
            BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_2) &&
            BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_3) &&
            BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_4) );
}

boolean CheckIfRightDropTargetsDown() {
  return (  BSOS_ReadSingleSwitchState(SW_RIGHT_DROP_TARGET_1) &&
            BSOS_ReadSingleSwitchState(SW_RIGHT_DROP_TARGET_2) &&
            BSOS_ReadSingleSwitchState(SW_RIGHT_DROP_TARGET_3) &&
            BSOS_ReadSingleSwitchState(SW_RIGHT_DROP_TARGET_4) );
}

void HandleLeftDropTarget() {
  // If we're in left-drop target mode, add score and pop them back up
  if (GameMode==GAME_MODE_LEFT_DROP_TARGETS) {
    // Player scores for each target down 
    if (BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_1)) CurrentScores[CurrentPlayer] += 750;
    if (BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_2)) CurrentScores[CurrentPlayer] += 750;
    if (BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_3)) CurrentScores[CurrentPlayer] += 750;
    if (BSOS_ReadSingleSwitchState(SW_LEFT_DROP_TARGET_4)) CurrentScores[CurrentPlayer] += 750;

    BSOS_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 1000);     
    if (LeftTargetGoal[CurrentPlayer]) LeftTargetGoal[CurrentPlayer] -= 1;
  } else {
    CurrentScores[CurrentPlayer] += 500;
    if (CheckIfLeftDropTargetsDown() && CheckIfRightDropTargetsDown()) {
      if (DropTargetsScoreSpecial) {
        AddCredit(true, 1);
      }
      CurrentScores[CurrentPlayer] += 50000;
      BSOS_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 500);     
      BSOS_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 750);     
      BSOS_SetLampState(LAST_TARGET_SCORES_SPECIAL, 1);
      DropTargetsScoreSpecial = true;
    }
  }  
}

void HandleRightDropTarget() {
  // If we're in left-drop target mode, add score and pop them back up
  if (GameMode==GAME_MODE_RIGHT_DROP_TARGETS) {
    // Player scores for each target down 
    CurrentScores[CurrentPlayer] += 1000;
    BSOS_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 1000);     
    if (RightTargetGoal[CurrentPlayer]) RightTargetGoal[CurrentPlayer] -= 1;    
  } else {
    CurrentScores[CurrentPlayer] += 500;
    if (CheckIfLeftDropTargetsDown() && CheckIfRightDropTargetsDown()) {
      if (DropTargetsScoreSpecial) {
        AddCredit(true, 1);
      }
      CurrentScores[CurrentPlayer] += 50000;
      BSOS_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 750);     
      BSOS_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 500);     
      BSOS_SetLampState(LAST_TARGET_SCORES_SPECIAL, 1);
      DropTargetsScoreSpecial = true;
    }
  }  
}


void AddABLandScore() {
  byte aNibble = ABLaneState & 0x0F;
  byte bNibble = (ABLaneState & 0xF0)>>4;
  byte lowestState = aNibble; 
  if (bNibble<lowestState) lowestState = bNibble;

  switch (lowestState) {
    case 1:
      CurrentScores[CurrentPlayer] += 1000;
    break;
    case 2:
      CurrentScores[CurrentPlayer] += 2000;
    break;
    case 3:
      CurrentScores[CurrentPlayer] += 3000;
    break;
    case 4:
      CurrentScores[CurrentPlayer] += 4000;
    break;
    case 6:
      SamePlayerShootsAgain = true;
    break;
    case 7:
      AddCredit(true, 1);
    break;
    default:
      CurrentScores[CurrentPlayer] += 5000;
  }
}

void AddABLaneState(boolean bLaneHit) {
  byte aNibble = ABLaneState & 0x0F;
  byte bNibble = (ABLaneState & 0xF0)>>4;

  if (!bLaneHit) {
    if (aNibble<=bNibble) aNibble += 1;  
  } else {
    if (bNibble<=aNibble) bNibble += 1;
  }
  if (aNibble>15) aNibble = 15;
  if (bNibble>15) bNibble = 15;
  ABLaneState = (bNibble<<4) | aNibble;
}


void WizardSwitchHit() {
  CurrentScores[CurrentPlayer] += WizardSwitchReward;
  PlaySoundEffect(SOUND_EFFECT_WIZARD_SCORE);
}


int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
//  byte bonusAtTop = Bonus;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];

  // Very first time into gameplay loop
  if (curState == MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay();
  } else if (curState == MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged, CurrentPlayer, CurrentBallInPlay);
  } else if (curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = NormalGamePlay();  
  } else if (curState == MACHINE_STATE_COUNTDOWN_BONUS) {
    returnState = CountdownBonus(curStateChanged);
  } else if (curState == MACHINE_STATE_BALL_OVER) {
    if (SamePlayerShootsAgain) {
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {
      CurrentPlayer += 1;
      if (CurrentPlayer >= CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay += 1;
      }

      if (CurrentBallInPlay > BallsPerGame) {
        CheckHighScores();
        PlaySoundEffect(SOUND_EFFECT_GAME_OVER);
        SetPlayerLamps(0);
        for (int count = 0; count < 4; count++) {
          if (count<CurrentNumPlayers) BSOS_SetDisplay(count, CurrentScores[count], true, 2);
          else BSOS_SetDisplayBlank(count, 0x00);
        }

        returnState = MACHINE_STATE_MATCH_MODE;
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState == MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);
  }

  byte switchHit;

  if (NumTiltWarnings <= MaxTiltWarnings) {
    while ( (switchHit = BSOS_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
      
      switch (switchHit) {
        case SW_SLAM:
          //          BSOS_DisableSolenoidStack();
          //          BSOS_SetDisableFlippers(true);
          //          BSOS_TurnOffAllLamps();
          //          BSOS_SetLampState(GAME_OVER, 1);
          //          delay(1000);
          //          return MACHINE_STATE_ATTRACT;
        break;
        case SW_TILT:
          // This should be debounced
          if ((CurrentTime - LastTiltWarningTime) > TILT_WARNING_DEBOUNCE_TIME) {
            LastTiltWarningTime = CurrentTime;
            NumTiltWarnings += 1;
            if (NumTiltWarnings > MaxTiltWarnings) {
              BSOS_DisableSolenoidStack();
              BSOS_SetDisableFlippers(true);
              BSOS_TurnOffAllLamps();
              BSOS_SetLampState(TILT, 1);
            }
            PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
          }
        break;
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LIGHTS;
          SetLastSelfTestChangedTime(CurrentTime);
        break;
        case SW_LEFT_INLANE:
        case SW_RIGHT_INLANE:
          if (GameMode != GAME_MODE_WIZARD) {
            CurrentScores[CurrentPlayer] += 500;
            PlaySoundEffect(SOUND_EFFECT_INLANE_UNLIT);
            if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
            if (GameMode == GAME_MODE_SLINGS_AND_LANES && SlingsAndLanesGoal[CurrentPlayer]) {
              SlingsAndLanesGoal[CurrentPlayer] -= 1;
              LastModeShotTime = CurrentTime;
              CurrentScores[CurrentPlayer] += 1000;
            }
          } else {
            WizardSwitchHit();
          }
          LastTimeSlingOrLaneHit = CurrentTime;
          AddToBonus(1);
        break;
        case SW_LEFT_A_LANE:
        case SW_TOP_A_LANE:
          LastAHit = CurrentTime;
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (GameMode==GAME_MODE_AB_LANES && ABLaneGoal[CurrentPlayer]) {
            ABLaneGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
            OverrideScoreDisplay(CurrentPlayer, ABLaneGoal[CurrentPlayer], true);
            AddToBonus(1);
          }
          AddABLandScore();
          AddABLaneState(false);
          AddToBonus(1);
        break;
        case SW_TOP_B_LANE:
        case SW_RIGHT_B_LANE:
          LastBHit = CurrentTime;
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (GameMode==GAME_MODE_AB_LANES && ABLaneGoal[CurrentPlayer]) {
            ABLaneGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
            OverrideScoreDisplay(CurrentPlayer, ABLaneGoal[CurrentPlayer], true);
            AddToBonus(1);
          }
          AddABLandScore();
          AddABLaneState(true);
          AddToBonus(1);
        break;
        case SW_LEFT_OUTLANE:
        case SW_RIGHT_OUTLANE:
          if (GameMode != GAME_MODE_WIZARD) {
            CurrentScores[CurrentPlayer] += 500;
            PlaySoundEffect(SOUND_EFFECT_OUTLANE_UNLIT);
            if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          } else {
            WizardSwitchHit();
          }
          LastTimeSlingOrLaneHit = CurrentTime;
        break;
        case SW_10_PTS:
          if (GameMode != GAME_MODE_WIZARD) {
            CurrentScores[CurrentPlayer] += 10;
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
            PlaySoundEffect(SOUND_EFFECT_10PT_SWITCH);
          } else {
            WizardSwitchHit();
          }
          break;
        case SW_SAUCER:
          if (GameMode==GAME_MODE_SKILL_SHOT) {
            PlaySoundEffect(SOUND_EFFECT_SKILL_SHOT);
            BSOS_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 1500); 
          } else if (GameMode==GAME_MODE_SELECT_MODE) {
            GameMode = ProspectiveGameMode;
            if (GameMode<GAME_MODE_AB_LANES || GameMode>GAME_MODE_SLINGS_AND_LANES) GameMode = GAME_MODE_AB_LANES;
            GameModeStartTime = CurrentTime;
            GameModeEndTime = 0;
            if (DEBUG_MESSAGES) {
              char buf[128];
              sprintf(buf, "Saucer hit mode=%d\n\r", GameMode);
              Serial.write(buf);
            }
            BSOS_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 1500); 
          } else if (GameMode==GAME_MODE_RIGHT_DROP_TARGETS) {
            if (CheckIfRightDropTargetsDown()) {
              BSOS_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 750); 
              BSOS_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 250);
              CurrentScores[CurrentPlayer] += 5000;
            } else {
              CurrentScores[CurrentPlayer] += 500;
              BSOS_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 750); 
            }
          } else {
            if (DEBUG_MESSAGES) {
              Serial.write("Generic Saucer hit\n\r");
            }
            CurrentScores[CurrentPlayer] += 500;
            BSOS_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 750); 
          }
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          AddToBonus(3);
        break;
        case SW_RIGHT_DROP_TARGET_1:
        case SW_RIGHT_DROP_TARGET_2:
        case SW_RIGHT_DROP_TARGET_3:
        case SW_RIGHT_DROP_TARGET_4:
          //          HandleDropTargetHit(switchHit, CurrentPlayer);
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (GameMode == GAME_MODE_WIZARD) CurrentScores[CurrentPlayer] += WizardSwitchReward;
          if (GameMode==GAME_MODE_RIGHT_DROP_TARGETS && RightTargetGoal[CurrentPlayer]) {
            RightTargetGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
            OverrideScoreDisplay(CurrentPlayer, RightTargetGoal[CurrentPlayer], true);
            AddToBonus(1);
          }
          AddToBonus(1);
        break;
        case SW_LEFT_DROP_TARGET_1:
        case SW_LEFT_DROP_TARGET_2:
        case SW_LEFT_DROP_TARGET_3:
        case SW_LEFT_DROP_TARGET_4:
          //          HandleDropTargetHit(switchHit, CurrentPlayer);
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (GameMode == GAME_MODE_WIZARD) CurrentScores[CurrentPlayer] += WizardSwitchReward;
          if (GameMode==GAME_MODE_RIGHT_DROP_TARGETS && LeftTargetGoal[CurrentPlayer]) {
            LeftTargetGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
            OverrideScoreDisplay(CurrentPlayer, LeftTargetGoal[CurrentPlayer], true);
            AddToBonus(1);
          }
          AddToBonus(1);
        break;
        case SW_BUMPER_1:
        case SW_BUMPER_2:
        case SW_BUMPER_3:
        case SW_BUMPER_4:
          LastPopBumperHit = CurrentTime;
          PopBumperPhase += 1;
          if ((PopBumperPhase%4)==0) {
            ProspectiveGameMode = GetNextUnfinishedMode(ProspectiveGameMode);
            if (DEBUG_MESSAGES) {
              char buf[128];
              sprintf(buf, "Prospective mode changed to=%d\n\r", ProspectiveGameMode);
              Serial.write(buf);
            }
            GameModeStartTime = CurrentTime;
          }
          if (GameMode != GAME_MODE_WIZARD) {
            CurrentScores[CurrentPlayer] += 100;
            PlaySoundEffect(SOUND_EFFECT_BUMPER_HIT);
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          } else {
            WizardSwitchHit();
          }
          if (GameMode==GAME_MODE_POP_BUMPERS && PopBumperGoal[CurrentPlayer]) {
            PopBumperGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
            OverrideScoreDisplay(CurrentPlayer, PopBumperGoal[CurrentPlayer], true);
          }
        break;
        case SW_RIGHT_SLING:
        case SW_LEFT_SLING:
          if (GameMode != GAME_MODE_WIZARD) {
            CurrentScores[CurrentPlayer] += 10;
            PlaySoundEffect(SOUND_EFFECT_SLING_SHOT);
            if (GameMode == GAME_MODE_SLINGS_AND_LANES && SlingsAndLanesGoal[CurrentPlayer]) {
              SlingsAndLanesGoal[CurrentPlayer] -= 1;
              LastModeShotTime = CurrentTime;
              OverrideScoreDisplay(CurrentPlayer, SlingsAndLanesGoal[CurrentPlayer], true);
            }
          } else {
            WizardSwitchHit();
          }
          if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          LastTimeSlingOrLaneHit = CurrentTime;
        break;
        case SW_COIN_1:
        case SW_COIN_2:
        case SW_COIN_3:
          AddCoinToAudit(switchHit);
          AddCredit(true, 1);
        break;
        case SW_CREDIT_RESET:
          if (CurrentBallInPlay < 2) {
            // If we haven't finished the first ball, we can add players
            AddPlayer();
          } else {
            // If the first ball is over, pressing start again resets the game
            if (Credits >= 1 || FreePlayMode) {
              if (!FreePlayMode) {
                Credits -= 1;
                BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, Credits);
                BSOS_SetDisplayCredits(Credits);
              }
              returnState = MACHINE_STATE_INIT_GAMEPLAY;
            }
          }
          if (DEBUG_MESSAGES) {
            Serial.write("Start game button pressed\n\r");
          }
        break;
      }
    }
  } else {
    // We're tilted, so just wait for outhole
    while ( (switchHit = BSOS_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
      switch (switchHit) {
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LIGHTS;
          SetLastSelfTestChangedTime(CurrentTime);
          break;
        case SW_COIN_1:
        case SW_COIN_2:
        case SW_COIN_3:
          AddCoinToAudit(switchHit);
          AddCredit(true, 1);
          break;
      }
    }
  }
  
  if (!ScrollingScores && CurrentScores[CurrentPlayer] > 999999) {
    CurrentScores[CurrentPlayer] -= 999999;
  } else if (CurrentScores[CurrentPlayer]>999999999) {
    CurrentScores[CurrentPlayer] -= 999999999;    
  }
  
  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
  
    if (!TournamentScoring) {
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (((ScoreAwardReplay >> awardCount) & 0x01) == 0x01) {
            AddCredit(false, 1);
            BSOS_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
            BSOS_WriteULToEEProm(BSOS_TOTAL_REPLAYS_EEPROM_START_BYTE, BSOS_ReadULFromEEProm(BSOS_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
          } else {
            SamePlayerShootsAgain = true;
            BSOS_SetLampState(SHOOT_AGAIN, SamePlayerShootsAgain);
            PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL);
          }
        }
      }
    }
  
    LastTimeScoreChanged = CurrentTime;
  }
  
  ShowPlayerScores(CurrentPlayer, (BallFirstSwitchHitTime==0)?true:false, (BallFirstSwitchHitTime>0 && ((CurrentTime-LastTimeScoreChanged)>2000))?true:false);
  
  return returnState;
}


void loop() {

  BSOS_DataRead(0);
  CurrentTime = millis();
  int newMachineState = MachineState;


  if (MachineState < 0) {
    newMachineState = RunSelfTest(MachineState, MachineStateChanged);
  } else if (MachineState == MACHINE_STATE_ATTRACT) {
    newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  } else {
    newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  }

  if (newMachineState != MachineState) {
    MachineState = newMachineState;
    MachineStateChanged = true;
  } else {
    MachineStateChanged = false;
  }

  BSOS_ApplyFlashToLamps(CurrentTime);
  BSOS_UpdateTimedSolenoidStack(CurrentTime);

} 
