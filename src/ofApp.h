#pragma once

#include "ofMain.h"
#include "ofxGui.h"



class ofApp : public ofBaseApp{

	public:

		struct Particle {
			glm::vec4 pos;
			glm::vec4 angle;
		};

		struct Cell {
			glm::vec4 val;
		};

		struct pingPongBuffer {
		public:
			void allocate(vector<Cell> data) {
				// Allocate
				for (int i = 0; i < 2; i++) {
					bufferObjects[i].allocate(data, GL_DYNAMIC_DRAW);
				}

				//Assign
				src = &bufferObjects[0];
				dst = &bufferObjects[1];
			}

			void swap() {
				std::swap(src, dst);
			}

			ofBufferObject& operator[](int n) { return bufferObjects[n]; }
			ofBufferObject* src;       // Source       ->  Ping
			ofBufferObject* dst;       // Destination  ->  Pong

		private:
			ofBufferObject   bufferObjects[2];    // Real addresses of ping/pong ofBufferObjects
		};

		void setup();
		void update();
		void draw();
		void setParameters();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		ofxPanel gui;
		ofShader fragShader;
		ofShader trailMapComputeShader;
		ofShader particleComputeShader;
		ofxFloatSlider steerSlider;
		ofxFloatSlider angleSlider;
		ofxFloatSlider senseDistanceSlider;
		ofxFloatSlider diffuseRateSlider;
		ofxFloatSlider decayRateSlider;
		ofxFloatSlider speedSlider;
		ofxFloatSlider maxTrailDensitySlider;
		ofxIntSlider sensorSizeSlider;
		ofxToggle speedAffectedByTrailDensityToggle;
		ofxColorSlider minColorSlider;
		ofxColorSlider maxColorSlider;
		ofColor color;
		pingPongBuffer doubleBufferedTrailMap;
		vector<Cell> trailMapSize;
		ofBufferObject particleBuffer;
		vector<Particle> particleSize;

		float steerStrength = 6.0;
		float senseAngle = 6.0;
		float senseDistance = 6.0;
		float decayRate = 0.01;
		float diffuseRate = 0.6;
		float speed = 1;
		float maxTrailDensity = 50.0;
		int sensorSize = 1;
		bool speedAffectedByTrailDensity = false;
		ofColor minColor;
		ofColor maxColor;

		int width;
		int height;
};
