#pragma once

#include "cinder/app/App.h"

#include "OscSender.h"
#include "pretzel\PretzelGui.h"

class OscManager;
typedef std::shared_ptr<OscManager> OscManagerRef;

class OscManager
{
public:
	OscManager();
	
	void draw();

	void sendTouch( ci::vec3 touchPos );
	void reconnectSender();

protected:
	ci::osc::Sender				mSender;
	Pretzel::PretzelGuiRef		mGui;

	std::string					mOscHost, mOscPortStr;
	bool						bConnected;
};

