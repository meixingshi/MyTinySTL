#ifndef _VECTOR_H_//#define#endif一起使用，防止头文件被包含两次相当于#pragma once
#define _VECTOR_H_

//本头文件包含一个模板类 vector
//vector:向量

/*notes:
异常保证：
mystl::vector<T> 满足基本异常保证，部分函数无异常保证，并对一下函数做强异常保证
*  emplace
* emplace_back
*push_back
当std::is_nothrow_move_assignable<T>::value==true 时，以下函数满足强异常保证
*reserve
*resize
*insert
*/

#include <algorithm>
#include <type_traits>

#include "Allocator.h"//分配器
#include "Algorithm.h"//算法
#include "Iterator.h"//迭代器
#include "ReverseIterator.h"//反转迭代器
#include "UninitializedFunctions.h"//？

namespace TinySTL {

//*****vector********
template<class T,class Alloc=allocator<T>> //默认分配器
class vector{
	private:
	//模仿三个迭代器
		T *start_;
		T *finish_;
		T *endOfStorage_;

		typedef Alloc dataAllocator;

	public:
	//定义别名
		typedef T									value_type;//class T
		typedef T*							iterator;//*T 智能指针
		//typedef const iterator					const_iterator;
		typedef const T*							const_iterator;//常指针->常迭代器
		typedef reverse_iterator_t<T*>				reverse_iterator;//？
		typedef reverse_iterator_t<const T*>				const_reverse_iterator;
		typedef iterator							pointer;//又把迭代器定义为指针
		typedef T&									reference;//引用
		typedef const T&							const_reference;//常引用
		typedef size_t								size_type;//size_t在crtdefs.h下一种unsigned __int64
		typedef ptrdiff_t							difference_type;//在crtdefs.h下两个指针之间的差距，可能为负

public:
//构造，复制，析构函数
	//构造函数
		vector()
			:start_(0), finish_(0), endOfStorage_(0){}//适用于vector<int> v;的情形，并对三个迭代器初始化
		explicit vector(const size_type n);//explicit，显式转换，避免vector<int> v=10;的隐式转换
		vector(const size_type n, const value_type& value);
		template<class InputIterator>
		vector(InputIterator first, InputIterator last);
		vector(const vector& v);
		vector(vector&& v);//转移构造函数

		//复制函数
		vector& operator = (const vector& v);
		vector& operator = (vector&& v);

		~vector();//析构函数

		//比较操作相关
		bool operator == (const vector& v)const;
		bool operator != (const vector& v)const;

		//迭代器相关
		iterator begin(){ return (start_); }
		const_iterator begin()const{ return (start_); }
		const_iterator cbegin()const{ return (start_); }
		iterator end(){ return (finish_); }
		const_iterator end()const{ return (finish_); }
		const_iterator cend()const{ return (finish_); }
		reverse_iterator rbegin(){ return reverse_iterator(finish_); }
		const_reverse_iterator crbegin()const{ return const_reverse_iterator(finish_); }
		reverse_iterator rend(){ return reverse_iterator(start_); }
		const_reverse_iterator crend()const{ return const_reverse_iterator(start_); }

		//与容量相关
		difference_type size()const{ return finish_ - start_; }
		difference_type capacity()const{ return endOfStorage_ - start_; }
		bool empty()const{ return start_ == finish_; }
		void resize(size_type n, value_type val = value_type());
		void reserve(size_type n);
		void shrink_to_fit();

		//访问元素相关
		reference operator[](const difference_type i){ return *(begin() + i); }
		const_reference operator[](const difference_type i)const{ return *(cbegin() + i); }
		reference front(){ return *(begin()); }
		reference back(){ return *(end() - 1); }
		pointer data(){ return start_; }

		//修改容器相关的操作
		//清空容器，销毁容器中的所有对象并使容器的size为0，但不回收容器已有的空间
		void clear();
		void swap(vector& v);
		void push_back(const value_type& value);
		void pop_back();
		iterator insert(iterator position, const value_type& val);
		void insert(iterator position, const size_type& n, const value_type& val);
		template <class InputIterator>
		void insert(iterator position, InputIterator first, InputIterator last);
		iterator erase(iterator position);
		iterator erase(iterator first, iterator last);

		//容器的空间配置器相关
		Alloc get_allocator(){ return dataAllocator; }

	private:
		void destroyAndDeallocateAll();
		void allocateAndFillN(const size_type n, const value_type& value);
		template<class InputIterator>
		void allocateAndCopy(InputIterator first, InputIterator last);

		template<class InputIterator>
		void vector_aux(InputIterator first, InputIterator last, std::false_type);
		template<class Integer>
		void vector_aux(Integer n, const value_type& value, std::true_type);
		template<class InputIterator>
		void insert_aux(iterator position, InputIterator first, InputIterator last, std::false_type);
		template<class Integer>
		void insert_aux(iterator position, Integer n, const value_type& value, std::true_type);
		template<class InputIterator>
		void reallocateAndCopy(iterator position, InputIterator first, InputIterator last);
		void reallocateAndFillN(iterator position, const size_type& n, const value_type& val);
		size_type getNewCapacity(size_type len)const;
	public:
		template<class T, class Alloc>
		friend bool operator == (const vector<T, Alloc>& v1, const vector<T, Alloc>& v2);
		template<class T, class Alloc>
		friend bool operator != (const vector<T, Alloc>& v1, const vector<T, Alloc>& v2);
	};
}

#include "Detail\Vector.cpp"
#endif

