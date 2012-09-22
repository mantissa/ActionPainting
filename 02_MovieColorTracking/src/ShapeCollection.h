
#pragma once

#include "ofxCvBlob.h"

class ShapeCollection {

public:
	
	//--------------------------------------------------------------
	
	void addShape(ofxCvBlob & newShape){
	
		shapes.push_back(newShape);
	}
	
	//--------------------------------------------------------------
	
	void addColor(ofColor & newColor){
		
		colors.push_back(newColor);
	}
	
	//--------------------------------------------------------------
	
	// draw the shape with some randomness
	// rotate the shape, offset the x,y positions
	
	void drawSplatter(){
		
		for(int i=0; i<shapes.size(); i++){
			
			//in order to rotate around the center of the shape, we need to translate into its center
			ofPushMatrix();
			
			ofTranslate(shapes[i].boundingRect.x, shapes[i].boundingRect.y, 0);
			ofTranslate(shapes[i].boundingRect.width/2, shapes[i].boundingRect.height/2, 0);
			
			// then we can rotate
			ofRotateZ(ofRandom(-30, 30));
			
			// then we translate back
			ofTranslate(-shapes[i].boundingRect.x, -shapes[i].boundingRect.y, 0);
			ofTranslate(-shapes[i].boundingRect.width/2, -shapes[i].boundingRect.height/2, 0);
			
			// set the color
			ofSetColor(colors[i]);
			
			// slight random offset
			ofTranslate(ofRandom(-20, 20), ofRandom(-20, 20), 0);
			
			// loop thru the points and add them to the shape
			ofBeginShape();
			
			for(int j=0; j<shapes[i].nPts; j++){
			
				ofVertex(shapes[i].pts[j]);
			}
			
			ofEndShape(true);
			
			ofPopMatrix();
		}
	}
	
	//--------------------------------------------------------------
	
	// draw the shape normally
	
	void draw(){
		
		for(int i=0; i<shapes.size(); i++){
			
			ofSetColor(colors[i]);
			
			ofBeginShape();
			
			for(int j=0; j<shapes[i].nPts; j++){
				
				ofVertex(shapes[i].pts[j]);
			}
			
			ofEndShape(true);
		}
	}
	
	//--------------------------------------------------------------
	
	// save our shape data from opencv's contourFinder 
	
	void saveShapeDataAsXml(string filePath){
		
		ofxXmlSettings xmlDoc;
		
		xmlDoc.addTag("shapes");
		xmlDoc.pushTag("shapes");
		
		int numShapes = shapes.size();
		
		for(int i=0; i<numShapes; i++){
			
			xmlDoc.addTag("shape");
			xmlDoc.pushTag("shape", i);
			
			// add the color
			// just use the picked color (there are other ways to get the color from the blob
			xmlDoc.addTag("color");
			xmlDoc.addAttribute("color", "r", colors[i].r, 0);
			xmlDoc.addAttribute("color", "g", colors[i].g, 0);
			xmlDoc.addAttribute("color", "b", colors[i].b, 0);
			
			// add the points
			xmlDoc.addTag("points");
			xmlDoc.pushTag("points");
			
			int numPoints = shapes[i].nPts;
			
			for(int j=0; j<numPoints; j++){
				
				// save the points as attributes
				xmlDoc.addTag("point");
				xmlDoc.addAttribute("point", "x", shapes[i].pts[j].x, j);
				xmlDoc.addAttribute("point", "y", shapes[i].pts[j].y, j);
			}
			
			xmlDoc.popTag();
			xmlDoc.popTag();
		}
		
		xmlDoc.saveFile(filePath);
		
		// ofLogNotice("Saved xml file");
	}
	
	vector<ofxCvBlob> shapes;
	vector<ofColor> colors;
};