/*
 * @(#)game.c
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
 * ("Confidential Information"). distribution or modification without 
 * prior consent of the original author is prohibited. 
 */

#include "palm.h"

// interface
static void GameDrawLevel(PreferencesType *)                          __GAME__;
static void GameDrawBoard(UInt16,int,int,WinHandle)                   __GAME__;
static void GameDrawWin(PreferencesType *)                            __GAME__;
static void GameDrawTile(UInt16,int,int,WinHandle)                    __GAME__;
static void GameSetTile(PreferencesType *,int,int,UInt8)              __GAME__;
static void GameBackupTile(int, int, int, int, WinHandle)             __GAME__;
static void GameRestoreTile(int, int, int, int, WinHandle)            __GAME__;
static void GameSetSelection(PreferencesType *)                       __GAME__;
static void GameSetDestination(PreferencesType *)                     __GAME__;
static void GameSetBoardSelection(PreferencesType *,int,int,
                                  int, int, Boolean)                  __GAME__;
static void GameResetBoardSelection(PreferencesType *)                __GAME__;
static void GameCollectTile(PreferencesType *,int,int)                __GAME__;
static Boolean GameSetNextMorph(PreferencesType *,Boolean)            __GAME__; 
static void GameResetMorph(PreferencesType *, Boolean)                __GAME__;
static Boolean GameEvaluate(PreferencesType *)                        __GAME__;
static Boolean GameMovesLeft(PreferencesType *, int [9][9],int , int) __GAME__; 
static void GameRecalculateScores(PreferencesType *)                  __GAME__;
static void GameAI(PreferencesType *)                                 __GAME__;
static void inline GameCopyMatrix(int [9][9], int [9][9])             __GAME__;
static Boolean inline GameInside(int x, int y)                        __GAME__;
static void GameDrawThink(PreferencesType *)                          __GAME__;
static void GameDrawSplash(PreferencesType *)                         __GAME__;
static Boolean GameResetSplash(PreferencesType *)                     __GAME__;
static int GameThink(PreferencesType *,int [9][9],int , 
                                               Boolean, Boolean)      __GAME__;
static MatrixMove inline GameGetTheBestMove(MatrixMove,
                                            MatrixMove, Boolean )     __GAME__;

// global variable structure
typedef struct
{
  WinHandle    winBoard;                    // the board bitmaps
  WinHandle    winBoardMask;                // the board mask bitmaps
  WinHandle    winBoardBackup;              // the board backup tile
  WinHandle    winTile;                     // the tile bitmaps
  WinHandle    winTileMask;                 // the tile mask bitmaps
  WinHandle    winTileBackup;               // backup a tile
  WinHandle    winArrowBackup;              // the backup tile for an arrow
  WinHandle    winSplash;                   // the splash bitmaps
  WinHandle    winSplashMask;               // the Splash mask bitmaps
  WinHandle    winSplashBackup[6];          // the splash backups
  WinHandle    winLightning;                // the lightning bitmaps
  WinHandle    winLightningMask;            // the lightning mask bitmaps

  UInt32       scrDepth;                    // the depth of the display
  Boolean      colorDisplay;                // are we running in color?
  Boolean      boardOnScreen;               // Is the board drawn already?
  Boolean      boardRedraw;                 // board needs to be redrawn?
  Boolean      scoreChanged;                // has the score changed?
  
  UInt16       gameLevel;                   // the difficulty level
  Boolean      screenDirty;                 // is the screen dirty?

  struct {

    Boolean    gamePadPresent;              // is the gamepad driver present
    UInt16     gamePadLibRef;               // library reference for gamepad

  } hardware;

  struct
  {
    Boolean device;                         // are we running on handera device?
    VgaScreenModeType scrMode;              // current mode (scale, 1:1 etc)
    VgaRotateModeType scrRotate;            // current display rotation

    BitmapType *bmpBoard;                   // the board bitmaps
    BitmapType *bmpBoardMask;               // the board mask bitmaps
    BitmapType *bmpBoardBackup;             // the board backup tile
    BitmapType *bmpTile;                    // the tile bitmaps
    BitmapType *bmpTileMask;                // the tile mask bitmaps
    BitmapType *bmpTileBackup;              // backup a tile
    BitmapType *bmpArrowBackup;             // the backup tile for an arrow
    BitmapType *bmpSplash;                  // the splash bitmaps
    BitmapType *bmpSplashMask;              // the Splash mask bitmaps
    BitmapType *bmpSplashBackup[6];         // the splash backups
    BitmapType *bmpLightning;               // the lightning bitmaps
    BitmapType *bmpLightningMask;           // the lightning mask bitmaps
  } handera;


} GameGlobals;

/**
 * Initialize the Game.
 *
 * @return true if game is initialized, false otherwise.
 */
Boolean 
GameInitialize()
{
  GameGlobals *gbls;
  Err         err, e;
  UInt16      i;
  UInt32      version;
  Boolean     result;
 
  // create the globals object, and register it
  gbls = (GameGlobals *)MemPtrNew(sizeof(GameGlobals));
  MemSet(gbls, sizeof(GameGlobals), 0);
  FtrSet(appCreator, ftrGameGlobals, (UInt32)gbls);

  // load the gamepad driver if available
  {
    Err err;
    
    // attempt to load the library
    err = SysLibFind(GPD_LIB_NAME,&gbls->hardware.gamePadLibRef);
    if (err == sysErrLibNotFound)
      err = SysLibLoad('libr',GPD_LIB_CREATOR,&gbls->hardware.gamePadLibRef);

    // lets determine if it is available
    gbls->hardware.gamePadPresent = (err == errNone);

    // open the library if available
    if (gbls->hardware.gamePadPresent)
      GPDOpen(gbls->hardware.gamePadLibRef);
  }
 
  // determine the screen depth
  gbls->scrDepth   = 1;
  if (DeviceSupportsVersion(romVersion3))
    WinScreenMode(winScreenModeGet,NULL,NULL,&gbls->scrDepth,NULL);

  // are we running in color?
  gbls->colorDisplay = (gbls->scrDepth == 4);
 
  gbls->handera.device =
    (FtrGet(TRGSysFtrID, TRGVgaFtrNum, &version) != ftrErrNoSuchFeature);
  if (gbls->handera.device)
  {
    VgaGetScreenMode(&gbls->handera.scrMode, &gbls->handera.scrRotate);

    // we only care, if in scale-to-fit mode :)
    gbls->handera.device =
      ((gbls->handera.scrMode == screenModeScaleToFit) &&
       (gbls->scrDepth        == 2));
  }

  // initialize our "bitmap" windows
  err = errNone;
  if (gbls->handera.device)
  {
    gbls->handera.bmpBoard =
      BmpCreate(108,12,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winBoard = 
      WinCreateBitmapWindow(gbls->handera.bmpBoard, &e); err |= e;
    err |= (gbls->winBoard == NULL);
    gbls->handera.bmpBoardMask =
      BmpCreate(108,12,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winBoardMask = 
      WinCreateBitmapWindow(gbls->handera.bmpBoardMask, &e); err |= e;
    err |= (gbls->winBoardMask == NULL);
    gbls->handera.bmpBoardBackup =
      BmpCreate(18,12,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winBoardBackup = 
      WinCreateBitmapWindow(gbls->handera.bmpBoardBackup, &e); err |= e;
    err |= (gbls->winBoardBackup == NULL);

    gbls->handera.bmpTile =
      BmpCreate(60,60,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winTile = 
      WinCreateBitmapWindow(gbls->handera.bmpTile, &e); err |= e;
    err |= (gbls->winTile == NULL);
    gbls->handera.bmpTileMask =
      BmpCreate(60,60,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winTileMask = 
      WinCreateBitmapWindow(gbls->handera.bmpTileMask, &e); err |= e;
    err |= (gbls->winTileMask == NULL);
    gbls->handera.bmpTileBackup =
      BmpCreate(12,10,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winTileBackup = 
      WinCreateBitmapWindow(gbls->handera.bmpTileBackup, &e); err |= e;
    err |= (gbls->winTileBackup == NULL);
    gbls->handera.bmpArrowBackup =
      BmpCreate(12,10,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winArrowBackup = 
      WinCreateBitmapWindow(gbls->handera.bmpArrowBackup, &e); err |= e;
    err |= (gbls->winArrowBackup == NULL);

    gbls->handera.bmpSplash =
      BmpCreate(160,90,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winSplash = 
      WinCreateBitmapWindow(gbls->handera.bmpSplash, &e); err |= e;
    err |= (gbls->winSplash == NULL);
    gbls->handera.bmpSplashMask =
      BmpCreate(160,90,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winSplashMask = 
      WinCreateBitmapWindow(gbls->handera.bmpSplashMask, &e); err |= e;
    err |= (gbls->winSplashMask == NULL);
    for (i=0; i<6; i++) 
    {      
      gbls->handera.bmpSplashBackup[i] =
        BmpCreate(16,15,gbls->scrDepth,NULL,&e); err |= e;
      gbls->winSplashBackup[i] = 
        WinCreateBitmapWindow(gbls->handera.bmpSplashBackup[i], &e); err |= e;
      err |= (gbls->winSplashBackup[i] == NULL);
    }

    gbls->handera.bmpLightning =
      BmpCreate(160,90,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winLightning = 
      WinCreateBitmapWindow(gbls->handera.bmpLightning, &e); err |= e;
    err |= (gbls->winLightning == NULL);
    gbls->handera.bmpLightningMask =
      BmpCreate(160,90,gbls->scrDepth,NULL,&e); err |= e;
    gbls->winLightningMask = 
      WinCreateBitmapWindow(gbls->handera.bmpLightningMask, &e); err |= e;
    err |= (gbls->winLightningMask == NULL);
  }
  else
  {
    gbls->winBoard = 
      WinCreateOffscreenWindow(108, 12, screenFormat, &e); err |= e;
    err |= (gbls->winBoard == NULL);
    gbls->winBoardMask = 
      WinCreateOffscreenWindow(108, 12, screenFormat, &e); err |= e;
    err |= (gbls->winBoardMask == NULL);
    gbls->winBoardBackup = 
      WinCreateOffscreenWindow(18,  12, screenFormat, &e); err |= e;
    err |= (gbls->winBoardBackup == NULL);
    gbls->winTile = 
      WinCreateOffscreenWindow(60,  60, screenFormat, &e); err |= e;
    err |= (gbls->winTile == NULL);
    gbls->winTileMask = 
      WinCreateOffscreenWindow(60,  60, screenFormat, &e); err |= e;
    err |= (gbls->winTileMask == NULL);
    gbls->winTileBackup = 
      WinCreateOffscreenWindow(12,  10, screenFormat, &e); err |= e;
    err |= (gbls->winTileBackup == NULL);
    gbls->winArrowBackup = 
      WinCreateOffscreenWindow(12,  10, screenFormat, &e); err |= e;
    err |= (gbls->winArrowBackup == NULL);
    gbls->winSplash = 
      WinCreateOffscreenWindow(160, 90, screenFormat, &e); err |= e;
    err |= (gbls->winSplash == NULL);
    gbls->winSplashMask = 
      WinCreateOffscreenWindow(160, 90, screenFormat, &e); err |= e;
    err |= (gbls->winSplashMask == NULL);
    for (i=0; i<6; i++) 
    {      
      gbls->winSplashBackup[i] = 
            WinCreateOffscreenWindow(16, 15, screenFormat, &e); err |= e;
      err |= (gbls->winSplashBackup[i] == NULL);
    }
    gbls->winLightning = 
      WinCreateOffscreenWindow(160, 90, screenFormat, &e); err |= e;
    err |= (gbls->winLightning == NULL);
    gbls->winLightningMask = 
      WinCreateOffscreenWindow(160, 90, screenFormat, &e); err |= e;
    err |= (gbls->winLightningMask == NULL);
  }

  // no problems creating back buffers? load images.
  if (err == errNone) {
  
    WinHandle currWindow;
    MemHandle bitmapHandle;

    currWindow = WinGetDrawWindow();

    // background window
    WinSetDrawWindow(GraphicsGetDrawWindow());
    GraphicsClear();
    
    // board
    WinSetDrawWindow(gbls->winBoard);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapBoard);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);
    
    // board mask
    WinSetDrawWindow(gbls->winBoardMask);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapBoardMask);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // tile
    WinSetDrawWindow(gbls->winTile);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapTile);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // tile mask
    WinSetDrawWindow(gbls->winTileMask);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapTileMask);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // splash
    WinSetDrawWindow(gbls->winSplash);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapSplash);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // splash mask
    WinSetDrawWindow(gbls->winSplashMask);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapSplashMask);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // Lightning
    WinSetDrawWindow(gbls->winLightning);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapLightning);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // Lightning mask
    WinSetDrawWindow(gbls->winLightningMask);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapLightningMask);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    WinSetDrawWindow(currWindow);
  }

  // standard "redraws"...
  gbls->boardOnScreen     = false;
  gbls->boardRedraw       = false;
  gbls->scoreChanged      = true;

  result = (err == errNone);

  return result;
}

/**
 * Reset the Game preferences.
 * 
 * @param prefs the global preference data.
 */
void 
GameResetPreferences(PreferencesType *prefs)
{
  GameGlobals *gbls;
  
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // now we are playing
  prefs->game.gamePlaying = true;

  // reset score 
  prefs->game.gameScore   = 0;

  // reset all scores
  prefs->game.ScoreMax    = 0;
  prefs->game.ScorePlayer = 0;
  prefs->game.ScoreCPU    = 0;
  
  gbls->gameLevel         = prefs->game.gameLevel;
  gbls->screenDirty       = true;

  // the board has not been drawn already
  gbls->boardOnScreen     = false;
  gbls->boardRedraw       = false;
  gbls->scoreChanged      = true;

  // initialize the board
  {
    int i,j;
    int y,x;
    
    for (i=0;i<9;i++) 
    {
      for (j=0; j<9; j++) 
      {
        // determine the upper y for each column
        x = (i>4) ? i-4 : i;
        y = 34 - (i*6);
            
        // determine the shape of the board...
        if (abs(j-i)>4) 
          prefs->game.Matrix[i][j].Status=SPACE;
        else 
        {
          prefs->game.Matrix[i][j].Status=BOARD;        
          prefs->game.ScoreMax++;
        }
        
        // determine the x and y of the cell
        prefs->game.Matrix[i][j].Position.x  = (i*18)+18 - ((i>0)?5*i:0);
        prefs->game.Matrix[i][j].Position.y  = (j*12)+y;
        prefs->game.Matrix[i][j].IsAnimating = false;

        // reset the tiles on the board
        GameSetTile(prefs,i,j,NO_TILE);
        
        // reset the expert score
      }
    }

    // set the initial holes on the board
    {
      GameAdjustmentType adjustType;

      // define the adjustment
      adjustType.adjustMode                      = gameLayoutBoard;
      adjustType.data.layoutBoard.Matrix         = 
        (MatrixType *)(&prefs->game.Matrix[0][0]);
      adjustType.data.layoutBoard.maxScore       = &prefs->game.ScoreMax;

      if (prefs->game.gameBoard == LAYOUT_RANDOM)
        adjustType.data.layoutBoard.gameBoardStyle = SysRandom(0) % LAYOUT_MAX;
      else
        adjustType.data.layoutBoard.gameBoardStyle = prefs->game.gameBoard;


   
      // do it! - if fail.. use standard
      if (!RegisterAdjustGame(prefs, &adjustType))
      {
        prefs->game.Matrix[2][3].Status=HOLE;
        prefs->game.Matrix[4][3].Status=HOLE;
        prefs->game.Matrix[6][5].Status=HOLE;

        prefs->game.ScoreMax -= 3;
      }
      else 
      {   
        // patch for final frontier.. AI couldnt deal with board :(
        if (adjustType.data.layoutBoard.gameBoardStyle == LAYOUT_FINALFRONTIER)
        {
          // make some new holes
      	  prefs->game.Matrix[1][1].Status=HOLE;
          prefs->game.Matrix[3][2].Status=HOLE;
          prefs->game.Matrix[5][3].Status=HOLE;
          prefs->game.Matrix[7][4].Status=HOLE;
          // close existing holes
      	  prefs->game.Matrix[1][2].Status=BOARD;
      	  prefs->game.Matrix[3][3].Status=BOARD;
      	  prefs->game.Matrix[5][4].Status=BOARD;
      	  prefs->game.Matrix[7][5].Status=BOARD;
      	}
      }
   
    }

    // set the initial stones on the board
    switch(prefs->game.gameStyle) 
    {
      case BOARD_AMERICAN:

           // PLAYER
           GameSetTile(prefs,0,4,TILE_PLAYER1);
           GameSetTile(prefs,4,8,TILE_PLAYER1);
           GameSetTile(prefs,8,8,TILE_PLAYER1);

           // CPU 
           GameSetTile(prefs,0,0,TILE_CPU1);
           GameSetTile(prefs,4,0,TILE_CPU1);
           GameSetTile(prefs,8,4,TILE_CPU1);
           break;

       case BOARD_SWEDISH:

           // PLAYER
           GameSetTile(prefs,0,0,TILE_PLAYER1);
           GameSetTile(prefs,4,8,TILE_PLAYER1);
           GameSetTile(prefs,8,4,TILE_PLAYER1);

           // CPU
           GameSetTile(prefs,0,4,TILE_CPU1);
           GameSetTile(prefs,4,0,TILE_CPU1);
           GameSetTile(prefs,8,8,TILE_CPU1);
           break;

       case BOARD_STANDARD:

           // PLAYER
           GameSetTile(prefs,4,8,TILE_PLAYER1);
           GameSetTile(prefs,4,7,TILE_PLAYER1);
           GameSetTile(prefs,4,6,TILE_PLAYER1);
           GameSetTile(prefs,3,6,TILE_PLAYER1);
           GameSetTile(prefs,3,7,TILE_PLAYER1);
           GameSetTile(prefs,5,7,TILE_PLAYER1);
           GameSetTile(prefs,5,8,TILE_PLAYER1);
           GameSetTile(prefs,2,6,TILE_PLAYER1);
           GameSetTile(prefs,6,8,TILE_PLAYER1);

           // CPU
           GameSetTile(prefs,4,0,TILE_CPU1);
           GameSetTile(prefs,4,1,TILE_CPU1);
           GameSetTile(prefs,4,2,TILE_CPU1);
           GameSetTile(prefs,3,0,TILE_CPU1);
           GameSetTile(prefs,3,1,TILE_CPU1);
           GameSetTile(prefs,5,1,TILE_CPU1);
           GameSetTile(prefs,5,2,TILE_CPU1);
           GameSetTile(prefs,2,0,TILE_CPU1);
           GameSetTile(prefs,6,2,TILE_CPU1);
           break;

       case BOARD_RANDOM:

           // PLAYER
           for (i=0; i<9; i++) 
           {
             do {
               x = SysRandom(0) % 9;
               y = SysRandom(0) % 9;
             } 
             while ((prefs->game.Matrix[x][y].Status != BOARD) || 
                    (prefs->game.Matrix[x][y].Tile   != NO_TILE)); 

             GameSetTile(prefs,x,y,TILE_PLAYER1);
           }

           // CPU
           for (i=0; i<9; i++) 
           {
             do {
               x = SysRandom(0) % 9;
               y = SysRandom(0) % 9;
             } 
             while ((prefs->game.Matrix[x][y].Status != BOARD) || 
                    (prefs->game.Matrix[x][y].Tile   != NO_TILE)); 

             GameSetTile(prefs,x,y,TILE_CPU1);
           }
           break;

       default:
           break;
    }

    // reset the scores based on the board.
    GameRecalculateScores(prefs);

    // set the arrow's initial state
    prefs->game.Arrow.Position.x     = 10;
    prefs->game.Arrow.Position.y     = 100;
    prefs->game.Arrow.Visible        = prefs->game.playerStarting;            
    prefs->game.Arrow.Selected       = false;            
    prefs->game.Arrow.SetDestination = false;            
    prefs->game.SelectionLocked      = false;
 
    // ensure it is not gameover
    prefs->game.GameOver             = false;
    
    // who moves first?
    prefs->game.PlayerMoves          = prefs->game.playerStarting;
    prefs->game.playerStarting       = !prefs->game.playerStarting;
    prefs->game.MoveDelay            = 1;

    // setup blitting data
    prefs->game.BlitTopLeftX         = 0;
    prefs->game.BlitTopLeftY         = 0;
    prefs->game.BlitType             = BLIT_UNDEFINED;
    
    // no animations are going on...
    prefs->game.Arrow.IsAnimating    = false;
    prefs->game.CPU.IsAnimating      = false;
    prefs->game.IsBeaming            = false;
    prefs->game.IsMorphing           = false;
    
    // some more initialization
    prefs->game.SelectionLocked      = false;
    prefs->game.Arrow.Selected       = false;
    prefs->game.Arrow.SetDestination = false;
    prefs->game.CPU.Selected         = false;
    prefs->game.CPU.SetDestination   = false;

    // cpu mouse pointer initialisation (if level == LEVEL_HUMAN)
    if (prefs->game.playerDuel)
    {
      prefs->game.CPU.Position.x       = 10;
      prefs->game.CPU.Position.y       = 100;
      prefs->game.CPU.Visible          = !prefs->game.Arrow.Visible;            
    }
    // if not playing against AI cpu arrow may never be visible
    else 
    {
      prefs->game.CPU.Visible = false;	
    }
    // initialization for selection in GameThink routine
    // fill ring 1 (reproduction ring)
    prefs->game.TAREA[0].x  = 0; prefs->game.TAREA[0].y  =-1;
    prefs->game.TAREA[1].x  = 0; prefs->game.TAREA[1].y  =+1;
    prefs->game.TAREA[2].x  =-1; prefs->game.TAREA[2].y  =-1;
    prefs->game.TAREA[3].x  =-1; prefs->game.TAREA[3].y  = 0;
    prefs->game.TAREA[4].x  =+1; prefs->game.TAREA[4].y  = 0;
    prefs->game.TAREA[5].x  =+1; prefs->game.TAREA[5].y  =+1;
    
    // fill ring 2 (deplacement ring)
    prefs->game.TAREA[6].x  = 0; prefs->game.TAREA[6].y  =-2;
    prefs->game.TAREA[7].x  = 0; prefs->game.TAREA[7].y  =+2;
    prefs->game.TAREA[8].x  =-1; prefs->game.TAREA[8].y  =-2;
    prefs->game.TAREA[9].x  =-1; prefs->game.TAREA[9].y  =+1;
    prefs->game.TAREA[10].x =+1; prefs->game.TAREA[10].y =-1;
    prefs->game.TAREA[11].x =+1; prefs->game.TAREA[11].y =+2;
    prefs->game.TAREA[12].x =-2; prefs->game.TAREA[12].y =-2;
    prefs->game.TAREA[13].x =-2; prefs->game.TAREA[13].y =-1;
    prefs->game.TAREA[14].x =-2; prefs->game.TAREA[14].y = 0;
    prefs->game.TAREA[15].x =+2; prefs->game.TAREA[15].y = 0;
    prefs->game.TAREA[16].x =+2; prefs->game.TAREA[16].y =+1;
    prefs->game.TAREA[17].x =+2; prefs->game.TAREA[17].y =+2;

    // force a PLAYER draw
    gbls->boardRedraw  = true;
    gbls->scoreChanged = true;
  
  }
}



/**
 * Process key input from the user.
 * 
 * @param prefs the global preference data.
 * @param keyStatus the current key state.
 */
void 
GameProcessKeyInput(PreferencesType *prefs, UInt32 keyStatus)
{
  GameGlobals   *gbls;
  Coord         oldX, oldY, x, y;
  RectangleType rect;
  Boolean       stuffHappening;
  
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  keyStatus &= (prefs->config.ctlKeySelect |
                prefs->config.ctlKeyUp     |
                prefs->config.ctlKeyDown   |
                prefs->config.ctlKeyLeft   |
                prefs->config.ctlKeyRight);

  // additonal checks here
  if (gbls->hardware.gamePadPresent) {

    UInt8 gamePadKeyStatus;
    Err   err;

    // read the state of the gamepad
    err = GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
    if (err == errNone) {

      // process
      if  ((gamePadKeyStatus & GAMEPAD_DOWN)      != 0) 
        keyStatus |= prefs->config.ctlKeyDown;
      if  ((gamePadKeyStatus & GAMEPAD_UP)        != 0) 
        keyStatus |= prefs->config.ctlKeyUp;
      if  ((gamePadKeyStatus & GAMEPAD_LEFT)      != 0) 
        keyStatus |= prefs->config.ctlKeyLeft;
      if  ((gamePadKeyStatus & GAMEPAD_RIGHT)     != 0) 
        keyStatus |= prefs->config.ctlKeyRight;

      // special purpose :)
      if  ((gamePadKeyStatus & 
           (GAMEPAD_LEFTFIRE | GAMEPAD_RIGHTFIRE | GAMEPAD_SELECT)) != 0)
      {
        // wait until they let it go :) 
        do {
          GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
        } while ((gamePadKeyStatus & 
                 (GAMEPAD_LEFTFIRE | GAMEPAD_RIGHTFIRE | GAMEPAD_SELECT)) != 0);

        keyStatus |= prefs->config.ctlKeySelect;
      }
    }
  }

  stuffHappening = (
                    (prefs->game.Arrow.IsAnimating) || 
                    (prefs->game.CPU.IsAnimating)   ||
                    (prefs->game.IsBeaming)         ||
                    (prefs->game.IsMorphing)        ||
                    (!gbls->boardOnScreen)          ||
                    (gbls->boardRedraw)            
                   ); 

  // gave still active?
  if (!prefs->game.GameOver)
  {
    // did they press at least one of the game keys?
    if (keyStatus != 0)
    {
      // only move when arrow is visble 
      if (((prefs->game.Arrow.Visible) || (prefs->game.CPU.Visible))&& (!stuffHappening))
      {
        // control the movement of the ARROW
        x=(prefs->game.Arrow.Visible)? prefs->game.Arrow.Position.x:prefs->game.CPU.Position.x;
        y=(prefs->game.Arrow.Visible)? prefs->game.Arrow.Position.y:prefs->game.CPU.Position.y;
          
        
        // are we going left?
        if (keyStatus & prefs->config.ctlKeyLeft)     
          x = x-3;
    
        if (keyStatus & prefs->config.ctlKeyRight)     
          x = x+3;
    
        if (keyStatus & prefs->config.ctlKeyUp)     
          y = y-3;
    
        if (keyStatus & prefs->config.ctlKeyDown)     
          y = y+3;

        // bounds checking
        x = (x < 0) ? 0 : x;
        x = (x > 150) ? 150 : x;
        y = (y < 0) ? 0 : y;
        y = (y > 118) ? 118 : y;
    
        if (keyStatus & prefs->config.ctlKeySelect)
        {
          // lets wait until they let go of the key
          while ((KeyCurrentState() & prefs->config.ctlKeySelect) != 0); 

          // set the destination
          if (prefs->game.Arrow.Visible) 
          {       
            if (prefs->game.Arrow.Selected) 
              prefs->game.Arrow.SetDestination = true;
         
            // set the selection
            prefs->game.Arrow.Selected         = true;
          }
          
          if (prefs->game.CPU.Visible) 
          {
            if (prefs->game.CPU.Selected) 
              prefs->game.CPU.SetDestination = true;
         
            // set the selection
            prefs->game.CPU.Selected         = true;
          	
          }
        }
    
        // we have to redraw :9
        if (
             ( ((x != prefs->game.Arrow.Position.x) || (y != prefs->game.Arrow.Position.y)) && (prefs->game.Arrow.Visible))
              || 
             ( ((x != prefs->game.CPU.Position.x) || (y != prefs->game.CPU.Position.y)) && (prefs->game.CPU.Visible))                   
           )
        {
          
          oldX =(prefs->game.Arrow.Visible)? prefs->game.Arrow.Position.x:prefs->game.CPU.Position.x;
          oldY =(prefs->game.Arrow.Visible)? prefs->game.Arrow.Position.y:prefs->game.CPU.Position.y;
          
          // assign the new coordinates to the arrow
          if (prefs->game.Arrow.Visible) 
          {
            prefs->game.Arrow.Position.x = x;
            prefs->game.Arrow.Position.y = y;
          }
          
          if (prefs->game.CPU.Visible)
          {
            prefs->game.CPU.Position.x = x;
            prefs->game.CPU.Position.y = y;	
          }
          
          
          // backup the area that will be behind out bitmap
          GameBackupTile(x,y,10,12,gbls->winArrowBackup);

          // draw the arrow to the back buffer
          GameDrawTile((prefs->game.Arrow.Visible)? ARROW:CPU_ARROW,
                       x, y, GraphicsGetDrawWindow());

          
          // reflect the offscreen changes to the screen
          rect.topLeft.x = oldX;
          rect.topLeft.y = oldY;
          rect.extent.x  = 10;
          rect.extent.y  = 12;
          GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
          GraphicsRepaint();

          rect.topLeft.x = x;
          rect.topLeft.y = y;
          rect.extent.x  = 10;
          rect.extent.y  = 12;
          GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
          GraphicsRepaint();

          // remove arrow from back buffer
          GameRestoreTile(x, y, 10,12,gbls->winArrowBackup);
        }
      }
    }
  }
  else
  {
    // did they press at least one of the game keys?
    if (keyStatus != 0)
    {
      // force exit event
      EventType event;
    
      // we aint playing anymore... 
      prefs->game.gamePlaying = false;

      // hide the arrows
      prefs->game.Arrow.Visible = false;
      prefs->game.CPU.Visible   = false;

      // return to main screen
      MemSet(&event, sizeof(EventType), 0);
      event.eType            = menuEvent;
      event.data.menu.itemID = gameMenuItemExit;
      EvtAddEventToQueue(&event);        
    }
  }
}
  
  
/**
 * Process stylus input from the user.
 * 
 * @param prefs the global preference data.
 * @param x the x co-ordinate of the stylus event.
 * @param y the y co-ordinate of the stylus event.
 * @param move was the penevent a move event?
 */
void 
GameProcessStylusInput(PreferencesType *prefs, Coord x, Coord y, Boolean move)
{
  GameGlobals   *gbls;
  Coord         penX, penY, oldX=0, oldY=0;
  Boolean       penDown, change;
  RectangleType rect;
  Boolean       stuffHappening;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  stuffHappening = (
                    (prefs->game.Arrow.IsAnimating) || 
                    (prefs->game.CPU.IsAnimating)   ||
                    (prefs->game.IsBeaming)         ||
                    (prefs->game.IsMorphing)        ||
                    (!gbls->boardOnScreen)          ||
                    (gbls->boardRedraw)            
                   ); 

  // assign the stylus coordinates to the stylus    
  if (((prefs->game.Arrow.Visible) || (prefs->game.CPU.Visible))&& (!stuffHappening))
  { 
    // wait until they lift the pen
    penDown = true;
    change  = true;
    penX    = x;
    penY    = y;

    do 
    {
      if (change)
      {
        x = penX; y = penY-16;

        // bounds checking
        x = (x < 0) ? 0 : x;
        x = (x > 150) ? 150 : x;
        y = (y < 0) ? 0 : y;
        y = (y > 118) ? 118 : y;
    
       
        if (prefs->game.Arrow.Visible) 
        {
          oldX = prefs->game.Arrow.Position.x;
          oldY = prefs->game.Arrow.Position.y;
          prefs->game.Arrow.Position.x = x;
          prefs->game.Arrow.Position.y = y;
        }
        if (prefs->game.CPU.Visible)
        {
          oldX = prefs->game.CPU.Position.x;
          oldY = prefs->game.CPU.Position.y;
          prefs->game.CPU.Position.x = x;
          prefs->game.CPU.Position.y = y;
        }
 
         
        // backup the area that will be behind out bitmap
        GameBackupTile(x,y,10,12,gbls->winArrowBackup);

        // draw the arrow to the back buffer
        GameDrawTile((prefs->game.Arrow.Visible)? ARROW:CPU_ARROW,
                      x, y, GraphicsGetDrawWindow());
        
 
        // reflect the offscreen changes to the screen
        rect.topLeft.x = oldX;
        rect.topLeft.y = oldY;
        rect.extent.x  = 10;
        rect.extent.y  = 12;
        GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
        GraphicsRepaint();

        rect.topLeft.x = x;
        rect.topLeft.y = y;
        rect.extent.x  = 10;
        rect.extent.y  = 12;
        GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
        GraphicsRepaint();

        // remove arrow from back buffer
        GameRestoreTile(x, y, 10,12,gbls->winArrowBackup);
      }

      EvtGetPen(&penX, &penY, &penDown);
      change = ((x != penX) || (y != penY)); 

    } while (penDown);

    // determine if we have to select or to set the destination...
    if (!move) 
    {
      if (prefs->game.Arrow.Visible)
      {	
        if (!prefs->game.Arrow.Selected) 
          prefs->game.Arrow.Selected       = true;
        else 
          prefs->game.Arrow.SetDestination = true;
      }
      
      if (prefs->game.CPU.Visible) 
      {
        if (!prefs->game.CPU.Selected) 
          prefs->game.CPU.Selected       = true;
        else 
          prefs->game.CPU.SetDestination = true;	
      }
      
    }
  }
  
  if (prefs->game.GameOver)
  {
    // force exit event
    EventType event;
    
    // we aint playing anymore... 
    prefs->game.gamePlaying = false;

    // hide the arrows
    prefs->game.Arrow.Visible = false;
    prefs->game.CPU.Visible   = false;

    // return to main screen
    MemSet(&event, sizeof(EventType), 0);
    event.eType            = menuEvent;
    event.data.menu.itemID = gameMenuItemExit;
    EvtAddEventToQueue(&event);        
  }
}

/**
 * Process the object movement in the game.
 * 
 * @param prefs the global preference data.
 */
void 
GameMovement(PreferencesType *prefs)
{
  GameGlobals   *gbls;
  int           i,j,Tile;
  Boolean       IsPlayer;
    
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // only move if permitted :) [computer delay for updating] = HACK
  if (prefs->game.MoveDelay == 0)
  {
    // only do the moving if not animating...
    if ((!prefs->game.CPU.IsAnimating) && (!prefs->game.Arrow.IsAnimating)) 
    { 
      // only play when not gameover...
      if (!prefs->game.GameOver) 
      {
        // who's turn is it to play?
        // PLAYER's move (player)
        if (prefs->game.PlayerMoves) 
        {
          // does the player force a selection?
          if (prefs->game.Arrow.Selected) 
            GameSetSelection(prefs);
  
          // set selection
          if (prefs->game.Arrow.SetDestination) 
            GameSetDestination(prefs);    
        }
  
        // CPU's move    
        else 
        { 
          if (prefs->game.playerDuel)
          {
            // human opponent is playing... (did opponent force a selection)?
            if (prefs->game.CPU.Selected) 
              GameSetSelection(prefs);
  
            // set selection for opponent
            if (prefs->game.CPU.SetDestination) 
              GameSetDestination(prefs);  
          }
          else 
          {
            // ok get the best move from the AI
            if (!prefs->game.CPU.Selected) GameAI(prefs);
      
            // set destination
            if (prefs->game.CPU.SetDestination) 
            {
              if (prefs->game.CPU.DrawMovement) 
              {
                // wait 50 ticks
                SysTaskDelay(50);
                GameSetDestination(prefs);    
                prefs->game.CPU.DrawMovement = false;
              }
            }
  
            // set selection
            if (prefs->game.CPU.Selected) 
            {
              GameSetSelection(prefs);
              prefs->game.CPU.DrawMovement = true;
            }
          }
        }
      }
      else 
      {
        // not all the tiles have been collected yet :)
        if ((prefs->game.ScorePlayer + prefs->game.ScoreCPU) < 
             prefs->game.ScoreMax)
        {
          // rule #1 - player has no more moves :)
          if (!prefs->game.Arrow.MovesLeft)
          {
            Tile     = TILE_MORPH_CPU4;
            IsPlayer = false;
          }

          // rule #2 - cpu has no more moves :)
          else
          if (!prefs->game.CPU.MovesLeft)
          {
            Tile     = TILE_MORPH_PLAYER4;    
            IsPlayer = true;
          }

          // rule #3 - player is moving, they get points
          else
          if (prefs->game.PlayerMoves)
          {
            Tile     = TILE_MORPH_PLAYER4;    
            IsPlayer = true;
          }

          // rule #4 - computer is moving, they get points
          else 
          {
            Tile     = TILE_MORPH_CPU4;
            IsPlayer = false;
          }
                  
          for (i=0; i<9; i++) 
          {
            for (j=0; j<9; j++) 
            {
              // take the empty spot...
              if ((prefs->game.Matrix[i][j].Status == BOARD) && 
                  (prefs->game.Matrix[i][j].Tile == NO_TILE)) 
              {
                // claim the tile
                prefs->game.Matrix[i][j].Tile = Tile;
              }
            }
          }
                  
          // force the update [may need to animate computer]
          prefs->game.PlayerMoves = !IsPlayer;

          prefs->game.Arrow.IsAnimating = true;
          prefs->game.CPU.IsAnimating   = true;
          prefs->game.IsMorphing        = true;

          // we are now morphing, a new area to update :)
          prefs->game.BlitTopLeftX = 18;
          prefs->game.BlitTopLeftY = 10;
          prefs->game.BlitType     = BLIT_BOARD;
        }
      }
    }
  }
  else
    prefs->game.MoveDelay--;
}

/**
 * Draw the game on the screen.
 * 
 * @param prefs the global preference data.
 */
void   
GameDraw(PreferencesType *prefs)
{
  RectangleType  rect;
  GameGlobals    *gbls;
  Char           displayStr[8];
  FontID         currFont;
  WinHandle      currWindow;
  Boolean        ArrowState = true, CPUArrowState = true;  
  Boolean        IsMorphing = false;
  Coord          x,y;
  SndCommandType arriveSnd = {sndCmdFreqDurationAmp,0,192,10,sndMaxAmp};
    
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // we dont want to update the screen unless we have to!!
  gbls->screenDirty |= (
                        (prefs->game.Arrow.IsAnimating) || 
                        (prefs->game.CPU.IsAnimating)   ||
                        (prefs->game.IsBeaming)         ||
                        (prefs->game.IsMorphing)        ||
                        (!gbls->boardOnScreen)          ||
                        (gbls->boardRedraw)            
                       ); 

  // need to draw the board (only do on first time instance)
  if (!gbls->boardOnScreen)
  {
    // erase the background (important)
    currWindow = WinGetDrawWindow();

    // background window
    WinSetDrawWindow(GraphicsGetDrawWindow());
    GraphicsClear();
    
    // display the stary background
    {
      MemHandle bitmapHandle;

      bitmapHandle = DmGet1Resource('Tbmp', bitmapStarBackground);
      WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
      MemHandleUnlock(bitmapHandle);
      DmReleaseResource(bitmapHandle);
    }

    WinSetDrawWindow(currWindow);

    // draw the whole board to the screen (and copy buffer to screen)
    GameDrawLevel(prefs);
    GraphicsSetUpdate(0, SCREEN_HEIGHT-1);
    GraphicsRepaint();

    gbls->boardOnScreen = true;
  }

  // draw the scoring for both players
  if (gbls->scoreChanged)
  {
    currWindow = WinGetDrawWindow();
    currFont = FntGetFont();

    // draw the tiles on the backbuffer!!!
    GameDrawTile(TILE_PLAYER1,2,116,GraphicsGetDrawWindow());
    GameDrawTile(TILE_CPU1,130,116,GraphicsGetDrawWindow());
      
    // draw the scoring on the backbuffer!!
    FntSetFont(boldFont);
    WinSetDrawWindow(GraphicsGetDrawWindow());
     
    StrPrintF(displayStr, 
              (prefs->game.ScorePlayer < 10) ? "0%d " : "%d ", 
              prefs->game.ScorePlayer);
    if (gbls->colorDisplay) 
      WinDrawInvertedChars(displayStr, StrLen(displayStr), 16, 116);
    else
      WinDrawChars(displayStr, StrLen(displayStr), 16, 116);
         
    StrPrintF(displayStr, 
              (prefs->game.ScoreCPU < 10) ? "0%d " : "%d ", 
              prefs->game.ScoreCPU);
    if (gbls->colorDisplay) 
      WinDrawInvertedChars(displayStr, StrLen(displayStr), 144, 116);
    else
      WinDrawChars(displayStr, StrLen(displayStr), 144, 116);

    FntSetFont(currFont);
    WinSetDrawWindow(currWindow);

    // wait until its changed :)
    gbls->scoreChanged = false;
  }

  if (gbls->screenDirty)
  {
    // do we need to redraw the board?
    if (gbls->boardRedraw)  
    {
      GameDrawLevel(prefs);
      gbls->boardRedraw = false;
    }
  
    // is there an animation going on?
    if ((prefs->game.Arrow.IsAnimating) || (prefs->game.CPU.IsAnimating)) 
    {
      // prevent the arrow from showing
      
      ArrowState    = prefs->game.Arrow.Visible;
      CPUArrowState = prefs->game.CPU.Visible;
      prefs->game.Arrow.Visible = false;
      prefs->game.CPU.Visible   = false;
       
       
      // tile movement is in progress...
      if ((!prefs->game.IsBeaming) && (!prefs->game.IsMorphing)) 
      {
        // get the current position for the animation
        x = prefs->game.CurrentAnimation.x;
        y = prefs->game.CurrentAnimation.y;
      
        // backup the background
        GameBackupTile(x,y,10,12,gbls->winTileBackup);
          
        // is player aniting or cpu?
        if (prefs->game.Arrow.IsAnimating) 
        {
          // player animation
  
          // draw the gametile!!
          GameDrawTile(prefs->game.Arrow.AnimationTile,x,y,
                       GraphicsGetDrawWindow());
              
          // get the next animation in the cyclus
          prefs->game.Arrow.AnimationTile =
            (prefs->game.Arrow.AnimationTile == TILE_PLAYER_LAST)
              ? TILE_PLAYER_FIRST
              : prefs->game.Arrow.AnimationTile + 1;
        }
        else 
        {
          // cpu animation
      
          // draw the gametile!!
          GameDrawTile(prefs->game.CPU.AnimationTile,x,y,
                       GraphicsGetDrawWindow());
              
          // get the next animation in the cyclus
          prefs->game.CPU.AnimationTile =
            (prefs->game.CPU.AnimationTile == TILE_CPU_LAST)
               ? TILE_CPU_FIRST
               : prefs->game.CPU.AnimationTile + 1;
        }
      }    
      else 
      {
        // set the next morphing tile
        if (prefs->game.IsMorphing) 
          IsMorphing =
            GameSetNextMorph(prefs,!prefs->game.PlayerMoves);
  
        // beaming 
        if (prefs->game.IsBeaming) 
          GameDrawSplash(prefs);
      }
    }
    else 
    {
      // game over - and, all tiles collected...?
      if ((prefs->game.GameOver) &&
          ((prefs->game.ScorePlayer + prefs->game.ScoreCPU) >= prefs->game.ScoreMax)) 
      {
        prefs->game.Arrow.Visible = false;
        prefs->game.CPU.Visible   = false;
        GameDrawWin(prefs);
      }
    }
    
    //
    // update the display
    //
  
    rect.topLeft.x = prefs->game.BlitTopLeftX; 
    rect.topLeft.y = prefs->game.BlitTopLeftY;
    switch (prefs->game.BlitType)
    {
      case BLIT_MORPH:
           rect.extent.x  = 42;
           rect.extent.y  = 36;
           break;

      case BLIT_BOARD:
           rect.extent.x  = 122;
           rect.extent.y  = 108;
           break;

      case BLIT_SELECTION:
      default:
           rect.extent.x  = 68;
           rect.extent.y  = 60;
           break;
    }
    GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
    GraphicsRepaint();
  
    // copy the score region to the real screen
    rect.topLeft.x = 0;
    rect.topLeft.y = 116;
    rect.extent.x  = 160;
    rect.extent.y  = 12;
    GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
    GraphicsRepaint();

    // animation.. clean up environment
    if ((prefs->game.Arrow.IsAnimating) || (prefs->game.CPU.IsAnimating)) 
    {
      // restore the arrow state!!
      prefs->game.Arrow.Visible = ArrowState;
      prefs->game.CPU.Visible   = CPUArrowState;
        
      if ((!prefs->game.IsBeaming) && (!prefs->game.IsMorphing)) 
      {
        // get the current position for the animation
        x = prefs->game.CurrentAnimation.x;
        y = prefs->game.CurrentAnimation.y;

        // recover the background!
        GameRestoreTile(x,y,10,12,gbls->winTileBackup);
          
        // update the animation positions
        prefs->game.CurrentAnimation.x += prefs->game.AnimationIncrementX;
        prefs->game.CurrentAnimation.y += prefs->game.AnimationIncrementY;
        
        // do we need to disable the animation
        prefs->game.AnimationSteps--;
        if (prefs->game.AnimationSteps < 1)              
        {
          // movement completed.. let's beam
          prefs->game.IsBeaming     = true;
          prefs->game.CurrentSplash = 0;
                            
          // destination position is visible!
          prefs->game.Matrix[prefs->game.MatrixDestX]
                            [prefs->game.MatrixDestY].IsAnimating = false;
          gbls->boardRedraw = true;

          // give some audible feedback
          DevicePlaySound(&arriveSnd);

          // lets recheck the scores
          GameRecalculateScores(prefs);

          // we are now morphing, a new area to update :)
          prefs->game.BlitTopLeftX = 
            (prefs->game.Matrix[prefs->game.MatrixDestX]
                               [prefs->game.MatrixDestY].Position.x + 9) - 21;
          prefs->game.BlitTopLeftY =
            (prefs->game.Matrix[prefs->game.MatrixDestX]
                               [prefs->game.MatrixDestY].Position.y + 6) - 18;
          prefs->game.BlitType     = BLIT_MORPH;
        }
      }
      else 
      {
        // morphing
        if (prefs->game.IsMorphing) 
        {
          // determine the next morphing stage!              
          prefs->game.IsMorphing = IsMorphing;
              
          if (!IsMorphing) 
          {
            // reset the morphing action
            GameResetMorph(prefs,!prefs->game.PlayerMoves);    
                  
            // stop animating!
            prefs->game.Arrow.IsAnimating = false;
            prefs->game.CPU.IsAnimating   = false;

            // lets recheck the scores
            GameRecalculateScores(prefs);

            // although animation is complete, should refresh to show tiles
            gbls->boardRedraw   = true;

            // we have to do this, to force update of tile after animation
            prefs->game.MoveDelay = 1;
          }
        }
  
        // beaming
        if (prefs->game.IsBeaming) 
        {
          // reset the beaming...
          prefs->game.IsBeaming = GameResetSplash(prefs);
                  
          // beaming completed... let's morph!
          if (!prefs->game.IsBeaming) 
          {
            prefs->game.IsMorphing = true;
            prefs->game.IsBeaming  = false;
  
            // collect the opponent's tiles
            // set the first tile in the morphing animation...
            GameCollectTile(prefs,
                            prefs->game.MatrixDestX,prefs->game.MatrixDestY);

            // although animation is complete, should refresh to show tiles
            gbls->boardRedraw = true;
          }
        }
      }
    }

    // dont draw until needed
    gbls->screenDirty = false;
  }

  // draw the arrow on the screen (only partial update)
  else
  {
    // draw the arrow
    if ((prefs->game.Arrow.Visible) || ((prefs->game.CPU.Visible) && (prefs->game.playerDuel)))
    {
      int ax=0,ay=0;
  
      ax=(prefs->game.Arrow.Visible)? prefs->game.Arrow.Position.x:prefs->game.CPU.Position.x;
      ay=(prefs->game.Arrow.Visible)? prefs->game.Arrow.Position.y:prefs->game.CPU.Position.y;
      
      GameBackupTile(ax,ay,10,12,gbls->winArrowBackup);
                         
      GameDrawTile((prefs->game.Arrow.Visible)? ARROW:CPU_ARROW, ax,ay, GraphicsGetDrawWindow());
    
      //
      // update the display
      //

      rect.topLeft.x = ax;
      rect.topLeft.y = ay;
      rect.extent.x  = 10;
      rect.extent.y  = 12;
      GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
      GraphicsRepaint();
      
      // copy the score region to the real screen
      rect.topLeft.x = 0;
      rect.topLeft.y = 116;
      rect.extent.x  = 160;
      rect.extent.y  = 12;
      GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
      GraphicsRepaint();
      
      GameRestoreTile(ax, ay, 10, 12, gbls->winArrowBackup);
     
    }
  }
}

/**
 * Terminate the game.
 */
void GameTerminate()
{
  GameGlobals *gbls;
  int i;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // unlock the gamepad driver (if available)
  if (gbls->hardware.gamePadPresent) {

    Err    err;
    UInt32 gamePadUserCount;

    err = GPDClose(gbls->hardware.gamePadLibRef, &gamePadUserCount);
    if (gamePadUserCount == 0)
      SysLibRemove(gbls->hardware.gamePadLibRef);
  }

  // remove the bitmaps
  if (gbls->handera.device)
  {
    BmpDelete(gbls->handera.bmpBoard);
    BmpDelete(gbls->handera.bmpBoardMask);
    BmpDelete(gbls->handera.bmpBoardBackup);
    BmpDelete(gbls->handera.bmpTile);
    BmpDelete(gbls->handera.bmpTileMask);
    BmpDelete(gbls->handera.bmpTileBackup);
    BmpDelete(gbls->handera.bmpArrowBackup);
    BmpDelete(gbls->handera.bmpSplash);
    BmpDelete(gbls->handera.bmpSplashMask);
    for (i=0; i<6; i++)
      BmpDelete(gbls->handera.bmpSplashBackup[i]);
    BmpDelete(gbls->handera.bmpLightning);
    BmpDelete(gbls->handera.bmpLightningMask);
  }

  // clean up windows/memory
  if (gbls->winBoard)
    WinDeleteWindow(gbls->winBoard,         false);
  if (gbls->winBoardMask)
    WinDeleteWindow(gbls->winBoardMask,     false);
  if (gbls->winBoardBackup)
    WinDeleteWindow(gbls->winBoardBackup,   false);
  if (gbls->winTile)
    WinDeleteWindow(gbls->winTile,          false);
  if (gbls->winTileMask)
    WinDeleteWindow(gbls->winTileMask,      false);
  if (gbls->winTileBackup)
    WinDeleteWindow(gbls->winTileBackup,    false);
  if (gbls->winArrowBackup)
    WinDeleteWindow(gbls->winArrowBackup,   false);
  if (gbls->winSplash)
    WinDeleteWindow(gbls->winSplash,        false);
  if (gbls->winSplashMask)
    WinDeleteWindow(gbls->winSplashMask,    false);
  for (i=0; i<6; i++) 
  {
    if (gbls->winSplashBackup[i])
      WinDeleteWindow(gbls->winSplashBackup[i],   false);
  }
  if (gbls->winLightning)
    WinDeleteWindow(gbls->winLightning,     false);
  if (gbls->winLightningMask)
    WinDeleteWindow(gbls->winLightningMask, false);
  MemPtrFree(gbls);
  
  // unregister global data
  FtrUnregister(appCreator, ftrGameGlobals);
}

/**
 * Backup a tile from the backgroud buffer.
 *
 * @param x the absolute x-coordinate in the background buffer
 * @param y the absolute y-coordinate in the background buffer
 * @param winBackup where to place the background buffer contents at x,y
 */
static void
GameBackupTile(int x,int y, int height, int width, WinHandle winBackup)
{
  RectangleType scrRect, rect;
  
  rect.topLeft.x    = 0;
  rect.topLeft.y    = 0;
  rect.extent.x     = width;
  rect.extent.y     = height;
  scrRect.topLeft.x = x;
  scrRect.topLeft.y = y;
  scrRect.extent.x  = width;
  scrRect.extent.y  = height;

  // take a backup copy!
  WinCopyRectangle(GraphicsGetDrawWindow(), winBackup,
                   &scrRect, rect.topLeft.x, rect.topLeft.y, winPaint);
}

/**
 * Restore a tile into the backgroud buffer.
 *
 * @param x the absolute x-coordinate in the background buffer
 * @param y the absolute y-coordinate in the background buffer
 * @param winBackup the contents of the old background buffer at x,y
 */
static void 
GameRestoreTile(int x,int y,int height, int width, WinHandle winBackup)
{
  RectangleType scrRect, rect;
  
  rect.topLeft.x    = 0;
  rect.topLeft.y    = 0;
  rect.extent.x     = width;
  rect.extent.y     = height;
  scrRect.topLeft.x = x;
  scrRect.topLeft.y = y;
  scrRect.extent.x  = width;
  scrRect.extent.y  = height;

  // restore the backup copy!
  WinCopyRectangle(winBackup, GraphicsGetDrawWindow(),
                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
}

/**
 * Draw  board onto the background buffer.
 *
 * @param board the boardtile to draw
 * @param x the x-coordinate in the level array (position)
 * @param y the y-coordinate in the level array (position)
 */
static void 
GameDrawBoard(UInt16 tile,int x,int y, WinHandle win)
{
  GameGlobals       *gbls;
  RectangleType     scrRect, rect;
        
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  rect.extent.x  = 18;    
  rect.extent.y  = 12;
  rect.topLeft.y = 0;
  rect.topLeft.x = tile * rect.extent.x;
  
  scrRect.topLeft.x = x; 
  scrRect.topLeft.y = y; 
  scrRect.extent.y  = 12;
  scrRect.extent.x  = 18; 
    
  // update the screen
  WinCopyRectangle(gbls->winBoardMask, win,
                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
      
  WinCopyRectangle(gbls->winBoard, win,
                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
}

/**
 * Draw Tile onto the background buffer.
 *
 * @param tile the boardtile to draw
 * @param x the x-coordinate in the level array (position)
 * @param y the y-coordinate in the level array (position)
 */
static void 
GameDrawTile(UInt16 tile,int x,int y, WinHandle window)
{
  GameGlobals   *gbls;
  RectangleType scrRect, rect;
        
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  rect.extent.x  = 12;    
  rect.extent.y  = 10;
  rect.topLeft.x = (tile%5) * rect.extent.x;
  rect.topLeft.y = (tile/5) * rect.extent.y;
  
  scrRect.topLeft.x = x; 
  scrRect.topLeft.y = y; 
  scrRect.extent.y  = 10;
  scrRect.extent.x  = 12; 
    
  WinCopyRectangle(gbls->winTileMask, window,
                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);

  WinCopyRectangle(gbls->winTile, window,
                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
}

/**
 * Draw I Win/You Win onto the background buffer.
 * update the statistics for human vs AI gameplay
 *
 * @param prefs 
 */
static void GameDrawWin(PreferencesType *prefs)
{
  WinHandle   currWindow;
  MemHandle   bitmapHandle;
  Boolean     Win=false, Loss=false, Draw=false;
  int         i;
        
  if (prefs->game.GameOver) 
  {
    currWindow = WinGetDrawWindow();

    // draw? 
    if (prefs->game.ScoreCPU == prefs->game.ScorePlayer)    
    {
      Draw=true;
      bitmapHandle = DmGet1Resource('Tbmp', bitmapDraw);
    }

    // cpu wins
    else
    if (prefs->game.ScoreCPU > prefs->game.ScorePlayer)    
    {
      Loss=true;
      
      if (!prefs->game.playerDuel)
         bitmapHandle = DmGet1Resource('Tbmp', bitmapIWin);
      else
         bitmapHandle = DmGet1Resource('Tbmp', bitmapPlyrWin);
    }

    // player wins
    else 
    {
      Win=true;
      if (!prefs->game.playerDuel)
         bitmapHandle = DmGet1Resource('Tbmp', bitmapYouWin);
      else
         bitmapHandle = DmGet1Resource('Tbmp', bitmapPlyrWin);
    }

    WinSetDrawWindow(GraphicsGetDrawWindow());
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 44, 24);
    MemHandleUnlock(bitmapHandle);
    GraphicsSetUpdate(0, SCREEN_HEIGHT-1);
   
    // if we play against a human... show the tile of the winner!!!!
    if ((prefs->game.playerDuel) && (!Draw))
    {
      for (i=0; i<4; i++) 
      {
        // Win == TILE_PLAYER1, Loss==TILE_CPU1
        GameDrawTile((Win)? TILE_PLAYER1:TILE_CPU1, 53+(i*14),34,GraphicsGetDrawWindow());
        GameDrawTile((Win)? TILE_PLAYER1:TILE_CPU1, 53+(i*14),84,GraphicsGetDrawWindow());
      }
    }
   
    GraphicsRepaint();

    // let's handle it properly... we have to update the statistics....  
    if (!prefs->game.playerDuel)
    {
      if (Win)  prefs->game.statisticsWin++;  else
      if (Loss) prefs->game.statisticsLoss++; else
      if (Draw) prefs->game.statisticsDraw++;
    }
  
    WinSetDrawWindow(currWindow);
  }      
}

/**
 * Draw the initial game level 
 *
 * @param prefs 
 */
static void 
GameDrawLevel(PreferencesType *prefs)
{
  GameGlobals *gbls;
  int         i,j,x,y;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);
 
  // the board has not been drawn yet, draw the whole fucking thing.
  if (!gbls->boardOnScreen)
  {
    // draw the board
    for (i=0; i<9; i++) 
    {
      for (j=0; j<9; j++) 
      {
        GameDrawBoard(prefs->game.Matrix[i][j].Status,
                      prefs->game.Matrix[i][j].Position.x,
                      prefs->game.Matrix[i][j].Position.y,
                      GraphicsGetDrawWindow());

        if (prefs->game.Matrix[i][j].IsFilled)
        {
          // only show tile when not animating
          if (!prefs->game.Matrix[i][j].IsAnimating)
          {
            // only draw standard tiles....
            if ((prefs->game.Matrix[i][j].Tile == TILE_PLAYER1) ||
                (prefs->game.Matrix[i][j].Tile == TILE_CPU1))
            {
              GameDrawTile(prefs->game.Matrix[i][j].Tile,
                           prefs->game.Matrix[i][j].Position.x+3,
                           prefs->game.Matrix[i][j].Position.y+1,
                           GraphicsGetDrawWindow());
            }
          }
        }
      }
    }
  }
  else 
  {
    for (i=0; i<19; i++) 
    {
      if (i == 0)
      {
        x = prefs->game.Area[2][2].Coordinate.x;
        y = prefs->game.Area[2][2].Coordinate.y;
      }
      else
      {
        x = prefs->game.Area[2][2].Coordinate.x + prefs->game.TAREA[i-1].x;
        y = prefs->game.Area[2][2].Coordinate.y + prefs->game.TAREA[i-1].y;
      }

      if (GameInside(x, y))
      {
        GameDrawBoard(prefs->game.Matrix[x][y].Status,
                      prefs->game.Matrix[x][y].Position.x,
                      prefs->game.Matrix[x][y].Position.y,
                      GraphicsGetDrawWindow());

        if (prefs->game.Matrix[x][y].IsFilled)
        {
          // only show tile when not animating
          if (!prefs->game.Matrix[x][y].IsAnimating)
          {
            // only draw standard tiles....
            if ((prefs->game.Matrix[x][y].Tile == TILE_PLAYER1) ||
                (prefs->game.Matrix[x][y].Tile == TILE_CPU1))
            {
              GameDrawTile(prefs->game.Matrix[x][y].Tile,
                           prefs->game.Matrix[x][y].Position.x+3,
                           prefs->game.Matrix[x][y].Position.y+1,
                           GraphicsGetDrawWindow());
            }
          }
        }
      }
    }
  }
}

/**
 * Set a tile on the board.. update the board status correctly
 *
 * @param prefs
 * @param x,y   coordinates in the matrix
 * @param Tile  tile to be inserted in the board 
 */
static void 
GameSetTile(PreferencesType *prefs,int x, int y, UInt8 Tile)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  prefs->game.Matrix[x][y].Tile     = Tile;
  prefs->game.Matrix[x][y].IsFilled = (Tile != NO_TILE);

  gbls->screenDirty = true;
}

/**
 * Set a selection on board.. update the board status correctly
 *
 * @param prefs
 * @param areax,areay   coordinates in the selection area matrix
 * @param x,y   coordinates in the matrix
 * @param Ring1 = true when the selection is in the ring around the selection  
 */
static void 
GameSetBoardSelection(PreferencesType *prefs,int areax, 
                      int areay,int x, int y, Boolean IsRing1)
{
  GameGlobals *gbls;
  Boolean     IsValid;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // are we in a valid position on the board
  if (!((x<0) || (x>=9) || (y<0) || (y>=9))) 
  {
    // is the position valid?
    IsValid= ((prefs->game.Matrix[x][y].Status == BOARD) 
              && (!prefs->game.Matrix[x][y].IsFilled));
    
    // update the matrix
    if (IsValid) 
    {
      // ring1 = green, ring 0, ring2 = yellow
      if (IsRing1) 
        prefs->game.Matrix[x][y].Status = GREEN;
      else
        prefs->game.Matrix[x][y].Status = YELLOW;
    }

    // fill the selection area for the valid movements
    prefs->game.Area[areax][areay].Coordinate.x = x;
    prefs->game.Area[areax][areay].Coordinate.y = y;
    prefs->game.Area[areax][areay].IsRing1      = IsRing1;
    prefs->game.Area[areax][areay].IsValid      = IsValid;

    gbls->screenDirty = true;
  }
}

/**
 * Reset a selection on board.. update the board status correctly
 *
 * @param prefs
 */
static void 
GameResetBoardSelection(PreferencesType *prefs)
{
  int i,j;
    
  // reset the matrix
  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      if ((prefs->game.Matrix[i][j].Status == YELLOW) ||        
          (prefs->game.Matrix[i][j].Status == GREEN)) 
      {
        // reset the selection (yellow and grean)
        prefs->game.Matrix[i][j].Status = BOARD;
      }
    }
  }
    
  // reset the selection area
  for (i=0; i<5; i++) 
  {
    for (j=0; j<5; j++) 
    {
      prefs->game.Area[i][j].IsValid=false;
    }
  }
}

/**
 * Set a selection on the board...  player selected a tile
 *
 * @param prefs
 */
static void 
GameSetSelection(PreferencesType *prefs)  
{
  SndCommandType selectSnd = {sndCmdFreqDurationAmp,0,256,10,sndMaxAmp};
  GameGlobals   *gbls;
  int           i, j, x=0, y=0, playingTile;
  int           cellx, celly, foundx=0, foundy=0;
  Boolean       found, IsValid;
    
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // ok when we have a selection, verify if the player want to undo it
  if ((prefs->game.SelectionLocked) && ((prefs->game.PlayerMoves) || (prefs->game.playerDuel)))
  {
    // get the top left coordinate of the cell
    i=prefs->game.Area[2][2].Coordinate.x;
    j=prefs->game.Area[2][2].Coordinate.y;
        
    cellx=prefs->game.Matrix[i][j].Position.x;
    celly=prefs->game.Matrix[i][j].Position.y;
    
    // ok we are on top of the original cell.. reset the entire selection!!
    if ((x>=cellx+4) && (x<=cellx+13) &&
        (y>=celly  ) && (y<=celly+11))  
    {
      GameResetBoardSelection(prefs);
  
      if (prefs->game.PlayerMoves)
      { 
        prefs->game.Arrow.Selected       = false;
        prefs->game.Arrow.SetDestination = false;
        prefs->game.SelectionLocked      = false;
      }
      else
      {
        prefs->game.CPU.Selected       = false;
        prefs->game.CPU.SetDestination = false;
        prefs->game.SelectionLocked    = false;
      }
    }
  }

  else  
  {
    // reset the board selection
    GameResetBoardSelection(prefs);
    
    if ((prefs->game.PlayerMoves) || (prefs->game.playerDuel))
    {
      // get the arrow coordinates
      x=(prefs->game.PlayerMoves)? prefs->game.Arrow.Position.x:prefs->game.CPU.Position.x;
      y=(prefs->game.PlayerMoves)? prefs->game.Arrow.Position.y:prefs->game.CPU.Position.y;
    
      // who is playing
      playingTile=(prefs->game.PlayerMoves)? TILE_PLAYER1:TILE_CPU1;
      
      // brute force method
      // per cell: determine the largest square and check 
      // if the coordinate falls in the square
      found   = false;
      IsValid = false;
        
      for (i=0; i<9; i++) 
      {
        for (j=0; j<9; j++) 
        {
          // get the top left coordinate of the cell
          cellx=prefs->game.Matrix[i][j].Position.x;
          celly=prefs->game.Matrix[i][j].Position.y;
            
          if ((x>=cellx+4) && (x<=cellx+13) &&
              (y>=celly  ) && (y<=celly+11))  
          {
            // found it 
            found = true;
                    
            // found the cell
            foundx = i;
            foundy = j;
                    
            // set the inner selection (ring0)
            // we are only allowed to do this on PLAYER stones
            if (prefs->game.Matrix[foundx][foundy].Tile==playingTile) 
            {
              // it is a valid move
              IsValid=true;
            }
            break;    
          }
        }    
            
        // can we bail out?
        if (found) break;
      }        
    
      // oops nothing has been found / or selection is illegal... leave .
      if ((!found) || (!IsValid))  
      {
      	if (prefs->game.PlayerMoves)
        {
          prefs->game.SelectionLocked      = false;
          prefs->game.Arrow.Selected       = false;
          prefs->game.Arrow.SetDestination = false;
        }
        else
        {
          prefs->game.SelectionLocked    = false;
          prefs->game.CPU.Selected       = false;
          prefs->game.CPU.SetDestination = false;
        }
            
        return;
      }
    }
    else 
    {
      // CPU's turn (moves come from the AI module)
      foundx = prefs->game.CPU.Coordinate.x;    
      foundy = prefs->game.CPU.Coordinate.y;
    }
    
    // set the selection in the area grid!
    prefs->game.Area[2][2].Coordinate.x = foundx;
    prefs->game.Area[2][2].Coordinate.y = foundy;
    prefs->game.Area[2][2].IsValid      = true;
    prefs->game.Area[2][2].IsRing1      = true; // actually it is ring 0!!
    
    // keep track of this :) need it for animations
    prefs->game.BlitTopLeftX = 
      (prefs->game.Matrix[foundx][foundy].Position.x + 9) - 34;
    prefs->game.BlitTopLeftY =
      (prefs->game.Matrix[foundx][foundy].Position.y + 6) - 30;
    prefs->game.BlitType     = BLIT_SELECTION;

    // update the matrix (yellow selection)
    prefs->game.Matrix[foundx][foundy].Status = YELLOW;
    
    // as we know the coordinates select the selected cells
    // do the inner ring (ring1)
    GameSetBoardSelection(prefs,2,1,foundx  ,foundy-1,true);
    GameSetBoardSelection(prefs,2,3,foundx  ,foundy+1,true);
    GameSetBoardSelection(prefs,1,1,foundx-1,foundy-1,true);
    GameSetBoardSelection(prefs,1,2,foundx-1,foundy  ,true);
    GameSetBoardSelection(prefs,3,2,foundx+1,foundy  ,true);
    GameSetBoardSelection(prefs,3,3,foundx+1,foundy+1,true);

    // do the outer ring (ring2)
    GameSetBoardSelection(prefs,2,0,foundx  ,foundy-2,false);
    GameSetBoardSelection(prefs,2,4,foundx  ,foundy+2,false);
    GameSetBoardSelection(prefs,1,0,foundx-1,foundy-2,false);
    GameSetBoardSelection(prefs,1,3,foundx-1,foundy+1,false);
    GameSetBoardSelection(prefs,3,1,foundx+1,foundy-1,false);
    GameSetBoardSelection(prefs,3,4,foundx+1,foundy+2,false);
    GameSetBoardSelection(prefs,0,0,foundx-2,foundy-2,false);
    GameSetBoardSelection(prefs,0,1,foundx-2,foundy-1,false);
    GameSetBoardSelection(prefs,0,2,foundx-2,foundy  ,false);
    GameSetBoardSelection(prefs,4,2,foundx+2,foundy  ,false);
    GameSetBoardSelection(prefs,4,3,foundx+2,foundy+1,false);
    GameSetBoardSelection(prefs,4,4,foundx+2,foundy+2,false);
    
    // lock the selection facility... player needs to unselect first...
    prefs->game.SelectionLocked = true;

    // force a redraw of the game board
    gbls->boardRedraw = true;

    // ok, time to play some sound 
    DevicePlaySound(&selectSnd);
  }
}

/**
 * Set a destination for a tile on the board...  player selected a tile
 *
 * @param prefs
 */
static void 
GameSetDestination(PreferencesType *prefs)  
{
  SndCommandType destinationSnd = {sndCmdFreqDurationAmp,0,384,10,sndMaxAmp};
  GameGlobals   *gbls;
  int x,y;
  int cellx,celly;
  int i,j;
  int mi=0,mj=0;
  Boolean found;
  Boolean IsValid;
  Boolean IsRing1=false;
  int Tile;
  int NumberOfStepsX, NumberOfStepsY;
    
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

    
  if ((prefs->game.PlayerMoves) || (prefs->game.playerDuel))
  {
    // set the approriate color
    Tile = (prefs->game.PlayerMoves)? TILE_PLAYER1:TILE_CPU1;
  
    // get the player/player2 (LEVEL_HUMAN) coordinates
    x = (prefs->game.PlayerMoves)? prefs->game.Arrow.Position.x:prefs->game.CPU.Position.x;
    y = (prefs->game.PlayerMoves)? prefs->game.Arrow.Position.y:prefs->game.CPU.Position.y;
          
    // brute force method, walk thru the area matrix and find the appropriate 
    // coordinates per cell: determine the largest square and check 
    // if the coordinate falls in the square
    found = false;
    for (i=0; i<5; i++) 
    {
      for (j=0; j<5; j++) 
      {
        // is the position valid?
        IsValid=prefs->game.Area[i][j].IsValid;

        // the original selection is not a valid position
        // tapping the selection causes the selection process to reset
        if ((i==2) && (j==2)) IsValid = false; 
                
        if (IsValid) 
        {
          // get the top left coordinate of the cell
          mi      = prefs->game.Area[i][j].Coordinate.x;
          mj      = prefs->game.Area[i][j].Coordinate.y;
          IsRing1 = prefs->game.Area[i][j].IsRing1;
            
          cellx   = prefs->game.Matrix[mi][mj].Position.x;
          celly   = prefs->game.Matrix[mi][mj].Position.y;
                
          // do we have the right cell?
          if ((x>=cellx+4) && (x<=cellx+13) && (y>=celly  ) && (y<=celly+11))
          {
            // found it 
            found = true;
            break;    
          }
        }
      }    
            
      // can we bail out?
      if (found) break;
    }        
  }
  else 
  {
    // CPU's turn (moves come from the AI module
    Tile    = TILE_CPU1;
    found   = true;
    i       = prefs->game.CPU.Coordinate.x;
    j       = prefs->game.CPU.Coordinate.y;
    mi      = prefs->game.CPU.Destination.x;
    mj      = prefs->game.CPU.Destination.y;
    IsRing1 = prefs->game.CPU.IsAReproduction;
  }
    
  // we found the tile on the board
  if (found) 
  {
    // we need to fill the new cell
    prefs->game.Matrix[mi][mj].Tile        = Tile;
    prefs->game.Matrix[mi][mj].IsFilled    = true;            
    prefs->game.Matrix[mi][mj].IsAnimating = true;
    
    //----------------------------------------------
    // ANIMATION PREPARATION AREA
    //----------------------------------------------
    
    // set start of animation
    if ((prefs->game.PlayerMoves) || (prefs->game.playerDuel))
    {
      // we are always standing on pos 2,2 in the area!!!
      prefs->game.CurrentAnimation.x =
        prefs->game.Matrix[prefs->game.Area[2][2].Coordinate.x]
                          [prefs->game.Area[2][2].Coordinate.y].Position.x+3;
      prefs->game.CurrentAnimation.y =
        prefs->game.Matrix[prefs->game.Area[2][2].Coordinate.x]
                          [prefs->game.Area[2][2].Coordinate.y].Position.y+1;
    }
    else 
    {
      prefs->game.CurrentAnimation.x = prefs->game.Matrix[i][j].Position.x+3;
      prefs->game.CurrentAnimation.y = prefs->game.Matrix[i][j].Position.y+1;
    }

    // set destination of animation
    prefs->game.DestinationAnimation.x=prefs->game.Matrix[mi][mj].Position.x+3;
    prefs->game.DestinationAnimation.y=prefs->game.Matrix[mi][mj].Position.y+1;

    // determine how we should animate horizontally (left/right)
    // give the animation a false start :)
    if (prefs->game.CurrentAnimation.x > prefs->game.DestinationAnimation.x) 
    {
      prefs->game.AnimationIncrementX = -2;    
      prefs->game.CurrentAnimation.x--;
      NumberOfStepsX                  =
       abs(prefs->game.CurrentAnimation.x-prefs->game.DestinationAnimation.x)/2;
    }
    else 
    {
      if (prefs->game.CurrentAnimation.x < prefs->game.DestinationAnimation.x) 
      {
        prefs->game.AnimationIncrementX = 2;    
        prefs->game.CurrentAnimation.x++;    
        NumberOfStepsX                  =
         abs(prefs->game.CurrentAnimation.x-prefs->game.DestinationAnimation.x)/2;
      }
      else 
      {
        // animation on the same line 
        prefs->game.AnimationIncrementX = 0;    
        NumberOfStepsX                  = 0;
      }
    }
        
    // determine how we should animate vertically (up/down)
    // give the animation a false start :)
    if (prefs->game.CurrentAnimation.y > prefs->game.DestinationAnimation.y) 
    {
      prefs->game.AnimationIncrementY = -1;

      prefs->game.CurrentAnimation.y -= 2;
      NumberOfStepsY                  =
        abs(prefs->game.CurrentAnimation.y-prefs->game.DestinationAnimation.y);
    }
    else 
    {
      if (prefs->game.CurrentAnimation.y<prefs->game.DestinationAnimation.y) 
      {
        prefs->game.AnimationIncrementY = 1;    
        prefs->game.CurrentAnimation.y += 2;
        NumberOfStepsY                  =
         abs(prefs->game.CurrentAnimation.y-prefs->game.DestinationAnimation.y);
      }
      else 
      {
        // animation on the same line 
        prefs->game.AnimationIncrementY = 0;    
        NumberOfStepsY                  = 0;        
      }
    }
  
    // how many steps is the animation taking?
    if (NumberOfStepsX >= NumberOfStepsY) 
    {
      prefs->game.AnimationSteps = NumberOfStepsX;
    }
    else 
    {
      // when we have a large number of Y steps, 
      // we need to alter the factor of the X movement
      // otherwise the tile may fly in a totally different direction
      if (NumberOfStepsY > 13)
      {
        if (prefs->game.AnimationIncrementX < 0) 
        {
          prefs->game.AnimationIncrementX = -1;
        }
        else 
        {
          prefs->game.AnimationIncrementX = (NumberOfStepsX > 0) ? 1 : 0;
        }
      }

      if (prefs->game.AnimationIncrementX != 0)
        prefs->game.AnimationSteps = NumberOfStepsY - 1;
      else
        prefs->game.AnimationSteps = NumberOfStepsY;
    }
  
    prefs->game.TotalAnimationSteps = prefs->game.AnimationSteps;

    // set the destination coordinates in the matrix
    // we need this later on to change status and stuff
    prefs->game.MatrixDestX = mi;
    prefs->game.MatrixDestY = mj;
          
    //----------------------------------------------
    // END OF ANIMATION PREPARATION AREA
    //----------------------------------------------
          
    if (!IsRing1) 
    {
      // get the orginal matrix coordinates
      i = prefs->game.Area[2][2].Coordinate.x;
      j = prefs->game.Area[2][2].Coordinate.y;
      
      // clear the old tile, as we needed to jump
      prefs->game.Matrix[i][j].IsFilled = false;
      prefs->game.Matrix[i][j].Tile     = NO_TILE;
    }

    // assign the tile to the animation
    if (prefs->game.PlayerMoves) 
    {
      prefs->game.Arrow.AnimationTile = TILE_PLAYER_FIRST;
      prefs->game.Arrow.IsAnimating   = true;    
    }
    else 
    {
      prefs->game.CPU.AnimationTile   = TILE_CPU_FIRST;
      prefs->game.CPU.IsAnimating     = true;
    }
      
    // switch turn's
    prefs->game.PlayerMoves = !prefs->game.PlayerMoves;
          
    // the arrow is onlt visible when the player is playing...
    prefs->game.Arrow.Visible = prefs->game.PlayerMoves;
    if (prefs->game.playerDuel)
    {
      prefs->game.CPU.Visible = !prefs->game.PlayerMoves;
    }
    else
    {
      // if playing against AI cpu arrow may never be visible
      prefs->game.CPU.Visible = false;
    }
    
    // ok, time to play some sound
    DevicePlaySound(&destinationSnd);
  }

  // reset the board selection
  GameResetBoardSelection(prefs);
    
  // reset the selection and the destination of the arrow
  if (Tile == TILE_PLAYER1)
  {
    prefs->game.Arrow.Selected       = false;
    prefs->game.Arrow.SetDestination = false;
    prefs->game.SelectionLocked      = false;
  }
  else 
  {
    prefs->game.CPU.Selected         = false;    
    prefs->game.CPU.SetDestination   = false;
    prefs->game.SelectionLocked      = false;
  }
                
  // force a redraw of the game board
  gbls->boardRedraw = true;
}

/**
 * Collect the enemy tiles around the destination
 *
 * @param prefs
 * @param mx,my.. coordinates in the matrix of the destination
 */
static void 
GameCollectTile(PreferencesType *prefs,int mx,int my)
{
  GameGlobals *gbls;
  int         Tile, OppTile, MorphTile;
  int         i=0;
  int         mi=0,mj=0;
    
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // set the appropriate color and the opponent's tile
  // attention: player's switched turns already 
  // so actually !prefs0>game.PlayerMoves = player moving!!
  if (!prefs->game.PlayerMoves)  
  {
    Tile      = TILE_PLAYER1;
    OppTile   = TILE_CPU1;
    MorphTile = TILE_MORPH_PLAYER_FIRST;
  }
  else 
  {
    Tile      = TILE_CPU1;
    OppTile   = TILE_PLAYER1;
    MorphTile = TILE_MORPH_CPU_FIRST;
  }

  // get the ring of tiles around the destination
  while (i<6) 
  {
    switch(i) 
    {
      case 0: mi=mx  ; mj=my-1;break;
      case 1: mi=mx  ; mj=my+1;break;
      case 2: mi=mx+1; mj=my  ;break;
      case 3: mi=mx+1; mj=my+1;break;
      case 4: mi=mx-1; mj=my-1;break;
      case 5: mi=mx-1; mj=my  ;break;
      default: break;
    }
        
    // collect the tile to the own collection :)
    if ((mi>=0) && (mi<9) && (mj>=0) && (mj<9)) 
    {
      if (prefs->game.Matrix[mi][mj].Tile == OppTile) 
      {
        // assign the first morphing tile...
        prefs->game.Matrix[mi][mj].Tile = MorphTile;
      }
    }

    // get the next tile
    i++; 
  }    

  // evaluation (any more moves left)?
  prefs->game.GameOver = GameEvaluate(prefs);
}

/**
 * Set the next morphing tile
 *
 * @param prefs
 * @param IsPlayer
 */
static Boolean 
GameSetNextMorph(PreferencesType *prefs,Boolean IsPlayer) 
{
  int     i=0,j=0;
  int     MinTile, MaxTile;
  int     NextMorphTile=-1,CurrentMorphTile=-1;
  Boolean Done   = false;
  Boolean RetVal = true;
  Boolean Found  = false;
  
  // find the morphing tiles in the matrix
  if (IsPlayer) 
  {
    MinTile = TILE_MORPH_PLAYER_FIRST;
    MaxTile = TILE_MORPH_PLAYER_LAST;            
  } 
  else 
  {
    MinTile = TILE_MORPH_CPU_FIRST;
    MaxTile = TILE_MORPH_CPU_LAST;                
  }
    
  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      if (!Done) 
      {
        // is the cell in morphing state?
        if ((prefs->game.Matrix[i][j].Tile >= MinTile) && 
            (prefs->game.Matrix[i][j].Tile <= MaxTile)) 
        {
          Done  = true;
          Found = true;

          // set the next morph state
          CurrentMorphTile = prefs->game.Matrix[i][j].Tile;
          NextMorphTile    = CurrentMorphTile+1;
                    
          // do we need to stop morphing
          RetVal = (NextMorphTile < MaxTile);  
        }
      }
        
      if (prefs->game.Matrix[i][j].Tile==CurrentMorphTile) 
      {
        prefs->game.Matrix[i][j].Tile=NextMorphTile;    
        
        // draw a clean board tile
        GameDrawBoard(prefs->game.Matrix[i][j].Status,
                      prefs->game.Matrix[i][j].Position.x,
                      prefs->game.Matrix[i][j].Position.y,
                      GraphicsGetDrawWindow());
          
        // draw the morphing tile
        GameDrawTile(prefs->game.Matrix[i][j].Tile,
                     prefs->game.Matrix[i][j].Position.x+3,
                     prefs->game.Matrix[i][j].Position.y+1,
                     GraphicsGetDrawWindow());
      }
    }
  }    
    
  // ensure the morphing is terminated quickly!
  if (!Found) RetVal = false;
    
  // do we need to morph on...
  return RetVal;
}

/**
 * Reset the morphing action... restore the board!!
 *
 * @param prefs
 * @param IsPlayer
 */
static void 
GameResetMorph(PreferencesType *prefs, Boolean IsPlayer)
{
  int i=0,j=0;
  int NewMorphTile=0,CurrentMorphTile=0;
    
  // find the morphing tiles in the matrix
  if (IsPlayer) 
  {
    NewMorphTile     = TILE_PLAYER1;
    CurrentMorphTile = TILE_MORPH_PLAYER_LAST;
  } 
  else 
  {
    NewMorphTile     = TILE_CPU1;
    CurrentMorphTile = TILE_MORPH_CPU_LAST;
  }
    
  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      if (prefs->game.Matrix[i][j].Tile == CurrentMorphTile) 
      {
        // set the default tile to the ended morphing scene
        prefs->game.Matrix[i][j].Tile     = NewMorphTile;    
        prefs->game.Matrix[i][j].IsFilled = true;

        // draw a clean board tile
        GameDrawBoard(prefs->game.Matrix[i][j].Status,
                      prefs->game.Matrix[i][j].Position.x,
                      prefs->game.Matrix[i][j].Position.y,
                      GraphicsGetDrawWindow());
          
        // draw the morphing tile
        GameDrawTile(prefs->game.Matrix[i][j].Tile,
                     prefs->game.Matrix[i][j].Position.x+3,
                     prefs->game.Matrix[i][j].Position.y+1,
                     GraphicsGetDrawWindow());

        // we stopped animating!
        prefs->game.Matrix[i][j].IsAnimating = false;
      }
    }
  }    
}

/**
 * GameEvaluate
 *
 * determine if the game has ended.... a.k.a. gameover
 * when one of the players does not have moving capabilities... the game ends!
 *
 * @param prefs
 * @return true if gameover, false otherwise.
 */
static Boolean 
GameEvaluate(PreferencesType *prefs)
{
  Boolean RetVal;
  Boolean playerCanMove, cpuCanMove;
  int     i,j;
  int     M[9][9];

  // lets assume NO MORE MOVES!!
  RetVal = true;

  // if game over, leave the function
  if (prefs->game.GameOver) 
    goto GameEvaluate_EXIT;
    
  M[0][0]=NOT_USED;
    
  // lets recheck the scores
  GameRecalculateScores(prefs);

  // check if one of the players has 0 tiles left
  if (prefs->game.ScorePlayer == 0) 
  {
    prefs->game.Arrow.MovesLeft = false;  // must mark this
    goto GameEvaluate_EXIT;
  }

  if (prefs->game.ScoreCPU == 0) 
  {
    prefs->game.CPU.MovesLeft = false;    // must mark this
    goto GameEvaluate_EXIT;
  }

  // is the board completely filled?
  if ((prefs->game.ScorePlayer + prefs->game.ScoreCPU) >= prefs->game.ScoreMax) 
    goto GameEvaluate_EXIT;
    
  // has the player moves left?
  playerCanMove = false;
  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      // scan ring 1 anf ring 2 for a possible free tile
      if (prefs->game.Matrix[i][j].Tile == TILE_PLAYER1) 
      {
        if (GameMovesLeft(prefs,M,i,j))  
        {
          playerCanMove = true;
          goto GameEvaluate_PLAYER_SCAN_DONE;
        }
      }
    }
  }

GameEvaluate_PLAYER_SCAN_DONE:

  // does the player have some moves left?
  prefs->game.Arrow.MovesLeft = playerCanMove;

  // has the cpu moves left?
  cpuCanMove = false;
  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      // scan ring 1 anf ring 2 for a possible free tile
      if (prefs->game.Matrix[i][j].Tile == TILE_CPU1) 
      {
        if (GameMovesLeft(prefs,M,i,j))  
        {
          cpuCanMove = true;
          goto GameEvaluate_CPU_SCAN_DONE;
        }
      }
    }
  }

GameEvaluate_CPU_SCAN_DONE:

  // does the cpu have some moves left?
  prefs->game.CPU.MovesLeft = cpuCanMove;

  // cpu and player MUST be able to move
  if (playerCanMove && cpuCanMove)
    RetVal = false;

GameEvaluate_EXIT:

  // is it gameover?
  return RetVal;
}

/**
 * GameMovesLeft
 *
 * determine if there are moves left for the player/cpu
 *
 * @param prefs, int x,y --> current coordinate  M[9][9] = virtual board...
 */
static Boolean 
GameMovesLeft(PreferencesType *prefs, int M[9][9],int x, int y) 
{
  int i;
  int mx,my;
    
  for (i=0; i<18; i++) 
  {
    mx = x+prefs->game.TAREA[i].x;
    my = y+prefs->game.TAREA[i].y;
        
    // only check valid coordinates!!
    if (GameInside(mx,my)) 
    {
      // do we use the matrix or the virtual board (M)
      if (M[0][0] != NOT_USED) 
      {
        if (M[mx][my] == NO_TILE) return true;
      }
      else 
      {
        if ((prefs->game.Matrix[mx][my].Tile   == NO_TILE) && 
            (prefs->game.Matrix[mx][my].Status == BOARD)) 
        {
          return true;    
        }
      }
    }
  }
    
  return false;
}

/**
 * GameCopyMatrix: copy the matrix in which the AI can think it's best move
 *
 * 
 * @param M,M2  reference to the matrixes,  
 */
static void inline 
GameCopyMatrix(int Msource[9][9], int Mdest[9][9])
{
  int i,j;
    
  // copy the matrix to the new matrix (M2)
  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      if (GameInside(i,j)) 
      {
        Mdest[i][j] = Msource[i][j];
      }
      else 
      {
        // tile is not inside valid board!
        Mdest[i][j] = INVALID_TILE;    
      }
    }
  }
}

/**
 * GameInside: is the tile inside the gameboard
 *
 * @param x,y  position to be checked
 */
static Boolean inline 
GameInside(int x, int y) 
{
  return ((x>=0) && (x<9) && (y>=0) && (y<9) && (abs(x-y)<=4));
}

/**
 * GameDrawThink: draw the thinking light bulb during AI
 * 
 * @param prefs
 */
static void 
GameDrawThink(PreferencesType *prefs)
{
  RectangleType rect;
  GameGlobals   *gbls;
    
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // switch think bitmaps
  prefs->game.ThinkingTile =
    (prefs->game.ThinkingTile == BULB2) ? BULB1 : BULB2;

  // backup the background
  GameBackupTile(130,16,12,18,gbls->winBoardBackup);
  GameDrawBoard(prefs->game.ThinkingTile,130,16,GraphicsGetDrawWindow());
    
  //
  // update the display
  //

  rect.topLeft.x = 130;
  rect.topLeft.y = 16;
  rect.extent.x  = 18;
  rect.extent.y  = 12;
  GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
  GraphicsRepaint();
    
  // remove the thinking bulb
  GameRestoreTile(130,16,12,18,gbls->winBoardBackup);
}

/**
 * GameDrawSplash: draw the splash animation
 * 
 * @param prefs
 */
static void 
GameDrawSplash(PreferencesType *prefs)
{
  int           i,x,y,mx,my;
  int           offsetx=0,offsety=0,mi=0,mj=0;
  int           OppTile;
  WinHandle     wintile, wintilemask;
  GameGlobals   *gbls;
  RectangleType scrRect, rect;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);
    
  // get the matrix destination 
  mx = prefs->game.MatrixDestX;
  my = prefs->game.MatrixDestY;
    
  // scan all directions around the matix destination
  for (i=SPLASH_RIGHT_UP; i<=SPLASH_UP; i++) 
  {
    // set the appropriate color and the opponent's tile
    // attention: player's switched turns already 
    // so actually !prefs0>game.PlayerMoves = player moving!!
    if (!prefs->game.PlayerMoves)  
    {
      OppTile     = TILE_CPU1;
      wintile     = gbls->winSplash;
      wintilemask = gbls->winSplashMask;
    }
    else 
    {
      OppTile     = TILE_PLAYER1;
      wintile     = gbls->winLightning;
      wintilemask = gbls->winLightningMask;
    }

    // get the ring of tiles around the destination
    switch(i) 
    {
      case SPLASH_UP         : mi=mx  ; mj=my-1; offsetx= 0; offsety= 2; break;
      case SPLASH_DOWN       : mi=mx  ; mj=my+1; offsetx= 0; offsety=-7; break;
      case SPLASH_RIGHT_UP   : mi=mx+1; mj=my  ; offsetx=-7; offsety= 0; break;
      case SPLASH_RIGHT_DOWN : mi=mx+1; mj=my+1; offsetx=-7; offsety=-8; break;
      case SPLASH_LEFT_UP    : mi=mx-1; mj=my-1; offsetx= 9; offsety= 0; break;
      case SPLASH_LEFT_DOWN  : mi=mx-1; mj=my  ; offsetx= 9; offsety=-8; break;
    }
        
    // beam to the opposite tile
    if ((mi>=0) && (mi<9) && (mj>=0) && (mj<9)) 
    {
      if (prefs->game.Matrix[mi][mj].Tile==OppTile) 
      {
        // set top left x/y
        x = prefs->game.Matrix[mi][mj].Position.x+offsetx;
        y = prefs->game.Matrix[mi][mj].Position.y+offsety;
    
        // backup the background
        GameBackupTile(x,y,15,16,gbls->winSplashBackup[i]);    
                
        // handle coordinate system
        rect.extent.x  = 16;    
        rect.extent.y  = 15;
        rect.topLeft.y = i * rect.extent.y;
        rect.topLeft.x = prefs->game.CurrentSplash * rect.extent.x;
      
        scrRect.topLeft.x = x; 
        scrRect.topLeft.y = y; 
        scrRect.extent.y  = 15;
        scrRect.extent.x  = 16; 

        // draw the splash/lightning mask
        WinCopyRectangle(wintilemask, GraphicsGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);

        // draw the splash/lightning
        WinCopyRectangle(wintile, GraphicsGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
      }
    }
  }
}

/**
 * GameResetSplash: reset the splash animation
 * 
 * @param prefs
 */
static Boolean GameResetSplash(PreferencesType *prefs)    
{
  int            i,x,y,mx,my;
  int            offsetx=0,offsety=0,mi=0,mj=0;
  int            OppTile;
  GameGlobals    *gbls;
  Boolean        splashHappening;
  SndCommandType zapSnd = {sndCmdFreqDurationAmp,0,128,10,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);
    
  // get the matrix destination 
  mx = prefs->game.MatrixDestX;
  my = prefs->game.MatrixDestY;
    
  // assume nothing is happening here
  splashHappening = false;

  // scan all directions around the matix destination
  for (i=SPLASH_UP; i>=SPLASH_RIGHT_UP; i--) 
  {
    // set the appropriate color and the opponent's tile
    // attention: player's switched turns already 
    // so actually !prefs->game.PlayerMoves = player moving!!
    if (!prefs->game.PlayerMoves)  
    {
      OppTile = TILE_CPU1;
    }
    else 
    {
      OppTile = TILE_PLAYER1;
    }
        
    // get the ring of tiles around the destination
    switch(i) 
    {
      case SPLASH_UP         : mi=mx  ; mj=my-1; offsetx= 0; offsety= 2; break;
      case SPLASH_DOWN       : mi=mx  ; mj=my+1; offsetx= 0; offsety=-7; break;
      case SPLASH_RIGHT_UP   : mi=mx+1; mj=my  ; offsetx=-7; offsety= 0; break;
      case SPLASH_RIGHT_DOWN : mi=mx+1; mj=my+1; offsetx=-7; offsety=-8; break;
      case SPLASH_LEFT_UP    : mi=mx-1; mj=my-1; offsetx= 9; offsety= 0; break;
      case SPLASH_LEFT_DOWN  : mi=mx-1; mj=my  ; offsetx= 9; offsety=-8; break;
    }
        
    // beam to the opposite tile
    if ((mi>=0) && (mi<9) && (mj>=0) && (mj<9)) 
    {
      if (prefs->game.Matrix[mi][mj].Tile == OppTile) 
      {
        // set top left x/y
        x = prefs->game.Matrix[mi][mj].Position.x+offsetx;
        y = prefs->game.Matrix[mi][mj].Position.y+offsety;
                
        // ok, some action! :)
        splashHappening = true;

        // restore the background
        GameRestoreTile(x,y,15,16,gbls->winSplashBackup[i]);    
      }
    }
  }
    
  // ok, time to play some sound bzzzt.
  if (splashHappening)
  {
    // computer sound..
    if (prefs->game.PlayerMoves)  
      zapSnd.param1 += (SysRandom(0) % 32) - 16;

    // player sound..
    else
      zapSnd.param1 += 256 + (prefs->game.CurrentSplash * 16);

    // play it!
    DevicePlaySound(&zapSnd);
  }

  // set the next animation
  prefs->game.CurrentSplash++;
    
  // do we need to go on animating
  return (prefs->game.CurrentSplash <= SPLASH10);
}

/**
 * Force a manual re-count of the scores (cpu+player)
 *
 * @param prefs global preferences type.
 */
static void 
GameRecalculateScores(PreferencesType *prefs)
{
  GameGlobals *gbls;
  int         i,j;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  prefs->game.ScorePlayer = 0;
  prefs->game.ScoreCPU    = 0;

  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      switch (prefs->game.Matrix[i][j].Tile)
      {
        case TILE_CPU1:    prefs->game.ScoreCPU++;    break;
        case TILE_PLAYER1: prefs->game.ScorePlayer++; break;
        default: break;
      }
    }
  }

  gbls->scoreChanged = true;
}

/**
 * Game AI... this is the place to be... let's see how smart the computer is!
 *
 * determine which tile in matrix is beeing moved
 *
 * @param prefs
 */
static void 
GameAI(PreferencesType *prefs)
{
  GameGlobals *gbls;
  int         M[9][9];
  int         i,j;
  Boolean     DoRecursion=false;
       
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      if (GameInside(i,j)) 
      {
        // eliminate the holes on the virtual board!
        if (prefs->game.Matrix[i][j].Status == HOLE) 
          M[i][j] = INVALID_TILE;    
        else 
          M[i][j] = prefs->game.Matrix[i][j].Tile;
      }
      else
        M[i][j] = INVALID_TILE;
    }
  }

    
  switch (gbls->gameLevel)
  {
    case LEVEL_EASY  : DoRecursion = false; break;
    case LEVEL_MEDIUM: DoRecursion = false; break;
    case LEVEL_HARD  : DoRecursion = true;  break;	
  }  
    
  GameThink(prefs,M,TILE_CPU1,DoRecursion,true);

  // tell the game that the cpu is ready with thinking and selecting
  prefs->game.CPU.Selected       = true;
  prefs->game.CPU.SetDestination = true;
}

/**
 * Game Think... thinking engine...
 *
 * calculate optimal move for cpu player (recursive use)
 *
 * @param prefs, M[9][9] (virtual board), Tile (current playing tile), depth 
 * (depth of recursion), FirstCall (on first call return the best move)
 */
static int 
GameThink(PreferencesType *prefs, 
          int M[9][9],int Tile, Boolean DoRecursion, Boolean FirstCall)
{
  GameGlobals   *gbls;
  RectangleType rect;
  int           MDest[9][9];
  int           i,j,k,l;
  int           x,y;
  int           OppTile;
  int           scoreTile=0, scoreOppTile=0;
  int           scoreTileInitial, scoreOppTileInitial;
  int           Result;
  int           PossibleLoss;
  MatrixMove    Move,BestMove;
  Boolean       found=false;
  
  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // who's playing.. who's the opponent
  OppTile = (Tile == TILE_CPU1) ? TILE_PLAYER1 : TILE_CPU1;
    
  // init the movement section
  BestMove.value      = -1000;    
    
  // get the initial tile count
  scoreTileInitial    = 0;
  scoreOppTileInitial = 0;
  for (i=0; i<9; i++) 
  {
    for (j=0; j<9; j++) 
    {
      if (M[i][j] == Tile)    scoreTileInitial++;
      if (M[i][j] == OppTile) scoreOppTileInitial++;
    }
  }
    
  for (i=0; i<9; i++) 
  {
    // draw the gfx
    GameDrawThink(prefs);

    for (j=0; j<9; j++) 
    {
      // only calculate move for tile
      if (M[i][j] == Tile) 
      {
        // is the tile able to move?
        if (GameMovesLeft(prefs,M,i,j)) 
        {
          // calculate all possible moves for the tile
          for (k=0; k<18; k++) 
          {
            // reset the scoring
            scoreTile    = scoreTileInitial;
            scoreOppTile = scoreOppTileInitial;
                        
            // determine destination                    
            x = i+prefs->game.TAREA[k].x;
            y = j+prefs->game.TAREA[k].y;
                        
            // do the move...         
            if (GameInside(x,y)) 
            {
              // we can only move to empty tiles
              if (M[x][y]==NO_TILE)  
              {
                // copy the board (only for FirstCall)
                if ((DoRecursion) && (FirstCall)) GameCopyMatrix(M,MDest);

                // movement of tile
                if (k>=6) 
                {
                  if ((DoRecursion) && (FirstCall)) MDest[i][j] = NO_TILE;
                }
                else 
                  scoreTile++;
              
                // set the new destination
                if ((DoRecursion) && (FirstCall)) MDest[x][y] = Tile;
                                
                // conquer the surroundings/calc losses
                PossibleLoss = 0;
  	        for (l=0; l<6; l++) 
                {
                  // conquer = points
                  if (GameInside(x+prefs->game.TAREA[l].x,
                                 y+prefs->game.TAREA[l].y)) 
                  {
                    if (M[x+prefs->game.TAREA[l].x]
                         [y+prefs->game.TAREA[l].y] == OppTile) 
                    {
                      if ((DoRecursion) && (FirstCall))
                      {
                      	MDest[x+prefs->game.TAREA[l].x]
                      	     [y+prefs->game.TAREA[l].y] = Tile;
                      }
                      scoreTile++;
                      scoreOppTile--;
                    }
                  }
                
                  // dont leave a hole (at source) = only do if jump 2 spots!
                  if (GameInside(i+prefs->game.TAREA[l].x,
                                 j+prefs->game.TAREA[l].y) && (k>=6)) 
                  {
                    if (M[i+prefs->game.TAREA[l].x]
                         [j+prefs->game.TAREA[l].y] == Tile)
                      PossibleLoss++;
                  }
                
                
                }
                
                // we only suffer a possible loss if opponent tiles are around
                // therefor we check first on the presence of opponent tiles
                //in the surrounding (ring 1 & 2)
                if ((k>6) && (FirstCall))
                {
                  found=false;
                  for (l=0; l<18; l++)
                  {
                    if (GameInside(i+prefs->game.TAREA[l].x,
                                 j+prefs->game.TAREA[l].y) && (k>=6)) 
                    {
                      if (M[i+prefs->game.TAREA[l].x]
                           [j+prefs->game.TAREA[l].y] == OppTile)
                      {  
                         found=true;
                         break;
                      }
                    }
                  }
                }
                // make the hole loss even worse!!! (dont like to give away)
                PossibleLoss=(found)? PossibleLoss*2:0;


		// let's see what the opponent's reaction will be...
		Result=0;
		if ((DoRecursion) && (FirstCall)) {
		  Result = GameThink(prefs, MDest, OppTile, DoRecursion, false);
  		}
  		
                // store the score of this move
                Move.value        = scoreTile-scoreOppTile-PossibleLoss-Result;
                Move.sx           = i;
                Move.sy           = j;
                Move.dx           = x;
                Move.dy           = y;
                Move.Reproduction = (k<6);

		
		// Store the best move
                BestMove = GameGetTheBestMove(BestMove,Move,FirstCall);
              }
            }
          }
        }
      }
    }
  }

  rect.topLeft.x = 130;
  rect.topLeft.y = 16;
  rect.extent.x  = 18;
  rect.extent.y  = 12;
  GraphicsSetUpdate(rect.topLeft.y, rect.topLeft.y+rect.extent.y-1);
  GraphicsRepaint();
    
  // set the result to the opptiles movement (only done on top level)
  if (FirstCall) 
  {
    prefs->game.CPU.Coordinate.x    = BestMove.sx;
    prefs->game.CPU.Coordinate.y    = BestMove.sy;
    prefs->game.CPU.Destination.x   = BestMove.dx;
    prefs->game.CPU.Destination.y   = BestMove.dy;        
    prefs->game.CPU.IsAReproduction = BestMove.Reproduction;
  }
    
  // this is the best score the playing party is going to get
  Result = BestMove.value;

  return Result;
}

/**
 * GameGetTheBestMove... store the best move during calculations
 *
 * @param BestMove = bestmove, Move = current move, FirstCall.. is it called by cpu?
 */
static MatrixMove inline 
GameGetTheBestMove(MatrixMove BestMove, MatrixMove Move, Boolean FirstCall)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);


  if (Move.value == BestMove.value)
  {
    if ((SysRandom(0) % 2) == 0) Move.value++;  // allow for suggestion (50%)
  }
  
  switch (gbls->gameLevel)
  {
    case LEVEL_EASY:

         // choose best move, 33% chance of mess up
         if ((Move.value > BestMove.value) ||
             ((Move.value < BestMove.value) && 
              ((SysRandom(0) % 10) < 3) && (FirstCall)))
         {
           BestMove.sx           = Move.sx;
           BestMove.sy           = Move.sy;    
           BestMove.dx           = Move.dx;
           BestMove.dy           = Move.dy;
           BestMove.value        = Move.value;
           BestMove.Reproduction = Move.Reproduction;
         }
         break;

    case LEVEL_MEDIUM:

         // choose best move
         if (Move.value > BestMove.value) 
         {
           BestMove.sx           = Move.sx;
           BestMove.sy           = Move.sy;    
           BestMove.dx           = Move.dx;
           BestMove.dy           = Move.dy;
           BestMove.value        = Move.value;
           BestMove.Reproduction = Move.Reproduction;
         }
         break;

    case LEVEL_HARD:

         // choose best move
         if (Move.value > BestMove.value)
         {
           BestMove.sx           = Move.sx;
           BestMove.sy           = Move.sy;    
           BestMove.dx           = Move.dx;
           BestMove.dy           = Move.dy;
           BestMove.value        = Move.value;
           BestMove.Reproduction = Move.Reproduction;
         }
         break;


    default:
         break;
  }    

  return BestMove;
}
