#include "hkcamera.h"
#include "HCNetSDK.h"
#include "log4cpp.h"
using namespace OeasyMid_HK;


HKCamera::HKCamera()
	:m_cameraID(-1),
	m_liveplayID(-1),
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

void CALLBACK HKCamera::realDataCallback( LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser )
{
	OEASY_ASSERT(pUser,,)
	HKCamera *currentCamera = (HKCamera*)pUser;
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //ϵͳͷ����
		break;
	case NET_DVR_STREAMDATA: //�����ݣ�����������������Ƶ�ֿ�����Ƶ�����ݣ�
		if (dwBufSize > 0)
		{
			if (currentCamera->m_bmainstartSMS)  //��������
			{
			}else if(currentCamera->m_bsubstartSMS){ //��������

			}else if(currentCamera->m_liveDataCB){ //������
				(*currentCamera->m_liveDataCB)((long)lPlayHandle, (unsigned char*)pBuffer, (unsigned long)dwBufSize, currentCamera->m_pUserData);
			}
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
	//��������ʱ��������ʱ��
	NET_DVR_SetConnectTime(2000, 1);
	//��¼�����������豸��ַ����¼�û��������
	NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
	struLoginInfo.bUseAsynLogin = 0; //ʹ��ͬ����¼��ʽ
	strcpy(struLoginInfo.sDeviceAddress, loginfo.deviceAddress); //�豸IP��ַ
	struLoginInfo.wPort = loginfo.devicePort; //�豸����˿�
	strcpy(struLoginInfo.sUserName, loginfo.username); //�豸��¼�û���
	strcpy(struLoginInfo.sPassword, loginfo.password); //�豸��¼����
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
	//NET_DVR_SetExceptionCallBack_V30(0, NULL, (ExceptionCallBack)HKCamera::exceptionCB, this);
	OEASYLOG_I("HKCamera::loginCamera success, m_cameraID = %d", m_cameraID);
	return m_cameraID;
}

OEASY_S32 HKCamera::logoutCamera()
{
	OEASY_ASSERT(m_cameraID,,OEASY_FALSE);
	OEASYLOG_I("HKCamera::loginCamera m_cameraID = %d", m_cameraID);
	return (OEASY_S32)NET_DVR_Logout(m_cameraID);
}

LiveplayId HKCamera::openVideo(_STREAMTYPE streamtype, OEASY_BOOL bstartsms /*= OEASY_FALSE*/, OEASY_CHAR* mediaserverurl /*= "" */)
{
	OEASYLOG_I("HKCamera::openVideo m_cameraID = %d", m_cameraID);
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
	m_liveplayID = NET_DVR_RealPlay_V40(m_cameraID, &struPlayInfo, (REALDATACALLBACK)HKCamera::realDataCallback, this);

	return m_liveplayID;
}

OEASY_BOOL HKCamera::closeVideo(LiveplayId liveid)
{
	OEASYLOG_I("HKCamera::openVideo LiveplayId = %d", liveid);
	return (OEASY_BOOL)NET_DVR_StopRealPlay(liveid);
}


void OeasyMid_HK::HKCamera::setLiveDataCB( LIVEDATACALLBACK videoDataCB, void *pUser )
{
	m_liveDataCB = videoDataCB;
	m_pUserData = pUser;
}

OeasyMid::OEASY_S32 OeasyMid_HK::HKCamera::captureImage( OEASY_U8 *picBuffer, OEASY_DWORD bufferSize, OEASY_DWORD* sizeReturned )
{
	NET_DVR_JPEGPARA jpegPara;
	jpegPara.wPicSize=0xff;
	jpegPara.wPicQuality=1;
	BOOL ret = NET_DVR_CaptureJPEGPicture_NEW(m_cameraID,  1, &jpegPara, (char*)picBuffer, bufferSize, sizeReturned);
	return ret;
}

OeasyMid::OEASY_S32 OeasyMid_HK::HKCamera::setAlarmParam( _ALARMSETTING *param )
{
	//�����ƶ����
	//NET_DVR_PICCFG_V40 struParams;
	//memset(&struParams, 0, sizeof(struParams));
	//BOOL bRet;
	//DWORD dwReturnLen;
	//bRet = NET_DVR_GetDVRConfig(m_cameraID, NET_DVR_GET_PICCFG_V40, 1, &struParams, sizeof(NET_DVR_PICCFG_V40), &dwReturnLen);
	//if (!bRet)
	//{
	//	OEASYLOG_E("NET_DVR_PICCFG_V40 error:%ld", NET_DVR_GetLastError());
	//	return -1;
	//}
	////struParams.dwShowChanName = 0;
	////struParams.dwShowOsd = 0;
	//struParams.struMotion.byEnableHandleMotion = 1;
	//struParams.struMotion.byEnableDisplay = 0;
	//struParams.struMotion.byConfigurationMode = 0;
	//memset(struParams.struMotion.struMotionMode.struMotionSingleArea.byMotionScope, 0 ,
	//	sizeof(struParams.struMotion.struMotionMode.struMotionSingleArea.byMotionScope));
	//for (int i = m_ipcInfo.motionStartY; i < (m_ipcInfo.motionStartY + m_ipcInfo.motionHeight); i++)
	//{
	//	for (int j = m_ipcInfo.motionStartX; j < (m_ipcInfo.motionStartX + m_ipcInfo.motionWidth); j++)
	//	{
	//		struParams.struMotion.struMotionMode.struMotionSingleArea.byMotionScope[i][j] = 1;
	//	}
	//}
	//struParams.struMotion.struMotionMode.struMotionSingleArea.byMotionSensitive = m_ipcInfo.motionSensitive;
	//struParams.struMotion.dwHandleType |= 0x04;
	//bRet = NET_DVR_SetDVRConfig(m_cameraID, NET_DVR_SET_PICCFG_V40, 1, &struParams, sizeof(NET_DVR_PICCFG_V40));
	return 0;
}

OeasyMid::OEASY_S32 OeasyMid_HK::HKCamera::startAlarm( ALARMMESGCALLBACK alarmMsgCB, void *pUser )
{
	m_alarmMsgCB = alarmMsgCB;
	m_pAlarmUserData = pUser;
	NET_DVR_SetDVRMessageCallBack_V31(MSGCallBack_V31(MsgCallBack), this);
	NET_DVR_SETUPALARM_PARAM struAlarmParam;
	memset(&struAlarmParam, 0, sizeof(struAlarmParam));
	struAlarmParam.dwSize = sizeof(struAlarmParam);
	struAlarmParam.byAlarmInfoType = 0;
	return NET_DVR_SetupAlarmChan_V41(m_cameraID, &struAlarmParam);
}

OeasyMid::OEASY_S32 OeasyMid_HK::HKCamera::stopAlarm()
{
	return NET_DVR_CloseAlarmChan_V30(m_cameraID);
}

BOOL CALLBACK OeasyMid_HK::HKCamera::MsgCallBack( LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser )
{
	OEASY_ASSERT(pUser,,0)
	HKCamera *currentCamera = (HKCamera*)pUser;
	NET_DVR_ALARMINFO alarmInfo;
	memcpy(&alarmInfo, pAlarmInfo, sizeof(NET_DVR_ALARMINFO));
	switch(lCommand)
	{
	case COMM_ALARM:
		{
			switch(alarmInfo.dwAlarmType)
			{
			case 3: //motion detect alarm
				for (int i =0 ;i < 16; i++)
				{
					if (alarmInfo.dwChannel[i] == 1)
					{
						(*currentCamera->m_alarmMsgCB)((ALARMTYPE)_MOVEDETECT_ALARM, pAlarmer->lUserID, pAlarmInfo, dwBufLen, pAlarmer->sDeviceIP, 0, currentCamera->m_pAlarmUserData );
						OEASYLOG_I("move detect from channel %d",i+1);
					}
				}
			}
		}
		break;
	case COMM_ALARM_V30:
		{
			switch(alarmInfo.dwAlarmType)
			{
			case 3: //motion detect alarm
			//	for (int i =0 ;i < 16; i++)
			//	{
			//		if (alarmInfo.dwChannel[i] == 1)
			//		{
						(*currentCamera->m_alarmMsgCB)((ALARMTYPE)_MOVEDETECT_ALARM, pAlarmer->lUserID, pAlarmInfo, dwBufLen, pAlarmer->sDeviceIP, 0, currentCamera->m_pAlarmUserData );
						 OEASYLOG_I("move detect from ip: %s",pAlarmer->sDeviceIP);
			//		}
			//	}
			}
		}
		break;
	}
	return 0;
}
