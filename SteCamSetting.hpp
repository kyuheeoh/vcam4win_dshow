#define STECAM_WIDTH  1280
#define STECAM_HEIGHT 960
typedef enum
{
	STECAM_COLOR_FILTER_RGGB = 0,
	STECAM_COLOR_FILTER_GRBG = 1,
	STECAM_COLOR_FILTER_BGGR = 2,
	STECAM_COLOR_FILTER_GBRG = 3,
} enumColorFilter;

static const enumColorFilter colorFilter{ STECAM_COLOR_FILTER_RGGB };

//SCamRVISION USB SETTING
//Vender id
#define SCam_USB_VENDERID		(0x2C33)
//Product id
#define SCam_USB_PRODUCTID	(0x0001)	//Reversal

