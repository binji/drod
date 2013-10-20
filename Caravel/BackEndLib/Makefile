##############################################################################
# 
# Version: MPL 1.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is Deadly Rooms of Death.
#
# The Initial Developer of the Original Code is
# Caravel Software.
# Portions created by the Initial Developer are Copyright (C) 2002 
# Caravel Software. All Rights Reserved.
#
# Contributor(s):
# Ross Jones, based on Makefile by Matt Schikore (Schik)
#
##############################################################################


SHELL      = /bin/sh
LIBDIR     = -L../lib -L/Users/ross/Devel/lib -L/Users/ross/Devel/fmod/api/lib
INCLUDE    = -I/Users/ross/Devel/include/
BELIB	   = ../lib/belib.a
DEFINES    = -D_DEBUG
FRAMEWORK  = -F/Users/ross/Library/Frameworks/ -framework SDL -framework SDL_ttf -framework Carbon -framework Cocoa

SOURCES    =  Assert.cpp Base64.cpp CoordIndex.cpp Files.cpp GameStream.cpp \
			  IDList.cpp IniFile.cpp Ports.cpp StretchyBuffer.cpp \
			  SysTimer.cpp Wchar.cpp 
			  
CXX			= g++
CFLAGS      = -g 
OBJECTS     = $(SOURCES:%.cpp=%.o)			  
			  
all: $(BELIB)

$(BELIB): $(OBJECTS)
	$(AR) -cr $(BELIB) $(OBJECTS)
	ranlib $(BELIB)
	
clean:
	rm -f $(OBJECTS) $(BELIB)
	
%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE)  -c $< -o $@

	