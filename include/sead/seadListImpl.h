#ifndef SEAD_LIST_IMPL_H_
#define SEAD_LIST_IMPL_H_

namespace sead {

class ListNode
{
public:
    void erase_();

    ListNode* previous;
    ListNode* next;
};

class ListImpl
{
public:
    ListNode root;
    s32 count;
    u32 nodeOffset;
};

class UnkList
{
public:
    UnkList()
    {
        ptrToSelf = this;
        root.next = NULL;
        root.previous = NULL;
        _C = 0;
    }

    ListNode root;
    void* ptrToSelf;
    u32 _C;
};

} // namespace sead

#endif // SEAD_LIST_IMPL_H_
