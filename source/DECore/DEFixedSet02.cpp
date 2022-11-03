#include "../../include/DECore.hpp"

void DvigEngine2::FixedSet::Init(const deint32 memoryPoolIndex, const DvigEngine2::deusize reservedCapacity, const DvigEngine2::deusize entryByteWidth)
{
    m_Capacity = 0;
    m_EntryByteWidth = entryByteWidth;
    m_DataObject = DvigEngine2::Engine::Allocate( memoryPoolIndex, entryByteWidth + (entryByteWidth * reservedCapacity) );
    m_ReservedDataByteWidth = entryByteWidth * reservedCapacity;
    m_AllocatedDataByteWidth = entryByteWidth + m_ReservedDataByteWidth;
    m_DataByteWidth = 0;
    m_MemoryPoolIndex = memoryPoolIndex;
}

DvigEngine2::deint32 DvigEngine2::FixedSet::Insert(void* entry)
{
    if (this->m_DataByteWidth + this->m_EntryByteWidth >= this->m_AllocatedDataByteWidth)
    {
        // Allocate new block of memory
        // And copy previous data to newly created
        Engine* engine = this->GetEngine();

        const deusize allocByteWidth = this->m_EntryByteWidth + (this->m_Capacity * this->m_EntryByteWidth) + this->m_ReservedDataByteWidth;
        MemoryObject* newMemoryObject = DvigEngine2::Engine::Allocate( this->m_MemoryPoolIndex, allocByteWidth );

        Engine::CopyMemory( newMemoryObject->Unwrap<void*>(), this->m_DataObject->Unwrap<void*>(), this->m_DataByteWidth );
        engine->Delete( &this->m_DataObject );

        this->m_DataObject = newMemoryObject;
        this->m_AllocatedDataByteWidth = allocByteWidth;
    }

    // Insert new entry
    const deuint32 capacity = this->m_Capacity;
    deuchar* entryAddress = this->m_DataObject->Unwrap<deuchar*>();
    entryAddress = DvigEngine2::Ptr<deuchar*>::Add( &entryAddress, capacity * this->m_EntryByteWidth );
    Engine::CopyMemory( entryAddress, entry, this->m_EntryByteWidth );
    this->m_Capacity += 1;
    this->m_DataByteWidth += this->m_EntryByteWidth;

    return this->m_Capacity - 1;
}

void DvigEngine2::FixedSet::Replace(const deint32 index, void* entry)
{
    if (index >= (deint32)this->m_Capacity) { return; }
    void* entryAddress = this->m_DataObject->Unwrap<void*>();
    entryAddress = DvigEngine2::Ptr<void*>::Add( &entryAddress, index * this->m_EntryByteWidth );
    Engine::CopyMemory( entryAddress, entry, this->m_EntryByteWidth );
}

void DvigEngine2::FixedSet::Remove(const deint32 index)
{
    if (index != (deint32)this->m_Capacity)
    {
        // Not last index
        void* dataAddress = this->m_DataObject->Unwrap<void*>();
        void* entryAddress = DvigEngine2::Ptr<void*>::Add( &dataAddress, index * this->m_EntryByteWidth );
        void* nextEntryAddress = DvigEngine2::Ptr<void*>::Add( &entryAddress, this->m_EntryByteWidth );
        const deusize moveByteWidth = this->m_AllocatedDataByteWidth - ((index + 1) * this->m_EntryByteWidth);
        Engine::MoveMemory( entryAddress, nextEntryAddress, moveByteWidth );
    }

    this->m_Capacity -= 1;
    this->m_DataByteWidth -= this->m_EntryByteWidth;
}

DvigEngine2::deint32 DvigEngine2::FixedSet::FindValue(void* entry)
{
    for (deisize i = 0; i < (deisize)this->m_Capacity; ++i)
    {
        void* dataAddress = this->m_DataObject->Unwrap<void*>();
        void* entryAddress = DvigEngine2::Ptr<void*>::Add( &dataAddress, i * this->m_EntryByteWidth );

        deuchar* compSetAddress = (deuchar*)entryAddress;
        deuchar* compEntryAddress = (deuchar*)entry;
        if (String::CompareCharacters( compSetAddress, compEntryAddress, this->m_EntryByteWidth ) == DV_TRUE)
        {
            return i;
        }
    }

    return DV_NULL;
}