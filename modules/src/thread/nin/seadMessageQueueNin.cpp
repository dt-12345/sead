#include "basis/seadNew.h"
#include "basis/seadRawPrint.h"
#include "thread/seadMessageQueue.h"

namespace sead
{
MessageQueue::MessageQueue() = default;

MessageQueue::~MessageQueue() = default;

void MessageQueue::allocate(s32 size, Heap* heap)
{
    if (size <= 0)
    {
        SEAD_ASSERT_MSG(false, "MessageQueue size must not be zero");
        return;
    }

    mBuffer = new (heap) Element[size];
    nn::os::InitializeLightMessageQueue(&mMessageQueueInner, reinterpret_cast<u64*>(mBuffer), size);
}

void MessageQueue::free()
{
    nn::os::FinalizeLightMessageQueue(&mMessageQueueInner);
    if (mBuffer)
    {
        delete[] mBuffer;
        mBuffer = nullptr;
    }
}

bool MessageQueue::push(MessageQueue::Element message, MessageQueue::BlockType block_type)
{
    if (block_type == BlockType::Blocking)
    {
        nn::os::SendLightMessageQueue(&mMessageQueueInner, message);
        return true;
    }

    return nn::os::TrySendLightMessageQueue(&mMessageQueueInner, message);
}

MessageQueue::Element MessageQueue::pop(MessageQueue::BlockType block_type)
{
    u64 message;

    if (block_type == BlockType::Blocking)
    {
        nn::os::ReceiveLightMessageQueue(&message, &mMessageQueueInner);
        return message;
    }

    if (nn::os::TryReceiveLightMessageQueue(&message, &mMessageQueueInner))
        return message;
    return 0;
}

MessageQueue::Element MessageQueue::peek(MessageQueue::BlockType block_type) const
{
    u64 message;

    if (block_type == BlockType::Blocking)
    {
        nn::os::PeekLightMessageQueue(&message, &mMessageQueueInner);
        return message;
    }

    if (nn::os::TryPeekLightMessageQueue(&message, &mMessageQueueInner))
        return message;
    return 0;
}

bool MessageQueue::jam(MessageQueue::Element message, MessageQueue::BlockType block_type)
{
    if (block_type == BlockType::Blocking)
    {
        nn::os::JamLightMessageQueue(&mMessageQueueInner, message);
        return true;
    }

    return nn::os::TryJamLightMessageQueue(&mMessageQueueInner, message);
}
}  // namespace sead
