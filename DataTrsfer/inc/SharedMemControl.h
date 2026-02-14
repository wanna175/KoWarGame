// ********************************************************************
// * 헤더정의: SharedMemControl.h
// * 설    명: 다양한 유틸클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************

#pragma once

#include <string>
#include <atomic>
#include <memory>
#include <semaphore.h>

#include "TypeDef.h"
#include "MACRO.h"
#include "Config.h"

using std::string;
using namespace DATATRSFER;

namespace IPCSHAREDMEM{

// ********************************************************************
// * 구조체명: _IPC_CONFIG
// * 설    명: IPC를 관리하기 위한 구조체를 정의한다
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
typedef struct _IPC_CONFIG
{
	string READ_SHM_NAME;			// Read Shared Memory Name
	string WRITE_SHM_NAME;			// Write Shared Memory Name
	int_t  READ_SHM_SIZE;			// Read Shared Memory Size
	int_t  WRITE_SHM_SIZE;			// Write Shared Memory Size				
	string READ_SEMAPHORE_FULL;		// Read Semaphore Full Name
	string READ_SEMAPHORE_EMPTY;		// Read Semaphore Empty Name
	string WRITE_SEMAPHORE_FULL;		// Write Semaphore Full Name
	string WRITE_SEMAPHORE_EMPTY;	// Write Semaphore Empty Name
}IPC_CONFIG;

// ********************************************************************
// * 구조체명: _ShmQueue
// * 설    명: 공유메모리의 안정적 동기화를 위한 공유메모리큐를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 14
// ********************************************************************
typedef struct _ShmData
{
	char_t pShmData[SHM_DATA_SIZE];

}StShmData;

// ********************************************************************
// * 구조체명: _ShmQueue
// * 설    명: 공유메모리의 안정적 동기화를 위한 공유메모리큐를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 14
// ********************************************************************
typedef struct _ShmQueue
{
	std::atomic<uint32_t> nHead;
	std::atomic<uint32_t> nTail;
	StShmData strtPacketQueue[SHM_QUEUE_SIZE];
}StSharedMemQueue;

// ********************************************************************
// * 클래스명: SharedMemControl
// * 설    명: IPC를 관리하기 위한 클래스를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
class SharedMemControl {
private:
	IPC_CONFIG m_strtIpcConfig;

	int_t m_nReadShmFd;				//읽기용 공유메모리 fd
	sem_t* m_pReadSemaphoreFull;
	sem_t* m_pReadSemaphoreEmpty;
	StSharedMemQueue* m_pReadSharedQueue;

	int_t m_nWriteShmFd;			//쓰기용 공유메모리 fd
	sem_t* m_pWriteSemaphoreFull;
	sem_t* m_pWriteSemaphoreEmpty;
	StSharedMemQueue* m_pWriteSharedQueue;

	std::atomic<bool_t> m_bIsExit;
public:
    explicit SharedMemControl();
    virtual ~SharedMemControl();

    void InitializeShm();       //공유메모리 사용 초기값 로딩

    int_t GetReadShm(char_t *pDataBuf, const size_t nDataLen);	
	void PutWriteShm(const char_t* pDataBuf, const size_t nDataLen);

	//종료처리전에 Semaphore wait blocking 상태를 깨운다.
	void WakeUpSemWaitBlocking();	

private:
	void CreateReadShm(void);			// 읽기용 메모리 생성
	void CreateWriteShm(void);			// 쓰기용 메모리 생성
	void DeleteReadShm(void);			// 읽기용 메모리 삭제
	void DeleteWriteShm(void);			// 쓰기용 메모리 삭제
};

}