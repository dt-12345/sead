#ifndef SEAD_BUFFER_H_
#define SEAD_BUFFER_H_

#include <algorithm>
#include <type_traits>

#include <basis/seadNew.h>
#include <basis/seadRawPrint.h>
#include <basis/seadTypes.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadPtrUtil.h>

namespace sead
{
class Heap;

template <typename T>
class Buffer
{
public:
    Buffer() : mSize(0), mBuffer(NULL) {}
    Buffer(s32 size, T* buffer) : mSize(size), mBuffer(buffer) {}
    template <s32 N>
    Buffer(T (&array)[N]) : Buffer(N, array)
    {
    }

    class iterator
    {
    public:
        explicit iterator(T* buffer, s32 index = 0) : mIndex(index), mBuffer(buffer) {}
        bool operator==(const iterator& rhs) const
        {
            return mIndex == rhs.mIndex && mBuffer == rhs.mBuffer;
        }
        bool operator!=(const iterator& rhs) const { return !operator==(rhs); }
        iterator& operator++()
        {
            ++mIndex;
            return *this;
        }
        T& operator*() const { return mBuffer[mIndex]; }
        T* operator->() const { return &mBuffer[mIndex]; }
        s32 getIndex() const { return mIndex; }

    private:
        s32 mIndex;
        T* mBuffer;
    };

    class constIterator
    {
    public:
        explicit constIterator(const T* buffer, s32 index = 0) : mIndex(index), mBuffer(buffer) {}
        bool operator==(const constIterator& rhs) const
        {
            return mIndex == rhs.mIndex && mBuffer == rhs.mBuffer;
        }
        bool operator!=(const constIterator& rhs) const { return !operator==(rhs); }
        constIterator& operator++()
        {
            ++mIndex;
            return *this;
        }
        const T& operator*() const { return mBuffer[mIndex]; }
        const T* operator->() const { return &mBuffer[mIndex]; }
        s32 getIndex() const { return mIndex; }

    private:
        s32 mIndex;
        const T* mBuffer;
    };

    iterator begin() { return iterator(mBuffer); }
    constIterator begin() const { return constIterator(mBuffer); }
    iterator end() { return iterator(mBuffer, mSize); }
    constIterator end() const { return constIterator(mBuffer, mSize); }

    void allocBuffer(s32 size, s32 alignment)
    {
        SEAD_ASSERT(mBuffer == nullptr);
        if (size > 0)
        {
            T* buffer = new (alignment) T[size];
            if (buffer)
            {
                mSize = size;
                mBuffer = buffer;
                SEAD_ASSERT_MSG(PtrUtil::isAlignedPow2(mBuffer, sead::abs(alignment)),
                                "don't set alignment for a class with destructor");
            }
        }
        else
        {
            SEAD_ASSERT_MSG(false, "size[%d] must be larger than zero", size);
        }
    }

    void allocBuffer(s32 size, Heap* heap, s32 alignment = sizeof(void*))
    {
        SEAD_ASSERT(mBuffer == nullptr);
        if (size > 0)
        {
            T* buffer = new (heap, alignment) T[size];
            if (buffer)
            {
                mSize = size;
                mBuffer = buffer;
                SEAD_ASSERT_MSG(PtrUtil::isAlignedPow2(mBuffer, sead::abs(alignment)),
                                "don't set alignment for a class with destructor");
            }
        }
        else
        {
            SEAD_ASSERT_MSG(false, "size[%d] must be larger than zero", size);
        }
    }

    bool tryAllocBuffer(s32 size, s32 alignment = sizeof(void*))
    {
        SEAD_ASSERT(mBuffer == nullptr);
        if (size > 0)
        {
            T* buffer = new (alignment, std::nothrow) T[size];
            if (buffer)
            {
                mSize = size;
                mBuffer = buffer;
                SEAD_ASSERT_MSG(PtrUtil::isAlignedPow2(mBuffer, sead::abs(alignment)),
                                "don't set alignment for a class with destructor");
                return true;
            }
            return false;
        }
        SEAD_ASSERT_MSG(false, "size[%d] must be larger than zero", size);
        return false;
    }

    bool tryAllocBuffer(s32 size, Heap* heap, s32 alignment = sizeof(void*))
    {
        SEAD_ASSERT(mBuffer == nullptr);
        if (size > 0)
        {
            T* buffer = new (heap, alignment, std::nothrow) T[size];
            if (buffer)
            {
                mSize = size;
                mBuffer = buffer;
                SEAD_ASSERT_MSG(PtrUtil::isAlignedPow2(mBuffer, sead::abs(alignment)),
                                "don't set alignment for a class with destructor");
                return true;
            }
            return false;
        }
        SEAD_ASSERT_MSG(false, "size[%d] must be larger than zero", size);
        return false;
    }

    void allocBufferAssert(s32 size, Heap* heap, s32 alignment = sizeof(void*))
    {
        if (!tryAllocBuffer(size, heap, alignment))
            AllocFailAssert(heap, sizeof(T) * size, alignment);
    }

    void freeBuffer()
    {
        if (mBuffer)
        {
            delete[] mBuffer;
            mBuffer = nullptr;
            mSize = 0;
        }
    }

    void setBuffer(s32 size, T* bufferptr)
    {
        if (size < 1)
        {
            SEAD_ASSERT_MSG(false, "size[%d] must be larger than zero", size);
            return;
        }
        if (!bufferptr)
        {
            SEAD_ASSERT_MSG(false, "bufferptr is null");
            return;
        }
        mSize = size;
        mBuffer = bufferptr;
    }

    bool isBufferReady() const { return mBuffer != nullptr; }

    T& operator()(s32 idx) { return *unsafeGet(idx); }
    const T& operator()(s32 idx) const { return *unsafeGet(idx); }

    T& operator[](s32 idx)
    {
        if (u32(mSize) <= u32(idx))
        {
            SEAD_ASSERT_MSG(false, "index exceeded [%d/%d]", idx, mSize);
            return mBuffer[0];
        }
        return mBuffer[idx];
    }

    const T& operator[](s32 idx) const
    {
        if (u32(mSize) <= u32(idx))
        {
            SEAD_ASSERT_MSG(false, "index exceeded [%d/%d]", idx, mSize);
            return mBuffer[0];
        }
        return mBuffer[idx];
    }

    T* get(s32 idx)
    {
        if (u32(mSize) <= u32(idx))
        {
            SEAD_ASSERT_MSG(false, "index exceeded [%d/%d]", idx, mSize);
            return nullptr;
        }
        return &mBuffer[idx];
    }

    const T* get(s32 idx) const
    {
        if (u32(mSize) <= u32(idx))
        {
            SEAD_ASSERT_MSG(false, "index exceeded [%d/%d]", idx, mSize);
            return nullptr;
        }
        return &mBuffer[idx];
    }

    T* unsafeGet(s32 idx) { return &mBuffer[idx]; }
    const T* unsafeGet(s32 idx) const { return &mBuffer[idx]; }

    T& front() { return mBuffer[0]; }
    const T& front() const { return mBuffer[0]; }

    T& back() { return mBuffer[mSize - 1]; }
    const T& back() const { return mBuffer[mSize - 1]; }

    s32 size() const { return mSize; }
    s32 getSize() const { return mSize; }

    T* getBufferPtr() { return mBuffer; }
    const T* getBufferPtr() const { return mBuffer; }

    u32 getByteSize() const { return mSize * sizeof(T); }

    void fill(const T& v)
    {
        for (s32 i = 0; i < mSize; ++i)
            mBuffer[i] = v;
    }

    using CompareCallback = s32 (*)(const T* lhs, const T* rhs);

    s32 binarySearch(const T& item) const { return binarySearch(item, compareT); }

    s32 binarySearch(const T& item, CompareCallback cmp) const
    {
        s32 a = 0;
        s32 b = mSize - 1;
        while (a <= b)
        {
            s32 sum = a + b;
            if (sum < 0)
                ++sum;
            const s32 m = sum >> 1;
            const s32 c = cmp(&mBuffer[m], &item);
            if (c == 0)
                return m;
            if (c < 0)
                a = m + 1;
            else
                b = m - 1;
        }
        return -1;
    }

protected:
    static s32 compareT(const T* lhs, const T* rhs)
    {
        if (*lhs < *rhs)
            return -1;
        if (*rhs < *lhs)
            return 1;
        return 0;
    }

    s32 mSize;
    T* mBuffer;
};

}  // namespace sead

#endif  // SEAD_BUFFER_H_
