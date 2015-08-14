#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Blob {
  public:
	Blob() :
		mColor( Rand::randFloat(), Rand::randFloat(), Rand::randFloat() ),
		timestamp(0),
		age(0),
		bAlive(false)
		{}
		
	// --------- SET
	void setPolyLine(const PolyLine2f &poly)	{ mPolyLine = poly; };

	void setCenter(const vec2 center)
	{
		mCenterTar = center;

		if (mCenterCur == vec2(0)){
			mCenterCur = mCenterTar;
		}else {
			mCenterCur += (mCenterTar - mCenterCur) * 0.2f;
		}

		mCenter = center;
	};

	// -------- GET
	vec2 getCenter()			{ return mCenter;		}
	vec2 getCenterSmoothed()	{ return mCenterCur;	}


	float getDist(vec2 p)			{ return glm::distance(mCenter, p); };
	float getDistSmoothed(vec2 p)	{ return glm::distance(mCenterCur, p);	};

	PolyLine2f getPolyLine()	{ return mPolyLine; };
	ci::Color getColor()		{ return mColor;	};

	// --------- EXPOSED VARS
	Rectf   containingRect;
	bool    bAlive;
	int		age;
	bool	bIsFalling;
	int     timestamp;

protected:
	vec2				mCenterCur, mCenterTar;
	vec2           		mCenter;
	PolyLine2f          mPolyLine;
	Color               mColor;
};