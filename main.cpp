#include "main.h"


int main()
{
	DebugPrintOut("unsingned char size: %d", sizeof(char));
	StereoCamera *SteCam = new StereoCamera ();

	/*// Print-Info
	if (!SteCam->isOpened())
	{
		printf ( " Critical ERROR : Camera Start Failed !!!\n");
	}
	else 
	{
		printf("Width : %f, \tHeight : %f \n", SteCam->getProperty(CV_CAP_PROP_FRAME_WIDTH), SteCam->getProperty(CV_CAP_PROP_FRAME_HEIGHT));
		printf("FOURCC : %f,\tFPS : %f \n", SteCam->getProperty(CV_CAP_PROP_FOURCC), SteCam->getProperty(CV_CAP_PROP_FPS));
		printf("Brightness : %f,\tContrast : %f \n", SteCam->getProperty(CV_CAP_PROP_BRIGHTNESS), SteCam->getProperty(CV_CAP_PROP_CONTRAST));
		printf("Gain : %f\n", SteCam->getProperty(CV_CAP_PROP_GAIN));
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	cv::namedWindow("SteCam", CV_WINDOW_NORMAL);
	cv::resizeWindow("SteCam",STECAM_WIDTH,0.5*STECAM_HEIGHT);
	

	while (1)
	{
		//SteCam->grabFrame();
		if (SteCam->retrieveFrame(SteCam->pgVI->pOutputFrame))
		{
			cv::Mat OutputMat(SteCam->m_height_set, 2*SteCam->m_width_set,CV_8UC3,SteCam->pgVI->pOutputFrame);

			cv::imshow("SteCam", OutputMat);
		}

		char ch = cv::waitKey(10);
		if (ch == 27) break;		// 27 == ESC key
		if (ch == 32)					// 32 == SPACE key
		{
			while ((ch = cv::waitKey(10)) != 32 && ch != 27);
			if (ch == 27) break;
		}

	}

	
	delete SteCam;
	return 0;
	};


StereoCamera::StereoCamera()  
	: pgVI{ nullptr },
		desired_width{ STECAM_WIDTH }, desired_height{ STECAM_HEIGHT },
		m_index{ -1 }, m_width{ -1 }, m_height{ -1 }, m_fourcc{ 0 },
		m_width_set{ -1 }, m_height_set{ -1 }
{
	pgVI = new MyVideoInput()	;
	

	ComInit();
	this->OpenCamera();
}

StereoCamera::~StereoCamera()
{
	this->CloseCamera();
	ComUnInit();
}

double StereoCamera::getProperty(int propIdx) const
{

	long min_value, max_value, stepping_delta, current_value, flags, defaultValue;

	switch (propIdx)
	{
		// image format properties
	case CV_CAP_PROP_FRAME_WIDTH:
		return pgVI->getWidth(m_index);
	case CV_CAP_PROP_FRAME_HEIGHT:
		return pgVI->getHeight(m_index);
	case CV_CAP_PROP_FOURCC:
		return pgVI->getFourcc(m_index);
	case CV_CAP_PROP_FPS:
		return pgVI->getFPS(m_index);

		// video filter properties
	case CV_CAP_PROP_BRIGHTNESS:
	case CV_CAP_PROP_CONTRAST:
	case CV_CAP_PROP_HUE:
	case CV_CAP_PROP_SATURATION:
	case CV_CAP_PROP_SHARPNESS:
	case CV_CAP_PROP_GAMMA:
	case CV_CAP_PROP_MONOCHROME:
	case CV_CAP_PROP_WHITE_BALANCE_BLUE_U:
	case CV_CAP_PROP_BACKLIGHT:
	case CV_CAP_PROP_GAIN:
		if (pgVI->getVideoSettingFilter(m_index, pgVI->getVideoPropertyFromCV(propIdx), min_value, max_value, stepping_delta, current_value, flags, defaultValue))
			return (double)current_value;

		// camera properties
	case CV_CAP_PROP_PAN:
	case CV_CAP_PROP_TILT:
	case CV_CAP_PROP_ROLL:
	case CV_CAP_PROP_ZOOM:
	case CV_CAP_PROP_EXPOSURE:
	case CV_CAP_PROP_IRIS:
	case CV_CAP_PROP_FOCUS:
		if (pgVI->getVideoSettingCamera(m_index, pgVI->getCameraPropertyFromCV(propIdx), min_value, max_value, stepping_delta, current_value, flags, defaultValue))
			return (double)current_value;
	}

	// unknown parameter or value not available
	return -1;
}

bool StereoCamera::setProperty(int propIdx, double propVal)
{
	// image capture properties
	bool handled = false;
	switch (propIdx)
	{
	case CV_CAP_PROP_FRAME_WIDTH:
		m_width = cvRound(propVal);
		handled = true;
		break;

	case CV_CAP_PROP_FRAME_HEIGHT:
		m_height = cvRound(propVal);
		handled = true;
		break;

	case CV_CAP_PROP_FOURCC:
		m_fourcc = (int)(unsigned long)(propVal);
		if (-1 == m_fourcc)
		{
			// following cvCreateVideo usage will pop up caprturepindialog here if fourcc=-1
			// TODO - how to create a capture pin dialog
		}
		handled = true;
		break;

	case CV_CAP_PROP_FPS:
		int fps = cvRound(propVal);
		if (fps != pgVI->getFPS(m_index))
		{
			pgVI->stopDevice(m_index);
			pgVI->setIdealFramerate(m_index, fps);
			if (m_width_set > 0 && m_height_set > 0)
				pgVI->setupDevice(m_index, m_width_set, m_height_set);
			else
				pgVI->setupDevice(m_index);
		}
		return pgVI->isDeviceSetup(m_index);
	}

	if (handled)
	{
		// a stream setting
		if (m_width > 0 && m_height > 0)
		{
			if (m_width != pgVI->getWidth(m_index) || m_height != pgVI->getHeight(m_index))//|| fourcc != VI.getFourcc(index) )
			{
				int fps = static_cast<int>(pgVI->getFPS(m_index));
				pgVI->stopDevice(m_index);
				pgVI->setIdealFramerate(m_index, fps);
				pgVI->setupDeviceFourcc(m_index, m_width, m_height, m_fourcc);
			}

			bool success = pgVI->isDeviceSetup(m_index);
			if (success)
			{
				m_width_set = m_width;
				m_height_set = m_height;
				m_width = m_height = m_fourcc = -1;
			}
			return success;
		}
		return true;
	}

	// show video/camera filter dialog
	if (propIdx == CV_CAP_PROP_SETTINGS)
	{
		pgVI->showSettingsWindow(m_index);
		return true;
	}

	//video Filter properties
	switch (propIdx)
	{
	case CV_CAP_PROP_BRIGHTNESS:
	case CV_CAP_PROP_CONTRAST:
	case CV_CAP_PROP_HUE:
	case CV_CAP_PROP_SATURATION:
	case CV_CAP_PROP_SHARPNESS:
	case CV_CAP_PROP_GAMMA:
	case CV_CAP_PROP_MONOCHROME:
	case CV_CAP_PROP_WHITE_BALANCE_BLUE_U:
	case CV_CAP_PROP_BACKLIGHT:
	case CV_CAP_PROP_GAIN:
		return pgVI->setVideoSettingFilter(m_index, pgVI->getVideoPropertyFromCV(propIdx), (long)propVal);
	}

	//camera properties
	switch (propIdx)
	{
	case CV_CAP_PROP_PAN:
	case CV_CAP_PROP_TILT:
	case CV_CAP_PROP_ROLL:
	case CV_CAP_PROP_ZOOM:
	case CV_CAP_PROP_EXPOSURE:
	case CV_CAP_PROP_IRIS:
	case CV_CAP_PROP_FOCUS:
		return pgVI->setVideoSettingCamera(m_index, pgVI->getCameraPropertyFromCV(propIdx), (long)propVal);
	}

	return false;
}

bool StereoCamera::grabFrame()
{
	return pgVI->isDeviceDisconnected(m_index);
}

bool StereoCamera::retrieveFrame(int outputType, cv::OutputArray frame)
{
	return false;
}

bool StereoCamera::retrieveFrame(unsigned char* pFrame)
{
	//DebugPrintOut(" CV_Mat Size : %d x %d \n", pFrame->cols, pFrame->rows);
	if (pgVI->isFrameNew(m_index))
		return	pgVI->getPixels(m_index, pFrame, false, true);
	return false;
}

int StereoCamera::getCaptureDomain()
{
	return CV_CAP_DSHOW;
}

bool StereoCamera::isOpened() const
{
	return (-1 != m_index);
}

void StereoCamera::OpenCamera()
{
	CloseCamera();
	int c_index = pgVI->listDevices(true);

	if (c_index == -1)
	{
		printf("Critical ERROR : Cannot Open Camera !!");
		exit(-1); // No StereoCam Available
	}
	
	pgVI->setupDevice(c_index);
/// ///
	if (!pgVI->isDeviceSetup(c_index))
	{
		
		return;
	}
	m_index = c_index;
	m_width_set = this->getProperty(CV_CAP_PROP_FRAME_WIDTH);
	m_height_set = this->getProperty(CV_CAP_PROP_FRAME_HEIGHT);

	return;
/// ********************************************************	

}

void StereoCamera::CloseCamera()
{
	if (m_index >= 0)
	{
		pgVI->stopDevice(m_index);
		m_index = -1;
	}
	m_width_set = m_height_set = m_width = m_height = -1;
	
}

void StereoCamera::ComInit()
{
	HRESULT hr = NOERROR;

	//no need for us to start com more than once
	if (comInitCount == 0)
	{
		// Initialize the COM library.
		//CoInitializeEx so videoInput can run in another thread
#ifdef VI_COM_MULTI_THREADED
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
		hr = CoInitialize(NULL);
#endif
		//this is the only case where there might be a problem
		//if another library has started com as single threaded
		//and we need it multi-threaded - send warning but don't fail
		if (hr == RPC_E_CHANGED_MODE)
		{
			DebugPrintOut("SETUP - COM already setup - threaded VI might not be possible\n");
		}
		comInitCount++;
	}
	return;
}

void StereoCamera::ComUnInit()
{
	if (comInitCount > 0) comInitCount--;        //decrease the count of instances using com

	if (comInitCount == 0) 
	{
		CoUninitialize();    //if there are no instances left - uninitialize com
		return;
	}

	return;
	
}







