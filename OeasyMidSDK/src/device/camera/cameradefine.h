#ifndef CAMERADEFINE_H
#define CAMERADEFINE_H
#include "BaseType.h"
using namespace OeasyMid;


#define  RESERVED_LEN 128
#define  DEVICE_ADDRESS_LEN 129
#define  USERNAME_LEN 64
#define  PASSWORD_LEN 64
#define  SERIALNO_LEN 48


#pragma pack(push,1)

//��������
typedef enum{
	_MAIN_STREAM,
	_SUB_STREAM
}_STREAMTYPE;

//����ͷ����
typedef enum{
	_HIKVISION_IPC, //��������
	_DAHUA_IPC,  //��
	_TIANDY_IPC, //���ΰҵ
	_OTHER_IPC
}_CAMERATYPE;

//��¼���������ṹ
typedef struct{
	OEASY_CHAR  deviceAddress[DEVICE_ADDRESS_LEN];
	OEASY_U16 devicePort;
	OEASY_CHAR  username[USERNAME_LEN];
	OEASY_CHAR  password[PASSWORD_LEN];
	OEASY_U8    reserved[RESERVED_LEN]; //�����ֶ�
}_OEASY_LOGINFO,*_LPOEASY_LOGINFO;


//��¼������ɹ����ز����ṹ
typedef struct
{
	OEASY_U8 serialNumber[SERIALNO_LEN];
}_OEASY_DEVICEINFO;

typedef struct{
	_OEASY_DEVICEINFO Deviceinfo;
	//OEASY_U8 bySupportLock;
	//OEASY_U8 byRetryLoginTime;
	//OEASY_U8 byPasswordLevel;
	//OEASY_U8 byRes1;
	//OEASY_DWORD dwSurplusLockTime;
	//OEASY_U8 byCharEncodeType;
	//OEASY_U8 byRes2[255];
}_OEASY_LOGIN_RESULTINFO,*_LPOEASY_LOGIN_RESULTINFO;


typedef enum{
	_NO_ALARM,
	_MOVEDETECT_ALARM,
	_FACEDETECT_ALARM,
	_ALL_ALARM
}_ALARMTYPE;

typedef struct{
	_ALARMTYPE type; 
	union{

	};
	union{

	};

}_ALARMSETTING,*_PALARMSETTING;

 #pragma pack(pop) 

#endif //CAMERADEFINE_H