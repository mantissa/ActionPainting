
#include "testApp.h"

//--------------------------------------------------------------

void testApp::setup(){
	
	ofSetFrameRate(30);

	// load the path
	loadShape("shapes.xml", originalPath);
	
	// create a bezier-interpolated copy of that path
	interpolateShape(originalPath, interpolatedPath);
}

//--------------------------------------------------------------

void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){

	ofBackground(0, 0, 0);
	
	originalPath.draw(0, 0);
	
	interpolatedPath.draw(ofGetWidth()/2, 0);
}

//--------------------------------------------------------------

// load a shape from an xml file
// save its points and color into an instance of ofPath
// look at the xml file in the data folder to see the file structure

void testApp::loadShape(string url, ofPath & path){
	
	path.clear();
	
	ofxXmlSettings xmlDoc;
	
	xmlDoc.loadFile(url);
	
	if( xmlDoc.tagExists("shapes") ){
	
		xmlDoc.pushTag("shapes");
		xmlDoc.pushTag("shape", 0); // just load the first shape
		
		// get the RGB color components
		unsigned char r = xmlDoc.getAttribute("color", "r", 0);
		unsigned char g = xmlDoc.getAttribute("color", "g", 0);
		unsigned char b = xmlDoc.getAttribute("color", "b", 0);
		
		path.setColor(ofColor(r, g, b));
		
		xmlDoc.pushTag("points");
		
		// get the # of points
		int numPoints = xmlDoc.getNumTags("point");
		
		// loop thru the points and add to the ofPath
		for(int i=0; i<numPoints; i++){
		
			float x = xmlDoc.getAttribute("point", "x", 0, i);
			float y = xmlDoc.getAttribute("point", "y", 0, i);
			path.lineTo(x, y);
			
			// uncomment to see the points
			//printf("point %f %f\n", x, y);
		}
		
		// close the shape when its done
		path.close();
		
		xmlDoc.popTag();
		xmlDoc.popTag();
		xmlDoc.popTag();
		
	} else {
	
		ofLog(OF_LOG_ERROR, "Failed to load shape file");
	}

}

//--------------------------------------------------------------

// create a new smoothed shape from the original shape we loaded

void testApp::interpolateShape(ofPath & path, ofPath & newpath){
	
	newpath.clear(); // clear any points in the new shape 
	
	vector<ofPolyline> contours = path.getOutline();
	
	if( contours.size() > 0 ){
		
		// just look at the first subpath (assume we just have one)
		
		vector<ofPoint> points = contours[0].getVertices();
		vector<ofPoint> newPoints;
		
		// create a new set of contours, removing points on the same slope
		// don't read every point .. increment by 5
		
		newPoints.push_back(points[0]);
		
		for( int p = 5; p < points.size()-1; p+=5){
			
			float curSlope = 0;
			float lastSlope = 0;
			
			ofPoint d1 = points[p+4]-points[p];
			ofPoint d2 = points[p]-points[p-4];
			
			if( d1.x != 0 ) curSlope = d1.y/d1.x;
			if( d2.x != 0 ) lastSlope = d2.y/d2.x;
			
			if( curSlope != lastSlope ){ 
				
				newPoints.push_back(points[p]);
			}
		}
		
		int numPoints = newPoints.size() -1;
		
		// add the first point
		newpath.moveTo(newPoints[0]);
		
		float smooth_value = 1.5; // smoothing coefficient
		
		for( int i=1; i<numPoints; i++){
			
			int p0 = i-1;
			int p1 = i;
			int p2 = i+1;
			int p3 = i+2;
			
			if( p0 < 0) p0 += numPoints+1;
			if( p0 > numPoints ) p0 %= numPoints+1;
			if( p1 < 0) p1 += numPoints+1;
			if( p1 > numPoints ) p1 %= numPoints+1;
			if( p2 > numPoints ) p2 %= numPoints+1;
			if( p2 < 0) p2 += numPoints+1;
			if( p3 > numPoints ) p3 %= numPoints+1;
			
			float xc1 = (newPoints[p0].x + newPoints[p1].x) / 2.0;
			float yc1 = (newPoints[p0].y + newPoints[p1].y) / 2.0;
			float xc2 = (newPoints[p1].x + newPoints[p2].x) / 2.0;
			float yc2 = (newPoints[p1].y + newPoints[p2].y) / 2.0;
			float xc3 = (newPoints[p2].x + newPoints[p3].x) / 2.0;
			float yc3 = (newPoints[p2].y + newPoints[p3].y) / 2.0;
			
			float len1 = sqrt((newPoints[p1].x-newPoints[p0].x) * (newPoints[p1].x-newPoints[p0].x) + (newPoints[p1].y-newPoints[p0].y) * (newPoints[p1].y-newPoints[p0].y));
			float len2 = sqrt((newPoints[p2].x-newPoints[p1].x) * (newPoints[p2].x-newPoints[p1].x) + (newPoints[p2].y-newPoints[p1].y) * (newPoints[p2].y-newPoints[p1].y));
			float len3 = sqrt((newPoints[p3].x-newPoints[p2].x) * (newPoints[p3].x-newPoints[p2].x) + (newPoints[p3].y-newPoints[p2].y) * (newPoints[p3].y-newPoints[p2].y));
			
			float k1 = len1 / (len1 + len2);
			float k2 = len2 / (len2 + len3);
			
			float xm1 = xc1 + (xc2 - xc1) * k1;
			float ym1 = yc1 + (yc2 - yc1) * k1;
			
			float xm2 = xc2 + (xc3 - xc2) * k2;
			float ym2 = yc2 + (yc3 - yc2) * k2;
			
			// Resulting control points. Here smooth_value is mentioned
			// above coefficient K (smooth_value) whose value should be in range [0...1].
			float ctrl1_x = xm1 + (xc2 - xm1) * smooth_value + newPoints[p1].x - xm1;
			float ctrl1_y = ym1 + (yc2 - ym1) * smooth_value + newPoints[p1].y - ym1;
			
			float ctrl2_x = xm2 + (xc2 - xm2) * smooth_value + newPoints[p2].x - xm2;
			float ctrl2_y = ym2 + (yc2 - ym2) * smooth_value + newPoints[p2].y - ym2;
			
			// create a bezier curve to the next point
			newpath.bezierTo(ofPoint(ctrl1_x, ctrl1_y), ofPoint(ctrl2_x, ctrl2_y), newPoints[p2]);
		}
		
		newpath.close();
		
		newpath.setColor(path.getFillColor());
	}
}

//--------------------------------------------------------------

void testApp::keyPressed(int key){

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