#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "pretzel/PretzelGui.h"
#include "KinectManager.h"
#include "OscManager.h"

#include "BlobTracker.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DelqaTrackApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void resize() override;

	OscManagerRef				mOsc;		// this type is declared in OscManager.h
	KinectManagerRef			mKinect;	// this type is declared in KinectManager.h
	Pretzel::PretzelGuiRef		mGui;

	std::shared_ptr<BlobTracker>	mBlobs;
};

void DelqaTrackApp::setup()
{
	mKinect = std::make_shared<KinectManager>();
	mKinect->setup();

	mOsc = std::make_shared<OscManager>();

	mBlobs = std::make_shared<BlobTracker>();

	mGui = Pretzel::PretzelGui::create("Settings", 250, 350);
	mGui->addButton("Capture Bg", &BlobTracker::captureBackground, &*mBlobs);
	mGui->addToggle("Auto equalize hist", &mBlobs->bUseEqualizeHist);
	mGui->addSlider("Manual hist", &mBlobs->multiplyAmt, 0.001, 2.0);
	mGui->addSlider("Threshold near", &mBlobs->minThresh, 0.0f, 255.0f);
	mGui->addSlider("Threshold far", &mBlobs->maxThresh, 0.0f, 255.0f);
	mGui->addSlider("Bg learn rate", &mBlobs->bgLearnRate, 0.f, 1.0);
	mGui->addSlider("Blur Amt", &mBlobs->blurAmt, 0, 10);
	mGui->addSlider("Erode Amt", &mBlobs->mErodeAmt, 0, 10);
	mGui->addSlider("Dilate Amt", &mBlobs->mDilateAmt, 0, 10);
	mGui->addToggle("Use centroid", &mBlobs->bUseCentroid);

	mGui->addSlider("ROI UL", &mBlobs->mRoiUL, vec2(0), vec2(512, 424) );
	mGui->addSlider("ROI BR", &mBlobs->mRoiBR, vec2(0), vec2(512, 424) );

	mGui->addLabel("BLOBS");
	mGui->addSlider("Blob thresh", &mBlobs->mBlobMoveThresh, 0.0, 30.0);
	mGui->addSlider("Blob min area", &mBlobs->minArea, 0.0, 1000.0);

	mGui->addSaveLoad();

	mGui->setPos(vec2(0, 0));
	mGui->loadSettings();

	mOsc->reconnectSender();
}

void DelqaTrackApp::mouseDown( MouseEvent event )
{
}

void DelqaTrackApp::update()
{
	mBlobs->update(mKinect->getDepthChannel());
	mOsc->sendTouch(mBlobs->getTouchPosition());
}

void DelqaTrackApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	gl::pushMatrices(); {
		gl::translate(270, 0);
		//mKinect->draw();
		mBlobs->draw();
	}gl::popMatrices();

	gl::pushMatrices(); {
		gl::translate(0, getWindowHeight() - 250);
		gl::draw( gl::Texture::create( mBlobs->getThumbnail() ) );
	}gl::popMatrices();


	mGui->draw();
	mOsc->draw();
}

void DelqaTrackApp::resize()
{
	mGui->setPos(vec2(0, 0));
	mGui->setSize(vec2(250, getWindowHeight() - 250) );
}

CINDER_APP(DelqaTrackApp, RendererGl, [&](App::Settings *settings){
	settings->setWindowSize(1024, 768); 
	settings->setFrameRate(60.0);
})