/*=========================================================================

  Program:   Visualization Toolkit
  Module:    constants.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef constants_h
#define constants_h

// Most of the variables used below are heuristics, please feel free to experiment.

// -------Variables used in IIR(Infinite impulse response) magnification of the video-----
#define alpha 10
#define lambda_c 16
#define delta ((float)(lambda_c/(8*(1+alpha))))

//exaggeration_factor HAS to be a user defined constant, above values could be hardcoded as well, preferrably not
#define exaggeration_factor 200
// ---------------------------------------------------------------------------------------

// -------Spatial Filtering variables-----------
#define r1 0.4
#define r2 0.05
// ---------------------------------------------------------------------------------------

#define YChannel 0
#define IChannel 1
#define QChannel 2

//User given input
#define chromatic_abberation 0.1

// User choice, prefer to keep the default to 6.
#define NumberOfPyramidLevels 6

#endif //constants_h
