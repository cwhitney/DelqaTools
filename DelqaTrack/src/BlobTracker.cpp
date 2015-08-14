#include "..\include\BlobTracker.h"


BlobTracker::BlobTracker() :
bFirstRun(true),
bUseEqualizeHist(false),
multiplyAmt(1.0),
bgLearnRate(0.0),
blurAmt(0.0),
mErodeAmt(1),
mDilateAmt(0),
mBlobMoveThresh(5.0),
minArea(20.0),
minThresh(60),
maxThresh(255),
bUseCentroid(false),
mLargestBlob(0),
mRoiUL(0,0),
mRoiBR(512, 424)
{
}

Surface8u BlobTracker::getThumbnail()
{ 
	if ( cv::countNonZero(mThumbMat) > 0){
		return fromOcv(mThumbMat);
	}
	return Surface8u();
}

void BlobTracker::update(ci::Channel16uRef depth)
{
	if (!depth){
		return;
	}

	mChannelDepth = depth;
	mChannelDepth8u = Kinect2::channel16To8(mChannelDepth);

	mInput = toOcv(*mChannelDepth8u.get());

	if (bFirstRun){
		captureBackground();
		bFirstRun = false;
	}

	cv::Mat	wc;		// working copy Mat
	mInput.copyTo(wc);

	// THUMBNAIL
	cv::Size hSize(250, 250);
	cv::resize(wc, mThumbMat, hSize);
	cv::equalizeHist(mThumbMat, mThumbMat);

	// ------- BG LEARN
	if (bgLearnRate > 0.0f){
		float alpha = math<float>::clamp(bgLearnRate);
		float beta = 1.0f - alpha;
		cv::addWeighted(wc, alpha, mBgBuffer, beta, 0.0, mBgBuffer);
	}

	// DIFF
	cv::subtract(mBgBuffer, wc, wc);

	// THRESHOLD
	cv::threshold(wc, wc, minThresh, 255, cv::THRESH_TOZERO);

	// CONTRAST --------------------------------------------------------------
	if (bUseEqualizeHist){
		cv::equalizeHist(wc, wc);
	}
	else {
		cv::multiply(wc, wc, wc, multiplyAmt);
	}

	// ERODE
	if (mErodeAmt > 0){
		cv::erode(wc, wc, cv::Mat(), cv::Point(-1, -1), mErodeAmt);
	}
	if (mDilateAmt > 0){
		cv::dilate(wc, wc, cv::Mat(), cv::Point(-1, -1), mDilateAmt);
	}
	

	// SHRINK + BLUR
	bool bUseShrink = true;
	if (bUseShrink){
		cv::Size oSize = mInput.size();
		cv::Size hSize(oSize.width / 2.0f, oSize.height / 2.0f);

		cv::Mat shrunk;
		cv::resize(wc, shrunk, hSize);

		blurAmt = forceOdd(blurAmt);
		if (blurAmt > 0.0f){
			cv::medianBlur(shrunk, shrunk, blurAmt);
		}
		shrunk.copyTo(wc);
		cv::resize(wc, wc, oSize);
	}


	wc.copyTo(mOutput);
	findBlobs(mOutput);
	calcTouchPosition();
}

void BlobTracker::draw()
{
	if (mChannelDepth){
		auto sba = gl::ScopedBlendAlpha();

		// ------- BLOBS
		{
			auto sc = gl::ScopedColor(1, 1, 1);
			gl::draw(gl::Texture::create(fromOcv(mOutput)));

			auto sc2 = gl::ScopedColor(1, 0, 1, 1.0);
			int i = 0;
			for (auto it = mBlobs.begin(); it != mBlobs.end(); ++it){
				gl::drawStrokedRect(it->containingRect);
				//gl::draw(it->getPolyLine());
				if (i == mLargestBlob){
					gl::ScopedColor sc2(1, 0, 0, 1.0);
					gl::drawSolidCircle(it->getCenter(), 3, 10);
				}
				++i;
			}
		}
		{
			gl::ScopedColor tc1(0.3, 0.3, 0.3);
			gl::drawStrokedRect(Rectf(0, 0, 512, 424));

			gl::ScopedColor tc(0, 1, 1);
			gl::drawStrokedRect(Rectf(mRoiUL, mRoiBR));
		}
		// ----------- CENTROID
		if (mBlobs.size()){
			vec2 center = mBlobs[mLargestBlob].getCenterSmoothed();
			{
				gl::ScopedColor sc2(1, 0, 1, 0.3);
				gl::drawSolidCircle(mSamplePos, 7, 10);
				gl::ScopedColor sc3(1, 0, 1, 1.0);
				gl::drawStrokedCircle(mSamplePos, 7, 10);
			}
		}
	}
}

void BlobTracker::captureBackground()
{
	mBgBuffer = cv::Scalar(0, 0, 0);
	mInput.copyTo(mBgBuffer);
}

void BlobTracker::findBlobs(const cv::Mat &blobMat)
{
	cv::Mat blobs;
	cv::Rect roiRect(mRoiUL.x, mRoiUL.y, mRoiBR.x - mRoiUL.x, mRoiBR.y - mRoiUL.y );
	blobMat.copyTo(blobs);

	blobs = blobs(roiRect);

	mContours.clear();
	cv::findContours(blobs, mContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// ----------- ANALYZE
	for (ContourVector::const_iterator iter = mContours.begin(); iter != mContours.end(); ++iter)
	{
		vector<vec2> mPoints;
		PolyLine2f pl;
		pl.setClosed();
		vec2 tl = (vec2)fromOcv(*(iter->begin())) + mRoiUL;
		vec2 br = (vec2)fromOcv(*(iter->begin())) + mRoiUL;
		vec2 centroid;

		// FORM POLYLINE AND FIND CENTROID
		for (vector<cv::Point>::const_iterator pt = iter->begin(); pt != iter->end(); ++pt) {
			vec2 p( fromOcv(*pt) );
			p += mRoiUL;	// account for the fact that we might not be using the whole frame for detection

			mPoints.push_back(p);
			pl.push_back(p);

			centroid += p;

			if (p.x < tl.x) tl.x = p.x;
			if (p.y < tl.y) tl.y = p.y;

			if (p.x > br.x) br.x = p.x;
			if (p.y > br.y) br.y = p.y;
		}
		centroid = centroid / vec2(pl.size());

		// ------ check if it matches an existing blob
		bool bFound = false;
		for (auto it = mBlobs.begin(); it != mBlobs.end(); it++){
			if (it->getDist(centroid) < mBlobMoveThresh || it->getDistSmoothed(centroid) < mBlobMoveThresh){

				it->setCenter(centroid);
				it->setPolyLine(pl);
				it->containingRect = Rectf(tl, br);

				bFound = true;
				it->bAlive = true;
				it->age++;
				//	it->setDepth(blobDepth);
				break;
			}
		}
		// ------- Otherwise make a new blob
		if (!bFound){
			Rectf newArea(tl, br);

			if (newArea.calcArea() > minArea*minArea){	// only add it if its large enough
				Blob b;
				b.setCenter(centroid);
				b.setPolyLine(pl);
				b.containingRect = Rectf(tl, br);
				b.bAlive = true;
				b.age = 0;
				//	b.setDepth(blobDepth);

				mBlobs.push_back(b);
			}
		}

		Rectf containingRect(tl, br);
	}

	// delete updated blobs
	for (vector<Blob>::iterator it = mBlobs.begin(); it != mBlobs.end();){
		if (it->bAlive == false){
			it = mBlobs.erase(it);
		}
		else{
			it->bAlive = false;	// this will be set true in the loop above next round of updates
			it++;
		}
	}
}

vec3 BlobTracker::getTouchPosition()
{
	return mTouchPos;
}

void BlobTracker::calcTouchPosition()
{
	if (mBlobs.size()){

		// Pick the biggest blob to track
		mLargestBlob = 0;
		float la = 0.0;
		for (int i = 0; i < mBlobs.size(); i++){
			if (mBlobs[i].containingRect.calcArea() > la){
				la = mBlobs[i].containingRect.calcArea();
				mLargestBlob = i;
			}
		}


		ci::Rectf cr = mBlobs[mLargestBlob].containingRect;
		cv::Rect roiRect(cr.x1, cr.y1, cr.getWidth(), cr.getHeight());

		// GET ROI
		mRoiMat = mOutput(roiRect);

		// MIN MAX LOC
		double minVal;
		double maxVal;
		cv::Point minLoc;
		cv::Point maxLoc;
		cv::minMaxLoc(mRoiMat, &minVal, &maxVal, &minLoc, &maxLoc);

		vec2 touchPos;

		if (bUseCentroid){
			touchPos = mBlobs[mLargestBlob].getCenter();
		}
		else {
			touchPos = (vec2)fromOcv(maxLoc) + cr.getUpperLeft();
		}

		mSamplePos = touchPos;

		Surface8u sf(*mChannelDepth8u.get());
		float sqArea = sqrt(mBlobs[mLargestBlob].containingRect.calcArea());

		vec3 ret(touchPos, sqArea);

		// Remap and normalize our measurements inside of the rectangle we've defined
		Rectf roi(mRoiUL, mRoiBR);
		mTouchPos.x = lmap( ret.x, roi.x1, roi.x2, 0.0f, 1.0f );
		mTouchPos.y = lmap( ret.y, roi.y1, roi.y2, 0.0f, 1.0f );
		mTouchPos.z = lmap( ret.z, 0.0f, sqrt(roi.calcArea()), 0.0f, 1.0f );
	}
	else{
		// if we didn't find anything
		mTouchPos = vec3(0);
	}
}

int BlobTracker::forceOdd(const int &val)
{
	if (val > 0){
		if ((val % 2) == 0){
			return std::max(val - 1, 0);
		}
		return val;
	}
	return 0;
}