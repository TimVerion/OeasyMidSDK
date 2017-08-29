#ifndef OEASTMID_DEFINES_H
#define OEASTMID_DEFINES_H

#define  RESERVED_LEN 128
#define  DEVICE_ADDRESS_LEN 129
#define  USERNAME_LEN 64
#define  PASSWORD_LEN 64
#define  SERIALNO_LEN 48


#pragma pack(push,1)

typedef enum {
	OEASY_HANDLE_INIT,
	OEASY_HANDLE_CONNECTTING,
	OEASY_HANDLE_ONLINE,
	OEASY_HANDLE_OFFLINE,
	OEASY_HANDLE_ONLINE_LOGINERR,
	OEASY_HANDLE_INVALIDE,
	OEASY_HANDLE_LOGOUT,
}OEASY_HANDLE_STATE;

//��������
typedef enum{
	MAIN_STREAM,
	SUB_STREAM
}STREAMTYPE;

//����ͷ����
typedef enum{
	NO_IPC = -1,
	HIKVISION_IPC, //��������
	DAHUA_IPC,  //��
	TIANDY_IPC, //���ΰҵ
	OTHER_IPC
}CAMERATYPE;

//���ӶԽ�
typedef enum{
	OEASY_INTERCOM, 
	OTHER_INERCOM
}INTERCOMTYPE;

//��������
typedef enum{
	HIKVISION_ALARMHOST, //��������
	OTHER_ALARMHOST
}ALARMHOSTTYPE;

//�豸����
typedef enum{
	NO_DEVICE =-1,
	CAMERA,
	INTERCOM,
	ALARMHOST,
	OTHERDEVICE
}DEVICETYPE;

typedef enum{
	NO_ALARM,
	MOVEDETECT_ALARM,
	FACEDETECT_ALARM,
	ALL_ALARM
}ALARMTYPE;

typedef struct{
	ALARMTYPE type; 
	union{

	};
	union{

	};

}ALARMSETTING,*PALARMSETTING;

typedef struct tagEventMsg
{
	DEVICETYPE  deviceType;
	unsigned int	message;
	unsigned int    dataSize;
	char Data[0];
}OEASYLOG_EVENTMSG, *OEASY_PEVENTMSG;

typedef enum
{
	//CAMERA
	IPC_RECONNECT = 0x100,
	
	
	//INTERCOM
	INTERCON_RECONNECT = 0x300,


	//ALARMHOST
	ALARMHOST_RECONNECT = 0x500

}OEASY_MEASSGE;

//��¼���������ṹ
typedef struct{
	char  deviceAddress[DEVICE_ADDRESS_LEN];
	unsigned short devicePort;
	char  username[USERNAME_LEN];
	char  password[PASSWORD_LEN];
	char    reserved[RESERVED_LEN]; //�����ֶ�
}OEASY_LOGINFO,*LPOEASY_LOGINFO;


//��¼������ɹ����ز����ṹ
typedef struct
{
	unsigned char serialNumber[SERIALNO_LEN];
}OEASY_DEVICEINFO;

typedef struct{
	OEASY_DEVICEINFO Deviceinfo;
	//unsigned char bySupportLock;
	//unsigned char byRetryLoginTime;
	//unsigned char byPasswordLevel;
	//unsigned char byRes1;
	//DWORD dwSurplusLockTime;
	//unsigned char byCharEncodeType;
	//unsigned char byRes2[255];
}OEASY_LOGIN_RESULTINFO,*LPOEASY_LOGIN_RESULTINFO;


#pragma pack(pop) 

#endif //OEASTMID_DEFINES_H