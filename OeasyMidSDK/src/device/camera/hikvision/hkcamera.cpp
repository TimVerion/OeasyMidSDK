#include "hkcamera.h"
#include "HCNetSDK.h"
#include "log4cpp.h"
using namespace OeasyMid_HK;


HKCamera::HKCamera()
	:m_cameraID(-1),
	m_liveplayHandle(-1),
	m_bmainstartSMS(OEASY_FALSE),
	m_bsubstartSMS(OEASY_FALSE)
{
}

HKCamera::~HKCamera()
{

}
/**************************CALLBACK***********************/
void HKCamera::exceptionCB( DWORD dwType, LONG lUserID, LONG lHandle, void *pUser )
{
	char tempbuf[256] = {0};
	HKCamera* currentCamera = (HKCamera*)pUser;
	switch(dwType) 
	{
	case EXCEPTION_RECONNECT:    //Ԥ��ʱ����
		OEASYLOG_W("----------reconnect--------%d\n", time(NULL));
		break;
	default:
		break;
	}
}

void HKCamera::videoDataCallback( LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser )
{
	HKCamera *currentCamera = (HKCamera*)pUser;
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
		break;
	case NET_DVR_STREAMDATA:
		if (dwBufSize > 0)
		{
			(*currentCamera->m_liveDataCB)((long)lPlayHandle, (unsigned char*)pBuffer, (unsigned long)dwBufSize, currentCamera->m_pUserData);
			/*QByteArray *m_frame;

			if (lRealHandle == tmp->m_lRealPlayHandle)
			{
				m_frame = &tmp->m_frame_main;
			}
			else if (lRealHandle == tmp->m_lRealPlayHandle1)
			{
				m_frame = &tmp->m_frame_second;
			}
			else
			{
				return ;
			}

			QByteArray frame((const char*)pBuffer, dwBufSize);
			if (frame.size() >= 4 && frame.at(0) == '\0' && frame.at(1) == '\0' && frame.at(2) == '\1' && frame.at(3) == (char)0xba)
			{
				if (m_frame->size() > 0)
				{
					int totalLen;

					totalLen = GetH246FromPs(m_frame->data(), m_frame->size());
					if (totalLen == 0)
					{
						*m_frame = frame;
						return;
					}
					*m_frame = m_frame->mid(0, totalLen);

					if (lRealHandle == tmp->m_lRealPlayHandle)
					{
						emit tmp->packetReady(*m_frame);
					}
					else if (lRealHandle == tmp->m_lRealPlayHandle1)
					{
						emit tmp->packetReady1(*m_frame);
					}
				}
				m_frame->clear();
			}
			*m_frame += frame;*/
		}
		break;
	case NET_DVR_STD_VIDEODATA:
		OEASYLOG_I("NET_DVR_STD_VIDEODATA size = %d", dwBufSize);
		break;
	default:
		break;
	}
}
/********************************************************/

_CAMERATYPE HKCamera::getCameraType()
{
	return _HIKVISION_IPC;
}

CameraId HKCamera::loginCamera( _OEASY_LOGINFO loginfo, _OEASY_LOGIN_RESULTINFO resultInfo )
{
	OEASYLOG_I("HKCamera::loginCamera");
	//��������ʱ��������ʱ��
	NET_DVR_SetConnectTime(2000, 1);
	//��¼�����������豸��ַ����¼�û��������
	NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
	struLoginInfo.bUseAsynLogin = 0; //ʹ��ͬ����¼��ʽ
	strcpy_s(struLoginInfo.sDeviceAddress, loginfo.deviceAddress); //�豸IP��ַ
	struLoginInfo.wPort = loginfo.devicePort; //�豸����˿�
	strcpy_s(struLoginInfo.sUserName, loginfo.username); //�豸��¼�û���
	strcpy_s(struLoginInfo.sPassword, loginfo.password); //�豸��¼����
	//�豸��Ϣ, �������
	NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

	m_cameraID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
	if (m_cameraID < 0)
	{
		OEASY_DWORD errcode = NET_DVR_GetLastError();
		if (NET_DVR_PASSWORD_ERROR == errcode)
		{
			OEASYLOG_E("HKCamera::Login failed, username or password error!");
		}else
		{
			OEASYLOG_E("HKCamera::Login failed, error code: %d\n", errcode);
		}
		return m_cameraID;
	}
	memcpy(&resultInfo.Deviceinfo, &struDeviceInfoV40.struDeviceV30, sizeof(NET_DVR_DEVICEINFO_V30));
	NET_DVR_SetExceptionCallBack_V30(0, NULL, (ExceptionCallBack)HKCamera::exceptionCB, this);
	OEASYLOG_I("HKCamera::loginCamera success, m_cameraID = %d", m_cameraID);
	return m_cameraID;
}

OEASY_S32 HKCamera::logoutCamera()
{
	OEASY_ASSERT(m_cameraID,,OEASY_FALSE);
	OEASYLOG_I("HKCamera::loginCamera m_cameraID = %d", m_cameraID);
	return (OEASY_S32)NET_DVR_Logout(m_cameraID);
}

LiveplayHandle HKCamera::openVideo(_STREAMTYPE streamtype, OEASY_BOOL bstartsms /*= OEASY_FALSE*/, OEASY_CHAR* mediaserverurl /*= "" */)
{
	OEASY_ASSERT(m_cameraID,,OEASY_FALSE);
	OEASYLOG_I("HKCamera::openVideo m_cameraID = %d, streamtype = %d, bstartsms = %d", m_cameraID, streamtype, bstartsms);
	//����Ԥ�������ûص�������
	NET_DVR_PREVIEWINFO struPlayInfo;
	memset(&struPlayInfo, 0, sizeof(struPlayInfo));
	struPlayInfo.hPlayWnd = 0; //��ҪSDK ����ʱ�����Ϊ��Чֵ����ȡ��������ʱ����Ϊ��
	struPlayInfo.lChannel = 1; //hikivison IPC Ϊ1
	struPlayInfo.dwStreamType = streamtype;
	struPlayInfo.dwLinkMode = 0; //0- TCP ��ʽ��1- UDP ��ʽ��2- �ಥ��ʽ��3- RTP ��ʽ��4-RTP/RTSP��5-RSTP/HTTP
	struPlayInfo.bBlocked = 1; //0- ������ȡ����1- ����ȡ�� ����Ϊ����������ʾ�������豸�����Ӿ���Ϊ���ӳɹ������������������ʧ�ܡ�����ʧ�ܵ������Ԥ���쳣�ķ�ʽ֪ͨ�ϲ㡣��ѭ�����ŵ�ʱ����Լ���ͣ�ٵ�ʱ�䣬��NET_DVR_RealPlay����һ�¡�����Ϊ��������ʾֱ�����Ų�����ɲŷ��سɹ����
	if (_MAIN_STREAM == streamtype && bstartsms)
	{
		m_bmainstartSMS = OEASY_TRUE;
	}else if(_SUB_STREAM == streamtype && bstartsms){
		m_bsubstartSMS = OEASY_TRUE;
	}
	m_liveplayHandle = NET_DVR_RealPlay_V40(m_cameraID, &struPlayInfo, (REALDATACALLBACK)HKCamera::videoDataCallback, this);
	return m_liveplayHandle;
}

OEASY_BOOL HKCamera::closeVideo(LiveplayHandle livehandle)
{
	OEASY_ASSERT(m_cameraID,,OEASY_FALSE);
	OEASYLOG_I("HKCamera::closeVideo m_cameraID = %d", m_cameraID);
	return (OEASY_BOOL)NET_DVR_StopRealPlay(livehandle);
}


void OeasyMid_HK::HKCamera::setLiveDataCB( LIVEDATACALLBACK videoDataCB, void *pUser )
{
	m_liveDataCB = videoDataCB;
	m_pUserData = pUser;
}
