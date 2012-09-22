
/*
 
Action Painting Demos
Jeremy Rotsztain / jeremy@mantissa.ca / http://www.mantissa.ca 
September 2012
 
*/

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"
#include "ShapeCollection.h"

enum { APP_MODE_IDLE = 0, APP_MODE_TRACKING, APP_MODE_PLAYING, APP_MODE_SAVING };

class testApp : public ofBaseApp{
	
public:
	
	void setup();
	void update();
	void draw();
	
	ofColor getColorAtPos(ofPixels & pixels, int x, int y);
	void searchForColorInPixels(ofColor & color, ofPixels & pixels, int thresh);
	void convertToVectors(ofxCvGrayscaleImage & map);
	ofColor getColorOfShape(ofxCvBlob & shape);
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	
	ofVideoPlayer source;
	ofColor searchColor;
	int matchThreshold;
	int currentFrame;
	int appMode;
	bool bDataExtracted;
	
	ofxCvGrayscaleImage colorMap;
	ofxCvContourFinder 	contourFinder;
	vector <ShapeCollection> frames;
	ofFbo canvas;
};
