#include "dhcamera.h"
#include "log4cpp.h"
#include "Tick.h"
using namespace OeasyMid_DH;

_CAMERATYPE DHCamera::getCameraType()
{
	return _DAHUA_IPC;
}


OeasyMid_DH::DHCamera::DHCamera()
:m_cameraID(-1),
m_liveplayID(-1),
m_bmainstartSMS(OEASY_FALSE),
m_bsubstartSMS(OEASY_FALSE),
m_snapCmdSerial(0),
m_snapSize(-1)
{
	memcpy(m_snapBuffer, 0, SNAPBUFFER_SZIE);
	INIT_MUTEX(&m_snapMutex);
}

OeasyMid_DH::DHCamera::~DHCamera()
{
	RELEASE_MUTEX(&m_snapMutex);
}

void CALLBACK OeasyMid_DH::DHCamera::realDataCallback( LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize,LONG param, LDWORD dwUser )
{
	DHCamera *curCamera = (DHCamera*)dwUser;
	if (curCamera == NULL)
	{
		OEASYLOG_E("DHCamera::realDataCallback error!");
		return;
	}
	switch(dwDataType)
	{
		case 0: //ԭʼ����Ƶ�������
			break;
		case 1: //��׼��Ƶ����
			if(dwBufSize > 0)
			{
				
				if (curCamera->m_bmainstartSMS)  //��������
				{
				}else if(curCamera->m_bsubstartSMS){ //��������

				}else if(curCamera->m_liveDataCB){ //������
					(*curCamera->m_liveDataCB)((long)lRealHandle, (unsigned char*)pBuffer, (unsigned long)dwBufSize, curCamera->m_pUserData);
				}
			}
			break;
		case 2: //yuv����
			break;
		case 3: //pcm����
			break;
		case 4: //ԭʼ��Ƶ����
			break;
	}
}


CameraId OeasyMid_DH::DHCamera::loginCamera( _OEASY_LOGINFO loginfo, _OEASY_LOGIN_RESULTINFO resultInfo )
{
	NET_PARAM netparm = {0};
	netparm.nConnectTime = 2000;
	//��ʱʱ��
	CLIENT_SetNetworkParam(&netparm);

	NET_DEVICEINFO_Ex deviceInfo ={0};
	unsigned long lLogin;
	int errcode = 0;
	lLogin = CLIENT_LoginEx2(loginfo.deviceAddress,loginfo.devicePort, loginfo.username,loginfo.password, EM_LOGIN_SPEC_CAP_SERVER_CONN, NULL, &deviceInfo, &errcode);
	m_cameraID = lLogin;
	if (m_cameraID == 0)
	{
		OEASYLOG_E("DH loginCamera error, code = %d", errcode);
		m_cameraID = -1;
	}
	return m_cameraID;
}

OeasyMid::OEASY_S32 OeasyMid_DH::DHCamera::logoutCamera()
{
	BOOL ret = CLIENT_Logout(m_cameraID);
	if (ret < 0)
	{
		OEASYLOG_E("DH logoutCamera error, m_cameraID = %d", m_cameraID);
	}
	return ret;
}

void OeasyMid_DH::DHCamera::setLiveDataCB( LIVEDATACALLBACK videoDataCB, void *pUser )
{
	m_liveDataCB = videoDataCB;
	m_pUserData = pUser;
	CLIENT_SetRealDataCallBackEx( m_cameraID, (fRealDataCallBackEx)DHCamera::realDataCallback, (DWORD)this, 0x00000002);
}

LiveplayId OeasyMid_DH::DHCamera::openVideo( _STREAMTYPE streamtype, OEASY_BOOL bstartsms /*= OEASY_FALSE*/, OEASY_CHAR* mediaserverurl /*= ""*/ )
{
	if (_MAIN_STREAM == streamtype && bstartsms)
	{
		m_bmainstartSMS = OEASY_TRUE;
	}else if(_SUB_STREAM == streamtype && bstartsms){
		m_bsubstartSMS = OEASY_TRUE;
	}

	LiveplayId liveid = -1;
	DH_RealPlayType realtype = (streamtype == _MAIN_STREAM) ? DH_RType_Realplay:DH_RType_Realplay_1;
	int nChannelID = 0;
	LLONG ret = CLIENT_RealPlayEx(m_cameraID, nChannelID, NULL, realtype);
	if (ret == 0)
	{
		OEASYLOG_E("DH openVideo failed! ret = %d, m_cameraID = %d",ret, m_cameraID);
		liveid = -1;
	}
	liveid = ret;
	return liveid;
}

OeasyMid::OEASY_BOOL OeasyMid_DH::DHCamera::closeVideo( LiveplayId liveid )
{
	OEASY_BOOL ret = OEASY_TRUE;
	if (liveid > 0)
	{
		ret = (OEASY_BOOL)CLIENT_StopRealPlayEx(liveid);
	}
	return ret;
}

void CALLBACK OeasyMid_DH::DHCamera::SnapDataRev( LLONG lLoginID,BYTE *pBuf,UINT RevLen,UINT EncodeType, DWORD CmdSerial, LDWORD dwUser )
{
	DHCamera *curCamera = (DHCamera*)dwUser;
	curCamera->m_snapCmdSerial = 0;
	if (curCamera == NULL || curCamera->m_snapCmdSerial !=  CmdSerial)
	{
		OEASYLOG_E("DHCamera::SnapDataRev error!");
		return;
	}
	if (RevLen > 0 )
	{
		LOCK_MUTEX(&curCamera->m_snapMutex);
		if (SNAPBUFFER_SZIE > RevLen)
		{
			curCamera->m_snapSize = RevLen;
			memcpy(curCamera->m_snapBuffer, pBuf, RevLen);
		}
		UNLOCK_MUTEX(&curCamera->m_snapMutex);
	}
}


OeasyMid::OEASY_S32 OeasyMid_DH::DHCamera::captureImage( OEASY_CHAR *picBuffer, OEASY_DWORD bufferSize, OEASY_DWORD* sizeReturned )
{
	LOCK_MUTEX(&m_snapMutex);
	int timeout = 1000; //10s
	CLIENT_SetSnapRevCallBack((fSnapRev)SnapDataRev, (LDWORD)this);
	OEASY_U64 enterTime = CTick::GetTickCount();
	SNAP_PARAMS pram;
	pram.Channel = 0;
	pram.Quality = 3;
	pram.mode = 0;
	m_snapCmdSerial++;
	pram.CmdSerial = m_snapCmdSerial;
	BOOL ret = CLIENT_SnapPictureEx(m_cameraID, &pram);
	while(CTick::GetTickCount() - enterTime < timeout)
	{
		if (m_snapSize > 0) //geted image
		{
			if (bufferSize >= m_snapSize)
			{
				memcpy(picBuffer, m_snapBuffer, m_snapSize);
			}
			*sizeReturned = m_snapSize;
			break;
		}
	}
	if (m_snapSize < 0) //noimage
	{
		*sizeReturned = m_snapSize;
		UNLOCK_MUTEX(&m_snapMutex);
		return -1; //tiemout and no image
	}
	memcpy(m_snapBuffer, 0, SNAPBUFFER_SZIE);
	m_snapSize = -1;
	UNLOCK_MUTEX(&m_snapMutex);
	return 0;
}

