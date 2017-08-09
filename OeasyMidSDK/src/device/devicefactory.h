#ifndef DEVICE_FACTORY_H
#define DEVICE_FACTORY_H
#include "cameradefine.h"
#include "intercomdefine.h"
#include "icamera.h"
#include "iintercom.h"

#include "hkcamera.h"
#include "dhcamera.h"
#include "oeasyintercom.h"

#include <stdio.h>

using namespace OeasyMid_DH;
using namespace OeasyMid_HK;

class DeviceFactory
{
public:
	DeviceFactory();
	virtual ~DeviceFactory();
	//create �� device��Ҫ�Լ��ͷ��ڴ�
	virtual ICamera* createCamera(CAMERATYPE cameraType);
	virtual IItercom* CreateInterCom(INTERCOMTYPE intercomType);
};

#endif //DEVICE_FACTORY_H