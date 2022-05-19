/*
 * @(#)palm.h
 *
 * Copyright 2000-2001, Aaron Ardiri     (mailto:aaron@ardiri.com)
 *                      Bas van de Sande (mailto:bass@infosupport.com)
 * All rights reserved.
 *
 * This file was generated as part of the "hexxagon" program developed 
 * for the Palm Computing Platform designed by Palm: 
 *
 *   http://www.palm.com/ 
 *
 * The contents of this file is confidential and proprietrary in nature 
 * ("Confidential Information"). PLAYERistribution or modification without 
 * prior consent of the original author is prohibited. 
 */

#ifndef _PALM_H
#define _PALM_H

// system includes
#include <PalmOS.h>
#include <System/DLServer.h>

// resource "include" :P
#include "resource.h"

// special includes (additional hardware etc)
#include "hardware/GPDLib.h"
#include "hardware/HanderaVGA.h"

// application constants and structures
#define appCreator 'HXGN'
#define __REGISTER__ __attribute__ ((section ("register"))) // code0002.bin
#define __DEVICE__   __attribute__ ((section ("device")))   // code0003.bin
#define __GAME__     __attribute__ ((section ("game")))     // code0004.bin
#define __HELP__     __attribute__ ((section ("help")))     // code0005.bin
#define __SAFE0001__ __attribute__ ((section ("safe0001"))) // code0006.bin
#define __SAFE0002__ __attribute__ ((section ("safe0002"))) // code0007.bin

#define ftrGlobals             1000
#define ftrGlobalsCfgActiveVol 1001
#define ftrDeviceGlobals       2000
#define ftrGraphicsGlobals     3000
#define ftrGameGlobals         4000
#define ftrHelpGlobals         5000
#define ftrRegisterGlobals     6000

#define GAME_FPS  12 // 12 frames per second
#define VERSION   1

#define SCREEN_WIDTH       160
#define SCREEN_HEIGHT      128 // 160x128 visible display area (game)
#define SCREEN_START       16

enum Board
{
    BOARD = 0,
    HOLE,
    YELLOW,
    GREEN,
    BULB1,
    BULB2,
    SPACE        // virtual number...    
};

enum Tile
{
    TILE_PLAYER1 = 0,
    TILE_PLAYER2,
    TILE_PLAYER3,
    TILE_PLAYER4,
    TILE_CPU1,
    TILE_CPU2,
    TILE_CPU3,
    TILE_CPU4,
    ARROW,
    TILE_MORPH_PLAYER1,
    TILE_MORPH_PLAYER2,
    TILE_MORPH_PLAYER3,
    TILE_MORPH_PLAYER4,
    TILE_MORPH_PLAYER5,
    TILE_MORPH_PLAYER6,
    TILE_MORPH_PLAYER7,
    TILE_MORPH_PLAYER8,
    TILE_MORPH_CPU1,
    TILE_MORPH_CPU2,
    TILE_MORPH_CPU3,
    TILE_MORPH_CPU4,
    TILE_MORPH_CPU5,
    TILE_MORPH_CPU6,
    TILE_MORPH_CPU7,
    TILE_MORPH_CPU8,
    CPU_ARROW,          // arrow for human opponent (player duel)
    NO_TILE,            // virtual tile    
    INVALID_TILE,       // invalid tile = virtual tile
    NOT_USED            // indicator that Matrix is not being used...
};

enum BlitTypes
{
    BLIT_UNDEFINED = 0,
    BLIT_SELECTION,
    BLIT_MORPH,
    BLIT_BOARD
};

#define TILE_PLAYER_FIRST       TILE_PLAYER1        // first PLAYER tile
#define TILE_PLAYER_LAST        TILE_PLAYER4        // last PLAYER tile
#define TILE_CPU_FIRST          TILE_CPU1           // first CPU tile
#define TILE_CPU_LAST           TILE_CPU4           // last CPU tile
#define TILE_MORPH_PLAYER_FIRST TILE_MORPH_PLAYER1  // first PLAYER MORPH tile
#define TILE_MORPH_PLAYER_LAST  TILE_MORPH_PLAYER8  // last PLAYER MORPH tile
#define TILE_MORPH_CPU_FIRST    TILE_MORPH_CPU1     // first CPU MORPH tile
#define TILE_MORPH_CPU_LAST     TILE_MORPH_CPU8     // last CPU MORPH tile

// splash animation
enum Splash 
{
    SPLASH1 = 0,
    SPLASH2,
    SPLASH3,
    SPLASH4,
    SPLASH5,
    SPLASH6,
    SPLASH7,
    SPLASH8,
    SPLASH9,
    SPLASH10
};

#define SPLASH_FIRST            SPLASH1             // first splash tile
#define SPLASH_LAST             SPLASH10            // last splash tile

// splash directionss
enum Splash_Direction 
{
    SPLASH_RIGHT_UP = 0,
    SPLASH_RIGHT_DOWN,
    SPLASH_LEFT_UP,
    SPLASH_LEFT_DOWN,
    SPLASH_DOWN,
    SPLASH_UP
};

enum BoardLayout
{
    LAYOUT_TEMPLATE  = 0,   
    LAYOUT_STANDARD,
    LAYOUT_DIAMONDS,
    LAYOUT_HEXMADNESS,
    LAYOUT_BRIDGE,
    LAYOUT_MATRIX,
    LAYOUT_CROSS,
    LAYOUT_HEXTOMAX,
    LAYOUT_HOURGLASS,
    LAYOUT_FINALFRONTIER,
    LAYOUT_RANDOM
};

#define LAYOUT_MAX LAYOUT_RANDOM

enum BoardStyle
{
    BOARD_SWEDISH    = 0,
    BOARD_AMERICAN,
    BOARD_STANDARD,   
    BOARD_RANDOM    
};

enum GameLevel
{
    LEVEL_EASY = 0,
    LEVEL_MEDIUM,
    LEVEL_HARD
};

// type definition for positions
typedef struct
{
    int            x;
    int            y;
} PositionType;

// Type defintion for matrix tile
typedef struct
{
    PositionType   Position;            // Position of the element
    UInt8          Status;              // status of the board
    UInt8          Tile;                // tile on the board        
    Boolean        IsFilled;            // is the board element filled?
    Boolean        IsAnimating;         // animation indicator
} MatrixType;

// Type defintion for matrix moves
typedef struct 
{
    int            sx, sy;
    int            dx, dy;
    int            value;
    Boolean        Reproduction;
} MatrixMove;

typedef struct
{
  struct 
  {
    UInt8          signatureVersion;    // a preference version number
    Char           signature[16];       // a "signature" for decryption
    Char           *hotSyncUsername;    // the HotSync user name of the user
  } system;

  struct 
  {
    UInt16         ctlKeySelect;        // key definition for select
    UInt16         ctlKeyUp;            // key definition for move up
    UInt16         ctlKeyDown;          // key definition for move down
    UInt16         ctlKeyLeft;          // key definition for move left
    UInt16         ctlKeyRight;         // key definition for move right

    Boolean        sndMute;             // sound is muted?
    UInt16         sndVolume;           // the volume setting for sound

    UInt8          lgray;               // the light gray configuration setting
    UInt8          dgray;               // the dark gray configuration setting
  } config;

  struct 
  {
    Boolean        gamePlaying;         // is there a game in play?
    UInt16         gameScore;           // the score of the player
    UInt16         gameBoard;           // board layout
    UInt16         gameStyle;           // board style
    UInt16         gameLevel;           // level of strength / difficulty

    Boolean        playerDuel;          // is this a duel between players?
    Boolean        playerStarting;      // is the player starting or the AI
     
    UInt16         statisticsTotal;     // total number of games played against CPU
    UInt16         statisticsWin;       // total number of games won from CPU
    UInt16         statisticsLoss;      // total number of games lost from CPU
    UInt16         statisticsDraw;      // total number of games draw against CPU
    UInt16         statisticsAbandon;   // total number of games abandoned
    Boolean        statisticsShow;      // if true display statistics after each game
    
    Boolean        SelectionLocked;     // true if the player is not allowed to 
                                        // make a new selection...
    
    UInt16         ScorePlayer;         // score for PLAYER
    UInt16         ScoreCPU;            // score for CPU
    UInt16         ScoreMax;            // max score for current board

    Boolean        PlayerMoves;         // PLAYER moves (true)!
    Int16          MoveDelay;           // the delay between moves
    Coord          BlitTopLeftX;
    Coord          BlitTopLeftY;        // the last known (x,y) topleft area
    UInt16         BlitType;            // the last known blit type
    Boolean        GameOver;            // true, if no moves are left!
    
    // GameBoard
    MatrixType     Matrix[9][9];        // the game board

    // animation stuff
    int            ThinkingTile;        // tile used for cpu thinker
    PositionType   CurrentAnimation;    // current position of animation (
    PositionType   DestinationAnimation;// ending position of animation 
    int            AnimationIncrementX; // increment for x movements in ani
    int            AnimationIncrementY; // increment for y movements in ani
    int            AnimationSteps;      // number of animation steps remaining
    int            TotalAnimationSteps; // total number of animation steps
    int            MatrixDestX;         // x pos in matrix of destination
    int            MatrixDestY;         // y pos in matrix of destination
    Boolean        IsBeaming;           // is there any beaming activity
    Boolean        IsMorphing;          // is there any morphing activity
    int            CurrentSplash;       // which splash is the current one?
        
    // selection area
    struct
    {
      PositionType Coordinate;          // coordinate of selection in matrix
      Boolean      IsValid;             // is the position valid
      Boolean      IsRing1;             // ring1 = ring around initial selection
    } Area[5][5];

    PositionType   TAREA[18];           // all positions around selection  
                                        // pos 0-5 = ring1 6-17=ring2
                                
    // arrow
    struct 
    {
      PositionType Position;            // position of the arrow    
      Boolean      Visible;             // is the arrow visible 
      Boolean      Selected;            // did the arrow select a tile?
      Boolean      SetDestination;      // go for the destination
      Boolean      IsAnimating;         // animation indicator
      int          AnimationTile;       // current tile in animation
      Boolean      MovesLeft;           // does player have moves left?
    } Arrow;

    // CPUs movement structure
    struct
    {
      Boolean      Selected;            // CPU selected a tile
      Boolean      SetDestination;      // CPU set a destination
      PositionType Position;            // position of the arrow (if duel)
      Boolean      Visible;             // is the arrow visible (if duel)
      PositionType Coordinate;          // coordinate of selection in matrix
      PositionType Destination;         // coordinate of destination in matrix
      Boolean      IsAReproduction;     // is the move a reproduction?
      Boolean      DrawMovement;        // draw the movement...
      Boolean      IsAnimating;         // animation indicator..
      int          AnimationTile;       // current tile in animation
      Boolean      MovesLeft;           // does cpu have moves left?
    } CPU;
    
  } game;
  
} PreferencesType;

// this is our 'double check' for decryption - make sure it worked :P
#define CHECK_SIGNATURE(x) (StrCompare(x->system.signature, "|HaCkMe|") == 0)

// local includes
#include "device.h"
#include "graphics.h"
#include "help.h"
#include "game.h"
#include "register.h"
#include "gccfix.h"

// functions
extern UInt32  PilotMain(UInt16, MemPtr, UInt16);
extern void    InitApplication(void);
extern Boolean ApplicationHandleEvent(EventType *);
extern void    ApplicationDisplayDialog(UInt16);
extern void    EventLoop(void);
extern void    EndApplication(void);

#endif 
