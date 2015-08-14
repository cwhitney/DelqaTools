#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "Kinect2.h"
#include "CinderOpenCV.h"
#include "Blob.h"

class BlobTracker
{
public:
	BlobTracker();
	
	void update(ci::Channel16uRef depth);
	void draw();

	void captureBackground();
	Surface8u getThumbnail();

	vec3 getTouchPosition();

	// EXPOSED TO GUI
	bool	bUseEqualizeHist;
	float	multiplyAmt;
	float	bgLearnRate;
	float	blurAmt;
	int		mErodeAmt;
	int		mDilateAmt;
	float	mBlobMoveThresh;
	float	minArea;
	float	minThresh;
	float	maxThresh;
	bool	bUseCentroid;

	vec2	mRoiUL, mRoiBR;

  protected:
	void findBlobs(const cv::Mat &blobMat);
	int forceOdd(const int &val);
	void calcTouchPosition();

	// ---------------- RAW DEPTH
	ci::Channel16uRef	mChannelDepth;
	ci::Channel8uRef	mChannelDepth8u;

	// ---------------- OPEN CV
	cv::Mat				mInput;
	cv::Mat				mOutput;
	cv::Mat				mRoiMat;

	cv::Mat				mThumbMat;

	cv::Mat				mBgBuffer;

	// ----------------
	bool				bFirstRun;

	// ---------------- BLOBS
	typedef vector< vector<cv::Point> > ContourVector;
	ContourVector   mContours;
	vector<Blob>    mBlobs;
	int				mLargestBlob;
	vec2			mSamplePos;

	vec3			mTouchPos;
};

