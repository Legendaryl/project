#pragma once
#include "Allocator.h"
//#include "TraceLog.h"

///////////////////////////////////////////////////////////////////////
//�����ռ�������

template <bool threads, int inst>
class __DefaultAllocTemplate
{
public:
	// 65	72  -> index=8
	// 72	79
	static size_t FREELIST_INDEX(size_t n){
		return ((n + __ALIGN-1)/__ALIGN - 1);
	}

	// 65	72	-> 72
	// 72	79
	static size_t ROUND_UP(size_t bytes)  {
		return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
	}
	
	static void* ChunkAlloc(size_t size, size_t& nobjs);//��ȡ����ڴ�	
	static void* Refill(size_t bytes);                  //�����������	
	static void* Allocate(size_t n);                    //���䷵��С�ڴ��	
	static void Deallocate(void* p, size_t n);          //��������ڴ�

private:
	enum {__ALIGN = 8 };
	enum {__MAX_BYTES = 128 }; 
	enum {__NFREELISTS = __MAX_BYTES/__ALIGN };

	union Obj{
		union Obj* _freelistlink;
		char client_data[1];    /* The client sees this.  */
	};

	// ��������
	static Obj* _freelist[__NFREELISTS];

	// �ڴ��
	static char* _startfree;
	static char* _endfree;
	static size_t _heapsize;
};

//__DefaultAllocTemplate��Ա��ʼ��
template <bool threads, int inst>
typename __DefaultAllocTemplate<threads, inst>::Obj*
__DefaultAllocTemplate<threads, inst>::_freelist[__NFREELISTS] = {0};

// �ڴ��
template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_startfree = NULL;

template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_endfree = NULL;

template <bool threads, int inst>
size_t __DefaultAllocTemplate<threads, inst>::_heapsize = 0;


void Test_Alloc3();