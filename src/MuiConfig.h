//
//  MuiConfig.h
//  Oscilloscope
//
//  Created by Hansi on 10.08.15.
//
//

#ifndef __Oscilloscope__MuiConfig__
#define __Oscilloscope__MuiConfig__

#include "ofMain.h"

#if defined(TARGET_OSX)
#define MUI_KEY_ACTION OF_KEY_SUPER
#else
#define MUI_KEY_ACTION OF_KEY_CONTROL
#endif


// default font
#define MUI_FONT MuiConfig::font

// already deprecated, don't use
#define MUI_SCROLLPANE_BLEED mui::MuiConfig::scrollPaneBleed

// time it takes for a scrollpane to scroll back to it's initial position, time in ms
// should be 6*325, feels way too slow
#define MUI_SCROLL_TO_BASE_DURATION mui::MuiConfig::scrollToBaseDuration

// rate at which the scroll velocity decreases, not sure about the unit, pixels/s?
// guessing around ...
#define MUI_SCROLL_VELOCITY_DECREASE mui::MuiConfig::scrollVelocityDecrease

#define MUI_DEBUG_DRAW mui::MuiConfig::debugDraw

#define MUI_FONT_SIZE mui::MuiConfig::fontSize

#define MUI_DOUBLE_TAP_TIME mui::MuiConfig::doubleTapTime


namespace mui{
	class MuiConfig{
	public:
		// default font
		static std::string font;
		
		// enable/disable debug drawing. this will put rectangles
		// around each component
		static bool debugDraw;
		
		// already deprecated, don't use
		static int scrollPaneBleed;
		
		// time it takes for a scrollpane to scroll back to it's initial position, time in ms
		// should be 6*325, feels way too slow
		static int scrollToBaseDuration;
		
		// rate at which the scroll velocity decreases, not sure about the unit, pixels/s?
		// guessing around ...
		static int scrollVelocityDecrease;
		
		// duration during which another click counts as a double click/tap, in seconds. 
		static double doubleTapTime;
		
		// size of the fontstash font atlas
		// set this before calling mui_init()! 
		static int fontAtlasSize;
		
		// standard font size
		static int fontSize;
		
		// automaticaly detect retina on device like the ipad?
		// this must be set before calling mui_init()
		static bool detectRetina;
		static bool useRetinaAssets;
		
		// pixels per pixel.
		static float scaleFactor;
		
		// path to the data folder (containing the mui folder)
        static std::filesystem::path dataPath;
		
		static ofLogLevel logLevel;
		
		// enable shortcuts for debugging
		// (press action+d to enable)
		static bool enableDebuggingShortcuts;
		
		// function that can show a system "save..." dialog
		static std::function<ofFileDialogResult(string,string)> systemSaveDialog;
		
		// function that can show a system "load..." dialog
		static std::function<ofFileDialogResult(string)> systemLoadDialog;
	};
}

#endif /* defined(__Oscilloscope__MuiConfig__) */
