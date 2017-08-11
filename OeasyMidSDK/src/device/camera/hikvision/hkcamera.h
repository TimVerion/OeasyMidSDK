#ifndef HK_CAMERA_H
#define HK_CAMERA_H

#include "cameradefine.h"
#include "icamera.h"
#include <iostream>
#include<map>
#include<string>
#include "OeasyMidSdk.h"
using namespace OeasyMidIPC;

using namespace std;
using namespace OeasyMid;

namespace OeasyMid_HK{
	
	typedef void (CALLBACK* ExceptionCallBack)(DWORD dwType, LONG lUserID, LONG lHandle, void *pUsser);
	class HKCamera : public ICamera
	{
	public:
		HKCamera();
		virtual ~HKCamera();
		virtual _CAMERATYPE getCameraType();
		virtual CameraId loginCamera(_OEASY_LOGINFO loginfo, _OEASY_LOGIN_RESULTINFO resultInfo);
		virtual OEASY_S32 logoutCamera();
		virtual void setLiveDataCB( LIVEDATACALLBACK videoDataCB, void *pUser );
		virtual LiveplayHandle openVideo(_STREAMTYPE streamtype, OEASY_BOOL bstartsms = OEASY_FALSE, OEASY_CHAR* mediaserverurl = "");
		virtual OEASY_BOOL closeVideo(LiveplayHandle liveHandle);
		//callback 
		static void exceptionCB(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
		static void videoDataCallback(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser);

	public:
		CameraId m_cameraID;
		LiveplayHandle m_liveplayHandle;
		OEASY_BOOL m_bmainstartSMS; //�������Ƿ��ϴ�����������
		OEASY_BOOL m_bsubstartSMS; //�������Ƿ��ϴ�����������
		LIVEDATACALLBACK m_liveDataCB;
		void *m_pUserData; 
	};
}

#endif //HK_CAMERA_H