#include "..\include\KinectManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

KinectManager::KinectManager()
{
}

void KinectManager::setup()
{
	// Init Kinect
	mDevice = Kinect2::Device::create();
	mDevice->start();

	// Callback lambda
	mDevice->connectDepthEventHandler([&](const Kinect2::DepthFrame& frame)
	{
		mChannelDepth = frame.getChannel();
	});
}

void KinectManager::update()
{

}

void KinectManager::draw()
{
	if (mChannelDepth){
		gl::draw( gl::Texture::create( *Kinect2::channel16To8( mChannelDepth ) ) );
	}	
}

void KinectManager::cleanup()
{
	mDevice->disconnectDepthEventHandler();
	mDevice->stop();

	console() << "KinectManager :: Cleaning up!" << endl;
}