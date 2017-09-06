#pragma once
// opencv 3.2.0 
//#include "opencv2/videoio.hpp"

// directshow
#include "cap_dshow.hpp"
#include "opencv2/highgui.hpp"
#include <Dshow.h>


class StereoCamera : public cv::IVideoCapture
{
public:
	StereoCamera();
	virtual ~StereoCamera();

	virtual double getProperty(int property_index) const;
	virtual bool setProperty(int property_index, double property_value);

	virtual bool grabFrame();
	
	virtual bool retrieveFrame(int outputType, cv::OutputArray frame);
	
	virtual int getCaptureDomain();
	virtual bool isOpened() const;
	
	bool retrieveFrame(unsigned char *pFrame);
	int m_width_set, m_height_set;
	MyVideoInput *pgVI;

private:
	
	const int desired_width;
	const int desired_height;
	int m_index, m_width, m_height, m_fourcc;
	
	
	bool IsOpen;
	
	void OpenCamera();
	void CloseCamera();
	void ComInit();
	void ComUnInit();
};
