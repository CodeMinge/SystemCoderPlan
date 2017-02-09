#include <stdio.h>
#include <assert.h>
#include "dlist.h"

static DListRet sum_cb(void* ctx, void* data)
{
    long long* sum = ctx; //long long是一个数据类型
    *sum += (int)data;

    return DLIST_RET_OK;
}

typedef struct _MaxCtx
{
	int is_first;
	int max;
}MaxCtx;

static DListRet max_cb(void* ctx, void* data)
{
    MaxCtx* maxCtx = ctx;

    if(maxCtx->is_first)
    {
        maxCtx->is_first = 0;
        maxCtx->max = (int)data;
    }
    else
    {
        if(maxCtx->max < (int)data)
        {
            maxCtx->max = (int)data;
        }
    }

    return DLIST_RET_OK;
}

static DListRet print_int(void* ctx, void* data)
{
	printf("%d ", (int)data);

	return DLIST_RET_OK;
}

int main(int argc, char* argv[])
{
	int i = 0;
	int n = 100;
	long long sum = 0;
	MaxCtx max_ctx = {.is_first = 1, 0};
	DList* dlist = dlist_create();

	for(i = 0; i < n; i++)
	{
		assert(dlist_append(dlist, (void*)i) == DLIST_RET_OK);
	}

	dlist_foreach(dlist, print_int, NULL);
	dlist_foreach(dlist, max_cb, &max_ctx);
	dlist_foreach(dlist, sum_cb, &sum);

	printf("\nsum=%lld max=%d\n", sum, max_ctx.max);
	dlist_destroy(dlist);

	return 0;
}
