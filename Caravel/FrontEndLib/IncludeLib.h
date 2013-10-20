//NOTE:
//
//Some files in FrontEndLib might depend on an include from the game-specific back end library.
//(Specifically, on Db.h)
//It would be desirable to remove this dependency, however.
//This could be done by placing the needed parts of CDb as a base class in BackEndLib.
//I'm not sure that can be done in a general way, however.  In the mean time, this file
//presents a kludge to use the game-specific back end files.
//Set this include path as needed when compiling a specific game.
//
//If there is a cleaner way to insert this directory-dependent include into a specific
//project, please change this file, or the two (currently) files including it, to do so.

#ifndef INCLUDELIB_H
#define INCLUDELIB_H

#include "../DRODLib/DbXML.h"

#endif
