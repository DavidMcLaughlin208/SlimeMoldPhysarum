#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex();
	ofSeedRandom();

	width = ofGetWidth();
	height = ofGetHeight();

	
	gui.setup();
	gui.add(steerSlider.setup("Steer", 0.38, 0, 2));
	gui.add(angleSlider.setup("Angle", 0.4, 0, 2));
	gui.add(senseDistanceSlider.setup("SenseDistance", 19.75, 1, 100));
	gui.add(decayRateSlider.setup("Decay", 0.27, 0.0, 1.0));
	gui.add(diffuseRateSlider.setup("Diffuse", 0.6, 0, 1));
	gui.add(speedSlider.setup("Speed", 1.0, 0, 5));
	gui.add(maxTrailDensitySlider.setup("Max Trail Density", 2, 0, 50));
	gui.add(sensorSizeSlider.setup("Sensor Size", 1, 0, 4));
	gui.add(speedAffectedByTrailDensityToggle.setup("speedAffectedByTrailDensity"));
	speedAffectedByTrailDensityToggle = false;

	color.setHsb(239, 100, 68);
	ofParameter<ofColor> param;
	param.set(color);
	
	gui.add(minColorSlider.setup("Min Color", param, 100, 255));
	color = ofColor::white;
	param.set(color);
	gui.add(maxColorSlider.setup("Max Color", param, 100, 255));
	ofSetWindowTitle("Slime Mold");

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
	ofVec2f centerPoint = ofVec2f(width / 2, height / 2);
	centerPoint.normalize();
	int framing = 200;
	for (auto& p : particleSize) {
		p.pos.x = ofRandom(width / 2 - framing, width / 2 + framing);
		p.pos.y = ofRandom(height / 2 - framing, height / 2 + framing);
		p.pos.z = 1;
		p.pos.w = 1;
		/*ofVec2f point = ofVec2f(p.pos.x, p.pos.y);
		point.normalize();
		p.angle.r = point.angle(centerPoint);*/
		p.angle.r = ofRandom(0, 359);
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
	fragShader.setUniform1f("xRatio", float(ofGetWidth()) / float(width));
	fragShader.setUniform1f("yRatio", float(ofGetHeight()) / float(height));
	fragShader.setUniform4f("minColor", minColor);
	fragShader.setUniform4f("maxColor", maxColor);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	fragShader.end();
	
	gui.draw();
}

void ofApp::setParameters() {
	steerStrength = steerSlider;
	senseAngle = angleSlider;
	senseDistance = senseDistanceSlider;
	diffuseRate = diffuseRateSlider;
	decayRate = decayRateSlider;
	speed = speedSlider;
	maxTrailDensity = maxTrailDensitySlider;
	speedAffectedByTrailDensity = speedAffectedByTrailDensityToggle;
	sensorSize = sensorSizeSlider;
	minColor = minColorSlider;
	maxColor = maxColorSlider;
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

