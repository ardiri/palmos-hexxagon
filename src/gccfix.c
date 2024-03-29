/*
 * @(#)gccfix.h
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
 * - source code obtained from John Marshall (john_w_marshall@palm.com)
 */

#include "palm.h"

void
_GccReleaseCode(UInt16 cmd UNUSED, void *pbp UNUSED, UInt16 flags)
{
  if (flags & sysAppLaunchFlagNewGlobals) {

    MemHandle codeH;
    int resno;

    for (resno=2; (codeH = DmGet1Resource('code', resno)) != NULL; resno++) {
      MemHandleUnlock(codeH);
      DmReleaseResource(codeH);
    }
  }
}
