#ifndef MYTINYSTL_CONSTRUCT_H_
#define MYTINYSTL_CONSTRUCT_H_

// 这个头文件包含两个函数 construct，destroy
// construct : 负责对象的构造
// destroy   : 负责对象的析构

#include <new>

#include "type_traits.h"
#include "iterator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100) // unused parameter
#endif                          // _MSC_VER

namespace mystl
{

    // construct 构造对象

    template <class Ty>
    void construct(Ty *ptr){//给定迭代器，用默认值去构造
        ::new ((void *)ptr) Ty();
    }

    template <class Ty1, class Ty2>
    void construct(Ty1 *ptr, const Ty2 &value){//给定迭代器和值
        ::new ((void *)ptr) Ty1(value);
    }

    template <class Ty, class... Args>
    void construct(Ty *ptr, Args &&...args){//多参数原地构造
        ::new ((void *)ptr) Ty(mystl::forward<Args>(args)...);
    }

    // destroy 将对象析构


    template <class Ty>
    void destroy(Ty *pointer){//只有一个迭代器
        destroy_one(pointer, std::is_trivially_destructible<Ty>{});//trivial代表不重要的
    }

    template <class ForwardIter>
    void destroy(ForwardIter first, ForwardIter last)//给定一个区间
    {
        destroy_cat(first, last, std::is_trivially_destructible<typename iterator_traits<ForwardIter>::value_type>{});
    }

    template <class Ty>
    void destroy_one(Ty *, std::true_type) {}//不重要的，说明是内置类型，不用析构，后面有delete负责内存

    template <class ForwardIter>
    void destroy_cat(ForwardIter, ForwardIter, std::true_type) {}//同上

    template <class Ty>
    void destroy_one(Ty  *pointer, std::false_type){//有构造函数，所以要先析构
        if (pointer != nullptr)
        {
            pointer->~Ty();
        }
    }

    template <class ForwardIter>
    void destroy_cat(ForwardIter first, ForwardIter last, std::false_type){//同上
        for (; first != last; ++first)
            destroy(&*first);//问题是为何不直接调用destroy_one，第二个参数设定为true_type
    }


} // namespace mystl

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // !MYTINYSTL_CONSTRUCT_H_
