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
INCLUDE    = -I/Users/ross/Devel/include/ -I/Users/ross/Devel/include/SDL/
FELIB	   = ../lib/felib.a
LIBS       = 
DEFINES    = -D_DEBUG
FRAMEWORK  = -F/Users/ross/Library/Frameworks/ -framework SDL -framework SDL_ttf -framework Carbon -framework Cocoa

SOURCES    =  AnimatedTileEffect.cpp BitmapManager.cpp Bolt.cpp \
BumpObstacleEffect.cpp ButtonWidget.cpp Colors.cpp \
DialogWidget.cpp Effect.cpp EventHandlerWidget.cpp \
Fade.cpp FileDialogWidget.cpp FlashMessageEffect.cpp \
FocusWidget.cpp FontManager.cpp FrameRateEffect.cpp \
FrameWidget.cpp Inset.cpp KeypressDialogWidget.cpp \
LabelWidget.cpp ListBoxWidget.cpp ObjectMenuWidget.cpp \
OptionButtonWidget.cpp Outline.cpp Pan.cpp ScalerWidget.cpp \
Screen.cpp ScreenManager.cpp ScrollingTextWidget.cpp \
ShadeEffect.cpp SliderWidget.cpp Sound.cpp TabbedMenuWidget.cpp \
TextBoxWidget.cpp ToolTipEffect.cpp TransTileEffect.cpp \
Widget.cpp


CXX			= g++
CFLAGS      = -g 
OBJECTS     = $(SOURCES:%.cpp=%.o)			  
			  
all: $(FELIB)

$(FELIB): $(OBJECTS)
	$(AR) -cr $(FELIB) $(OBJECTS)
	ranlib $(FELIB)
	
clean:
	rm -f $(OBJECTS) $(FELIB)
	
%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE)  -c $< -o $@ $(LIBS)

	
