/*
 * @(#)graphics.h
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
 * portions written by  Charles Kerchner (mailto:timberdogsw@hotmail.com)
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "palm.h"

extern Boolean   GraphicsInitialize()                               __DEVICE__;
extern WinHandle GraphicsGetDrawWindow()                            __DEVICE__;
extern void      GraphicsClear()                                    __DEVICE__;
extern void      GraphicsSetUpdate(Int16, Int16)                    __DEVICE__;
extern void      GraphicsRepaint()                                  __DEVICE__;
extern void      GraphicsTerminate()                                __DEVICE__;

#endif 
