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
        //向上去8的倍数(2的n次方均可以使用)
        static size_t ROUND_UP(size_t bytes){
            return ((bytes + ALIGN - 1) & ~(ALIGN - 1));
        }

        static size_t FREELIST_INDEX(size_t bytes);
        static void *refill(size_t n);
        static char *chunk_alloc(size_t size, size_t &nobjs);
    };

    //静态变量初始化
    char *alloc::start_free = nullptr;
    char *alloc::end_free = nullptr;
    size_t alloc::heap_size = 0;

    alloc::obj* alloc::free_list[NFREELISTS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };



    //**************接口函数**************
    //分配内存
    //数组free_list每一个相当于一个链表。例如第#11块就是每个结点为8*12=98bytes的链表
    void *alloc::allocate(size_t n)
    {
        obj **my_free_list;
        obj *result;
        if (n > MAXBYTES)//如果n>预设内存，调用第一级适配器
            return std::malloc(n);
        my_free_list = free_list + FREELIST_INDEX(n);//寻找freelist中相匹配的
        result = *my_free_list;//先取出freelist[index]里面的地址，这个地址包含一个96bytes的内存
        if (result == nullptr)
        {
            void *r = refill(ROUND_UP(n));
            return r;
        }
        *my_free_list = result->next;//调整freelist[index]里面的地址，让它指向result链表的下一个结点
        return result;//返回reult
    }

    void alloc::deallocate(void *p, size_t n)//空间释放
    {
        if (n > (size_t)MAXBYTES)//如果n>MAX
        {
            alllocator::deallocate(p);
            return;
        }
        obj *q = static_cast<obj *>(p);//static_cast可以
        obj **my_free_list;
        my_free_list = free_list + FREELIST_INDEX(n);
        q->next = *my_free_list;
        *my_free_list = q;//问题挂上去之后，这个结点和其他不符合？
    }

    void *alloc::reallocate(void *p, size_t old_size, size_t new_size)//销毁和重新申请
    {
        deallocate(p, old_size);//销毁旧的
        p = allocate(new_size);//申请新的
        return p;
    }

    //****************内部函数******************
    //填充地址数组中某个位置
    void *alloc::refill(size_t n)
    {//chunk是引用传参，所以会更改
        size_t nobjs = ENOBJS;
        char *chunk = chunk_alloc(n, nobjs);//取新区块作为freelist的新结点地址
        obj **my_free_list;//
        obj *result, *cur_obj, *next_obj;
        if (nobjs == 1)//如果只取得了一个区块就给调用者使用
            return chunk;
        my_free_list = free_list + FREELIST_INDEX(n);//否则需要重新调整freelist
        result = (obj*)chunk;//强制转换地址类型,取的区块一块给客户使用其他传给freelist
        //chunk为char型数组，首先要转换为obj*,其次在freelist[n]位置上的链表，每一个结点的内存大小obj+n*obj
        *my_free_list = next_obj = (obj *)(chunk + n);//因为第一块已经要返回，所以剩下要挂到数组上的地址要+n
        for (size_t i = 1;; ++i)//把剩下的挂在数组链表上
        {
            cur_obj = next_obj;
            next_obj = (obj *)((char *)next_obj + n);//唯一的疑问，为什么要连续转换，next_obj明明已经是obj型指针了
            if (nobjs - 1 == i)//挂完了
            {
                cur_obj->next = nullptr;
                break;
            }
            else
            {
                cur_obj->next = next_obj;//继续挂
            }
        }
        return result;
    }

    char *alloc::chunk_alloc(size_t size, size_t &nobjs)//去堆区申请
    {
        char *result;//返回值
        size_t need_bytes = size * nobjs;//需要的区块数
        size_t pool_bytes = end_free - start_free;//内存中还剩下的区块数
        //如果剩下的还够
        if (pool_bytes >= need_bytes){
            result = start_free;//就返回起点
            start_free += need_bytes;//同时更新起点
            return result;
        }
        //如果需要不够，但是可以返回一个以上，size这里就是freelist里面的序号n
        else if (pool_bytes >= size){
            nobjs = pool_bytes / size;//计算能返回的数目
            need_bytes = size * nobjs;//更新能提供的空间和地址
            result = start_free;
            start_free += need_bytes;
            return result;
        }
        else{//一个都不够了
            if (pool_bytes > 0){//不够一个说明pool_bytes必然小于n，则必然可以找到在数组中找到一个位置放进去
                obj **my_free_list = free_list+FREELIST_INDEX(pool_bytes);
                ((obj *)start_free)->next = *my_free_list;//一开始的问题是为什么之后这个start_free—>为什么不为null
                //因为pool里面已经没有了。但是问题在于原来数组里面free_list[pool_bytes]不为零，为0的是free_list[size]
                *my_free_list = (obj *)start_free;//更新free_list
            }
            size_t bytes_to_get = (need_bytes << 1) + ROUND_UP(heap_size >> 4);//左移乘2
            start_free = (char *)malloc(bytes_to_get);//请求分配内存

            if (!start_free){//未能成功,去更序号更大的数组链表里找
                obj **my_free_list, *p;
                for (size_t i = size; i <= MAXBYTES; i += ALIGN)
                {
                    my_free_list = free_list+FREELIST_INDEX(i);
                    p = *my_free_list;
                    if (p)
                    {
                        *my_free_list = p->next;//更新free_list
                        start_free = (char *)p;//把拿出来的内存分配给内存池
                        end_free = start_free + i;
                        return chunk_alloc(size, nobjs);//内存池已经有了内存可以再次调用
                    }
                }
                //std::printf("out of memory");//以后的空间都没有了，所以出错了
                end_free = nullptr;//
                start_free=(char*)alllocator::malloc(bytes_to_get);//调用第一级配置器
                //throw std::bad_alloc();
            }
            end_free = start_free + bytes_to_get;//更新堆区指针
            heap_size += bytes_to_get;//更新堆区
            return chunk_alloc(size, nobjs);//再次递归调用自己
        }
    }

}
#endif
