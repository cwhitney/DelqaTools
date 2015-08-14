#include "..\include\OscManager.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

OscManager::OscManager() :
	mOscHost("127.0.0.1"),
	mOscPortStr("8000"),
	bConnected(false)
{
	mGui = Pretzel::PretzelGui::create("OSC Settings", 220, 200);
	mGui->addTextField("Send Host", &mOscHost);
	mGui->addTextField("Send Port", &mOscPortStr);
	mGui->addButton("Reconnect Sender", &OscManager::reconnectSender, this );
	mGui->setPos(vec2(0, 420));

	getWindow()->getSignalResize().connect([&](){
		mGui->setPos( vec2(getWindowWidth()-220, 0) );
	});

	reconnectSender();
}

void OscManager::reconnectSender()
{
	int oscPort = fromString<int>( mOscPortStr );
	mSender.setup( mOscHost, oscPort );

	try{
		int oscPort = fromString<int>(mOscPortStr);
		mSender.setup(mOscHost, oscPort);
		bConnected = true;
		CI_LOG_I("Osc connected ok");
	}
	catch (std::exception e){
		CI_LOG_EXCEPTION("OscManager :: reconnect sender", e);
		CI_LOG_E("Error :: OscManager::reconnect sender");
		CI_LOG_E(e.what());
		bConnected = false;
	}
}

void OscManager::sendTouch( vec3 touchPos )
{
	if (bConnected){
		osc::Message msgX;
		msgX.setAddress("/touch/x");
		msgX.addFloatArg(touchPos.x);

		osc::Message msgY;
		msgY.setAddress("/touch/y");
		msgY.addFloatArg(touchPos.y);

		osc::Message msgZ;
		msgZ.setAddress("/touch/z");
		msgZ.addFloatArg(touchPos.z);

		mSender.sendMessage(msgX);
		mSender.sendMessage(msgY);
		mSender.sendMessage(msgZ);
	}
}

void OscManager::draw()
{
	mGui->draw();
}