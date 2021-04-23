#pragma once

#include <cstdint>
#include <rage.hh>
#include <sysMemoryAllocator.hh>

class fwExtension
{
public:
    virtual ~fwExtension ()        = 0;
    virtual void v_1 ()            = 0;
    virtual void v_2 ()            = 0;
    virtual uint32_t GetExtensionId () = 0;
};

struct fwExtensionListItem : public sysUseAllocator
{
    fwExtension *        Data;
    fwExtensionListItem *Next;
};

class fwExtensionList
{
    fwExtensionListItem *Data;
    rage::bitset<32>     m_nMask;

    fwExtension *
    Get (uint32_t id)
    {
        if (id > 31 || m_nMask[id])
            {
                for (auto item = Data; item; item = Data->Next)
                    {
                        if (item->Data->GetExtensionId () == id)
                            return item->Data;
                    }
            }
        return nullptr;
    }

    void
    Add (fwExtension *extension)
    {
        fwExtensionListItem *item = new fwExtensionListItem;
        item->Next                = this->Data;
        item->Data                = extension;

        auto id = extension->GetExtensionId ();
        if (id < 32)
            m_nMask.Set (id, true);
    }
};
