/*
 * @(#)palm.c
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
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author is prohibited. 
 */

#include "palm.h"

// variables
typedef struct
{
  PreferencesType *prefs;
  Int32           evtTimeOut;
  Int16           timerDiff;
  Int16           ticksPerFrame;
  UInt32          timerPointA;
  UInt32          timerPointB;
} Globals;

// interface
static Boolean mainFormEventHandler(EventType *);
static Boolean gameFormEventHandler(EventType *);
static Boolean infoFormEventHandler(EventType *);
static Boolean dvlpFormEventHandler(EventType *);
static Boolean cfigFormEventHandler(EventType *);
static Boolean grayFormEventHandler(EventType *);
#ifdef PROTECTION_ON 
static Boolean regiFormEventHandler(EventType *); 
static Boolean rbugFormEventHandler(EventType *);
#endif
static Boolean helpFormEventHandler(EventType *);
static Boolean statFormEventHandler(EventType *);
static Boolean xmemFormEventHandler(EventType *);

/**
 * The Form:mainForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
mainFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         {
           FormType *frm = FrmGetActiveForm();

           // dynamically adjust the menu :)
           FrmSetMenu(frm,
                      DeviceSupportsGrayscale()
                        ? mainMenu_gray : mainMenu_nogray);
           FrmDrawForm(frm);

           RegisterShowMessage(globals->prefs);
         }
         processed = true;
         break;

    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case mainFormDuelButton:

                // regenerate a menu event
                MemSet(event, sizeof(EventType), 0);
                event->eType            = menuEvent;
                event->data.menu.itemID = mainMenuItemDuel; 
                EvtAddEventToQueue(event);

                SndPlaySystemSound(sndClick);
                processed = true;
                break;

           case mainFormPlayButton:

                // regenerate a menu event
                MemSet(event, sizeof(EventType), 0);
                event->eType            = menuEvent;
                event->data.menu.itemID = mainMenuItemPlay; 
                EvtAddEventToQueue(event);

                SndPlaySystemSound(sndClick);
                processed = true;
                break;

           default:
                break;
         }
         break;

    case menuEvent:

         // what menu?
         switch (event->data.menu.itemID)
         {
           case mainMenuItemPlay:

                // nag, nag, nag... (== bas  <-- hey, I want a life too :))
                if (!CHECK_SIGNATURE(globals->prefs))
                  ApplicationDisplayDialog(rbugForm);

                globals->prefs->game.playerDuel = false;
                GameResetPreferences(globals->prefs);
                FrmGotoForm(gameForm);

                processed = true;
                break;

           case mainMenuItemDuel:

                // can only play duel if registered
                if CHECK_SIGNATURE(globals->prefs) 
                {
                  globals->prefs->game.playerDuel = true;
                  GameResetPreferences(globals->prefs);

                  FrmGotoForm(gameForm);
                }
                else 
                {
                  FormType    *frm = FrmGetActiveForm();
                  ControlType *ctlDuelButton;

                  // sorry :(
                  FrmAlert(unregisteredAlert);

                  // fix that button
                  ctlDuelButton =
                    (ControlType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, mainFormDuelButton));
                  CtlSetValue(ctlDuelButton, 0);
                  CtlDrawControl(ctlDuelButton);
                }

                processed = true;
                break;

           case mainMenuItemShowStat:

                // can only play show stats if registered
                if CHECK_SIGNATURE(globals->prefs) 
                  ApplicationDisplayDialog(statForm);
                else 
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case mainMenuItemResetStat:

                // lets make sure!!
                if (FrmAlert(resetStatisticsAlert) == 0) 
                {
                  globals->prefs->game.statisticsTotal   = 0;
                  globals->prefs->game.statisticsWin     = 0;
                  globals->prefs->game.statisticsLoss    = 0;
                  globals->prefs->game.statisticsDraw    = 0;
                  globals->prefs->game.statisticsAbandon = 0;
                  globals->prefs->game.statisticsShow    = false;
                }
                processed = true;
                break;

           case mainMenuItemAutoShowStat:

                // lets make sure!!
                if (FrmAlert(autoshowStatisticsAlert) == 0) 
                  globals->prefs->game.statisticsShow = true;
                else
                  globals->prefs->game.statisticsShow = false;
                
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:gameForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
gameFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         {
           FormType *frm = FrmGetActiveForm();

           // dynamically adjust the menu :)
           FrmSetMenu(frm,
                      DeviceSupportsGrayscale()
                        ? gameMenu_gray : gameMenu_nogray);
           FrmDrawForm(frm);

           // draw seperators
           WinDrawLine(   0, 145, 159, 145);
           WinDrawLine(   0, 146, 159, 146);
         }
         processed = true;
         break;

    case penDownEvent:
    case penMoveEvent:
     
         // within the game play area?
         if (
             (globals->prefs->game.gamePlaying) &&
             (event->screenX >   0) && (event->screenX < 160) &&
             (event->screenY >  16) && (event->screenY < 144) 
            ) 
         {
           // if the player has tapped this area, they wants to move
           GameProcessStylusInput(globals->prefs, 
                                  event->screenX, event->screenY,
                                  (event->eType == penMoveEvent));

           // we have handled this event, lets continue
           processed = true;
         }

         {
           UInt32 timerDiff, timerPoint;

           // determine "delay" since last nilEvent
           timerPoint = TimGetTicks();
           timerDiff  = (timerPoint - globals->timerPointB);

           // do we need to manually enfore a nilEvent?
           if (timerDiff > globals->evtTimeOut) {
             EventType event;
             MemSet(&event, sizeof(EventType), 0);
             event.eType = nilEvent;
             EvtAddEventToQueue(&event);
           }
           else
             globals->evtTimeOut = globals->evtTimeOut - timerDiff;
         }
         break;

    case menuEvent:

         // what menu?
         switch (event->data.menu.itemID)
         {
           case gameMenuItemExit:

                if ((globals->prefs->game.GameOver) ||
                    (FrmAlert(quitGameAlert) == 0)) 
                {
                  // nag, nag, nag... (== bas)
                  if (!CHECK_SIGNATURE(globals->prefs))
                    ApplicationDisplayDialog(rbugForm);
                  else
                  {
                    // not game over - abandonment :P
                    if ((!globals->prefs->game.GameOver) && 
                        (!globals->prefs->game.playerDuel))
                      globals->prefs->game.statisticsAbandon++;

                    // fix totals
                    globals->prefs->game.statisticsTotal = 
                      globals->prefs->game.statisticsWin  +
                      globals->prefs->game.statisticsLoss +
                      globals->prefs->game.statisticsDraw +
                      globals->prefs->game.statisticsAbandon;

                    // do we have to display the statistics?
                    if ((globals->prefs->game.statisticsShow) && 
                        (!globals->prefs->game.playerDuel))
                      ApplicationDisplayDialog(statForm);
                  }
  
                  globals->prefs->game.gamePlaying = false;
                  globals->prefs->game.GameOver    = false;
                  FrmGotoForm(mainForm);
                }
 
                processed = true;
                break;

           default:
                break;
         }
         break;

    case ctlSelectEvent:
  
         switch (event->data.ctlSelect.controlID)
         {
           case gameFormSoundButton:
  
                // regenerate menu event
                MemSet(event, sizeof(EventType), 0);
                event->eType            = menuEvent;
                event->data.menu.itemID = menuItemConfig;
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    case nilEvent:

         // make sure the active window is the form 
         if (WinGetActiveWindow() == (WinHandle)FrmGetActiveForm()) 
         {
           globals->timerPointA = TimGetTicks();

           // play the game!
           GameProcessKeyInput(globals->prefs, KeyCurrentState());
           GameMovement(globals->prefs);

           // draw the game!
           GameDraw(globals->prefs);

           globals->timerPointB = TimGetTicks();

           // calculate the delay required
           globals->timerDiff = (globals->timerPointB - globals->timerPointA);
           globals->evtTimeOut = 
             (globals->timerDiff > globals->ticksPerFrame) 
               ? 0 
               : (globals->ticksPerFrame - globals->timerDiff);
         }
         processed = true;
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:cfigForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
cfigFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType    *frm;
           UInt16      index;
           ControlType *muteCheckbox, *volumeControl;
           ListType    *lstHard1, *lstHard2, *lstHard3, *lstHard4;
           ListType    *lstUp, *lstDown, *lstLevel, *lstBoard, *lstStyle;
           ControlType *ctlHard1, *ctlHard2, *ctlHard3, *ctlHard4;
           ControlType *ctlUp, *ctlDown, *ctlLevel, *ctlBoard, *ctlStyle;
           UInt16      *choices[] = { 
                                      &(globals->prefs->config.ctlKeyLeft), 
                                      &(globals->prefs->config.ctlKeyRight), 
                                      &(globals->prefs->config.ctlKeyUp), 
                                      &(globals->prefs->config.ctlKeyDown), 
                                      &(globals->prefs->config.ctlKeySelect)
                                    };

           frm = FrmGetActiveForm();
           ctlHard1 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey1Trigger));
           ctlHard2 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey2Trigger));
           ctlHard3 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey3Trigger));
           ctlHard4 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey4Trigger));
           ctlUp =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageUpTrigger));
           ctlDown =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageDownTrigger));
           ctlLevel =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormLevelTrigger));
           ctlBoard =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormBoardTrigger));
           ctlStyle =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStyleTrigger));

           lstHard1 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey1List));
           lstHard2 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey2List));
           lstHard3 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey3List));
           lstHard4 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey4List));
           lstUp =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageUpList));
           lstDown =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageDownList));

           LstSetSelection(lstHard1, 0);
           CtlSetLabel(ctlHard1, LstGetSelectionText(lstHard1, 0));
           LstSetSelection(lstHard2, 0);
           CtlSetLabel(ctlHard2, LstGetSelectionText(lstHard2, 0));
           LstSetSelection(lstHard3, 0);
           CtlSetLabel(ctlHard3, LstGetSelectionText(lstHard3, 0));
           LstSetSelection(lstHard4, 0);
           CtlSetLabel(ctlHard4, LstGetSelectionText(lstHard4, 0));
           LstSetSelection(lstDown, 0);
           CtlSetLabel(ctlDown, LstGetSelectionText(lstDown, 0));
           LstSetSelection(lstUp, 0);
           CtlSetLabel(ctlUp, LstGetSelectionText(lstUp, 0));
                  
           // show the "current" settings
           for (index=0; index<5; index++) 
           {
             if ((*(choices[index]) & keyBitHard1) != 0) 
             { 
               LstSetSelection(lstHard1, index+1);
               CtlSetLabel(ctlHard1, LstGetSelectionText(lstHard1, index+1));
             }  

             if ((*(choices[index]) & keyBitHard2) != 0)  
             { 
               LstSetSelection(lstHard2, index+1);
               CtlSetLabel(ctlHard2, LstGetSelectionText(lstHard2, index+1));
             }  

             if ((*(choices[index]) & keyBitHard3) != 0)  
             { 
               LstSetSelection(lstHard3, index+1);
               CtlSetLabel(ctlHard3, LstGetSelectionText(lstHard3, index+1));
             }  

             if ((*(choices[index]) & keyBitHard4) != 0)  
             { 
               LstSetSelection(lstHard4, index+1);
               CtlSetLabel(ctlHard4, LstGetSelectionText(lstHard4, index+1));
             }  

             if ((*(choices[index]) & keyBitPageUp) != 0)  
             { 
               LstSetSelection(lstUp, index+1);
               CtlSetLabel(ctlUp, LstGetSelectionText(lstUp, index+1));
             }  

             if ((*(choices[index]) & keyBitPageDown) != 0)  
             { 
               LstSetSelection(lstDown, index+1);
               CtlSetLabel(ctlDown, LstGetSelectionText(lstDown, index+1));
             }  
           } 

           // set the current settings for level & board          
           lstLevel =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormLevelList));
           lstBoard =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormBoardList));
           lstStyle =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStyleList));

           LstSetSelection(lstLevel, globals->prefs->game.gameLevel);
           CtlSetLabel(ctlLevel, 
             LstGetSelectionText(lstLevel, globals->prefs->game.gameLevel));
           LstSetSelection(lstBoard, globals->prefs->game.gameBoard);
           CtlSetLabel(ctlBoard, 
             LstGetSelectionText(lstBoard, globals->prefs->game.gameBoard));
           LstSetSelection(lstStyle, globals->prefs->game.gameStyle);
           CtlSetLabel(ctlStyle, 
             LstGetSelectionText(lstStyle, globals->prefs->game.gameStyle));

           // configure sounds settings
           muteCheckbox =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormMuteCheckbox));
           volumeControl =
             (ControlType *)FrmGetObjectPtr(frm, 
               FrmGetObjectIndex(frm, cfigFormSound0Button+DeviceGetVolume()));

           CtlSetValue(muteCheckbox, DeviceGetMute() ? 1 : 0); 
           CtlSetValue(volumeControl, 1);

           // save this
           FtrSet(appCreator, ftrGlobalsCfgActiveVol, (UInt32)volumeControl);
         }
         processed = true;
         break;

    case ctlEnterEvent:

         switch (event->data.ctlEnter.controlID)
         {
           case cfigFormSound0Button:
           case cfigFormSound1Button:
           case cfigFormSound2Button:
           case cfigFormSound3Button:
           case cfigFormSound4Button:
           case cfigFormSound5Button:
           case cfigFormSound6Button:
           case cfigFormSound7Button:
                {
                  ControlType *newCtl, *oldCtl;

                  newCtl = event->data.ctlEnter.pControl;

                  // we dont want an audible beep from the system
                  FtrGet(appCreator, ftrGlobalsCfgActiveVol, (UInt32 *)&oldCtl);
                  CtlSetValue(oldCtl, 0);
                  CtlSetValue(newCtl, 1);
                  FtrSet(appCreator, ftrGlobalsCfgActiveVol, (UInt32)newCtl);

                  // act as we needed :)
                  DeviceSetVolume( 
                     (event->data.ctlEnter.controlID - cfigFormSound0Button));
                  {
                    SndCommandType testBeep = 
                      {sndCmdFreqDurationAmp,0,512,32,sndMaxAmp};
                    DevicePlaySound(&testBeep);
                  }
                }
                processed = true;
                break;

           default:
                break;
         } 
         break;

    case popSelectEvent:

         switch (event->data.popSelect.listID)
         {
           case cfigFormLevelList:
                {
                  Boolean allow;

                  allow = true;

                  // RULE #1 - cannot be playing a game :)
                  if (globals->prefs->game.gamePlaying)
                  {
                    FrmAlert(gameActiveAlert);
                    allow = false;
                  }

                  // revert to the old selection if needed :)
                  if (!allow)
                  {
                    LstSetSelection(event->data.popSelect.listP, 
                                    event->data.popSelect.priorSelection);
                    CtlSetLabel(event->data.popSelect.controlP, 
                      LstGetSelectionText(event->data.popSelect.listP, 
                                       event->data.popSelect.priorSelection));
                    processed = true;
                  }
                  else
                    processed = false;
                }
                break;

           case cfigFormBoardList:
                {
                  Boolean allow;

                  // RULE #1 - must be registered in order to select others
                  allow = CHECK_SIGNATURE(globals->prefs);
                  if (!allow) FrmAlert(unregisteredAlert);

                  // RULE #2 - cannot be playing a game :)
                  if ((globals->prefs->game.gamePlaying) && (allow))
                  {
                    FrmAlert(gameActiveAlert);
                    allow = false;
                  }

                  // revert to the old selection if needed :)
                  if (!allow)
                  {
                    LstSetSelection(event->data.popSelect.listP, 
                                    event->data.popSelect.priorSelection);
                    CtlSetLabel(event->data.popSelect.controlP, 
                      LstGetSelectionText(event->data.popSelect.listP, 
                                       event->data.popSelect.priorSelection));
                    processed = true;
                  }
                  else
                    processed = false;
                }
                break;

           case cfigFormStyleList:
                {
                  Boolean allow;

                  // RULE #1 - must be registered in order to select others
                  allow = CHECK_SIGNATURE(globals->prefs);
                  if (!allow) FrmAlert(unregisteredAlert);

                  // RULE #2 - cannot be playing a game :)
                  if ((globals->prefs->game.gamePlaying) && (allow))
                  {
                    FrmAlert(gameActiveAlert);
                    allow = false;
                  }

                  // revert to the old selection if needed :)
                  if (!allow)
                  {
                    LstSetSelection(event->data.popSelect.listP, 
                                    event->data.popSelect.priorSelection);
                    CtlSetLabel(event->data.popSelect.controlP, 
                      LstGetSelectionText(event->data.popSelect.listP, 
                                       event->data.popSelect.priorSelection));
                    processed = true;
                  }
                  else
                    processed = false;
                }
                break;

           default:
                break;
         }
         break;

    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case cfigFormMuteCheckbox:
                DeviceSetMute(CtlGetValue(event->data.ctlEnter.pControl) != 0);
                processed = true;
                break;

           case cfigFormOkButton:
                {
                  ListType *lstHard1, *lstHard2, *lstHard3, *lstHard4;
                  ListType *lstUp, *lstDown, *lstBoard, *lstLevel, *lstStyle;
                  UInt16   index;
                  FormType *frm;
                  UInt16   keySignature;
                  UInt16   *choices[] = { 
                                         &(globals->prefs->config.ctlKeyLeft), 
                                         &(globals->prefs->config.ctlKeyRight), 
                                         &(globals->prefs->config.ctlKeyUp), 
                                         &(globals->prefs->config.ctlKeyDown), 
                                         &(globals->prefs->config.ctlKeySelect)
                                        };
                                  

                  frm = FrmGetActiveForm();
                  lstHard1 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey1List));
                  lstHard2 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey2List));
                  lstHard3 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey3List));
                  lstHard4 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey4List));
                  lstUp = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormPageUpList));
                  lstDown = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormPageDownList));

                  keySignature = 
                    (
                      (0x01 << LstGetSelection(lstHard1)) |
                      (0x01 << LstGetSelection(lstHard2)) |
                      (0x01 << LstGetSelection(lstHard3)) |
                      (0x01 << LstGetSelection(lstHard4)) |
                      (0x01 << LstGetSelection(lstUp))    |
                      (0x01 << LstGetSelection(lstDown))
                    );
                  keySignature = keySignature >> 1;  // ignore the '------'

                  // only process if good setting is selected.
                  if (keySignature == 0x1F) 
                  {
                    // set level & board settings
                    lstLevel = 
                      (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormLevelList));
                    lstBoard = 
                      (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormBoardList));
                    lstStyle = 
                      (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormStyleList));

                    globals->prefs->game.gameBoard = LstGetSelection(lstBoard);
                    globals->prefs->game.gameStyle = LstGetSelection(lstStyle);
                    globals->prefs->game.gameLevel = LstGetSelection(lstLevel);

                    // sounds preferences
                    globals->prefs->config.sndMute   = DeviceGetMute();
                    globals->prefs->config.sndVolume = DeviceGetVolume();

                    // key preferences
                    for (index=0; index<5; index++) 
                    {
                      *(choices[index]) = 0;
                    }
                    if (LstGetSelection(lstHard1) != 0) 
                      *(choices[LstGetSelection(lstHard1)-1]) |= keyBitHard1;
                    if (LstGetSelection(lstHard2) != 0) 
                      *(choices[LstGetSelection(lstHard2)-1]) |= keyBitHard2;
                    if (LstGetSelection(lstHard3) != 0) 
                      *(choices[LstGetSelection(lstHard3)-1]) |= keyBitHard3;
                    if (LstGetSelection(lstHard4) != 0) 
                      *(choices[LstGetSelection(lstHard4)-1]) |= keyBitHard4;
                    if (LstGetSelection(lstUp) != 0) 
                      *(choices[LstGetSelection(lstUp)-1])    |= keyBitPageUp;
                    if (LstGetSelection(lstDown) != 0) 
                      *(choices[LstGetSelection(lstDown)-1])  |= keyBitPageDown;

                    // send a close event
                    MemSet(event, sizeof(EventType), 0);
                    event->eType = frmCloseEvent;
                    event->data.frmClose.formID = FrmGetActiveFormID();
                    EvtAddEventToQueue(event);
                  }
                  else 
                  {
                    SndPlaySystemSound(sndError);
                  }
                }
                processed = true;
                break;

           case cfigFormCancelButton:

                // revert changes
                DeviceSetMute(globals->prefs->config.sndMute);
                DeviceSetVolume(globals->prefs->config.sndVolume);

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:

         // clean up
         FtrUnregister(appCreator, ftrGlobalsCfgActiveVol);

         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:grayForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean
grayFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType)
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType    *frm;
           ControlType *ctlWhite, *ctlLGray, *ctlDGray, *ctlBlack;

           frm = FrmGetActiveForm();
           ctlWhite =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, grayWhite1Button));
           ctlLGray =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm,
                 grayLightGray1Button + globals->prefs->config.lgray));
           ctlDGray =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm,
                 grayDarkGray1Button + globals->prefs->config.dgray));
           ctlBlack =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, grayBlack7Button));

           CtlSetValue(ctlWhite, 1); CtlDrawControl(ctlWhite);
           CtlSetValue(ctlLGray, 1); CtlDrawControl(ctlLGray);
           CtlSetValue(ctlDGray, 1); CtlDrawControl(ctlDGray);
           CtlSetValue(ctlBlack, 1); CtlDrawControl(ctlBlack);
         }

         // pre 3.5 - we must 'redraw' form to actually display PUSHBUTTONS
         if (!DeviceSupportsVersion(romVersion3_5)) 
         {
           FrmDrawForm(FrmGetActiveForm());
         }

         processed = true;
         break;

    case ctlEnterEvent:

         switch (event->data.ctlEnter.controlID) 
         {
           case grayLightGray1Button:
           case grayLightGray7Button:
           case grayDarkGray1Button:
           case grayDarkGray7Button:

           // stupid user, they must select one of the other options
           SndPlaySystemSound(sndError);
           processed = true;
         }
         break;

    case ctlSelectEvent:
         
         switch (event->data.ctlSelect.controlID) 
         {
           case grayLightGray2Button:
           case grayLightGray3Button:
           case grayLightGray4Button:
           case grayLightGray5Button:
           case grayLightGray6Button:

                globals->prefs->config.lgray = event->data.ctlEnter.controlID -
                                               grayLightGray1Button;
                DeviceGrayscale(graySet, 
                                &globals->prefs->config.lgray, 
                                &globals->prefs->config.dgray);
                processed = true;
                break;

           case grayDarkGray2Button:
           case grayDarkGray3Button:
           case grayDarkGray4Button:
           case grayDarkGray5Button:
           case grayDarkGray6Button:

                globals->prefs->config.dgray = event->data.ctlEnter.controlID -
                                               grayDarkGray1Button;
                DeviceGrayscale(graySet, 
                                &globals->prefs->config.lgray, 
                                &globals->prefs->config.dgray);
                processed = true;
                break;

           case grayFormOkButton:
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;
               
           default:
                break;
         } 
         break;

    default:
         break;
  }

  return processed;
} 

/**
 * The Form:infoForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
infoFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case infoFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:dvlpForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
dvlpFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case dvlpFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

#ifdef PROTECTION_ON 
/**
 * The Form:regiForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
regiFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());

         if (CHECK_SIGNATURE(globals->prefs))
           FrmSetTitle(FrmGetActiveForm(), "Registered - Thank you!");

         // display the HotSync username on the screen (in HEX)
         {
           Char  ID[40];
           Char  tmp[10], num[3];
           Coord x;
           UInt8 i, checksum;

           FontID font = FntGetFont();

           // initialize
           StrCopy(ID, "");

           // generate strings
           checksum = 0;
           for (i=0; i<MAX_IDLENGTH; i++) 
           {
             checksum ^= (UInt8)globals->prefs->system.hotSyncUsername[i];
             StrIToH(tmp, (UInt8)globals->prefs->system.hotSyncUsername[i]);
             StrNCopy(num, &tmp[StrLen(tmp)-2], 2); num[2] = '\0';
             StrCat(ID, num); StrCat(ID, ":");
           }
           StrIToH(tmp, checksum);
           StrNCopy(num, &tmp[StrLen(tmp)-2], 2); num[2] = '\0';
           StrCat(ID, num);

           FntSetFont(boldFont);
           x = (160 - FntCharsWidth(ID, StrLen(ID))) >> 1;
           WinDrawChars(ID, StrLen(ID), x, 68);
           FntSetFont(font);
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case regiFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:rbugForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
rbugFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case rbugFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}
#endif

/**
 * The Form:statForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
statFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;
  
  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType  *frm;
           FieldType *fldTotal, *fldWin, *fldLoss, *fldDraw, *fldAbandon;
           MemHandle memHandle;
           UInt8     winpercentage, losspercentage, drawpercentage;
           UInt8     bukpercentage, iComment, flag, max;
           Char      Comment[35];
           Char *comment_win_str[]= {"You play better than expected...",
                                     "Excellent job!",
                                     "Luck must be on your side.",
                                     "Hmmm.....",
                                     "You are not cheating, heh?",
                                     "Who did you bribe?",
                                     "Obvious you're too strong for me!"};
           Char *comment_loss_str[]={"Your mom plays better than you!",
                                     "I like the way you play.",
                                     "Better luck next time!",
                                     "You like losing, heh?",
                                     "I RULE!",
                                     "Try again!",
                                     "Come on... you can beat me!"};
           Char *comment_draw_str[]={"Another draw... *sigh*",
                                     "Bet I beat you next time!",
                                     "Argh... another draw.",
                                     "Is this all you can do?",
                                     "I guess you learned.",
                                     "Try again!",
                                     "Come on... you can beat me!"};
                                    
           frm = FrmGetActiveForm();

           fldTotal = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormTotalField));
           fldWin = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormWinField));
           fldLoss = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormLossField));
           fldDraw = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormDrawField));
           fldAbandon = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormAbandonField));
           
           // total
           memHandle = MemHandleNew(16);
           if (globals->prefs->game.statisticsTotal > 999)
             StrPrintF(MemHandleLock(memHandle), "XXX\t100\t%%");
           else
             StrPrintF(MemHandleLock(memHandle), "%d\t100\t%%",
                       globals->prefs->game.statisticsTotal);
           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldTotal, memHandle);
           FldDrawField(fldTotal);

           // calculate percentages.. 
           if (globals->prefs->game.statisticsTotal > 0) 
           {
             winpercentage  = (globals->prefs->game.statisticsWin * 100) /
                               globals->prefs->game.statisticsTotal;
             losspercentage = (globals->prefs->game.statisticsLoss * 100) /
                               globals->prefs->game.statisticsTotal;
             drawpercentage = (globals->prefs->game.statisticsDraw * 100) / 
                               globals->prefs->game.statisticsTotal;
             bukpercentage  = (globals->prefs->game.statisticsAbandon * 100) / 
                               globals->prefs->game.statisticsTotal;

             // ensure they get rounded to 100%
             if ((winpercentage  + losspercentage +
                  drawpercentage + bukpercentage) != 100)
             {
               iComment = 100 - (winpercentage  + losspercentage +
                                 drawpercentage + bukpercentage);
              
               max = winpercentage;      flag = 0; 
               if (losspercentage > max) flag = 1;
               if (drawpercentage > max) flag = 2;
               if (bukpercentage > max)  flag = 3;

               switch (flag)
               {
                 case 0:  winpercentage  += iComment; break;
                 case 1:  losspercentage += iComment; break;
                 case 2:  drawpercentage += iComment; break;
                 case 3:  bukpercentage  += iComment; break;
                 default: break;
               }
             }
           } 
           else
           {
             winpercentage  = 0;
             losspercentage = 0;
             drawpercentage = 0;
             bukpercentage  = 0;
           }   

           // win
           memHandle = MemHandleNew(35);
           if (globals->prefs->game.statisticsWin > 999)
             StrPrintF(MemHandleLock(memHandle), "XXX\t%d\t%%",
                       winpercentage);
           else
             StrPrintF(MemHandleLock(memHandle), "%d\t%d\t%%",
                       globals->prefs->game.statisticsWin, winpercentage);
           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldWin, memHandle);
           FldDrawField(fldWin);

           // loss
           memHandle = MemHandleNew(35);
           if (globals->prefs->game.statisticsLoss > 999)
             StrPrintF(MemHandleLock(memHandle), "XXX\t%d\t%%",
                       losspercentage);
           else
             StrPrintF(MemHandleLock(memHandle), "%d\t%d\t%%",
                       globals->prefs->game.statisticsLoss, losspercentage);
           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldLoss, memHandle);
           FldDrawField(fldLoss);

           // draw
           memHandle = MemHandleNew(35);
           if (globals->prefs->game.statisticsDraw > 999)
             StrPrintF(MemHandleLock(memHandle), "XXX\t%d\t%%",
                       drawpercentage);
           else
             StrPrintF(MemHandleLock(memHandle), "%d\t%d\t%%",
                       globals->prefs->game.statisticsDraw, drawpercentage);
           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldDraw, memHandle);
           FldDrawField(fldDraw);

           // abandoned
           memHandle = MemHandleNew(35);
           if (globals->prefs->game.statisticsAbandon > 999)
             StrPrintF(MemHandleLock(memHandle), "XXX\t%d\t%%",
                       bukpercentage);
           else
             StrPrintF(MemHandleLock(memHandle), "%d\t%d\t%%",
                       globals->prefs->game.statisticsAbandon, bukpercentage);
           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldAbandon, memHandle);
           FldDrawField(fldAbandon);

           // comment
           iComment = SysRandom(0)%6;
           
           // get the right comment
           if ((globals->prefs->game.statisticsTotal - 
                globals->prefs->game.statisticsAbandon) > 0) 
           {
             if (winpercentage > losspercentage) 
             {
                if (winpercentage > drawpercentage)
                  StrPrintF(Comment, "%s", comment_win_str[iComment]);
                else
                  StrPrintF(Comment, "%s", comment_draw_str[iComment]);
             }
             else
             { 
                if (losspercentage > drawpercentage)
                  StrPrintF(Comment, "%s", comment_loss_str[iComment]);
                else
                  StrPrintF(Comment, "%s", comment_draw_str[iComment]);
             }           
           }
           else
             StrPrintF(Comment, "We haven't started yet!");
           
           // draw that little line
           WinDrawLine(80,  98, 128,  98);
           WinDrawLine(80, 100, 128, 100);

           // print the comment
           WinDrawChars(Comment, StrLen(Comment), 
                     80 - (FntCharsWidth(Comment, StrLen(Comment)) >> 1), 120);
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case statFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:
         {
           FormType  *frm;
           FieldType *fldTotal, *fldWin, *fldLoss, *fldDraw;
           MemHandle memHandle;

           frm = FrmGetActiveForm();

           fldTotal = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormTotalField));
           fldWin = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormWinField));
           fldLoss = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormLossField));
           fldDraw = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, statFormDrawField));
     
           // total
           memHandle = FldGetTextHandle(fldTotal);
           FldSetTextHandle(fldTotal, NULL);
           MemHandleFree(memHandle);

           // win 
           memHandle = FldGetTextHandle(fldWin);
           FldSetTextHandle(fldWin, NULL);
           MemHandleFree(memHandle);

           // loss 
           memHandle = FldGetTextHandle(fldLoss);
           FldSetTextHandle(fldLoss, NULL);
           MemHandleFree(memHandle);

           // draw 
           memHandle = FldGetTextHandle(fldDraw);
           FldSetTextHandle(fldDraw, NULL);
           MemHandleFree(memHandle);

         }
         break;

    default:
         break;
  }

  return processed;
}


/**
 * The Form:helpForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
helpFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType      *frm;
           ScrollBarType *sclBar;
           UInt16        val, min, max, pge;

           frm    = FrmGetActiveForm();
           sclBar =  
             (ScrollBarType *)FrmGetObjectPtr(frm, 
               FrmGetObjectIndex(frm, helpFormScrollBar)); 

           SclGetScrollBar(sclBar, &val, &min, &max, &pge);
           val = InitInstructions();
           max = (val > pge) ? (val-(pge+16)) : 0;
           SclSetScrollBar(sclBar, 0, min, max, pge);

           DrawInstructions(0);
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case helpFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    case sclRepeatEvent:
         {
           FormType      *frm;
           ScrollBarType *sclBar;
           UInt16        val, min, max, pge;

           frm = FrmGetActiveForm();
           sclBar = 
             (ScrollBarType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, helpFormScrollBar));

           SclGetScrollBar(sclBar, &val, &min, &max, &pge);
           DrawInstructions(val);
         }
         break;

    case keyDownEvent:

         switch (event->data.keyDown.chr)
         {
           case pageUpChr:
                {
                  FormType      *frm;
                  ScrollBarType *sclBar;
                  UInt16        val, min, max, pge;

                  frm = FrmGetActiveForm();
                  sclBar = 
                    (ScrollBarType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, helpFormScrollBar));

                  SclGetScrollBar(sclBar, &val, &min, &max, &pge);
                  val = (pge > val) ? 0 : (val-pge); 
                  SclSetScrollBar(sclBar, val, min, max, pge);
                  DrawInstructions(val);
                }
                processed = true;
                break;

           case pageDownChr:
                {
                  FormType      *frm;
                  ScrollBarType *sclBar;
                  UInt16        val, min, max, pge;

                  frm = FrmGetActiveForm();
                  sclBar = 
                    (ScrollBarType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, helpFormScrollBar));

                  SclGetScrollBar(sclBar, &val, &min, &max, &pge);
                  val = (max < (val+pge)) ? max : (val+pge); 
                  SclSetScrollBar(sclBar, val, min, max, pge);
                  DrawInstructions(val);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:

         // clean up
         QuitInstructions();
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:xmemForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
xmemFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case xmemFormOkButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Palm Computing Platform initialization routine.
 */
void  
InitApplication()
{
  Globals *globals;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  // load preferences
  {
    Boolean reset;
    UInt16  prefSize;
    Int16   flag;

    // allocate memory for preferences
    globals->prefs = (PreferencesType *)MemPtrNew(sizeof(PreferencesType));
    MemSet(globals->prefs, sizeof(PreferencesType), 0);
    reset          = true;

    // lets see how large the preference is (if it is there)
    prefSize = 0;
    flag     = PrefGetAppPreferences(appCreator, 0, NULL, &prefSize, true);

    // we have some preferences, maybe a match :)
    if ((flag != noPreferenceFound) && (prefSize == sizeof(PreferencesType))) 
    {
      // extract all the bytes
      PrefGetAppPreferences(appCreator, 0, globals->prefs, &prefSize, true);

      // decrypt the memory chunk (based on the @message)
      RegisterDecryptChunk((UInt8 *)globals->prefs, prefSize, 0x0007, 0x01);

      // decryption screw up? reset
      reset = !CHECK_SIGNATURE(globals->prefs) || 
              (globals->prefs->system.signatureVersion != VERSION);
    }

    // do we need to reset the preferences?
    if (reset) 
    {
      // set default values
      MemSet(globals->prefs, prefSize, 0);

      globals->prefs->system.signatureVersion = VERSION;
      StrCopy(globals->prefs->system.signature, "|HaCkMe|");

      globals->prefs->config.ctlKeySelect    = keyBitHard1 | keyBitHard4;
      globals->prefs->config.ctlKeyUp        = keyBitPageUp;
      globals->prefs->config.ctlKeyDown      = keyBitPageDown;
      globals->prefs->config.ctlKeyLeft      = keyBitHard2;
      globals->prefs->config.ctlKeyRight     = keyBitHard3;

      // set the default board and the default level
      globals->prefs->game.gameBoard         = LAYOUT_STANDARD;
      globals->prefs->game.gameStyle         = BOARD_SWEDISH;
      globals->prefs->game.gameLevel         = LEVEL_MEDIUM;
      globals->prefs->game.playerStarting    = true;
      globals->prefs->game.playerDuel        = false;

      globals->prefs->config.sndMute         = false;
      globals->prefs->config.sndVolume       = 7;

      globals->prefs->game.gamePlaying       = false;

      // reset the statistics
      globals->prefs->game.statisticsTotal   = 0;
      globals->prefs->game.statisticsWin     = 0;
      globals->prefs->game.statisticsLoss    = 0;
      globals->prefs->game.statisticsDraw    = 0;
      globals->prefs->game.statisticsAbandon = 0;
      globals->prefs->game.statisticsShow    = false;
  
      if (DeviceSupportsGrayscale()) 
      {
        DeviceGrayscale(grayGet,
                        &globals->prefs->config.lgray,
                        &globals->prefs->config.dgray);
      }
    }
  }

  // get the HotSync user name
  globals->prefs->system.hotSyncUsername = 
    (Char *)MemPtrNew(dlkUserNameBufSize * sizeof(Char));
  MemSet(globals->prefs->system.hotSyncUsername, dlkUserNameBufSize, 0);
  DlkGetSyncInfo(NULL,NULL,NULL,
                 globals->prefs->system.hotSyncUsername,NULL,NULL);
  {
    Char *ptrStr;

    ptrStr = StrChr(globals->prefs->system.hotSyncUsername, spaceChr);
    if (ptrStr != NULL) 
    { 
      // erase everything after the FIRST space
      UInt8 index = ((UInt32)ptrStr - 
                     (UInt32)globals->prefs->system.hotSyncUsername);
      MemSet(ptrStr, dlkUserNameBufSize - index, 0);
    }

    ptrStr = StrChr(globals->prefs->system.hotSyncUsername, '\0');
    if (ptrStr != NULL) 
    { 
      // erase everything after the FIRST null char
      UInt8 index = ((UInt32)ptrStr - 
                     (UInt32)globals->prefs->system.hotSyncUsername);
      MemSet(ptrStr, dlkUserNameBufSize - index, 0);
    }
  }
  
  // configure grayscale registers
  if (DeviceSupportsGrayscale()) 
  {
    DeviceGrayscale(graySet,
                    &globals->prefs->config.lgray,
                    &globals->prefs->config.dgray);
  }

  // setup sound config
  DeviceSetMute(globals->prefs->config.sndMute);
  DeviceSetVolume(globals->prefs->config.sndVolume);

  // setup the valid keys available while the game is running
  KeySetMask(~(keyBitsAll ^ (keyBitPower   | keyBitCradle | 
                             keyBitAntenna | keyBitContrast)));

  // initialize the game environemnt
  RegisterInitialize(globals->prefs);

  globals->evtTimeOut    = evtWaitForever;
  globals->ticksPerFrame = SysTicksPerSecond() / GAME_FPS;

  // goto the appropriate form
  if ((globals->prefs->game.gamePlaying) && (!globals->prefs->game.GameOver)) 
    FrmGotoForm(gameForm);
  else 
  {
    globals->prefs->game.gamePlaying = false;
    FrmGotoForm(mainForm);
  }
}

/**
 * The Palm Computing Platform entry routine (mainline).
 *
 * @param cmd         a word value specifying the launch code.
 * @param cmdPBP      pointer to a structure associated with the launch code.
 * @param launchFlags additional launch flags.
 * @return zero if launch successful, non zero otherwise.
 */
UInt32  
PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};
  const RectangleType     rect  = {{0,0},{160,160}};
  Globals *globals;
  Boolean  ok;
  UInt32   result = 0;


  // what type of launch was this?
  switch (cmd) 
  {
    case sysAppLaunchCmdNormalLaunch:
         {
           // is this device compatable?
           if (DeviceCheckCompatability()) 
           {
             // initialize device
             DeviceInitialize();

             // create a globals object, and register it
             globals = (Globals *)MemPtrNew(sizeof(Globals));
             MemSet(globals, sizeof(Globals), 0);
             FtrSet(appCreator, ftrGlobals, (UInt32)globals);

             // show loading screen
             WinSetPattern(&erase);
             WinFillRectangle(&rect,0);
             {
               FontID font = FntGetFont();

               MemHandle bitmapHandle = DmGet1Resource('Tbmp', bitmapLogo);
               WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 16);
               MemHandleUnlock(bitmapHandle);
               DmReleaseResource(bitmapHandle);

               FntSetFont(largeFont);
               WinDrawChars("L O A D I N G", 13, 44, 124);
               FntSetFont(boldFont);
               WinDrawChars("... please wait ....", 20, 40, 140);

               FntSetFont(font);
             }

             ok = (GraphicsInitialize() && GameInitialize());
             if (ok)
             {
               InitApplication();

               WinSetPattern(&erase);
               WinFillRectangle(&rect,0);

               // run the application :))
               EventLoop();
               EndApplication();
             }

             // terminate the graphics and game engine
             GraphicsTerminate();
             GameTerminate();

             // must tell user no memory left :(
             if (!ok)
               ApplicationDisplayDialog(xmemForm);

             // unregister the feature
             MemPtrFree(globals);
             FtrUnregister(appCreator, ftrGlobals);

             // restore device state
             DeviceTerminate();
           }
         }
         break;

    default:
         break;
  }

  return result;
}

/**
 * The application event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
Boolean 
ApplicationHandleEvent(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get a globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType)
  {
    case frmLoadEvent:
         {
           UInt16   formID = event->data.frmLoad.formID;
           FormType *frm   = FrmInitForm(formID);

           FrmSetActiveForm(frm);
           switch (formID) 
           {
             case mainForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)mainFormEventHandler);

                  processed = true;
                  break;

             case gameForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)gameFormEventHandler);

                  processed = true;
                  break;

             case infoForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)infoFormEventHandler);

                  processed = true;
                  break;

             case dvlpForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)dvlpFormEventHandler);

                  processed = true;
                  break;

             case cfigForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)cfigFormEventHandler);

                  processed = true;
                  break;

             case grayForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)grayFormEventHandler);

                  processed = true;
                  break;

#ifdef PROTECTION_ON 
             case regiForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)regiFormEventHandler);

                  processed = true;
                  break;

             case rbugForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)rbugFormEventHandler);

                  processed = true;
                  break;
#endif
             case statForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)statFormEventHandler);

                  processed = true;
                  break;
             
             case helpForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)helpFormEventHandler);

                  processed = true;
                  break;

             case xmemForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)xmemFormEventHandler);

                  processed = true;
                  break;

             default:
                  break;
           }
         }
         break;

    case winEnterEvent:
         {
           if (event->data.winEnter.enterWindow ==
                (WinHandle)FrmGetFormPtr(gameForm)) 
           {
             // when game screen is active, animate
             globals->evtTimeOut = 0;
             processed           = true;
           }
         }
         break;

    case winExitEvent:
         {
           if (event->data.winExit.exitWindow ==
                (WinHandle)FrmGetFormPtr(gameForm)) 
           {
             // when game screen is not active, stop animation
             globals->evtTimeOut = evtWaitForever;
             processed           = true;
           }
         }
         break;
         
    case menuEvent:

         switch (event->data.menu.itemID) 
         {
           case menuItemGrayscale:
                ApplicationDisplayDialog(grayForm);
                processed = true;
                break;

           case menuItemConfig:
                ApplicationDisplayDialog(cfigForm);
                processed = true;
                break;

#ifdef PROTECTION_ON 
           case menuItemRegister:
                ApplicationDisplayDialog(regiForm);
                RegisterShowMessage(globals->prefs);
                processed = true;
                break;
#endif
           case menuItemHelp:
                ApplicationDisplayDialog(helpForm);
                processed = true;
                break;

           case menuItemDeveloper:
                ApplicationDisplayDialog(dvlpForm);
                processed = true;
                break;

           case menuItemAbout:
                ApplicationDisplayDialog(infoForm);
                processed = true;
                break;

           default:
                break;
         }
         break;

    case ctlSelectEvent:
  
         switch (event->data.ctlSelect.controlID)
         {
           case globalFormHelpButton:
  
                // regenerate menu event
                MemSet(event, sizeof(EventType), 0);
                event->eType            = menuEvent;
                event->data.menu.itemID = menuItemHelp;
                EvtAddEventToQueue(event);

                processed = true;
                break;

           case globalFormAboutButton:

                // regenerate menu event
                MemSet(event, sizeof(EventType), 0);
                event->eType            = menuEvent;
                event->data.menu.itemID = menuItemAbout;
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;
         
    default:
         break;
  }

  return processed;
}

/**
 * Display a MODAL dialog to the user (this is a modified FrmDoDialog)
 *
 * @param formID the ID of the form to display.
 */
void
ApplicationDisplayDialog(UInt16 formID)
{
  Globals             *globals;
  FormActiveStateType frmCurrState;
  FormType            *frmActive      = NULL;
  WinHandle           winDrawWindow   = NULL;
  WinHandle           winActiveWindow = NULL;

  // get a globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  // save the active form/window
  if (DeviceSupportsVersion(romVersion3)) 
  {
    FrmSaveActiveState(&frmCurrState);
  }
  else 
  {
    frmActive       = FrmGetActiveForm();
    winDrawWindow   = WinGetDrawWindow();
    winActiveWindow = WinGetActiveWindow();  // < palmos3.0, manual work
  }

  {
    EventType event;
    UInt16    err;
    Boolean   keepFormOpen;

    MemSet(&event, sizeof(EventType), 0);

    // send a load form event
    event.eType = frmLoadEvent;
    event.data.frmLoad.formID = formID;
    EvtAddEventToQueue(&event);

    // send a open form event
    event.eType = frmOpenEvent;
    event.data.frmLoad.formID = formID;
    EvtAddEventToQueue(&event);

    // handle all events here (trap them before the OS does) :)
    keepFormOpen = true;
    while (keepFormOpen) 
    {
      EvtGetEvent(&event, globals->evtTimeOut);

      // this is our exit condition! :)
      keepFormOpen = (event.eType != frmCloseEvent);

      if (!SysHandleEvent(&event)) 
        if (!MenuHandleEvent(0, &event, &err)) 
          if (!ApplicationHandleEvent(&event))
            FrmDispatchEvent(&event);

      if (event.eType == appStopEvent) 
      {
        keepFormOpen = false;
        EvtAddEventToQueue(&event);  // tap "applications", need to exit
      }
    }
  }

  // restore the active form/window
  if (DeviceSupportsVersion(romVersion3)) 
  {
    FrmRestoreActiveState(&frmCurrState);
  }
  else 
  {
    FrmSetActiveForm(frmActive);
    WinSetDrawWindow(winDrawWindow);
    WinSetActiveWindow(winActiveWindow);     // < palmos3.0, manual work
  }
}

/**
 * The Palm Computing Platform event processing loop.
 */
void  
EventLoop()
{
  Globals   *globals;
  EventType event;
  UInt16    err;

  // get a globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  do {
    EvtGetEvent(&event, globals->evtTimeOut);

    if (!SysHandleEvent(&event)) 
      if (!MenuHandleEvent(0, &event, &err)) 
        if (!ApplicationHandleEvent(&event)) 
          FrmDispatchEvent(&event);

  } while (event.eType != appStopEvent);
}

/**
 * The Palm Computing Platform termination routine.
 */
void  
EndApplication()
{
  Globals *globals;
  UInt16  prefSize;

  // get a globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  // restore the key state
  KeySetMask(keyBitsAll);

  // terminate the game environemnt
  RegisterTerminate();

  // ensure all forms are closed
  FrmCloseAllForms();

  // save preferences
  MemPtrFree(globals->prefs->system.hotSyncUsername);
  globals->prefs->system.hotSyncUsername = NULL;

  // lets add our 'check' data chunk
  prefSize = sizeof(PreferencesType); 
  StrCopy(globals->prefs->system.signature, "|HaCkMe|");
  RegisterDecryptChunk((UInt8 *)globals->prefs, prefSize, 0x0007, 0x01);
  PrefSetAppPreferences(appCreator, 0, 1, globals->prefs, prefSize, true);
  MemPtrFree(globals->prefs);
}
