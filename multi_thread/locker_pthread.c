#include <stdlib.h>
#include <pthread.h>
#include "locker_pthread.h"

typedef struct _PrivInfo
{
	pthread_mutex_t mutex;
}PrivInfo;

static Ret locker_pthread_lock(Locker* thiz)
{
    PrivInfo* priv = (PrivInfo*)thiz->priv;

    int ret = pthread_mutex_lock(&priv->mutex);

    return ret == 0 ? RET_OK : RET_FAIL;
}

static Ret locker_pthread_unlock(Locker* thiz)
{
    PrivInfo* priv = (PrivInfo*)thiz->priv;

    int ret = pthread_mutex_unlock(&priv->mutex);

    return ret == 0 ? RET_OK : RET_FAIL;
}

static void locker_pthread_destroy(Locker* thiz)
{
    PrivInfo* priv = (PrivInfo*)thiz->priv;

    pthread_mutex_destroy(&priv->mutex);

    SAFE_FREE(thiz);
}

Locker* locker_pthread_create(void)
{
    Locker* locker = (Locker*)malloc(sizeof(Locker) + sizeof(PrivInfo));

    if(locker != NULL)
    {
        PrivInfo* priv = (PrivInfo*)locker->priv;

        locker->lock = locker_pthread_lock;
        locker->unlock = locker_pthread_unlock;
        locker->destroy = locker_pthread_destroy;

        pthread_mutex_init(&(priv->mutex), NULL);
    }

    return locker;
}

