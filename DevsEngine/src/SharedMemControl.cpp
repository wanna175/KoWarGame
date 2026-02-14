// ********************************************************************
// * 소 스 명: SharedMemControl.cpp
// * 설    명: IPC를 관리하기 위한 클래스 구현한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <vector>

#include "SharedMemControl.h"
#include "MACRO.h"
#include "TypeDef.h"
#include "Config.h"
#include <unistd.h>
#include <semaphore.h>
#include <cstring>


using std::vector;
using namespace IPCSHAREDMEM;
// ********************************************************************
// * 함 수 명: SharedMemControl
// * 설    명: SharedMemControl 생성자.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
SharedMemControl::SharedMemControl()
{
    m_nReadShmFd = -1;
    m_nWriteShmFd = -1;

    m_pReadSemaphoreFull = SEM_FAILED;
    m_pReadSemaphoreEmpty = SEM_FAILED;
    m_pWriteSemaphoreFull = SEM_FAILED;
    m_pWriteSemaphoreEmpty = SEM_FAILED;

}

// ********************************************************************
// * 함 수 명: ~SharedMemControl
// * 설    명: SharedMemControl 소멸자. 
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
SharedMemControl::~SharedMemControl()
{
    DeleteReadShm();
    DeleteWriteShm();
}

// ********************************************************************
// * 함 수 명: InitializeShm
// * 설    명: 공유메모리, 세마포어를 초기화하고 설정파일에서 값을 읽는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
void SharedMemControl::InitializeShm()
{
    CreateReadShm();
    CreateWriteShm();
}
// ********************************************************************
// * 함 수 명: CreateReadShm
// * 설    명: (posix 계열) 데이터를 전송받을 공유 메모리를 생성한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
void SharedMemControl::CreateReadShm()
{
    auto& pConfig = CONFIG::ConfigManager::GetInstance();
    string strSectionName = IPCSHAREDMEM::READ_SECTION_NAME;
    string strConfigPath = IPCSHAREDMEM::CONFIG_PATH;
    string strShmKeyName = IPCSHAREDMEM::READ_SHM_NAME;
    string strSemFullKeyName = IPCSHAREDMEM::READ_SEMAPHORE_FULL;
    string strSemEmptyKeyName = IPCSHAREDMEM::READ_SEMAPHORE_EMPTY;

    m_strtIpcConfig.READ_SHM_NAME = pConfig.GetValue(strShmKeyName,strSectionName,strConfigPath);
    m_strtIpcConfig.READ_SEMAPHORE_FULL = pConfig.GetValue(strSemFullKeyName,strSectionName,strConfigPath);
    m_strtIpcConfig.READ_SEMAPHORE_EMPTY = pConfig.GetValue(strSemEmptyKeyName,strSectionName,strConfigPath);
    m_nReadShmFd = shm_open(m_strtIpcConfig.READ_SHM_NAME.c_str(), O_RDWR, 0644);
    if (m_nReadShmFd == -1) 
    {
        if (errno != ENOENT){
            ERROR("shm_open() fail");
            return;
        }
        m_nReadShmFd = shm_open(m_strtIpcConfig.READ_SHM_NAME.c_str(), O_CREAT | O_RDWR, 0644);
        
        int_t ret_v = ftruncate(m_nReadShmFd, sizeof(StSharedMemQueue));
        if (ret_v == -1) 
        {
            ERROR("ftruncate() fail");
            return;
        }
    }
    
    //TODO : read만 할 것이니깐 Prot_read만 하면 될까?
    void* pSharedMemoryAddr = mmap(nullptr, sizeof(StSharedMemQueue), PROT_READ | PROT_WRITE, MAP_SHARED, m_nReadShmFd, 0);
    if (MAP_FAILED == pSharedMemoryAddr)
    {
        ERROR("mmap() fail");
        return;
    }

    m_pReadSharedQueue = reinterpret_cast<StSharedMemQueue*>(pSharedMemoryAddr);

    //queue 초기화
    m_pReadSharedQueue->nHead = 0;
    m_pReadSharedQueue->nTail = 0;

    m_pReadSemaphoreFull = sem_open(m_strtIpcConfig.READ_SEMAPHORE_FULL.c_str(), O_CREAT, 0644, 0);                  //읽을 공간 없음
    m_pReadSemaphoreEmpty = sem_open(m_strtIpcConfig.READ_SEMAPHORE_EMPTY.c_str(), O_CREAT, 0644, SHM_QUEUE_SIZE);   //전체 공간 읽기가능

    if (SEM_FAILED == m_pReadSemaphoreFull || SEM_FAILED == m_pReadSemaphoreEmpty)
    {
        ERROR("sem_open() fail");
        return;
    }

    INFO("Read Shared Memory setting complete");


}
// ********************************************************************
// * 함 수 명: CreateWriteShm
// * 설    명: SharedMemControl 생성자.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
void SharedMemControl::CreateWriteShm()
{
    auto& pConfig = CONFIG::ConfigManager::GetInstance();
    string strSectionName = IPCSHAREDMEM::WRITE_SECTION_NAME;
    string strConfigPath = IPCSHAREDMEM::CONFIG_PATH;
    string strShmKeyName = IPCSHAREDMEM::WRITE_SHM_NAME;
    string strSemFullKeyName = IPCSHAREDMEM::WRITE_SEMAPHORE_FULL;
    string strSemEmptyKeyName = IPCSHAREDMEM::WRITE_SEMAPHORE_EMPTY;

    m_strtIpcConfig.WRITE_SHM_NAME = pConfig.GetValue(strShmKeyName,strSectionName,strConfigPath);
    m_strtIpcConfig.WRITE_SEMAPHORE_FULL = pConfig.GetValue(strSemFullKeyName,strSectionName,strConfigPath);
    m_strtIpcConfig.WRITE_SEMAPHORE_EMPTY = pConfig.GetValue(strSemEmptyKeyName,strSectionName,strConfigPath);
    //TODO:둘중 하나의 프로세스에서 이미 만들었다면 어떤식으로 동작하게 될까?
    m_nWriteShmFd = shm_open(m_strtIpcConfig.WRITE_SHM_NAME.c_str(), O_RDWR, 0644);
    if (m_nWriteShmFd == -1) 
    {
        if (errno != ENOENT){
            ERROR("shm_open() fail");
            return;
        }
        m_nWriteShmFd = shm_open(m_strtIpcConfig.WRITE_SHM_NAME.c_str(), O_CREAT | O_RDWR, 0644);
        
        int_t ret_v = ftruncate(m_nWriteShmFd, sizeof(StSharedMemQueue));
        if (ret_v == -1) 
        {
            ERROR("ftruncate() fail");
            return;
        }
    }

    
    //TODO : read만 할 것이니깐 Prot_read만 하면 될까?
    void* pSharedMemoryAddr = mmap(nullptr, sizeof(StSharedMemQueue), PROT_READ | PROT_WRITE, MAP_SHARED, m_nWriteShmFd, 0);
    if (MAP_FAILED == pSharedMemoryAddr)
    {
        ERROR("mmap() fail");
        return;
    }

    m_pWriteSharedQueue = reinterpret_cast<StSharedMemQueue*>(pSharedMemoryAddr);

    //queue 초기화
    m_pWriteSharedQueue->nHead = 0;
    m_pWriteSharedQueue->nTail = 0;

    m_pWriteSemaphoreFull = sem_open(m_strtIpcConfig.WRITE_SEMAPHORE_FULL.c_str(), O_CREAT, 0644, 0);                  //쓸 공간 없음
    m_pWriteSemaphoreEmpty = sem_open(m_strtIpcConfig.WRITE_SEMAPHORE_EMPTY.c_str(), O_CREAT, 0644, SHM_QUEUE_SIZE);   //전체 공간 쓰기가능

    if (SEM_FAILED == m_pWriteSemaphoreFull || SEM_FAILED == m_pWriteSemaphoreEmpty)
    {
        ERROR("sem_open() fail");
        return;
    }

    INFO("Write Shared Memory setting complete");


}
// ********************************************************************
// * 함 수 명: DeleteReadShm
// * 설    명: read용 공유메모리를 제거한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
void SharedMemControl::DeleteReadShm()
{
    if (m_pReadSharedQueue != nullptr)
    {
        munmap(m_pReadSharedQueue, sizeof(StSharedMemQueue));
        m_pReadSharedQueue = nullptr;
    }

    if (m_nReadShmFd > 0) 
    {
        close(m_nReadShmFd);
        //shm_unlink(m_strtIpcConfig.READ_SHM_NAME.c_str());
        m_nReadShmFd = -1;
    }

    if (m_pReadSemaphoreFull !=SEM_FAILED) 
    {
        sem_close(m_pReadSemaphoreFull);
        //sem_unlink(m_strtIpcConfig.READ_SEMAPHORE_FULL.c_str());
        m_pReadSemaphoreFull = SEM_FAILED;
    }

    if (m_pReadSemaphoreEmpty !=SEM_FAILED) 
    {
        sem_close(m_pReadSemaphoreEmpty);
        //sem_unlink(m_strtIpcConfig.READ_SEMAPHORE_EMPTY.c_str());
        m_pReadSemaphoreEmpty = SEM_FAILED;
    }

    INFO("Read Shared Memory unlinked");
}
// ********************************************************************
// * 함 수 명: DeleteWriteShm
// * 설    명: Write용 공유메모리를 제거한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
void SharedMemControl::DeleteWriteShm()
{
    if (m_pWriteSharedQueue != nullptr)
    {
        munmap(m_pWriteSharedQueue, sizeof(StSharedMemQueue));
        m_pWriteSharedQueue = nullptr;
    }

    if (m_nWriteShmFd > 0) 
    {
        close(m_nWriteShmFd);
        //shm_unlink(m_strtIpcConfig.WRITE_SHM_NAME.c_str());
        m_nWriteShmFd = -1;
    }

    if (m_pWriteSemaphoreFull !=SEM_FAILED) 
    {
        sem_close(m_pWriteSemaphoreFull);
        //sem_unlink(m_strtIpcConfig.WRITE_SEMAPHORE_FULL.c_str());
        m_pWriteSemaphoreFull = SEM_FAILED;
    }

    if (m_pWriteSemaphoreEmpty !=SEM_FAILED) 
    {
        sem_close(m_pWriteSemaphoreEmpty);
        //sem_unlink(m_strtIpcConfig.WRITE_SEMAPHORE_EMPTY.c_str());
        m_pWriteSemaphoreEmpty = SEM_FAILED;
    }

    INFO("Write Shared Memory unlinked");
}
// ********************************************************************
// * 함 수 명: GetReadShm
// * 설    명: 읽기 전용 공유메모리로 부터 데이터를 읽는다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
int_t SharedMemControl::GetReadShm(char_t* pDataBuf, const size_t nBufLen)
{
    int_t nReadCnt = 0;
    size_t offset= 0;
    while(true)
    {
        if (sem_trywait(m_pReadSemaphoreFull) == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            ERROR("sem_wait() fail");
            return -1;
        }

        uint32_t nHead = m_pReadSharedQueue->nHead.load();
        uint32_t nIndex = nHead % SHM_QUEUE_SIZE;

        const StShmData& strtShmData = m_pReadSharedQueue->strtPacketQueue[nIndex]; 

        if (offset + sizeof(strtShmData) > nBufLen)
        {
            //semaphore 다시 되돌린다.
            sem_post(m_pReadSemaphoreFull);
            break;
        }
        memcpy(pDataBuf+offset, &strtShmData,sizeof(strtShmData));
        offset += sizeof(strtShmData);
        nReadCnt++;

        m_pReadSharedQueue->nHead.store(nHead + 1);

        if (sem_post(m_pReadSemaphoreEmpty) == -1)
        {
            ERROR("sem_post() fail");
            return -1;
        }
    }
    

    return offset;
}

// ********************************************************************
// * 함 수 명: PutWriteShm
// * 설    명: 쓰기 전용 공유메모리로 부터 데이터를 쓴다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 07. 13
// ********************************************************************
void SharedMemControl::PutWriteShm(const char_t* pDataBuf,const size_t nDataLen)
{
    if (sem_wait(m_pWriteSemaphoreEmpty) == -1)
    {
        ERROR("sem_wait() fail");
        return;
    }

    uint32_t nTail = m_pWriteSharedQueue->nTail.load();
    uint32_t nIndex = nTail % SHM_QUEUE_SIZE;

    memcpy(&m_pWriteSharedQueue->strtPacketQueue[nIndex], pDataBuf,nDataLen);

    m_pWriteSharedQueue->nTail.store(nTail + 1);

    if (sem_post(m_pWriteSemaphoreFull) == -1) 
    {
        ERROR("sem_post() fail");
        return;
    }

}
