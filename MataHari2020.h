/**************************************************************************
 *     This file is part of MataHari2020.
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

#define NUMBER_OF_LAMPS        8

// Lamp Numbers (defines for lamps)
#define BONUS_1                 0
#define BONUS_2                 1
#define BONUS_3                 2
#define BONUS_4                 3
#define BONUS_5                 4
#define BONUS_6                 5
#define BONUS_7                 6
#define BONUS_8                 7
#define BONUS_9                 8
#define BONUS_10                9
#define BONUS_20                10
#define LAST_TARGET_SCORES_SPECIAL  11
#define B_LANE                  14
#define A_LANE                  15
#define AB_SCORES_1000          20
#define AB_SCORES_2000          21
#define AB_SCORES_3000          22
#define AB_SCORES_4000          23
#define AB_SCORES_5000          24
#define AB_SCORES_EB            25 
#define AB_SCORES_SPECIAL       26
#define RIGHT_OUTLANE_50        30
#define LEFT_OUTLANE_50         31
#define POP_BUMPER_2            34
#define POP_BUMPER_1            35
#define BONUS_5X_POTENTIAL      37
#define BONUS_3X_POTENTIAL      38
#define BONUS_2X_POTENTIAL      39
#define SAME_PLAYER_SHOOTS_AGAIN    40
#define MATCH                   41
#define SHOOT_AGAIN             42
#define APRON_CREDIT            43
#define BONUS_5X                45
#define BONUS_3X                46
#define BONUS_2X                47
#define BALL_IN_PLAY            48
#define HIGH_SCORE_TO_DATE      49
#define GAME_OVER               50
#define TILT                    51
#define PLAYER_1                52
#define PLAYER_2                53
#define PLAYER_3                54
#define PLAYER_4                55
#define PLAYER_1_UP             56
#define PLAYER_2_UP             57
#define PLAYER_3_UP             58
#define PLAYER_4_UP             59


#define NUM_OF_SWITCHES     27

// Defines for switches
#define SW_CREDIT_RESET   5
#define SW_TILT           6
#define SW_OUTHOLE        7
#define SW_COIN_3         8
#define SW_COIN_2         9
#define SW_COIN_1         10
#define SW_SLAM           15

#define SW_RIGHT_DROP_TARGET_4  16
#define SW_RIGHT_DROP_TARGET_3  17
#define SW_RIGHT_DROP_TARGET_2  18
#define SW_RIGHT_DROP_TARGET_1  19

#define SW_LEFT_DROP_TARGET_4   20
#define SW_LEFT_DROP_TARGET_3   21
#define SW_LEFT_DROP_TARGET_2   22
#define SW_LEFT_DROP_TARGET_1   23

#define SW_RIGHT_INLANE         24
#define SW_LEFT_INLANE          25
#define SW_10_PTS               26
#define SW_RIGHT_B_LANE         27
#define SW_LEFT_A_LANE          28
#define SW_TOP_B_LANE           29
#define SW_TOP_A_LANE           30
#define SW_SAUCER               31
#define SW_RIGHT_OUTLANE        32
#define SW_LEFT_OUTLANE         33
#define SW_RIGHT_SLING          34
#define SW_LEFT_SLING           35
#define SW_BUMPER_4             36
#define SW_BUMPER_3             38
#define SW_BUMPER_2             39
#define SW_BUMPER_1             37

// Defines for solenoids
#define SOL_SAUCER          0
#define SOL_CHIME_100       1
#define SOL_CHIME_1000      2
#define SOL_CHIME_10000     3
#define SOL_CHIME_10        4
#define SOL_KNOCKER         5
#define SOL_OUTHOLE         6
#define SOL_BUMPER_1        7
#define SOL_BUMPER_2        8
#define SOL_BUMPER_3        9
#define SOL_BUMPER_4        10
#define SOL_LEFT_SLING      11
#define SOL_LEFT_DROP_TARGETS   12
#define SOL_RIGHT_SLING     13
#define SOL_RIGHT_DROP_TARGETS  14  

// SWITCHES_WITH_TRIGGERS are for switches that will automatically
// activate a solenoid (like in the case of a chime that rings on a rollover)
// but SWITCHES_WITH_TRIGGERS are fully debounced before being activated
#define NUM_SWITCHES_WITH_TRIGGERS         6

// PRIORITY_SWITCHES_WITH_TRIGGERS are switches that trigger immediately
// (like for pop bumpers or slings) - they are not debounced completely
#define NUM_PRIORITY_SWITCHES_WITH_TRIGGERS 6

// Define automatic solenoid triggers (switch, solenoid, number of 1/120ths of a second to fire)
struct PlayfieldAndCabinetSwitch TriggeredSwitches[] = {
  { SW_LEFT_SLING, SOL_LEFT_SLING, 4 },
  { SW_RIGHT_SLING, SOL_RIGHT_SLING, 4 },
  { SW_BUMPER_1, SOL_BUMPER_1, 4 },
  { SW_BUMPER_2, SOL_BUMPER_2, 4 },
  { SW_BUMPER_3, SOL_BUMPER_3, 4 },
  { SW_BUMPER_4, SOL_BUMPER_4, 4 },
};
