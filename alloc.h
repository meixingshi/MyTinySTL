#ifndef MYTINYSTL_ALLOC_H_
#define MYTINYSTL_ALLOC_H_

#include <cstdlib>
#include "allocator.h"


namespace mystl
{

    enum{   ALIGN = 8   };

    enum{   MAXBYTES = 128  };

    enum{   NFREELISTS = MAXBYTES / ALIGN    };

    enum{   ENOBJS =20   };

class alloc{
          
    private:
        union obj{
            union obj *next;
            char client_data[1];
        };
        static char *start_free;
        static char *end_free;
        static size_t heap_size;
        static obj *free_list[NFREELISTS];

    public:
        static void *allocate(size_t n);
        static void deallocate(void *p, size_t n);
        static void *reallocate(void *p, size_t old_size, size_t new_size);

    private:
        //����ȥ8�ı���(2��n�η�������ʹ��)
        static size_t ROUND_UP(size_t bytes){
            return ((bytes + ALIGN - 1) & ~(ALIGN - 1));
        }

        static size_t FREELIST_INDEX(size_t bytes);
        static void *refill(size_t n);
        static char *chunk_alloc(size_t size, size_t &nobjs);
    };

    //��̬������ʼ��
    char *alloc::start_free = nullptr;
    char *alloc::end_free = nullptr;
    size_t alloc::heap_size = 0;

    alloc::obj* alloc::free_list[NFREELISTS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };



    //**************�ӿں���**************
    //�����ڴ�
    //����free_listÿһ���൱��һ�����������#11�����ÿ�����Ϊ8*12=98bytes������
    void *alloc::allocate(size_t n)
    {
        obj **my_free_list;
        obj *result;
        if (n > MAXBYTES)//���n>Ԥ���ڴ棬���õ�һ��������
            return std::malloc(n);
        my_free_list = free_list + FREELIST_INDEX(n);//Ѱ��freelist����ƥ���
        result = *my_free_list;//��ȡ��freelist[index]����ĵ�ַ�������ַ����һ��96bytes���ڴ�
        if (result == nullptr)
        {
            void *r = refill(ROUND_UP(n));
            return r;
        }
        *my_free_list = result->next;//����freelist[index]����ĵ�ַ������ָ��result�������һ�����
        return result;//����reult
    }

    void alloc::deallocate(void *p, size_t n)//�ռ��ͷ�
    {
        if (n > (size_t)MAXBYTES)//���n>MAX
        {
            alllocator::deallocate(p);
            return;
        }
        obj *q = static_cast<obj *>(p);//static_cast����
        obj **my_free_list;
        my_free_list = free_list + FREELIST_INDEX(n);
        q->next = *my_free_list;
        *my_free_list = q;//�������ȥ֮������������������ϣ�
    }

    void *alloc::reallocate(void *p, size_t old_size, size_t new_size)//���ٺ���������
    {
        deallocate(p, old_size);//���پɵ�
        p = allocate(new_size);//�����µ�
        return p;
    }

    //****************�ڲ�����******************
    //����ַ������ĳ��λ��
    void *alloc::refill(size_t n)
    {//chunk�����ô��Σ����Ի����
        size_t nobjs = ENOBJS;
        char *chunk = chunk_alloc(n, nobjs);//ȡ��������Ϊfreelist���½���ַ
        obj **my_free_list;//
        obj *result, *cur_obj, *next_obj;
        if (nobjs == 1)//���ֻȡ����һ������͸�������ʹ��
            return chunk;
        my_free_list = free_list + FREELIST_INDEX(n);//������Ҫ���µ���freelist
        result = (obj*)chunk;//ǿ��ת����ַ����,ȡ������һ����ͻ�ʹ����������freelist
        //chunkΪchar�����飬����Ҫת��Ϊobj*,�����freelist[n]λ���ϵ�����ÿһ�������ڴ��Сobj+n*obj
        *my_free_list = next_obj = (obj *)(chunk + n);//��Ϊ��һ���Ѿ�Ҫ���أ�����ʣ��Ҫ�ҵ������ϵĵ�ַҪ+n
        for (size_t i = 1;; ++i)//��ʣ�µĹ�������������
        {
            cur_obj = next_obj;
            next_obj = (obj *)((char *)next_obj + n);//Ψһ�����ʣ�ΪʲôҪ����ת����next_obj�����Ѿ���obj��ָ����
            if (nobjs - 1 == i)//������
            {
                cur_obj->next = nullptr;
                break;
            }
            else
            {
                cur_obj->next = next_obj;//������
            }
        }
        return result;
    }

    char *alloc::chunk_alloc(size_t size, size_t &nobjs)//ȥ��������
    {
        char *result;//����ֵ
        size_t need_bytes = size * nobjs;//��Ҫ��������
        size_t pool_bytes = end_free - start_free;//�ڴ��л�ʣ�µ�������
        //���ʣ�µĻ���
        if (pool_bytes >= need_bytes){
            result = start_free;//�ͷ������
            start_free += need_bytes;//ͬʱ�������
            return result;
        }
        //�����Ҫ���������ǿ��Է���һ�����ϣ�size�������freelist��������n
        else if (pool_bytes >= size){
            nobjs = pool_bytes / size;//�����ܷ��ص���Ŀ
            need_bytes = size * nobjs;//�������ṩ�Ŀռ�͵�ַ
            result = start_free;
            start_free += need_bytes;
            return result;
        }
        else{//һ����������
            if (pool_bytes > 0){//����һ��˵��pool_bytes��ȻС��n�����Ȼ�����ҵ����������ҵ�һ��λ�÷Ž�ȥ
                obj **my_free_list = free_list+FREELIST_INDEX(pool_bytes);
                ((obj *)start_free)->next = *my_free_list;//һ��ʼ��������Ϊʲô֮�����start_free��>Ϊʲô��Ϊnull
                //��Ϊpool�����Ѿ�û���ˡ�������������ԭ����������free_list[pool_bytes]��Ϊ�㣬Ϊ0����free_list[size]
                *my_free_list = (obj *)start_free;//����free_list
            }
            size_t bytes_to_get = (need_bytes << 1) + ROUND_UP(heap_size >> 4);//���Ƴ�2
            start_free = (char *)malloc(bytes_to_get);//��������ڴ�

            if (!start_free){//δ�ܳɹ�,ȥ����Ÿ����������������
                obj **my_free_list, *p;
                for (size_t i = size; i <= MAXBYTES; i += ALIGN)
                {
                    my_free_list = free_list+FREELIST_INDEX(i);
                    p = *my_free_list;
                    if (p)
                    {
                        *my_free_list = p->next;//����free_list
                        start_free = (char *)p;//���ó������ڴ������ڴ��
                        end_free = start_free + i;
                        return chunk_alloc(size, nobjs);//�ڴ���Ѿ������ڴ�����ٴε���
                    }
                }
                //std::printf("out of memory");//�Ժ�Ŀռ䶼û���ˣ����Գ�����
                end_free = nullptr;//
                start_free=(char*)alllocator::malloc(bytes_to_get);//���õ�һ��������
                //throw std::bad_alloc();
            }
            end_free = start_free + bytes_to_get;//���¶���ָ��
            heap_size += bytes_to_get;//���¶���
            return chunk_alloc(size, nobjs);//�ٴεݹ�����Լ�
        }
    }

}
#endif
