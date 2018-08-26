#include "DefaultAllocator.h"

#include <stdio.h>

//function�����ڴ������һ����ڴ�
template <bool threads, int inst>
void* __DefaultAllocTemplate<threads, inst>::ChunkAlloc(size_t size, size_t& nobjs)
{
	size_t totalbytes = nobjs*size;
	size_t leftbytes = _endfree - _startfree;

	//a) �ڴ�������㹻�ڴ�
	if (leftbytes >= totalbytes){
		printf("�ڴ�����㹻��%lu��������ڴ��\n", nobjs);
		void* ret = _startfree;
		_startfree += totalbytes;
		return ret;

		//b) �ڴ�ؽ�ʣ���ֶ����ڴ��
	}else if (leftbytes > size){
		nobjs = leftbytes/size;  /*�����ܹ�ʹ�ö������*/
		totalbytes = size*nobjs;
		printf("�ڴ��ֻ��%lu��������ڴ��\n", nobjs);

		void* ret = _startfree;
		_startfree += totalbytes;
		return ret;

		//c) �ڴ����ʣ���ڴ治��һ��������С
	}else{
		// 1.�ȴ�����ڴ��ʣ���С���ڴ棬����ͷ�嵽��Ӧ����������
		if(leftbytes > 0){
			size_t index = FREELIST_INDEX(leftbytes);
			((Obj*)_startfree)->_freelistlink = _freelist[index];
			_freelist[index] = (Obj*)_startfree;
		}

		// 2.����malloc��������һ���ڴ�����ڴ��
		size_t bytesToGet = totalbytes*2 + ROUND_UP(_heapsize>>4);
		_startfree = (char*)malloc(bytesToGet);

		printf("�ڴ��û���ڴ棬��ϵͳ����%lubytes\n", bytesToGet);

		if (_startfree == NULL){	
			//3. malloc�����ڴ�ʧ�ܣ��ڴ��û���ڴ油�����������������������
			size_t index = FREELIST_INDEX(size);
			for (; index < __NFREELISTS; ++index){
				//���������ó�һ��ŵ��ڴ�أ���
				if (_freelist[index]){				
					_startfree = (char*)_freelist[index]; //BUG ??
					Obj* obj = _freelist[index];
					_freelist[index] = obj->_freelistlink;
					return ChunkAlloc(size, nobjs);  
				}
			}
			//������ɽ�����һ��. ���ڴ�ʵ�ڳԽ�����һ�����������쳣����
			_startfree = (char*)__MallocAllocTemplate<0>::Allocate(bytesToGet);
		}
		_endfree = NULL;  /*in case of exception.  ������֤�쳣��ȫ*/

		_heapsize += bytesToGet;
		_endfree = _startfree + bytesToGet;

		return ChunkAlloc(size, nobjs);
	}
}

template <bool threads, int inst>
void* __DefaultAllocTemplate<threads, inst>::Refill(size_t bytes)
{
	size_t nobjs = 20;   /*Ĭ�ϴ��ڴ��ȡ20��������*/
	//���ڴ�����õ�һ����ڴ�
	char* chunk = (char*)ChunkAlloc(bytes, nobjs);
	if (nobjs == 1)      /*ֻȡ����һ��*/
		return chunk;

	size_t index = FREELIST_INDEX(bytes);
	printf("����һ�����󣬽�ʣ��%lu������ҵ�freelist[%lu]����\n", nobjs-1, index);

	Obj* cur = (Obj*)(chunk + bytes);
	_freelist[index] = cur;
	for (size_t i = 0; i < nobjs-2; ++i){
		Obj* next = (Obj*)((char*)cur + bytes);
		cur->_freelistlink = next;

		cur = next;
	}

	cur->_freelistlink = NULL;

	return chunk;
}

template <bool threads, int inst>
void* __DefaultAllocTemplate<threads, inst>::Allocate(size_t n)
{ 
	printf("�����ռ�����������%lubytes\n", n);

	if (n > __MAX_BYTES)//����һ���ռ�������
		return __MallocAllocTemplate<0>::Allocate(n); 

	size_t index = FREELIST_INDEX(n);
	if (_freelist[index] == NULL){
		printf("freelist[%lu]����û���ڴ�����,��Ҫ���ڴ������\n", index);

		return Refill(ROUND_UP(n));//��������б�,����ȡ��

	}else{
		printf("freelist[%lu]ȡһ�����󷵻�\n", index);

		Obj* ret = _freelist[index];
		_freelist[index] = ret->_freelistlink;
		return ret;
	}
}

//��Ҫ�ڶ����������Դ˼���������freelist�б��ж�Ӧ�±�
	template <bool threads, int inst>
void __DefaultAllocTemplate<threads, inst>::Deallocate(void* p, size_t n)
{
	if (n > __MAX_BYTES){
		__MallocAllocTemplate<0>::Deallocate(p, n);
	}else{
		size_t index = FREELIST_INDEX(n);
		printf("�����ռ��������ͷŶ��󣬹ҵ�freelist[%lu]��\n", index);
		//ͷ�嵽������
		((Obj*)p)->_freelistlink = _freelist[index];
		_freelist[index] = (Obj*)p;
	}
}


//test����
void Test_Alloc3()
{
	for(size_t i = 0; i < 20; ++i)
	{
		void* p = __DefaultAllocTemplate<false, 0>::Allocate(6);
		__DefaultAllocTemplate<false, 0>::Deallocate(p, 6);
	}

	__DefaultAllocTemplate<false, 0>::Allocate(6);
}

//////////////////////////////////////////////////////////////////
#ifdef __USE_MALLOC
typedef __MallocAllocTemplate<0> alloc;
#else
typedef __DefaultAllocTemplate<false, 0> alloc;
#endif


template<class T, class Alloc>
class SimpleAlloc 
{
	public:
		static T* Allocate(size_t n){ 
			return 0 == n? 0 : (T*) Alloc::Allocate(n * sizeof (T));
		}

		static T* Allocate(void){ 
			return (T*) Alloc::Allocate(sizeof (T));
		}

		static void Deallocate(T *p, size_t n){ 
			if (0 != n)
				Alloc::Deallocate(p, n * sizeof (T));
		}

		static void Deallocate(T *p){ 
			Alloc::Deallocate(p, sizeof (T));
		}
};
