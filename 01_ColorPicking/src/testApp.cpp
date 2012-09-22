
#include "testApp.h"

//--------------------------------------------------------------

void testApp::setup(){
	
	ofSetFrameRate(30);

	// load the path
	source.loadImage("Robocop01.png");
	
	// create a CV "map" to show the areas of matching color
	colorMap.allocate(source.getWidth(), source.getHeight());
	
	// create a window as big as the image
	ofSetWindowShape(source.getWidth()*2, source.getHeight());
	
	// how close should the color be to the picked color
	matchThreshold = 20;
}

//--------------------------------------------------------------

void testApp::update(){

}

//--------------------------------------------------------------

void testApp::draw(){

	ofBackground(0, 0, 0);
	
	// draw our source image
	ofSetColor(255, 255, 255);
	source.draw(0, 0);
	
	if( ofGetMousePressed() ){
	
		ofSetColor(pickedColor);
		ofCircle(mouseX, mouseY, 10);
		
		ofNoFill();
		ofSetColor(0);
		ofCircle(mouseX, mouseY, 10);
		ofFill();
	}
	
	// draw the map
	// the white pixels indicate matching colors
	ofSetColor(255, 255, 255);
	colorMap.draw(ofGetWidth()/2, 0);
	
	// draw the blobs found in the open cv search
	contourFinder.draw(ofGetWidth()/2, 0);
	
	ofSetColor(0, 255, 255);
	ofDrawBitmapString("There are "+ofToString(contourFinder.nBlobs)+" shapes", ofGetWidth()/2+20, 20);
	ofDrawBitmapString("Press 's' to save the shapes", ofGetWidth()/2+20, 40);
}

//--------------------------------------------------------------

// get the color value from an image at position

ofColor testApp::getColorAtPos(ofPixels & pixels, int x, int y){
	
	ofColor pickedColor;
	
	if( x >= 0 && x < pixels.getWidth() && y >= 0 && y < pixels.getHeight() ){
	
		unsigned char * pix = pixels.getPixels();
		int channels = pixels.getNumChannels();
		
		int posInMem = ( y * pixels.getWidth() + x) * channels;
			
		unsigned char r = pix[posInMem]; 
		unsigned char g = pix[posInMem+1]; 
		unsigned char b = pix[posInMem+2]; 
		
		pickedColor.set(r, g, b);
	}
	
	return pickedColor;
}

//--------------------------------------------------------------

// get all of similar colors in an image

void testApp::searchForColorInPixels(ofColor & color, ofPixels & pixels, int thresh){
	
	unsigned char * pix = pixels.getPixels();
	unsigned char * mapPix = colorMap.getPixels();
	int numPix = pixels.getWidth() * pixels.getHeight();
	int channels = pixels.getNumChannels();
	int minDist = thresh * thresh * thresh;
	
	for(int i=0; i<numPix; i++){
	
		int posInMem = i * channels;
		int dstInMem = i;
	
		unsigned char r = pix[posInMem]; 
		unsigned char g = pix[posInMem+1]; 
		unsigned char b = pix[posInMem+2]; 
		
		int diffR = color.r - r;
		int diffG = color.g - g; 
		int diffB = color.b - b; 
		
		float dist = ( diffR * diffR ) + ( diffG * diffG ) + ( diffB * diffB );
		
		if( dist < minDist ){
			
			mapPix[dstInMem] = 255;
			
		} else {
			
			mapPix[dstInMem] = 0;
		}
	}
	
	// update the pixels
	colorMap.setFromPixels(mapPix, colorMap.getWidth(), colorMap.getHeight());
	colorMap.updateTexture();
	
	// do a little blurring & thresholding to smooth out the edges
	colorMap.blur(5);
	colorMap.threshold(128);
}

//--------------------------------------------------------------

// use opencv's contour finder to get the outlines from a greyscale image

void testApp::convertToVectors(ofxCvGrayscaleImage & map){

	contourFinder.findContours(map, 5, ofGetWidth() * ofGetHeight(), 20000, true, false);
	
	//printf("there are %i contours\n", contourFinder.nBlobs);
}

//--------------------------------------------------------------

// save our shape data from opencv's contourFinder 

void testApp::saveShapeDataAsXml(string filePath){
	
	ofxXmlSettings xmlDoc;
	
	xmlDoc.addTag("shapes");
	xmlDoc.pushTag("shapes");
	
	int numShapes = contourFinder.nBlobs;
	
	for(int i=0; i<numShapes; i++){
	
		xmlDoc.addTag("shape");
		xmlDoc.pushTag("shape", i);

		// add the color
		// just use the picked color (there are other ways to get the color from the blob
		xmlDoc.addTag("color");
		xmlDoc.addAttribute("color", "r", pickedColor.r, 0);
		xmlDoc.addAttribute("color", "g", pickedColor.g, 0);
		xmlDoc.addAttribute("color", "b", pickedColor.b, 0);
		
		// add the points
		xmlDoc.addTag("points");
		xmlDoc.pushTag("points");
		
		int numPoints = contourFinder.blobs[i].nPts;
		
		for(int j=0; j<numPoints; j++){
		
			// save the points as attributes
			xmlDoc.addTag("point");
			xmlDoc.addAttribute("point", "x", contourFinder.blobs[i].pts[j].x, j);
			xmlDoc.addAttribute("point", "y", contourFinder.blobs[i].pts[j].y, j);
		}
		
		xmlDoc.popTag();
		xmlDoc.popTag();
	}
	
	xmlDoc.saveFile(filePath);
	
	ofLogNotice("Saved xml file");
}
 
//--------------------------------------------------------------

void testApp::keyPressed(int key){

	if( key == 's' ){
	
		saveShapeDataAsXml("shapes.xml");
	}
}

//--------------------------------------------------------------

void testApp::keyReleased(int key){

}

//--------------------------------------------------------------

void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------

void testApp::mouseDragged(int x, int y, int button){

	// get the color
	pickedColor = getColorAtPos(source.getPixelsRef(), x, y);
	
	// search for similar colors within the image
	searchForColorInPixels(pickedColor, source.getPixelsRef(), matchThreshold);
	
	// get the vector outlines from the matching areas
	convertToVectors(colorMap);
}

//--------------------------------------------------------------

void testApp::mousePressed(int x, int y, int button){

	// get the color
	pickedColor = getColorAtPos(source.getPixelsRef(), x, y);
	
	// search for similar colors within the image
	searchForColorInPixels(pickedColor, source.getPixelsRef(), matchThreshold);
	
	// get the vector outlines from the matching areas
	convertToVectors(colorMap);
}

//--------------------------------------------------------------

void testApp::mouseReleased(int x, int y, int button){

}