
#include "testApp.h"

//--------------------------------------------------------------

void testApp::setup(){
	
	ofSetFrameRate(30);

	// set our app mode
	appMode = APP_MODE_TRACKING;
	
	// load movie 
	// Selection from "Sprengung der Fliegerbombe / Schwabing, MŸnchen / 28.8.2012"
	// By Simon Aschenbrenner
	// Available on Vimeo https://vimeo.com/48399328
	source.loadMovie("Explosion.mov");
	
	// create a "map" texture to show the areas of matching color
	colorMap.allocate(source.getWidth(), source.getHeight());
	
	// create a window 2x as big as the image
	ofSetWindowShape(source.getWidth()*2, source.getHeight());
	
	// set the color to search for 
	searchColor.set( 225, 140, 60); 
	
	// how close should the color be to the picked color
	matchThreshold = 13;
	
	// current frame
	currentFrame = 0;
	
	// we haven't saved our data yet
	bDataExtracted = false;
	
	// create a 'canvas' texture to accumulate shapes
	canvas.allocate(source.getWidth(), source.getHeight(), GL_RGB);
}

//--------------------------------------------------------------

void testApp::update(){

	if( appMode == APP_MODE_TRACKING ){
		
		// move to the current frame
		source.setFrame(currentFrame);
		source.update();
		
		// look for the matching pixels & convert shapes to vectors
		searchForColorInPixels( searchColor, source.getPixelsRef(), matchThreshold);
		convertToVectors(colorMap);
		 
		// update the frame count
		currentFrame++;
		
		// if we're at the end of the movie, stop
		if( currentFrame == source.getTotalNumFrames() ){
			
			ofLogNotice("Finished tracking colors in file");
			appMode = APP_MODE_IDLE;
			bDataExtracted = true;
		}
		
	} else if( appMode == APP_MODE_PLAYING ){
		
		// update the movie
		source.update();
		
	} else if( appMode == APP_MODE_SAVING ){
		
		// move to the current frame
		source.setFrame(currentFrame);
		source.update();
		
		// dynamically set the xml file name
		char fileName[30];
		sprintf(fileName, "frames/frame_%.5i.xml", currentFrame);
		frames[currentFrame].saveShapeDataAsXml(fileName);
		
		// update the frame count
		currentFrame++;
		
		// if we've saved all fo the frames' shapes
		if( currentFrame >= frames.size()){
			
			ofLogNotice("Finished saving shape data to disk");
			appMode = APP_MODE_IDLE;
		}
	}
}

//--------------------------------------------------------------

void testApp::draw(){

	ofBackground(0, 0, 0);
	
	if( appMode == APP_MODE_IDLE ){
	
		// draw our source image
		ofSetColor(255, 255, 255);
		source.draw(0, 0);
		
		if( bDataExtracted ){
			
			// draw instructions
			ofDrawBitmapString("Press RETURN to play animation", 20, 20);
			ofDrawBitmapString("There 's' to save animation data ", 20, 40);
		}
		
	} else if ( appMode == APP_MODE_TRACKING ){
	
		// draw our source image
		ofSetColor(255, 255, 255);
		source.draw(0, 0);
		
		// draw the map
		// the white pixels indicate matching colors
		ofSetColor(255, 255, 255);
		colorMap.draw(ofGetWidth()/2, 0);
		
		// draw the blobs found in the open cv search
		contourFinder.draw(ofGetWidth()/2, 0);
		
		ofSetColor(0, 255, 255);
		ofDrawBitmapString("Tracking frame "+ofToString(currentFrame)+"/"+ofToString(source.getTotalNumFrames()), ofGetWidth()/2+20, 20);
		ofDrawBitmapString("There are "+ofToString(contourFinder.nBlobs)+" shapes", ofGetWidth()/2+20, 40);
		
	} else if(  appMode == APP_MODE_PLAYING) {
	
		// draw the movie
		ofSetColor(255, 255, 255);
		source.draw(0, 0);
		
		if( source.isFrameNew() ){
		
			// only when there's a new frame 
			// so we don't draw shapes multiple times
			int currentFrame = source.getCurrentFrame();

			// draw splatter into texture
			canvas.begin();
			
			if( currentFrame < frames.size()) {
			
				frames[currentFrame].drawSplatter();
			}
			
			canvas.end();
			
		}
		
		// draw that texture
		ofSetColor(255, 255, 255);
		canvas.draw(ofGetWidth()/2, 0);
		
		if( source.getPosition() == 1.0 ){
			
			ofDrawBitmapString("Press RETURN to play animation", 20, 20);
			ofDrawBitmapString("There 's' to save animation data ", 20, 40);
		}
		
	} else if(appMode == APP_MODE_SAVING) {
			
		// draw the movie
		ofSetColor(255, 255, 255);
		source.draw(0, 0);
		
		ofPushMatrix();
		
		// move to the right
		ofTranslate(ofGetWidth()/2, 0, 0);
		
		// draw the original shapes (not splattered)
		frames[currentFrame].draw();
		
		ofPopMatrix();
		
		ofSetColor(0, 255, 255);
		ofDrawBitmapString("Saving frame "+ofToString(currentFrame)+"/"+ofToString(source.getTotalNumFrames()), ofGetWidth()/2+20, 20);
	}
}

//--------------------------------------------------------------

// get the color value from an image at position

ofColor testApp::getColorAtPos(ofPixels & pixels, int x, int y){
	
	ofColor pickedColor;
	
	// make sure it's within the bounds of the image
	if( x >= 0 && x < pixels.getWidth() && y >= 0 && y < pixels.getHeight() ){
	
		// get a pointer to the pixel arrays for the search image
		unsigned char * pix = pixels.getPixels();
		int channels = pixels.getNumChannels();
		
		// calculate the position in memory of that x,y point
		int posInMem = ( y * pixels.getWidth() + x) * channels;
			
		// get the RGB values
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
	
	// get a pointer to the pixel arrays for the search image and the map image
	unsigned char * pix = pixels.getPixels();
	unsigned char * mapPix = colorMap.getPixels();
	
	// figure out the size of the images and the # of channels
	int numPix = pixels.getWidth() * pixels.getHeight();
	int channels = pixels.getNumChannels();
	
	// calculate the minimum distance to be considered a matching color
	int minDist = thresh * thresh * thresh;
	
	for(int i=0; i<numPix; i++){
	
		int posInMem = i * channels;
		int dstInMem = i;
	
		// get the RGB values
		unsigned char r = pix[posInMem]; 
		unsigned char g = pix[posInMem+1]; 
		unsigned char b = pix[posInMem+2]; 
		
		// get the difference
		int diffR = color.r - r;
		int diffG = color.g - g; 
		int diffB = color.b - b; 
		
		// get the distance
		float dist = ( diffR * diffR ) + ( diffG * diffG ) + ( diffB * diffB );
		
		// set to black or white depending on the difference 
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
	
	// a container for all of the shapes from the current frame
	ShapeCollection frameShapes;
	
	// get the shape count
	int numShapes = contourFinder.nBlobs;
	
	for(int i=0; i<numShapes; i++){
		
		// copy all of the shape data over to a new cvBlob
		ofxCvBlob newBlob;
		newBlob.nPts = contourFinder.blobs[i].nPts;
		newBlob.pts.insert(newBlob.pts.begin(), contourFinder.blobs[i].pts.begin(), contourFinder.blobs[i].pts.end());
		newBlob.boundingRect = contourFinder.blobs[i].boundingRect;
		frameShapes.addShape(newBlob);
		
		// get the color
		ofColor shapeColor = getColorOfShape(contourFinder.blobs[i]);
		frameShapes.addColor(shapeColor);
	}
	
	// add the curren frame to the queue
	frames.push_back(frameShapes);
}

//--------------------------------------------------------------

// this is one of many ways to grab the color from a blob
// it's actually a very fast and not-so accurate way to do so 
// we're grabbing 5 random colors and averaging them
// but we need make sure that the randomly selected colors are from those 
// pixels that has changed since the last frame (which are the white pixels 
// saved in the cvImage changedPixelsMap)

ofColor testApp::getColorOfShape(ofxCvBlob & shape){
	
	ofColor results;
	ofRectangle searchRect = shape.boundingRect;
	
	unsigned char * mapPix = colorMap.getPixels();
	unsigned char * pix = source.getPixels();
	
	int sumR = 0;
	int sumG = 0;
	int sumB = 0;
	
	int pixFound = 0;
	
	while( pixFound < 5 ){
	
		// randomly look in the blob area for a white color
		int randX = searchRect.x + ofRandom(searchRect.width);
		int randY = searchRect.y + ofRandom(searchRect.height);
		int memPos = randY * colorMap.getWidth() + randX;
		
		// if the map pixel is white
		if( mapPix[memPos] == 255 ){
		
			// mult by 3 (because it's a color source)
			memPos = memPos * 3;
			
			// get the RGB values
			sumR += pix[memPos];
			sumG += pix[memPos+1];
			sumB += pix[memPos+2];
			
			pixFound++;
		}
	}
	
	// update the color with the average values
	results.r = sumR/pixFound;
	results.g = sumG/pixFound;
	results.b = sumB/pixFound;
	
	return results;
}
 
//--------------------------------------------------------------

void testApp::keyPressed(int key){
	
	// don't do anything unless we've extracted all of our movement data
	if( bDataExtracted ){

		if( key == OF_KEY_RETURN ){
		
			// start playback
			currentFrame = 0;
			appMode = APP_MODE_PLAYING;
			
			// clear the canvas texture by filling it with a solid color
			canvas.begin();
			ofClear(238, 234, 213);
			canvas.end();
			
			// prepare the movie
			source.setPosition(0.0);
			source.play();
			source.setLoopState(OF_LOOP_NONE);
		
		} else if( key == 's' ){
			
			// start save mode
			currentFrame = 0;
			appMode = APP_MODE_SAVING;
		}
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

}

//--------------------------------------------------------------

void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------

void testApp::mouseReleased(int x, int y, int button){

}