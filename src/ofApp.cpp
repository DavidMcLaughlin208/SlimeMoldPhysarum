#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex();
	ofSeedRandom();

	width = ofGetWidth() - guiWidth;
	height = ofGetHeight();

	
	gui.setup();
	gui.add(steerSlider.setup("Steer", 0.6, 0, 2));
	gui.add(angleSlider.setup("Angle", 0.4, 0, 2));
	gui.add(senseDistanceSlider.setup("SenseDistance", 20, 1, 100));
	gui.add(decayRateSlider.setup("Decay", 0.27, 0.0, 1.0));
	gui.add(diffuseRateSlider.setup("Diffuse", 0.3, 0, 1));
	gui.add(speedSlider.setup("Speed", 0.1, 0, 5));
	gui.add(maxTrailDensitySlider.setup("Max Trail Density", 7.5, 0, 50));
	gui.add(sensorSizeSlider.setup("Sensor Size", 1, 0, 4));
	gui.add(speedAffectedByTrailDensityToggle.setup("speedAffectedByTrailDensity"));
	speedAffectedByTrailDensityToggle = false;

	
	ofParameter<ofColor> param;
	color.setHsb(239, 100, 200);
	param.set(color);
	gui.add(teamColor1Slider.setup("Team Color 1", param, 100, 255));
	color.setHsb(100, 100, 200);
	param.set(color);
	gui.add(teamColor2Slider.setup("Team Color 2", param, 100, 255));
	color.setHsb(50, 100, 200);
	param.set(color);
	gui.add(teamColor3Slider.setup("Team Color 3", param, 100, 255));
	color.setHsb(176, 100, 200);
	param.set(color);
	gui.add(teamColor4Slider.setup("Team Color 4", param, 100, 255));
	color = ofColor::black;
	param.set(color);
	gui.add(baseColorSlider.setup("BaseColor", param, 100, 255));
	ofSetWindowTitle("Slime Mold");
	gui.setPosition(ofGetWidth() - guiWidth, 10);

	// Load and link shaders
	fragShader.load("shadersGL3/shader");
	fragShader.linkProgram();
	trailMapComputeShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shadersGL3/trailMapShader.compute");
	trailMapComputeShader.linkProgram();
	particleComputeShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shadersGL3/particleShader.compute");
	particleComputeShader.linkProgram();

	ofBackground(0);
	ofEnableBlendMode(OF_BLENDMODE_ADD);

	// Initialize and allocate buffers
	trailMapSize.resize(width * height);
	for (int i = 0; i < trailMapSize.size(); i++) {
		auto & c = trailMapSize[i];
		c.val.r = 0;
		c.val.g = 0;
		c.val.b = 0;
		c.val.a = 0;
	}
	doubleBufferedTrailMap.allocate(trailMapSize);
	particleSize.resize(1000000);
	int radius = 300;
	ofVec2f centerPoint = ofVec2f(width / 2, height / 2);
	int team1Count = 0;
	int team2Count = 0;
	int team3Count = 0;
	int team4Count = 0;
	for (auto& p : particleSize) {
		int team = (int) ofRandom(0, numTeams);

		float distance = ofRandom(0, radius);
		float angle = ofRandom(0, 359);
		ofVec2f dir = ofVec2f(cos(angle), sin(angle));
		ofVec2f point = centerPoint + dir * distance;

		p.pos.x = point.x;
		p.pos.y = point.y;
		p.pos.z = (180 + angle);

		p.speciesMask.r = team == 0;
		p.speciesMask.g = team == 1;
		p.speciesMask.b = team == 2;
		p.speciesMask.a = team > 2;
	}
	particleBuffer.allocate(particleSize, GL_DYNAMIC_DRAW);

	// Bind buffers for use in shaders
	particleBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	doubleBufferedTrailMap.src->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	doubleBufferedTrailMap.dst->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle(ofToString(ofGetFrameRate()));

	setParameters();

	// Rebind buffers since the pointers for these were swapped last frame
	doubleBufferedTrailMap.swap();
	doubleBufferedTrailMap.src->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	doubleBufferedTrailMap.dst->bindBase(GL_SHADER_STORAGE_BUFFER, 2);

	// Update particle location and modify trail map
	particleComputeShader.begin();
	particleComputeShader.setUniform1i("screenWidth", width);
	particleComputeShader.setUniform1i("screenHeight", height);
	particleComputeShader.setUniform1f("senseAngle", senseAngle);
	particleComputeShader.setUniform1f("steerStrength", steerStrength);
	particleComputeShader.setUniform1f("senseDistance", senseDistance);
	particleComputeShader.setUniform1f("maxTrailDensity", maxTrailDensity);
	particleComputeShader.setUniform1f("speed", speed);
	particleComputeShader.setUniform1f("deltaTime", 1.0);
	particleComputeShader.setUniform1i("sensorSize", sensorSize);
	particleComputeShader.setUniform1i("speedAffectedByTrailDensity", speedAffectedByTrailDensity);
	particleComputeShader.dispatchCompute((particleSize.size() + 1024 - 1) / 1024, 1, 1);
	particleComputeShader.end();

	// Perform diffusion and decay on trail map
	trailMapComputeShader.begin();
	trailMapComputeShader.setUniform1i("screenWidth", width);
	trailMapComputeShader.setUniform1i("screenHeight", height);
	trailMapComputeShader.setUniform1f("diffuseRate", diffuseRate);
	trailMapComputeShader.setUniform1f("decayRate", decayRate);
	trailMapComputeShader.dispatchCompute(height * width / 100 , 1, 1);
	trailMapComputeShader.end();

	
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Use fragment shader to draw a rectangle over the whole screen
	// Color the pixels based on the data in the trail map buffer
	// auto modifiedTrailBuffer = doubleBufferedTrailMap.dst->map<Cell>(GL_READ_WRITE);
	fragShader.begin();
	fragShader.setUniform1i("screenWidth", width);
	fragShader.setUniform1i("screenHeight", height);
	fragShader.setUniform1f("decay", decayRate);
	fragShader.setUniform1f("maxTrailDensity", maxTrailDensity);
	fragShader.setUniform1f("xRatio", float(ofGetWidth() - guiWidth) / float(width));
	fragShader.setUniform1f("yRatio", float(ofGetHeight()) / float(height));
	fragShader.setUniform4f("teamColor1", teamColor1);
	fragShader.setUniform4f("teamColor2", teamColor2);
	fragShader.setUniform4f("teamColor3", teamColor3);
	fragShader.setUniform4f("teamColor4", teamColor4);
	fragShader.setUniform4f("baseColor", baseColor);
	ofDrawRectangle(0, 0, ofGetWidth() - guiWidth, ofGetHeight());
	fragShader.end();
	
	gui.draw();
}

void ofApp::setParameters() {
	gui.setPosition(ofGetWidth() - guiWidth, 10);
	steerStrength = steerSlider;
	senseAngle = angleSlider;
	senseDistance = senseDistanceSlider;
	diffuseRate = diffuseRateSlider;
	decayRate = decayRateSlider;
	speed = speedSlider;
	maxTrailDensity = maxTrailDensitySlider;
	speedAffectedByTrailDensity = speedAffectedByTrailDensityToggle;
	sensorSize = sensorSizeSlider;
	teamColor1 = teamColor1Slider;
	teamColor2 = teamColor2Slider;
	teamColor3 = teamColor3Slider;
	teamColor4 = teamColor4Slider;
	baseColor = baseColorSlider;
	/*double lastFrameTime = ofGetLastFrameTime() * 1000000;
	double currentTime = ofGetCurrentTime().getAsMilliseconds();

	deltaTime = (currentTime - lastFrameTime) / 1000000000.0;*/
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'r') {
		
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

