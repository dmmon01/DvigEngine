#include "../include/DvigEngine.h"

#include <iostream>
#include <typeinfo>

DV_MACRO_DEFINE_SINGLETON(DvigEngine::Engine)

void DvigEngine::Engine::Init(DvigEngine::ENGINE_USER_DATA* engineUserData)
{
    DV_ASSERT((!m_Instance))
    DV_ASSERT((engineUserData->m_Version == DV_ENGINE_VERSION_NUMBER))

    DvigEngine::MEMORY_POOL_DATA* memoryPoolsInfos = engineUserData->m_MemoryPoolsData;
    dvusize memoryPoolsCount = engineUserData->m_MemoryPoolsCount;

    // Count total pools memory occupancy
    dvusize totalPoolByteWidth = 0;
    for (dvusize i = 0; i < memoryPoolsCount; ++i) { totalPoolByteWidth += memoryPoolsInfos[i].m_ByteWidth; }
    totalPoolByteWidth += sizeof(DvigEngine::MEMORY_POOL_DATA); // for global pool info

    // Allocate global memory pool
    void* globalMemoryPoolAddres = malloc(totalPoolByteWidth);
    void* globalMemoryPoolOffset = globalMemoryPoolAddres;
    dvusize globalMemoryPoolByteWidth = totalPoolByteWidth;
    memset(globalMemoryPoolAddres, 0, globalMemoryPoolByteWidth);

    // Update memory pools addresses based on global memory pool
    totalPoolByteWidth = 0;
    for (dvisize i = 0; i < memoryPoolsCount; ++i)
    {
        DvigEngine::MEMORY_POOL_DATA* poolInfo = &memoryPoolsInfos[i];
        poolInfo->m_Address = (void*)((dvusize)globalMemoryPoolOffset + totalPoolByteWidth);
        poolInfo->m_AddressOffset = poolInfo->m_Address;
        totalPoolByteWidth += poolInfo->m_ByteWidth;
    }

    // Start Reserved memory pool
    dvmachint reservedMemoryPoolID = engineUserData->m_ReservedMemoryPoolID;
    DvigEngine::MEMORY_POOL_DATA* reservedMemoryPoolInfo = &memoryPoolsInfos[reservedMemoryPoolID];


    // Memory allocation for Engine
    m_Instance = (DvigEngine::Engine*)DvigEngine::Engine::AllocateWithInfo(reservedMemoryPoolInfo, sizeof(DvigEngine::Engine));
    
    // Allocate memory for memory pools infos
    // usize allocInfoByteWidth = (1 + memoryPoolsCount) * sizeof(DvigEngine::MEMORY_POOL_DATA);
    // DvigEngine::MEMORY_POOL_DATA* allocMemoryPoolsInfos = (DvigEngine::MEMORY_POOL_DATA*)DvigEngine::Engine::AllocateWithInfo(reservedMemoryPoolInfo, allocInfoByteWidth);

    // Allocate memory for memory pools classes
    dvusize allocClassByteWidth = (1 + memoryPoolsCount) * sizeof(DvigEngine::MemoryPool);
    DvigEngine::MemoryPool* allocMemoryPools = (DvigEngine::MemoryPool*)DvigEngine::Engine::AllocateWithInfo(reservedMemoryPoolInfo, allocClassByteWidth);

    // Assign info for each memory pool in EnginengineInfoe
    m_Instance->m_MemoryPools = allocMemoryPools;
    for (dvisize i = 0; i < memoryPoolsCount; ++i)
    {
        DvigEngine::MEMORY_POOL_DATA* curMemoryPoolInfo = (DvigEngine::MEMORY_POOL_DATA*)m_Instance->GetMemoryPoolByID(i)->GetData();
        DvigEngine::Engine::CopyMemory(curMemoryPoolInfo, &memoryPoolsInfos[i], sizeof(DvigEngine::MEMORY_POOL_DATA));
    }

    // Assign Global memory pool for Engine
    DvigEngine::MEMORY_POOL_DATA globalMemoryPoolInfo;
    globalMemoryPoolInfo.m_ID = memoryPoolsCount;   
    globalMemoryPoolInfo.m_Address = globalMemoryPoolAddres;
    globalMemoryPoolInfo.m_AddressOffset = globalMemoryPoolInfo.m_Address;
    globalMemoryPoolInfo.m_ByteWidth = globalMemoryPoolByteWidth;
    DvigEngine::MemoryPool* globalMemoryPool = m_Instance->GetMemoryPoolByID(memoryPoolsCount);
    DvigEngine::Engine::CopyMemory(globalMemoryPool->GetData(), &globalMemoryPoolInfo, sizeof(DvigEngine::MEMORY_POOL_DATA));

    DvigEngine::ENGINE_DATA* engineData = (DvigEngine::ENGINE_DATA*)m_Instance->GetData();
    engineData->m_GlobalMemoryPoolAddress = globalMemoryPoolInfo.m_Address;
    engineData->m_GlobalMemoryPoolByteWidth = globalMemoryPoolInfo.m_ByteWidth;
    engineData->m_RegisteredComponentCount = 0;

    // Allocate memory for Component storage
    dvusize componentStorageByteWidth = reservedMemoryPoolInfo->m_ByteWidth - ((dvusize)reservedMemoryPoolInfo->m_AddressOffset - (dvusize)reservedMemoryPoolInfo->m_Address);
    engineData->m_ComponentStorage = DvigEngine::Engine::AllocateWithInfo(reservedMemoryPoolInfo, componentStorageByteWidth);
    engineData->m_ComponentStorageOffset = (void*)((dvusize)engineData->m_ComponentStorage + componentStorageByteWidth);
    engineData->m_ComponentStorageByteWidth = componentStorageByteWidth;
}

void DvigEngine::Engine::Free()
{
    DV_ASSERT_PTR(m_Instance)
    DV_ASSERT_PTR(((DvigEngine::ENGINE_DATA*)m_Instance->GetData())->m_GlobalMemoryPoolAddress)

    ((DvigEngine::ENGINE_DATA*)m_Instance->GetData())->m_GlobalMemoryPoolByteWidth = 0;
    free(((DvigEngine::ENGINE_DATA*)m_Instance->GetData())->m_GlobalMemoryPoolAddress);
}

void* DvigEngine::Engine::Allocate(dvusize memoryPoolID, dvusize byteWidth)
{
    DV_ASSERT_PTR(m_Instance)
    DV_ASSERT(byteWidth)

    dvusize allocByteWidth = byteWidth;

    DvigEngine::MemoryPool* memoryPool = m_Instance->GetMemoryPoolByID(memoryPoolID);
    void* prevPoolOffset = ((DvigEngine::MEMORY_POOL_DATA*)memoryPool->GetData())->m_AddressOffset;
    ((DvigEngine::MEMORY_POOL_DATA*)memoryPool->GetData())->m_AddressOffset = (void*)((dvusize)((DvigEngine::MEMORY_POOL_DATA*)memoryPool->GetData())->m_AddressOffset + allocByteWidth);

    return prevPoolOffset;
}

DvigEngine::MemoryChunk* DvigEngine::Engine::AllocateChunk(dvusize memoryPoolID, dvusize byteWidth)
{
    DV_ASSERT_PTR(m_Instance)
    DV_ASSERT(byteWidth)

    dvusize allocByteWidth = sizeof(DvigEngine::MemoryChunk) + byteWidth;

    DvigEngine::MEMORY_POOL_DATA* memoryPoolInfo = (DvigEngine::MEMORY_POOL_DATA*)m_Instance->GetMemoryPoolByID(memoryPoolID)->GetData();
    void* prevPoolOffset = memoryPoolInfo->m_AddressOffset;
    memoryPoolInfo->m_AddressOffset = (void*)((dvusize)memoryPoolInfo->m_AddressOffset + allocByteWidth);

    DvigEngine::MemoryChunk* memoryChunk = (DvigEngine::MemoryChunk*)prevPoolOffset;
    DvigEngine::MEMORY_CHUNK_DATA* memoryChunkData = (DvigEngine::MEMORY_CHUNK_DATA*)memoryChunk->GetData();
    memoryChunkData->m_Address = (void*)((dvusize)prevPoolOffset + sizeof(DvigEngine::MemoryChunk));
    memoryChunkData->m_ByteWidth = byteWidth;

    return memoryChunk;
}

void* DvigEngine::Engine::AllocateWithInfo(MEMORY_POOL_DATA* memoryPool, dvusize byteWidth)
{
    DV_ASSERT(byteWidth)
    DV_ASSERT_PTR(memoryPool)

    void* prevPoolOffset = memoryPool->m_AddressOffset;
    memoryPool->m_AddressOffset = (void*)((dvusize)memoryPool->m_AddressOffset + byteWidth);

    return prevPoolOffset;
}

void DvigEngine::Engine::CopyMemory(void* dstAddress, void* srcAddress, dvusize byteWidth)
{
    memcpy(dstAddress, srcAddress, byteWidth);
}