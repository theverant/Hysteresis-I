#include "ofApp.h"

void ofApp::setup() {
	ofSetWindowTitle("H1 TENTACLE TUNING INTERFACE");
	ofSetWindowShape(1300, 950);  // Window size adjusted for proper layout
	ofSetFrameRate(60);
	ofBackground(bgColor);
	
	// Comment out font loading for now
	// titleFont.load("Bauhaus.ttf", 18);
	// labelFont.load("Arial.ttf", 12);
	// infoFont.load("Baskerville.ttf", 10);
	
	// Setup serial
	serial.listDevices();
	vector<ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	
	bSerialConnected = false;
	for(auto& device : deviceList) {
		if(device.getDevicePath().find("tty.usb") != string::npos ||
		   device.getDevicePath().find("ttyUSB") != string::npos) {
			bSerialConnected = serial.setup(device.getDevicePath(), 115200);
			if(bSerialConnected) {
				ofLogNotice() << "Connected to: " << device.getDevicePath();
				break;
			}
		}
	}
	
	// Setup GUI with custom styling
	gui.setup("SERVO CONTROL", "settings.xml", 20, 120);
	
	// Customize GUI colors
	gui.setHeaderBackgroundColor(crimson);
	gui.setBackgroundColor(ofColor(30, 30, 30, 180));
	gui.setBorderColor(crimson);
	gui.setTextColor(ofColor(255));
	
	baseGroup.setName("BASE XY SERVOS");
	baseGroup.add(baseX.set("Base X", 90, 0, 180));
	baseGroup.add(baseY.set("Base Y", 90, 0, 180));
	gui.add(baseGroup);
	
	tipGroup.setName("TIP XY SERVOS");
	tipGroup.add(tipX.set("Tip X", 90, 0, 180));
	tipGroup.add(tipY.set("Tip Y", 90, 0, 180));
	gui.add(tipGroup);
	
	compensationGroup.setName("HORIZONTAL COMPENSATION");
	compensationGroup.add(enableCompensation.set("Enable", false));
	compensationGroup.add(compensationRatio.set("Ratio", 1.0, 0.0, 3.0));
	compensationGroup.add(compensationOffset.set("Offset", 0.0, -90, 90));  // Increased offset range
	gui.add(compensationGroup);
	
	gui.add(recording.set("RECORD POSITIONS", false));
	
	segmentPositions.resize(NUM_SEGMENTS + 1);
	updateKinematics();  // Call the parameterless version during setup
}

void ofApp::update() {
	// Send positions to Arduino
	if(bSerialConnected && ofGetFrameNum() % 3 == 0) {
		sendServoPositions();
	}
	
	// Update visualization (compensation is handled in updateKinematics)
	updateKinematics();
	
	// Record positions if enabled
	if(recording) {
		recordedPositions.push_back(ofVec4f(baseX, baseY, tipX, tipY));
	}
}

void ofApp::draw() {
	ofSetColor(255);
	
	// Title in red square
	ofPushStyle();
	ofSetColor(crimson);
	ofSetLineWidth(3);
	ofNoFill();
	ofDrawRectangle(18, 18, 404, 44);
	ofFill();
	ofSetColor(255);
	ofDrawBitmapStringHighlight("H1 TENTACLE TUNING INTERFACE", 25, 45, crimson, ofColor(255));
	ofPopStyle();
	
	// Connection status
	ofSetColor(bSerialConnected ? duskyBlue : duskyOrange);
	string status = bSerialConnected ? "CONNECTED" : "DISCONNECTED";
	ofDrawBitmapString("Serial: " + status, 20, 85);
	
	// GUI
	gui.draw();
	
	// Main visualization area - repositioned to avoid overlap
	ofPushMatrix();
	ofTranslate(450, 120);  // Moved right and adjusted positioning
	
	// Draw position grids in organized layout
	drawPositionGrid();
	
	ofPopMatrix();
	
	// Tentacle visualization - separate area, well below grids
	ofPushMatrix();
	ofTranslate(450, 500);  // Positioned much further below grids
	drawTentacleVisualization();
	ofPopMatrix();
	
	// Compensation curve - positioned to the right
	ofPushMatrix();
	ofTranslate(950, 120);  // Positioned to fit in window
	drawCompensationCurve();
	ofPopMatrix();
	
	// Info text
	ofSetColor(200);
	ofDrawBitmapString("Base XY controls segments 1-4 | Tip XY controls segments 5-8", 20, 900);
	ofDrawBitmapString("Press 'e' to export recorded data | Press 'c' to clear", 20, 920);
	
	// Show compensation debug info
	if(enableCompensation) {
		float baseTilt = baseY - 90.0;
		if(baseTilt < 0) {
			float compensation = -baseTilt * compensationRatio + compensationOffset;
			float actualTipY = ofClamp(tipY + compensation, 0, 180);
			
			ofSetColor(duskyYellow);
			string debugInfo = "COMPENSATION ACTIVE | Base Tilt: " + ofToString(baseTilt, 1) +
							  "° | Compensation: " + ofToString(compensation, 1) +
							  "° | Actual Tip Y: " + ofToString(actualTipY, 1) + "°";
			ofDrawBitmapString(debugInfo, 450, 900);
		} else {
			ofSetColor(100);
			ofDrawBitmapString("COMPENSATION ENABLED (no tilt detected)", 450, 900);
		}
	}
}

void ofApp::updateKinematics() {
	float scale = 1.5;
	segmentPositions[0] = ofVec3f(0, 0, 0);
	
	// Calculate compensated tip Y
	float actualTipY = tipY;
	if(enableCompensation) {
		float baseTilt = baseY - 90.0;
		if(baseTilt < 0) {  // Only when base tilts backward
			float compensation = -baseTilt * compensationRatio + compensationOffset;
			actualTipY = tipY + compensation;
			actualTipY = ofClamp(actualTipY, 0, 180);
		}
	}
	
	for(int i = 1; i <= NUM_SEGMENTS; i++) {
		float baseInfluence = (i <= 4) ? 1.0 - (i-1) * 0.25 : 0.25;
		float tipInfluence = (i > 4) ? (i-4) * 0.25 : 0.0;
		
		float angleX = ofLerp(baseX - 90, tipX - 90, tipInfluence) * DEG_TO_RAD;
		float angleY = ofLerp(baseY - 90, actualTipY - 90, tipInfluence) * DEG_TO_RAD;
		
		float dx = sin(angleX) * SEGMENT_LENGTH * scale;
		float dy = -cos(angleX) * cos(angleY) * SEGMENT_LENGTH * scale;
		float dz = sin(angleY) * SEGMENT_LENGTH * scale * 0.5;
		
		segmentPositions[i] = segmentPositions[i-1] + ofVec3f(dx, dy, dz);
	}
}

void ofApp::updateKinematics(float actualTipY) {
	// This overloaded version is kept for compatibility but not used
	updateKinematics();
}

void ofApp::drawTentacleVisualization() {
	// Red outlined box for tentacle visualization
	ofPushStyle();
	ofSetColor(crimson);
	ofSetLineWidth(2);
	ofNoFill();
	ofDrawRectangle(-10, -40, 420, 300);  // Adjusted size and position
	
	// Title
	ofFill();
	ofSetColor(crimson);
	ofDrawBitmapStringHighlight("TENTACLE PROFILE", 150, -20, ofColor(0, 0, 0, 180), crimson);
	ofPopStyle();
	
	// Center the visualization
	ofPushMatrix();
	ofTranslate(200, 150);  // Adjusted center position
	
	ofSetLineWidth(4);
	
	// Draw segments with thickness variation for 3D effect
	for(int i = 0; i < segmentPositions.size() - 1; i++) {
		float thickness = ofMap(i, 0, NUM_SEGMENTS, 6, 2);  // Tapered thickness
		ofSetLineWidth(thickness);
		
		if(i < 4) {
			ofSetColor(duskyBlue.lerp(crimson, 0.4));
		} else {
			ofSetColor(duskyYellow.lerp(crimson, 0.4));
		}
		
		// Add shadow/depth effect
		ofSetColor(0, 0, 0, 80);
		ofDrawLine(segmentPositions[i].x + 2, segmentPositions[i].y + 2,
				   segmentPositions[i+1].x + 2, segmentPositions[i+1].y + 2);
		
		// Main line
		if(i < 4) {
			ofSetColor(duskyBlue.lerp(crimson, 0.4));
		} else {
			ofSetColor(duskyYellow.lerp(crimson, 0.4));
		}
		ofDrawLine(segmentPositions[i].x, segmentPositions[i].y,
				   segmentPositions[i+1].x, segmentPositions[i+1].y);
	}
	
	// Draw joints
	ofSetColor(crimson);
	for(int i = 0; i < segmentPositions.size(); i++) {
		float size = (i == 0) ? 10 : 6;
		// Shadow
		ofSetColor(0, 0, 0, 100);
		ofDrawCircle(segmentPositions[i].x + 1, segmentPositions[i].y + 1, size);
		// Joint
		ofSetColor(crimson);
		ofDrawCircle(segmentPositions[i].x, segmentPositions[i].y, size);
	}
	
	// Grid lines
	ofSetColor(80);
	ofSetLineWidth(1);
	ofDrawLine(-100, 0, 100, 0);  // Shorter grid lines
	ofDrawLine(0, -100, 0, 100);
	
	// Grid circles for depth reference
	ofNoFill();
	ofSetColor(60);
	for(int r = 50; r <= 100; r += 50) {  // Smaller circles
		ofDrawCircle(0, 0, r);
	}
	
	ofPopMatrix();
}

void ofApp::drawPositionGrid() {
	// Red outlined box for position grids
	ofPushStyle();
	ofSetColor(crimson);
	ofSetLineWidth(2);
	ofNoFill();
	ofDrawRectangle(-10, -30, 420, 240);  // Fixed box dimensions
	
	// Title
	ofFill();
	ofSetColor(crimson);
	ofDrawBitmapStringHighlight("SERVO POSITION CONTROL", 120, -10, ofColor(0, 0, 0, 180), crimson);
	ofPopStyle();
	
	// Base XY Grid
	ofSetColor(50);
	ofSetLineWidth(1);
	ofNoFill();
	ofDrawRectangle(10, 10, 180, 180);
	
	ofSetLineWidth(0.5);
	for(int i = 0; i <= 180; i += 30) {
		ofSetColor(40);
		ofDrawLine(10 + i, 10, 10 + i, 190);
		ofDrawLine(10, 10 + i, 190, 10 + i);
	}
	
	ofFill();
	ofSetColor(duskyBlue);
	ofDrawCircle(10 + baseX, 190 - baseY, 8);
	
	// Tip XY Grid
	ofSetColor(50);
	ofSetLineWidth(1);
	ofNoFill();
	ofDrawRectangle(220, 10, 180, 180);
	
	ofSetLineWidth(0.5);
	for(int i = 0; i <= 180; i += 30) {
		ofSetColor(40);
		ofDrawLine(220 + i, 10, 220 + i, 190);
		ofDrawLine(220, 10 + i, 400, 10 + i);
	}
	
	ofFill();
	ofSetColor(duskyYellow);
	ofDrawCircle(220 + tipX, 190 - tipY, 8);
	
	// Labels
	ofSetColor(crimson);
	ofDrawBitmapStringHighlight("BASE XY", 80, 210, ofColor(0, 0, 0, 180), crimson);
	ofDrawBitmapStringHighlight("TIP XY", 290, 210, ofColor(0, 0, 0, 180), crimson);
}

void ofApp::drawCompensationCurve() {
	// Red outlined box for compensation curve
	ofPushStyle();
	ofSetColor(crimson);
	ofSetLineWidth(2);
	ofNoFill();
	ofDrawRectangle(-10, -30, 320, 260);  // Proper box sizing
	
	// Title
	ofFill();
	ofSetColor(crimson);
	ofDrawBitmapStringHighlight("COMPENSATION CURVE", 80, -10, ofColor(0, 0, 0, 180), crimson);
	ofPopStyle();
	
	// Background
	ofSetColor(30);
	ofFill();
	ofDrawRectangle(0, 0, 300, 200);
	
	// Grid
	ofSetColor(50);
	ofSetLineWidth(1);
	for(int i = 0; i <= 300; i += 50) {
		ofDrawLine(i, 0, i, 200);
	}
	for(int i = 0; i <= 200; i += 40) {
		ofDrawLine(0, i, 300, i);
	}
	
	// Center line
	ofSetColor(100);
	ofSetLineWidth(2);
	ofDrawLine(0, 100, 300, 100);
	
	// Compensation curve
	if(enableCompensation) {
		ofSetColor(crimson);
		ofSetLineWidth(3);
		ofNoFill();  // This is crucial - prevents filled shape
		
		// Draw the actual curve as a line
		for(float baseAngle = 0; baseAngle <= 180; baseAngle += 1) {
			float baseTilt = baseAngle - 90.0;
			float compensation = 0;
			if(baseTilt < 0) {
				compensation = -baseTilt * compensationRatio + compensationOffset;
			}
			float x = ofMap(baseAngle, 0, 180, 0, 300);
			float y = ofMap(compensation, -90, 90, 200, 0);  // Updated range for compensation
			
			// Draw line segments instead of using ofBeginShape
			if(baseAngle > 0) {
				float prevAngle = baseAngle - 1;
				float prevTilt = prevAngle - 90.0;
				float prevComp = 0;
				if(prevTilt < 0) {
					prevComp = -prevTilt * compensationRatio + compensationOffset;
				}
				float prevX = ofMap(prevAngle, 0, 180, 0, 300);
				float prevY = ofMap(prevComp, -90, 90, 200, 0);  // Updated range
				
				ofDrawLine(prevX, prevY, x, y);
			}
		}
		
		// Current position indicator
		ofSetColor(duskyYellow);
		ofFill();
		float currentX = ofMap(baseY, 0, 180, 0, 300);
		float baseTilt = baseY - 90.0;
		float currentComp = (baseTilt < 0) ? -baseTilt * compensationRatio + compensationOffset : 0;
		float currentY = ofMap(currentComp, -90, 90, 200, 0);  // Updated range
		ofDrawCircle(currentX, currentY, 6);
	} else {
		// Show a flat line when compensation is disabled
		ofSetColor(100);
		ofSetLineWidth(2);
		ofNoFill();
		ofDrawLine(0, 100, 300, 100);
	}
	
	// Labels
	ofSetColor(150);
	ofDrawBitmapString("Base Angle", 10, 230);
	ofDrawBitmapString("Compensation", 200, 230);
}

void ofApp::sendServoPositions() {
	// Calculate compensated tip Y for sending to hardware
	float actualTipY = tipY;
	if(enableCompensation) {
		float baseTilt = baseY - 90.0;
		if(baseTilt < 0) {  // Only when base tilts backward
			float compensation = -baseTilt * compensationRatio + compensationOffset;
			actualTipY = tipY + compensation;
			actualTipY = ofClamp(actualTipY, 0, 180);
		}
	}
	
	string message = "bX:" + ofToString((int)baseX) +
					",bY:" + ofToString((int)baseY) +
					",tX:" + ofToString((int)tipX) +
					",tY:" + ofToString((int)actualTipY) + "\n";
	
	serial.writeBytes(message.c_str(), message.length());
}

void ofApp::sendServoPositions(float actualTipY) {
	// This overloaded version is kept for compatibility but not used
	sendServoPositions();
}

void ofApp::exportData() {
	if(recordedPositions.size() > 0) {
		ofFile file;
		file.open("tentacle_calibration_" + ofGetTimestampString() + ".csv", ofFile::WriteOnly);
		file << "baseX,baseY,tipX,tipY\n";
		for(auto& pos : recordedPositions) {
			file << pos.x << "," << pos.y << "," << pos.z << "," << pos.w << "\n";
		}
		file.close();
		ofLogNotice() << "Exported " << recordedPositions.size() << " positions";
	}
}

void ofApp::exit() {
	if(bSerialConnected) {
		serial.close();
	}
}
