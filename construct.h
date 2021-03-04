#ifndef MYTINYSTL_CONSTRUCT_H_
#define MYTINYSTL_CONSTRUCT_H_

// ���ͷ�ļ������������� construct��destroy
// construct : �������Ĺ���
// destroy   : ������������

#include <new>

#include "type_traits.h"
#include "iterator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100) // unused parameter
#endif                          // _MSC_VER

namespace mystl
{

    // construct �������

    template <class Ty>
    void construct(Ty *ptr){//��������������Ĭ��ֵȥ����
        ::new ((void *)ptr) Ty();
    }

    template <class Ty1, class Ty2>
    void construct(Ty1 *ptr, const Ty2 &value){//������������ֵ
        ::new ((void *)ptr) Ty1(value);
    }

    template <class Ty, class... Args>
    void construct(Ty *ptr, Args &&...args){//�����ԭ�ع���
        ::new ((void *)ptr) Ty(mystl::forward<Args>(args)...);
    }

    // destroy ����������


    template <class Ty>
    void destroy(Ty *pointer){//ֻ��һ��������
        destroy_one(pointer, std::is_trivially_destructible<Ty>{});//trivial������Ҫ��
    }

    template <class ForwardIter>
    void destroy(ForwardIter first, ForwardIter last)//����һ������
    {
        destroy_cat(first, last, std::is_trivially_destructible<typename iterator_traits<ForwardIter>::value_type>{});
    }

    template <class Ty>
    void destroy_one(Ty *, std::true_type) {}//����Ҫ�ģ�˵�����������ͣ�����������������delete�����ڴ�

    template <class ForwardIter>
    void destroy_cat(ForwardIter, ForwardIter, std::true_type) {}//ͬ��

    template <class Ty>
    void destroy_one(Ty  *pointer, std::false_type){//�й��캯��������Ҫ������
        if (pointer != nullptr)
        {
            pointer->~Ty();
        }
    }

    template <class ForwardIter>
    void destroy_cat(ForwardIter first, ForwardIter last, std::false_type){//ͬ��
        for (; first != last; ++first)
            destroy(&*first);//������Ϊ�β�ֱ�ӵ���destroy_one���ڶ��������趨Ϊtrue_type
    }


} // namespace mystl

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // !MYTINYSTL_CONSTRUCT_H_
