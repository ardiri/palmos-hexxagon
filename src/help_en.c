/*
 * @(#)help_en.c
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
 *
 * --------------------------------------------------------------------
 *             THIS FILE CONTAINS THE ENGLISH LANGUAGE TEXT
 * --------------------------------------------------------------------
 */

#include "palm.h"

typedef struct 
{
  UInt32    keyMask;
  WinHandle helpWindow;
} HelpGlobals;

/**
 * Initialize the instructions screen.
 * 
 * @return the height in pixels of the instructions data area.
 */
UInt16
InitInstructions()
{
  const RectangleType     rect  = {{0,0},{142,331}};
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};
  HelpGlobals *gbls;
  UInt16      err;

  // create the globals object, and register it
  gbls = (HelpGlobals *)MemPtrNew(sizeof(HelpGlobals));
  MemSet(gbls, sizeof(HelpGlobals), 0);
  FtrSet(appCreator, ftrHelpGlobals, (UInt32)gbls);

  // setup the valid keys available at this point in time
  gbls->keyMask = KeySetMask(~(keyBitsAll ^ 
                              (keyBitPower   | keyBitCradle   |
                               keyBitPageUp  | keyBitPageDown |
                               keyBitAntenna | keyBitContrast)));

  // initialize windows
  gbls->helpWindow = 
    WinCreateOffscreenWindow(rect.extent.x,rect.extent.y,screenFormat,&err);
  err |= (gbls->helpWindow == NULL);

  // did something go wrong?
  if (err != errNone) {

    ApplicationDisplayDialog(xmemForm);

    // close the form
    {
      EventType event;
      MemSet(&event, sizeof(EventType), 0);
      event.eType = frmCloseEvent;
      event.data.frmClose.formID = FrmGetActiveFormID();
      EvtAddEventToQueue(&event);
    }
  }

  // draw the help
  else {
    FontID    font;
    WinHandle currWindow;

    currWindow = WinGetDrawWindow();
    font       = FntGetFont();

    // draw to help window
    WinSetDrawWindow(gbls->helpWindow);
    WinSetPattern(&erase);
    WinFillRectangle(&rect,0);

    {
      Char  *str, *ptrStr;
      Coord x, y;

      // initialize
      y   = 2;
      str = (Char *)MemPtrNew(256 * sizeof(Char));

      // draw title
      StrCopy(str, "HOW TO PLAY");
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;

      WinSetUnderlineMode(grayUnderline);
      WinDrawChars(str, StrLen(str), x, y); y += FntLineHeight();
      WinSetUnderlineMode(noUnderline);

      // add space (little)
      y += FntLineHeight() >> 1;

      x = 4;
      StrCopy(str,
"The aim of the game is to fill the board with as many pieces that belong \
to you as possible.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

        x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
        WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

        ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // show the logic
      x = 16;
      {
        MemHandle bitmapHandle = DmGet1Resource('Tbmp', bitmapHelpGamePlay);
        WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), x, y);
        MemHandleUnlock(bitmapHandle);
        DmReleaseResource(bitmapHandle);
      }

      // add space (little)
      y += 47 + (FntLineHeight() >> 1);

      // general text
      x = 4;
      StrCopy(str,
"Moving a piece one space creates a new piece (a duplicate), while \
moving over two spaces physically moves the piece.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

        x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
        WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

        ptrStr += count;
      }

      // add space (little)
      y += (FntLineHeight() >> 1);

      // general text
      x = 4;
      StrCopy(str,
"Move the cursor above the piece that you wish to move. With the stylus \
it will automatically be selected when lifted, press the select button \
if using the keys.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

        x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
        WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

        ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // show the logic
      x = 16;
      {
        MemHandle bitmapHandle = DmGet1Resource('Tbmp', bitmapHelpGameSelect);
        WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), x, y);
        MemHandleUnlock(bitmapHandle);
        DmReleaseResource(bitmapHandle);
      }

      // add space (little)
      y += 71 + (FntLineHeight() >> 1);

      // general text
      x = 4;
      StrCopy(str,
"The valid moves are then shown. Tap one and the move will occur.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

        x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
        WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

        ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, "GOOD LUCK!");
      FntSetFont(boldFont);
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y += FntLineHeight();

      // clean up
      MemPtrFree(str);
    }

    FntSetFont(font);
    WinSetDrawWindow(currWindow);
  }

  return rect.extent.y;
}

/**
 * Draw the instructions on the screen.
 * 
 * @param offset the offset height of the window to start copying from.
 */
void 
DrawInstructions(UInt16 offset)
{
  const RectangleType helpArea = {{0,offset},{142,116}};
  HelpGlobals *gbls;

  // get globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // blit the required area
  WinCopyRectangle(gbls->helpWindow, 
                   WinGetDrawWindow(), &helpArea, 3, 16, winPaint);
}

/**
 * Terminate the instructions screen.
 */
void
QuitInstructions()
{
  HelpGlobals *gbls;

  // get globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // return the state of the key processing
  KeySetMask(gbls->keyMask);

  // clean up memory
  WinDeleteWindow(gbls->helpWindow, false);
  MemPtrFree(gbls);

  // unregister global data
  FtrUnregister(appCreator, ftrHelpGlobals);
}
