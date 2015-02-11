#ifndef ARLIB_VIDEOPLAYER_H
#define ARLIB_VIDEOPLAYER_H

namespace ARLib {

class CCapture;
struct ocam_model;

class VideoPlayer
{
public:
    VideoPlayer(
		int cameraNumber = 0,
		const char *ocamModelParametersFilename = NULL, float videoDistance = 4.0f,
		const char *homographyMatrixFilename = NULL);
    ~VideoPlayer();
	/// Returns the width of the video image
	int getVideoWidth();
	/// Returns the height of the video image
	int getVideoHeight();
	// Returns a pointer to the current video frame memory in the BGR format
	// or NULL if there is no new frame
	void * update();
	// Precalculates a coordinate mapping for undistortion
	void calculateUndistortionMap(float *xyMap);

private:
	CCapture *capture;
	float videoDistance;
	ocam_model *ocamModel;
	float homographyMatrix[3 * 3];
};

}; // ARLib namespace

#endif
