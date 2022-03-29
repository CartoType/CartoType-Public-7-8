/*
cartotype_iter.h
Copyright (C) 2004 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_ITER_H__
#define CARTOTYPE_ITER_H__

#include <cartotype_types.h>
#include <cartotype_errors.h>

namespace CartoType
{

/** The iterator interface provides a sequence of objects of any type. */
template<class T> class MIter
    {
    public:
    /**
    Get the next object. Return KErrorNone if an object was retrieved,
    KErrorEndOfData if there are no more objects, or some other result code
    if there was a different type of error.
    */
    virtual TResult Next(T& aValue) = 0;
    /**
    Back up one position if possible so that the next call to Next
    returns the same object as the previous call.
    */
    virtual void Back() = 0;
    };

} // namespace CartoType

#endif
