#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex();

	color.setHsb(23, 200, 200);
	gui.setup();
	gui.add(radius.setup("radius", 140, 10, 300));

	ofParameter<ofColor> param;
	param.set(color);
	
	gui.add(colorSlider.setup("Blended Color", param, 100, 255));
	ofSetCircleResolution(50);
	ofSetWindowTitle("Slime Mold");

	// Load and link shaders
	fragShader.load("shadersGL3/shader");
	fragShader.linkProgram();
	computeShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shadersGL3/shader.compute");
	computeShader.linkProgram();
	particleComputeShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shadersGL3/particleShader.compute");
	particleComputeShader.linkProgram();

	ofBackground(0);
	ofEnableBlendMode(OF_BLENDMODE_ADD);

	// Initialize and allocate buffers
	trailMapSize.resize(ofGetWidth() * ofGetHeight());
	for (int i = 0; i < trailMapSize.size(); i++) {
		auto & c = trailMapSize[i];
		c.val.r = 0;
		c.val.g = 0;
		c.val.b = 0;
		c.val.a = 0;
	}
	doubleBufferedTrailMap.allocate(trailMapSize);
	particleSize.resize(1024);
	for (auto& p : particleSize) {
		p.pos.x = ofRandom(0, ofGetWidth());
		p.pos.y = ofRandom(0, ofGetHeight());
		p.pos.z = 1;
		p.pos.w = 1;
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

	// Rebind buffers since the pointers for these were swapped last frame
	doubleBufferedTrailMap.src->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	doubleBufferedTrailMap.dst->bindBase(GL_SHADER_STORAGE_BUFFER, 2);

	// Update particle location and modify trail map
	particleComputeShader.begin();
	particleComputeShader.setUniform1i("screenWidth", ofGetWidth());
	particleComputeShader.setUniform1i("screenHeight", ofGetHeight());
	particleComputeShader.dispatchCompute(particleSize.size(), 1, 1);
	particleComputeShader.end();

	// Perform diffusion and decay on trail map
	computeShader.begin();
	computeShader.setUniform1i("screenWidth", ofGetWidth());
	computeShader.setUniform1i("screenHeight", ofGetHeight());
	computeShader.dispatchCompute(1, 1, 1);
	computeShader.end();

	doubleBufferedTrailMap.swap();
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Use fragment shader to draw a rectangle over the whole screen
	// Color the pixels based on the data in the trail map buffer
	auto modifiedTrailBuffer = doubleBufferedTrailMap.dst->map<Cell>(GL_READ_WRITE);
	fragShader.begin();
	fragShader.setUniform1i("screenWidth", ofGetWidth());
	fragShader.setUniform1i("screenHeight", ofGetHeight());
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	fragShader.end();
	
	//gui.draw();
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

