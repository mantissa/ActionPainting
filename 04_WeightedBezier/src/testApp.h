
/*
 
Action Painting Demos
Jeremy Rotsztain / jeremy@mantissa.ca / http://www.mantissa.ca 
September 2012
 
*/

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class testApp : public ofBaseApp{
	
public:
	
	void setup();
	void update();
	void draw();
	
	void loadShape(string url, ofPath & path);
	void interpolateShape(ofPath & path, ofPath & newpath);
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	
	ofPath originalPath;
	ofPath interpolatedPath;
};
