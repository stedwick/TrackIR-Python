//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "Core/ThreadLock.h"
#include "cameralibraryglobals.h"

#ifdef __PLATFORM__LINUX__
#include <semaphore.h>
#include <stdlib.h>
#endif

// Trick to keep from blowing out the include tree =--

extern void QueueTriggerEvent( void* event );
extern void* QueueCreateEvent();
extern void QueueCloseEvent( void* event );
extern void QueueWaitForEvent( void* event );

// Queue Template Class Definition

#ifdef WIN32
template <class T>
class CLAPI Queue
{
public:
    template <class U>
    struct Item
    {
        Item* next;
        U item;
    };

    Queue()
    {
        mHead = mTail = nullptr;
        mEvent = QueueCreateEvent();
        mQueueSize = 0;
    }

    ~Queue()
    {
        QueueCloseEvent( mEvent );
    }

    bool IsEmpty() const
    {
        bool ret = false;

        mLock.Lock();

        if( mTail == nullptr && mHead == nullptr )
            ret = true;

        mLock.Unlock();

        return ret;
    }

    void Push( const T& newItem )
    {
        Queue::Item<T> *entry = new Queue::Item<T>();

        entry->item = newItem;
        entry->next = nullptr;

        mLock.Lock();

        if( mTail != nullptr )
            mTail->next = entry;
        else
            mHead = entry;

        mTail = entry;
        mQueueSize++;

        mLock.Unlock();

        QueueTriggerEvent( mEvent );
    }

    T Pop()
    {
        T value = 0;
        mLock.Lock();

        if( mHead != nullptr )
        {
            value = mHead->item;

            Item<T> *temp = mHead;

            mHead = mHead->next;

            delete temp;

            if( mHead == nullptr )
                mTail = nullptr;

            mQueueSize--;
        }

        mLock.Unlock();
        return value;
    }

    T Peek()
    {
        bool ret = false;
        mLock.Lock();

        T value = 0;

        if( mHead )
            value = mHead->item;

        mLock.Unlock();
        return value;
    }

    void Wait()
    {
        QueueWaitForEvent( mEvent );
    }

    /// <summary>This will cause any pending Wait() call to fall through to execution.</summary>
    void StopWaiting()
    {
        QueueTriggerEvent( mEvent );
    }

    int Size() const { return mQueueSize; }

private:
    Item<T>* mHead;
    Item<T>* mTail;
    void* mEvent;
    Core::cThreadLock mLock;
    int mQueueSize;
};
#endif

#ifdef __PLATFORM__LINUX__
template <class T>
class CLAPI Queue
{
public:
    template <class U>
    struct Item
    {
        Item* next;
        U item;
    };

    Queue()
    {
        mHead = mTail = 0; mEvent = QueueCreateEvent();
        mQueueSize = 0;
    }

    ~Queue()
    {
        QueueCloseEvent( mEvent );
    }

    bool IsEmpty() const
    {
        bool ret = false;

        mLock.Lock();

        if( mTail == 0 && mHead == 0 )
            ret = true;

        mLock.Unlock();

        return ret;
    }

    void Push( T newItem )
    {
        Queue::Item<T> *entry = new Queue::Item<T>();

        entry->item = newItem;
        entry->next = 0;

        mLock.Lock();

        if( mTail != 0 )
            mTail->next = entry;
        else
            mHead = entry;

        mTail = entry;
        mQueueSize++;

        mLock.Unlock();

        QueueTriggerEvent( mEvent );
    }

    T Pop()
    {
        bool ret = false;
        mLock.Lock();
        T value = 0;

        if( mHead != 0 )
        {
            value = mHead->item;

            Item<T> *temp = mHead;

            mHead = mHead->next;

            delete temp;

            if( mHead == 0 )
                mTail = 0;

            ret = true;

            mQueueSize--;
        }

        mLock.Unlock();
        return value;
    }

    T Peek()
    {
        bool ret = false;
        mLock.Lock();

        T value = 0;

        if( mHead )
            value = mHead->item;

        mLock.Unlock();
        return value;
    }

    void Wait()
    {
        QueueWaitForEvent( mEvent );
    }

    void StopWaiting()
    {
        QueueTriggerEvent( mEvent );
    }

    int Size() const { return mQueueSize; }

private:
    Item<T>* mHead;
    Item<T>* mTail;
    void* mEvent;
    Core::cThreadLock mLock;
    int mQueueSize;
};
#endif
