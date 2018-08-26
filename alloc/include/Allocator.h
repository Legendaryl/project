#pragma once

#include <iostream>
#include <unistd.h>
using namespace std;
//#include "TraceLog.h"
//һ���ռ�������
typedef void(*HANDLE_FUNC)();
template <int inst> // instΪԤ�������������Ժ���չ
class __MallocAllocTemplate 
{
private:
	/*���庯��ָ�����ͳ�Ա������ص�ִ���û�
	�Զ�����ڴ��ͷź������ó�ԱĬ�����ò�ִ��*/
	static HANDLE_FUNC __malloc_alloc_oom_handler;

	static void* OOM_Malloc(size_t n){
		while (1){
			if (0 == __malloc_alloc_oom_handler){
				throw bad_alloc();
			}else{
				__malloc_alloc_oom_handler();  //�ͷ��ڴ�
				usleep(3000);
				void* ret = malloc(n);
				if (ret)
					return ret;
			}
		}
	}
public:
	static void* Allocate(size_t n)
	{
		void *result = malloc(n);
		//malloc����ʧ�ܣ�ִ��OOM_Malloc�����������ڴ�
		if (0 == result)
			result = OOM_Malloc(n);
		cout<<"����ɹ�!"<<endl;
		return result;
	}

	static void Deallocate(void *p, size_t /* n */)
	{
		free(p);
	}
	/*����oom_malloc���������*/
	static HANDLE_FUNC SetMallocHandler(HANDLE_FUNC f)
	{
		HANDLE_FUNC old = f;
		__malloc_alloc_oom_handler = f;
		return old;
	}
};

template<int inst>
HANDLE_FUNC __MallocAllocTemplate<inst>::__malloc_alloc_oom_handler = 0;

//�Զ�����ڴ��ͷź���
static void FreeMemory()
{
	cout<<"ִ���û��Զ��庯������ʼ�ͷ��ڴ�..."<<endl;
}
void Test_Alloc1();

void Test_Alloc2();





