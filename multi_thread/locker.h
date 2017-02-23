#include "common.h"

#ifndef LOCKER_H_INCLUDED
#define LOCKER_H_INCLUDED

DECLS_BEGIN

struct _Locker;
typedef struct _Locker Locker;

typedef Ret  (*LockerLockFunc)(Locker* thiz);
typedef Ret  (*LockerUnlockFunc)(Locker* thiz);
typedef void (*LockerDestroyFunc)(Locker* thiz);

struct _Locker
{
    LockerLockFunc lock;
    LockerUnlockFunc unlock;
    LockerDestroyFunc destroy;

    char priv[0];
};

static inline Ret locker_lock(Locker* thiz)
{
    return_val_if_fail(thiz != NULL && thiz->lock != NULL, RET_INVALID_PARAMS);
    return thiz->lock(thiz);
}

static inline Ret locker_unlock(Locker* thiz)
{
    return_val_if_fail(thiz != NULL && thiz->unlock != NULL, RET_INVALID_PARAMS);
    return thiz->unlock(thiz);
}

static inline void locker_destroy(Locker* thiz)
{
    return_if_fail(thiz != NULL && thiz->destroy != NULL);
    thiz->destroy(thiz);
}

DECLS_END

#endif // LOCKER_H_INCLUDED
