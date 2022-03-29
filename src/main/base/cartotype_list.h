/*
cartotype_list.h
Copyright (C) 2004-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_LIST_H__
#define CARTOTYPE_LIST_H__

#include <cartotype_types.h>
#include <cartotype_errors.h>

namespace CartoType
{

/** The base class for list items in the CList and CPointerList template classes. */
class TListLink
    {
    public:
    /**
    A pointer to the previous item in the list, or, if this is the first item,
    the last item in the list.
    */
    TListLink* iPrev = nullptr;

    /**
    A pointer to the next item in the list, or, if this is the last item,
    the first item in the list.
    */
    TListLink* iNext = nullptr;
    };

/** The base class for the CList and CPointerList template classes. */
class CListBase
    {
    public:
    CListBase() = default;
    /** Links aLink, which must not be null, into the list before aNext, or at the end of the list if aNext is null. Does not change the start item. */
    void Link(TListLink* aLink,TListLink* aNext);
    /** Inserts aLink, which must not be null, into the list before aNext. */
    void Insert(TListLink* aLink,TListLink* aNext);
    /** Deletes aLink, which must not be null. */
    void Delete(TListLink* aLink);
    /** Moves aLink to the start of the list. */
    void MoveToStart(TListLink* aLink)
        {
        if (aLink && aLink != iStart)
            {
            aLink->iPrev->iNext = aLink->iNext;
            aLink->iNext->iPrev = aLink->iPrev;
            aLink->iNext = iStart;
            aLink->iPrev = iStart->iPrev;
            iStart->iPrev->iNext = aLink;
            iStart->iPrev = aLink;
            iStart = aLink;
            }
        }

    /** Moves aLink to the position before aNext. */
    void MoveTo(TListLink* aLink,TListLink* aNext)
        {
        assert(aLink && aNext);
        if (aLink != aNext && aLink->iNext != aNext)
            {
            aLink->iPrev->iNext = aLink->iNext;
            aLink->iNext->iPrev = aLink->iPrev;
            aLink->iNext = aNext;
            aLink->iPrev = aNext->iPrev;
            aNext->iPrev->iNext = aLink;
            aNext->iPrev = aLink;
            }
        if (aNext == iStart)
            iStart = aLink;
        }

    /** Returns the first item in the list, or null if the list is empty. */
    const TListLink* Start() const { return iStart; }
    /** Returns a non-const pointer to the first item in the list, or null if the list is empty. */
    TListLink* Start() { return iStart; }

    protected:
    /** The first link in the list. If the list is empty it is null. */
    TListLink* iStart = nullptr;
    /** The number of elements in the list. */
    int32_t iElements = 0;

    private:
    CListBase(const CListBase&) = delete;
    CListBase(CListBase&&) = delete;
    void operator=(const CListBase&) = delete;
    void operator=(CListBase&&) = delete;
    };

/** A list class designed to hold small objects directly. */
template<class T> class CList: private CListBase
    {
    public:
    ~CList()
        {
        Clear();
        }
    /** Deletes all objects from the list. */
    void Clear()
        {
        TLink* p = (TLink*)iStart;
        while (iElements > 0)
            {
            TLink* next = (TLink*)(p->iNext);
            delete p;
            p = next;
            iElements--;
            }
        iStart = nullptr;
        }

    private:
    class TLink: public TListLink
        {
        public:
        TLink(const T& aT): iT(aT) { }
        T iT;
        };

    public:
    /**
    A non-constant iterator for traversing lists.
    The iterator's 'operator TYPE*' function returns a pointer to the iterator's current element,
    or null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TIter
        {
        public:
        /** Creates an iterator referring to the link aLink. If aLink is null, the iterator is a null iterator and cannot be incremented or decremented. */
        TIter(CListBase& aList,TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        /** Returns a pointer to the current item, or null if this is a null iterator. */
        operator T*() { return iCur ? &iCur->iT : 0 ; }
        /** Moves to the next item. The iterator becomes a null iterator if there are no more items. */
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        /** Moves to the previous item. The iterator becomes a null iterator if there are no more items. */
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        /** Returns a pointer to the current link. */
        TLink* Cur() { return iCur; }
        /** Moves the current item to the start of the list. */
        void MoveCurrentToStart() { iList->MoveToStart(iCur); }
        /** Inserts the item aT before the current item. */
        TResult Insert(const T& aT) { return iList->Insert(new TLink(aT),iCur); }
        /** Moves the link aLink to the current position. */
        void MoveToCurrentPosition(TListLink* aLink) { iList->MoveTo(aLink,iCur); }
        /** Returns true if this iterator is at the start of the list. */
        bool AtStart() const { return iCur == iList->Start(); }

        private:
        CListBase* iList;
        TLink* iCur;
        };
    /**
    A constant iterator for traversing lists.
    The iterator's 'operator const TYPE*' function returns a const pointer to the iterator's current element,
    or null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TConstIter
        {
        public:
        /** Creates a constant iterator referring to the link aLink. If aLink is null, the iterator is a null iterator and cannot be incremented or decremented. */
        TConstIter(const CListBase& aList,const TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        /** Returns the current item pointed to, or null if this is a null iterator. */
        operator const T*() const { return iCur ? &iCur->iT : 0 ; }
        /** Moves to the next item. The iterator becomes a null iterator if there are no more items. */
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        /** Moves to the previous item. The iterator becomes a null iterator if there are no more items. */
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        /** Returns true if this iterator is at the start of the list. */
        bool AtStart() const { return iCur == iList->Start(); }

        private:
        const CListBase* iList;
        const TLink* iCur;
        };

    /** Returns the number of elements in the list. */
    int32_t Count() const { return iElements; }
    /** Inserts a new element at the start of the list. */
    void Prefix(const T& aT) { CListBase::Insert(new TLink(aT),iStart); }
    /** Inserts a new element at the end of the list. */
    void Append(const T& aT) { CListBase::Link(new TLink(aT),iStart); }
    /** Deletes the element referred to by the iterator aIter. */
    void Delete(TIter& aIter)
        {
        TListLink* p = aIter.Cur(); assert(p); aIter.Next(); CListBase::Delete(p); delete p;
        }
    /** Returns a non-constant iterator referring to the first element of the list. */
    TIter First() { return TIter(*this,iStart); }
    /** Returns a non-constant iterator referring to the last element of the list. */
    TIter Last() { return TIter(*this,iStart ? iStart->iPrev : 0); }
    /** Returns a constant iterator referring to the first element of the list. */
    TConstIter First() const { return TConstIter(*this,iStart); }
    /** Returns a constant iterator referring to the last element of the list. */
    TConstIter Last() const { return TConstIter(*this,iStart ? iStart->iPrev : 0); }
    /** Returns a non-constant iterator referring to a specified link. */
    TIter Iter(TListLink* aLink) { return TIter(*this,aLink); }
    /** Returns a constant iterator referring to a specified link. */
    TConstIter Iter(const TListLink* aLink) { return TConstIter(*this,aLink); }
    };

/**
A list class for storing large objects via pointers.
The list takes ownership of the objects.
*/
template<class T> class CPointerList: private CListBase
    {
    public:
    CPointerList() = default;
    ~CPointerList()
        {
        Clear();
        }
    /** Deletes all objects from the list. */
    void Clear()
        {
        TLink* p = (TLink*)iStart;
        while (iElements > 0)
            {
            TLink* next = (TLink*)(p->iNext);
            delete (T*)(p->iPtr);
            delete p;
            p = next;
            iElements--;
            }
        iStart = nullptr;
        }

    private:
    class TLink: public TListLink
        {
        public:
        TLink(const T* aPtr): iPtr(aPtr) { }
        const T* iPtr;
        };

    public:
    /**
    A non-constant iterator for traversing lists of pointers.
    There are both const and non-const 'operator TYPE*' functions to return the iterator's current element.
    They return null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TIter
        {
        public:
        /** Creates an iterator referring to the link aLink. If aLink is null, the iterator is a null iterator and cannot be incremented or decremented. */
        TIter(CListBase& aList,TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        /** Returns a const pointer to the current item, or null if this is a null iterator. */
        operator const T*() { return iCur ? iCur->iPtr : 0 ; }
        /** Returns a pointer to the current item, or null if this is a null iterator. */
        operator T*() { return iCur ? (T*)(iCur->iPtr) : 0 ; }
        /** Moves to the next item. The iterator becomes a null iterator if there are no more items. */
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        /** Moves to the previous item. The iterator becomes a null iterator if there are no more items. */
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        /** Returns a pointer to the current link. */
        TLink* Cur() { return iCur; }
        /** Moves the current item to the start of the list. */
        void MoveCurrentToStart() { iList->MoveToStart(iCur); }
        /** Inserts the item aT before the current item. */
        TResult Insert(T* aPtr) { return iList->Insert(new TLink(aPtr),iCur); }
        /** Moves the link aLink to the current position. */
        void MoveToCurrentPosition(TListLink* aLink) { iList->MoveTo(aLink,iCur); }
        /** Returns true if this iterator is at the start of the list. */
        bool AtStart() const { return iCur == iList->Start(); }

        private:
        CListBase* iList;
        TLink* iCur;
        };
    /**
    A constant iterator for traversing lists of pointers.
    The iterator's 'operator const TYPE*' function returns a const pointer to the iterator's current element,
    or null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TConstIter
        {
        public:
        /** Creates a constant iterator referring to the link aLink. If aLink is null, the iterator is a null iterator and cannot be incremented or decremented. */
        TConstIter(const CListBase& aList,const TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        /** Returns a const pointer to the current item, or null if this is a null iterator. */
        operator const T*() const { return iCur ? iCur->iPtr : 0 ; }
        /** Moves to the next item. The iterator becomes a null iterator if there are no more items. */
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        /** Moves to the previous item. The iterator becomes a null iterator if there are no more items. */
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        /** Returns true if this iterator is at the start of the list. */
        bool AtStart() const { return iCur == iList->Start(); }
        private:
        const CListBase* iList;
        const TLink* iCur;
        };

    /** Returns the number of elements in the list. */
    int32_t Count() const { return iElements; }
    /** Inserts a new element at the start of the list. */
    void Prefix(const T* aPtr) { CListBase::Insert(new TLink(aPtr),iStart); }
    /** Inserts a new element at the end of the list. */
    void Append(const T* aPtr) { CListBase::Link(new TLink(aPtr),iStart); }
    /** Deletes the element referred to by the iterator aIter. */
    void Delete(TIter& aIter)
        {
        TLink* p = aIter.Cur();
        assert(p);
        aIter.Next();
        CListBase::Delete(p);
        delete (T*)(p->iPtr);
        delete p;
        }
    /** Returns a non-constant iterator referring to the first element of the list. */
    TIter First() { return TIter(*this,iStart); }
    /** Returns a non-constant iterator referring to the last element of the list. */
    TIter Last() { return TIter(*this,iStart ? iStart->iPrev : 0); }
    /** Returns a constant iterator referring to the first element of the list. */
    TConstIter First() const { return TConstIter(*this,iStart); }
    /** Returns a constant iterator referring to the last element of the list. */
    TConstIter Last() const { return TConstIter(*this,iStart ? iStart->iPrev : 0); }
    /** Returns a non-constant iterator referring to a specified link. */
    TIter Iter(TListLink* aLink) { return TIter(*this,aLink); }
    /** Returns a constant iterator referring to a specified link. */
    TConstIter Iter(const TListLink* aLink) { return TConstIter(*this,aLink); }
    };

} // namespace CartoType

#endif
