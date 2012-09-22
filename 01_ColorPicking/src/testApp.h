
/*
 
Action Painting Demos
Jeremy Rotsztain / jeremy@mantissa.ca / http://www.mantissa.ca 
September 2012
 
*/

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"

class testApp : public ofBaseApp{
	
public:
	
	void setup();
	void update();
	void draw();
	
	ofColor getColorAtPos(ofPixels & pixels, int x, int y);
	void searchForColorInPixels(ofColor & color, ofPixels & pixels, int thresh);
	void convertToVectors(ofxCvGrayscaleImage & map);
	void saveShapeDataAsXml(string filePath);
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	
	ofImage source;
	ofColor pickedColor;
	int matchThreshold;
	
	ofxCvGrayscaleImage colorMap;
	ofxCvContourFinder 	contourFinder;
};
