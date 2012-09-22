
#include "testApp.h"

//--------------------------------------------------------------

void testApp::setup(){
	
	ofSetFrameRate(30);

	// set our app mode
	appMode = APP_MODE_TRACKING;
	
	// load the movie (The Target by Jacob Dow)
	// available on vimeo: https://vimeo.com/35391502
	source.loadMovie("TheTarget.mov");
	
	// create a CV "map" to show the areas of matching color
	changedPixelsMap.allocate(source.getWidth(), source.getHeight());
	currentFrameCvRGB.allocate(source.getWidth(), source.getHeight());
	currentFrameCv.allocate(source.getWidth(), source.getHeight());
	previouFrameCv.allocate(source.getWidth(), source.getHeight());
	
	// create a window as big as the image
	ofSetWindowShape(source.getWidth()*2, source.getHeight());
	
	// how close should the color be to the picked color
	motionThreshold = 35;
	
	// current frame
	currentFrame = 0;
	
	// we haven't saved our data yet
	bDataExtracted = false;
	
	// create a canvas texture to accumulate paint shapes
	canvas.allocate(source.getWidth(), source.getHeight(), GL_RGB);
}

//--------------------------------------------------------------

void testApp::update(){

	if( appMode == APP_MODE_TRACKING ){
		
		// set movie to current frame (one frame at a time)
		source.setFrame(currentFrame);
		source.update();
		
		// set the prev frame to the current frame
		previouFrameCv = currentFrameCv;
		
		// get the current frame from the movie and convert to grayscale cvImage
		currentFrameCvRGB.setFromPixels(source.getPixels(), source.getWidth(), source.getHeight());
		currentFrameCv = currentFrameCvRGB;
		
		if( currentFrame > 0 ){
		
			// search for motion and create vector shapes
			searchForMotion(currentFrameCv, previouFrameCv, motionThreshold);
			convertToVectors(changedPixelsMap);
		}
		
		currentFrame++;
		
		// do this until we're at the last frame of the movie
		if( currentFrame == source.getTotalNumFrames() ){
			
			ofLogNotice("Finished tracking colors in movie");
			appMode = APP_MODE_IDLE;
			bDataExtracted = true;
		}
		
	} else if( appMode == APP_MODE_PLAYING ){
		
		// update the video
		source.update();
	
	} else if( appMode == APP_MODE_SAVING ){
		
		// set movie to current frame
		source.setFrame(currentFrame);
		source.update();
		
		// dynamic file name based on current frame
		char fileName[30];
		sprintf(fileName, "frames/frame_%.5i.xml", currentFrame);
		frames[currentFrame].saveShapeDataAsXml(fileName);
		 
		currentFrame++;
		
		// do this until we've saved all the frames' shapes
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
		
			// playback instructions
			ofDrawBitmapString("Press RETURN to play animation", 20, 20);
			ofDrawBitmapString("There 's' to save animation data ", 20, 40);
		}
		
	} else if ( appMode == APP_MODE_TRACKING ){
	
		// draw our source image
		ofSetColor(255, 255, 255);
		source.draw(0, 0);
		
		// draw the changed pixels map
		// the white pixels indicate matching colors
		ofSetColor(255, 255, 255);
		changedPixelsMap.draw(ofGetWidth()/2, 0);
		
		// draw the blobs found in the open cv search
		contourFinder.draw(ofGetWidth()/2, 0);
		
		// info about tracking
		ofSetColor(0, 255, 255);
		ofDrawBitmapString("Tracking frame "+ofToString(currentFrame)+"/"+ofToString(source.getTotalNumFrames()), ofGetWidth()/2+20, 20);
		ofDrawBitmapString("There are "+ofToString(contourFinder.nBlobs)+" shapes", ofGetWidth()/2+20, 40);
		
	} else if(appMode == APP_MODE_PLAYING) {
	
		ofSetColor(255, 255, 255);
		source.draw(0, 0);
		
		if( source.isFrameNew() ){
		
			int currentFrame = source.getCurrentFrame();

			// draw into the canvas texture
			canvas.begin();
			
			if( currentFrame < frames.size()) {
			
				// draw the shapes in splatter form
				frames[currentFrame].drawSplatter();
			}
			
			canvas.end();
			
		}
		
		// then draw the canvas texture
		ofSetColor(255, 255, 255);
		canvas.draw(ofGetWidth()/2, 0);
		
		if( source.getPosition() == 1.0 ){
			
			ofDrawBitmapString("Press RETURN to play animation", 20, 20);
			ofDrawBitmapString("There 's' to save animation data ", 20, 40);
		}

	} else if(appMode == APP_MODE_SAVING) {
		
		ofSetColor(255, 255, 255);
		source.draw(0, 0);
		
		ofPushMatrix();
		
		ofTranslate(ofGetWidth()/2, 0, 0);
		
		// draw the shapes normally
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

void testApp::searchForMotion(ofxCvGrayscaleImage & curFrame, ofxCvGrayscaleImage & prevFrame, int thresh){
	
	changedPixelsMap = curFrame;
	
	changedPixelsMap.absDiff(prevFrame);
	
	changedPixelsMap.threshold(thresh, false);
	
	changedPixelsMap.blur(5);
	
	changedPixelsMap.threshold(128);
}

//--------------------------------------------------------------

// use opencv's contour finder to get the outlines from a greyscale image

void testApp::convertToVectors(ofxCvGrayscaleImage & map){

	contourFinder.findContours(map, 5, ofGetWidth() * ofGetHeight() / 25, 20000, true, false);
	
	ShapeCollection frameShapes;
	
	int numShapes = contourFinder.nBlobs;
	
	for(int i=0; i<numShapes; i++){
		
		// create a new blob
		// copy over the blob data from the source blob
		ofxCvBlob newBlob;
		newBlob.nPts = contourFinder.blobs[i].nPts;
		newBlob.pts.insert(newBlob.pts.begin(), contourFinder.blobs[i].pts.begin(), contourFinder.blobs[i].pts.end());
		newBlob.boundingRect = contourFinder.blobs[i].boundingRect;
		frameShapes.addShape(newBlob);
		
		ofColor shapeColor = getColorOfShape(contourFinder.blobs[i]);
		frameShapes.addColor(shapeColor);
	}
	
	//printf("we have %i shapes\n", frameShapes.shapes.size());
	
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
	
	unsigned char * mapPix = changedPixelsMap.getPixels();
	unsigned char * pix = source.getPixels();
	
	int sumR = 0;
	int sumG = 0;
	int sumB = 0;
	
	int pixFound = 0;
	
	while( pixFound < 5 ){
	
		// randomly look in the blob area for a white color
		int randX = searchRect.x + ofRandom(searchRect.width);
		int randY = searchRect.y + ofRandom(searchRect.height);
		int memPos = randY * source.getWidth() + randX;
		
		// if the pixel is white
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
	
			// start play mode (at the first frame)
			appMode = APP_MODE_PLAYING;
			currentFrame = 0;
			
			// clear the canvas/fbo with a specific color
			canvas.begin();
			ofClear(238, 234, 213);
			canvas.end();
			
			// start the movie
			source.setPosition(0.0);
			source.play();
			source.setLoopState(OF_LOOP_NONE);
	
		} else if( key == 's' ){

			// start save mode
			appMode = APP_MODE_SAVING;
			currentFrame = 0;
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