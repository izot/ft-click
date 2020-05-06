/*
 * Filename: LonBegin.h
 *
 * Description: This file contains the beginning of declarations that
 * are required to compile the ShortStack API on a particular platform.
 *
 * Copyright (c) Echelon Corporation 2002-2009.  All rights reserved.
 *
 * This file is ShortStack LonTalk Compact API Software as defined in the 
 * Software License Agreement that governs its use.
 *
 * ECHELON MAKES NO REPRESENTATION, WARRANTY, OR CONDITION OF
 * ANY KIND, EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE OR IN
 * ANY COMMUNICATION WITH YOU, INCLUDING, BUT NOT LIMITED TO,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, SATISFACTORY
 * QUALITY, FITNESS FOR ANY PARTICULAR PURPOSE, 
 * NONINFRINGEMENT, AND THEIR EQUIVALENTS.
 */

#ifndef _LON_BEGIN_H
#define _LON_BEGIN_H

/*
 * TO DO: Provide any compiler directives or additional definitions needed 
 * to compile the API files on your platform. This could include compiler 
 * directives to control packing and alignment of data structures and unions, 
 * unless controlled through the mechanisms provided in LonPlatform.h. 
 * 
 * This file is included prior to the first declaration made by the 
 * ShortStack API files. Its counterpart, LonEnd.h, is included after the last 
 * declaration made by the API, allowing to reverse preferences expressed in LonBegin.h 
 */

/* This will allign the structures at 1 byte boundaries. */
#pragma pack(push, 1)

#endif /* _LON_BEGIN_H */
