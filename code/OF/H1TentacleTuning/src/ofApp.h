// ofApp.h
#pragma once
#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	// Serial communication
	ofSerial serial;
	bool bSerialConnected;
	void sendServoPositions();
	void sendServoPositions(float actualTipY);  // Overloaded version (kept for compatibility)
	
	// GUI Controls
	ofxPanel gui;
	ofParameterGroup baseGroup;
	ofParameterGroup tipGroup;
	ofParameterGroup compensationGroup;
	
	ofParameter<float> baseX;
	ofParameter<float> baseY;
	ofParameter<float> tipX;
	ofParameter<float> tipY;
	
	ofParameter<bool> enableCompensation;
	ofParameter<float> compensationRatio;
	ofParameter<float> compensationOffset;
	
	// Visualization
	void drawTentacleVisualization();
	void drawPositionGrid();
	void drawCompensationCurve();
	
	// Recording
	ofParameter<bool> recording;
	vector<ofVec4f> recordedPositions;
	void exportData();
	
	// Visual Style
	ofColor bgColor = ofColor(0);
	ofColor crimson = ofColor(220, 20, 60);
	ofColor duskyYellow = ofColor(180, 160, 80);
	ofColor duskyOrange = ofColor(180, 100, 60);
	ofColor duskyBlue = ofColor(70, 90, 120);
	
	ofTrueTypeFont titleFont;
	ofTrueTypeFont labelFont;
	ofTrueTypeFont infoFont;
	
	// Tentacle parameters
	const int NUM_SEGMENTS = 8;
	const float SEGMENT_LENGTH = 11.0;
	const float TENTACLE_LENGTH = 250.0;
	
	vector<ofVec3f> segmentPositions;
	void updateKinematics();
	void updateKinematics(float actualTipY);  // Overloaded version (kept for compatibility)
};
