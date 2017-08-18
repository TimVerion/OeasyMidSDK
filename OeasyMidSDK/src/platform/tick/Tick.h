#ifndef __OEASY_PLATFORM_TICK__
#define __OEASY_PLATFORM_TICK__
#include "BaseType.h"
#include "Thread.h"
#include "log4cpp.h"

namespace OeasyMid{
class CTick
{
	private:
		CTick();
		~CTick();
	public:
		//GetInstance �� ReleaseInstance �Ƿ��̰߳�ȫ����
		static CTick* GetInstance();
		static OEASY_VOID ReleaseInstance();
		
		static OEASY_U64 GetTickCount(); //��λ 10ms
		static void Delay(int ms);	   //��λms
	protected:
		OEASY_BOOL init();
		OEASY_VOID deInit();
		OEASY_THREADMETHOD_DEC(threadRun, lpParam);
	protected:
		static OEASY_U64		s_tickCount;//��λ 10ms
		static CTick		*s_pTick;
		CThread		m_hThread;
		OEASY_BOOL	m_bRun;
	};

};
#endif //__OEASY_PLATFORM_TICK__

