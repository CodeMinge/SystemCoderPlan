/*
 *通用双向链表的实现，包含链表的创建、删除和链表节点的创建、删除、检索等
 *关注点：通用性的实现方式、内存申请、size_t的使用
 */

#include <stdlib.h>
#include "dlist.h"

typedef struct _DListNode
{
    struct _DListNode *prev;
    struct _DListNode *next;
    void *data;
}DListNode;

typedef struct _DList
{
    Locker* locker;
	DListNode* first;
	void* data_destroy_ctx;
	DListDataDestroyFunc data_destroy;
};

static void dlist_destroy_data(DList* thiz, void* data)
{
    if(thiz->data_destroy != NULL)
    {
        thiz->data_destroy(thiz->data_destroy_ctx, data);
    }
}

static DListNode* dlist_node_create(DList* thiz, void* data)
{
    DListNode* node = malloc(sizeof(DListNode));

    if(node != NULL)
    {
        node->data = data;
        node->prev = NULL;
        node->next = NULL;
    }

    return node;
}

static void dlist_destroy_node(DList* thiz, DListNode* node)
{
    if(node != NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        dlist_destroy_data(thiz, node->data);
        SAFE_FREE(node);
    }
}

static void dlist_lock(DList* thiz)
{
    if(thiz->locker != NULL)
    {
        locker_lock(thiz->locker);
    }
}

static void dlist_unlock(DList* thiz)
{
    if(thiz->locker != NULL)
    {
        locker_unlock(thiz->locker);
    }
}

static void dlist_destroy_locker(DList* thiz)
{
    if(thiz->locker != NULL)
    {
        locker_unlock(thiz->locker);
        locker_destroy(thiz->locker);
    }
}

DList* dlist_create(DListDataDestroyFunc data_destroy, void* ctx, Locker* locker)
{
    DList* dList = malloc(sizeof(DList));

    if(dList != NULL)
    {
        dList->data_destroy = data_destroy;
        dList->first = NULL;
        dList->locker = locker;
        dList->data_destroy_ctx = ctx;
    }

    return dList;
}

static DListNode* dlist_get_node(DList* thiz, size_t index, int fail_return_last)
{
    return_val_if_fail(thiz != NULL, NULL);

	DListNode* iter = thiz->first;

	while(iter != NULL && iter->next != NULL && index > 0)
	{
		iter = iter->next;
		index--;
	}

	if(!fail_return_last)
	{
		iter = index > 0 ? NULL : iter;
	}

	return iter;
}

Ret dlist_insert(DList* thiz, size_t index, void* data)
{
    Ret ret = RET_OK;
    DListNode* node = NULL;
    DListNode* cursor = NULL;

    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    dlist_lock(thiz);

    if((node = dlist_node_create(thiz, data)) == NULL)
    {
        ret = RET_OOM;
    }

    if(thiz->first == NULL)
    {
        thiz->first = node;
        return RET_OK;
    }

    cursor = dlist_get_node(thiz, index, 1);

    if(index < dlist_length(thiz))
    {
        if(thiz->first == cursor)
        {
            thiz->first = node;
        }
        else
        {
           cursor->prev->next = node;
           node->prev =  cursor->prev;
        }
        node->next = cursor;
        cursor->prev = node;
    }
    else
    {
        cursor->next = node;
        node->prev = cursor;
    }

    dlist_unlock(thiz);

    return ret;
}

Ret dlist_prepend(DList* thiz, void* data)
{
    return dlist_insert(thiz, 0, data);
}

Ret dlist_append(DList* thiz, void* data)
{
    return dlist_insert(thiz, -1, data);
}

Ret dlist_delete(DList* thiz, size_t index)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    dlist_lock(thiz);

    Ret ret = RET_OK;
	DListNode* cursor = dlist_get_node(thiz, index, 0);

	if(cursor == NULL)
    {
        ret = RET_INVALID_PARAMS;
    }
	else
	{
		if(cursor == thiz->first)
		{
			thiz->first = cursor->next;
		}

		if(cursor->next != NULL)
		{
			cursor->next->prev = cursor->prev;
		}

		if(cursor->prev != NULL)
		{
			cursor->prev->next = cursor->next;
		}

		dlist_destroy_node(thiz, cursor);
	}

    dlist_unlock(thiz);

	return ret;
}

Ret dlist_get_by_index(DList* thiz, size_t index, void** data)
{
    return_val_if_fail(thiz != NULL && data != NULL, RET_INVALID_PARAMS);
    dlist_lock(thiz);

    DListNode* cursor = dlist_get_node(thiz, index, 0);

    if(cursor != NULL)
    {
        *data = cursor->data;
    }
    dlist_unlock(thiz);

    return cursor != NULL ? RET_OK : RET_FAIL;
}

Ret dlist_set_by_index(DList* thiz, size_t index, void* data)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    dlist_lock(thiz);

    DListNode* cursor = dlist_get_node(thiz, index, 0);

    if(cursor != NULL)
    {
        cursor->data = data;
    }
    dlist_unlock(thiz);

    return cursor != NULL ? RET_OK : RET_FAIL;
}

size_t   dlist_length(DList* thiz)
{
    return_val_if_fail(thiz != NULL, 0);
    dlist_lock(thiz);

    size_t length = 0;
    DListNode* iter = thiz->first;

    while(iter != NULL)
    {
       length ++;
       iter = iter->next;
    }
    dlist_unlock(thiz);

    return length;
}

Ret dlist_foreach(DList* thiz, DListDataVisitFunc visit, void* ctx)
{
    return_val_if_fail(thiz != NULL && visit != NULL, RET_INVALID_PARAMS);
    dlist_lock(thiz);

	Ret ret = RET_OK;
	DListNode* iter = thiz->first;

	while(iter != NULL && ret != RET_STOP)
	{
		ret = visit(ctx, iter->data);

		iter = iter->next;
	}

	dlist_unlock(thiz);

	return ret;
}

int dlist_find(DList* thiz, DListDataCompareFunc cmp, void* ctx)
{
	int i = 0;
	DListNode* iter = NULL;

	return_val_if_fail(thiz != NULL && cmp != NULL, -1);

	dlist_lock(thiz);

	iter = thiz->first;
	while(iter != NULL)
	{
		if(cmp(ctx, iter->data) == 0)
		{
			break;
		}
		i++;
		iter = iter->next;
	}

	dlist_unlock(thiz);

	return i;
}

void dlist_destroy(DList* thiz)
{
    return_if_fail(thiz != NULL);
    dlist_lock(thiz);

    DListNode* iter = thiz->first;
	DListNode* next = NULL;

	while(iter != NULL)
	{
		next = iter->next;
		dlist_destroy_node(thiz, iter);
		iter = next;
	}

	thiz->first = NULL;
	dlist_destroy_locker(thiz);
	SAFE_FREE(thiz);

	dlist_unlock(thiz);

	return;
}

#ifdef DLIST_TEST

#include <assert.h>
#include <pthread.h>
#include "locker_pthread.h"

static int cmp_int(void* ctx, void* data)
{
	return (int)data - (int)ctx;
}

static Ret print_int(void* ctx, void* data)
{
	printf("%d ", (int)data);

	return RET_OK;
}


static void test_int_dlist(void)
{
	int i = 0;
	int n = 10;
	int data = 0;
	DList* dlist = dlist_create(NULL, NULL, NULL);

    printf("---test append---\n");
	for(i = 0; i < n; i++)
	{
		dlist_append(dlist, (void*)i);
	}
	dlist_foreach(dlist, print_int, NULL);
	printf("---test append over---\n");

	printf("---test prepend---\n");
	for(i = 0; i < n; i++)
	{
		dlist_prepend(dlist, (void*)(2*i));
	}
	dlist_foreach(dlist, print_int, NULL);
	printf("---test prepend over---\n");

    printf("---test length, get_by_index(5), find(10), set_by_index(11) ---\n");
    i = 5;
	dlist_get_by_index(dlist, i, (void**)&data);
	printf("length = %d, get %d, find %d\n", dlist_length(dlist), data, dlist_find(dlist, cmp_int, (void*)10));
	i = 11;
	dlist_set_by_index(dlist, i, (void*)(2*i));
	dlist_foreach(dlist, print_int, NULL);
	printf("---test over---\n");

    printf("---test delete(5 and 0)---\n");
	dlist_delete(dlist, 5);
	dlist_delete(dlist, 0);
    dlist_foreach(dlist, print_int, NULL);
    printf("---test delete over---\n");

	return;
}

static void test_invalid_params(void)
{
	printf("===========Warning is normal begin==============\n");
	assert(dlist_length(NULL) == 0);
	assert(dlist_prepend(NULL, 0) == RET_INVALID_PARAMS);
	assert(dlist_append(NULL, 0) == RET_INVALID_PARAMS);
	assert(dlist_delete(NULL, 0) == RET_INVALID_PARAMS);
	assert(dlist_insert(NULL, 0, 0) == RET_INVALID_PARAMS);
	assert(dlist_set_by_index(NULL, 0, 0) == RET_INVALID_PARAMS);
	assert(dlist_get_by_index(NULL, 0, NULL) == RET_INVALID_PARAMS);
	assert(dlist_find(NULL, NULL, NULL) < 0);
	assert(dlist_foreach(NULL, NULL, NULL) == RET_INVALID_PARAMS);
	printf("===========Warning is normal end==============\n");

	return;
}

static void single_thread_test(void)
{
	test_int_dlist();
	test_invalid_params();

	return;
}

static void* producer(void* param)
{
	int i = 0;
	DList* dlist = (DList*)param;

	for(i = 0; i < 100; i++)
	{
		assert(dlist_append(dlist, (void*)i) == RET_OK);
	}
	sleep(1);
	for(i = 0; i < 100; i++)
	{
		assert(dlist_prepend(dlist, (void*)i) == RET_OK);
	}

	return NULL;
}

static void* consumer(void* param)
{
	int i = 0;
	DList* dlist = (DList*)param;

	for(i = 0; i < 200; i++)
	{
		usleep(20);
		assert(dlist_delete(dlist, 0) == RET_OK);
	}

	return NULL;
}

static void multi_thread_test(void)
{
	pthread_t consumer_tid = 0;
	pthread_t producer_tid = 0;
	DList* dlist = dlist_create(NULL, NULL, locker_pthread_create());
	pthread_create(&producer_tid, NULL, producer, dlist);
	pthread_create(&consumer_tid, NULL, consumer, dlist);

	pthread_join(consumer_tid, NULL);
	pthread_join(producer_tid, NULL);

	return;
}
int main(int argc, char* argv[])
{
	//single_thread_test();
	multi_thread_test();

	return 0;
}
#endif


