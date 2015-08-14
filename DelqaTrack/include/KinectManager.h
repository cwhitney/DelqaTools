#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "Kinect2.h"

class KinectManager;
typedef std::shared_ptr<KinectManager> KinectManagerRef;

class KinectManager {
  public:
	KinectManager();

	void setup();
	void update();
	void draw();

	void cleanup();

	ci::Channel16uRef getDepthChannel(){ return mChannelDepth; }

  protected:
	Kinect2::DeviceRef			mDevice;

	ci::Channel8uRef			mChannelBodyIndex;
	ci::Channel16uRef			mChannelDepth;
	ci::Channel16uRef			mChannelInfrared;
	ci::Surface8uRef			mSurfaceColor;
};

