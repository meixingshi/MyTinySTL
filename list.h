#ifndef _LIST_H_
#define _LIST_H_

/*第16行、第33行、第49行。第72行，第73行
第112行
*/

#include "Allocator.h"
#include "Iterator.h"
#include "ReverseIterator.h"
#include "UninitializedFunctions.h"

#include <type_traits>

namespace TinySTL{
	template<class T>
	class list;
	namespace Detail{//为什么要再用一个命名空间
		//the class of node
		template<class T>
		struct node{
			T data;
			node *prev;
			node *next;
			list<T> *container;
			node(const T& d, node *p, node *n, list<T> *c):
				data(d), prev(p), next(n), container(c){}
			bool operator ==(const node& n){
				return data == n.data && prev == n.prev && next == n.next && container == n.container;
			}
		};
		//the class of list iterator
		template<class T>
		struct listIterator :public iterator<bidirectional_iterator_tag, T>{//外部的public什么意思？
			template<class T>
			friend class list;//list可以使用listIterator
		public:
			typedef node<T>* nodePtr;
			nodePtr p;
		public:
			explicit listIterator(nodePtr ptr = nullptr) :p(ptr){}

			listIterator& operator++();//前缀
			listIterator operator++(int);//后缀
			listIterator& operator --();
			listIterator operator --(int);
			T& operator *(){ return p->data; }//解引用
			T* operator ->(){ return &(operator*()); }
            /*
            T* operator ->(){ return &(operator*()); }
            当被调用时，例如
            list<T>::iterator it=s.begin();
            it->data;
            相当于调用 it.operator->()->data;
            首先调用operator*()返回的是data的引用，然后对data取地址，但是这里的地址实际上也是结构体的地址。
            残余问题：如果说it->相当于返回it.operator->()但此时返回的是一个地址，后面的->从何而来
            */

			template<class T>
			friend bool operator ==(const listIterator<T>& lhs, const listIterator<T>& rhs);
			template<class T>
			friend bool operator !=(const listIterator<T>& lhs, const listIterator<T>& rhs);
		};
	}//end of namespace


	//the class of list
	template<class T>
	class list{
		template<class T>
		friend struct listIterator;//互为友元？
	private:
		typedef allocator<Detail::node<T>> nodeAllocator;//分配器？
		typedef Detail::node<T> *nodePtr;//是不是错了，应该是typedef Detail::node<T>* nodePtr;
	public:
		typedef T value_type;
		typedef Detail::listIterator<T> iterator;//迭代器类对象。迭代器类设计--包含一个指针变量：node结构体的地址
		typedef Detail::listIterator<const T> const_iterator;
		typedef reverse_iterator_t<iterator> reverse_iterator;//反转迭代器
		typedef T& reference;
		typedef size_t size_type;
	private:
		iterator head;
		iterator tail;
	public:
        //构造、复制、析构函数
		list();
		explicit list(size_type n, const value_type& val = value_type());//显式初始化
		template <class InputIterator>
		list(InputIterator first, InputIterator last);
		list(const list& l);
		list& operator = (const list& l);
		~list();

        //迭代器相关函数
		iterator begin();
		iterator end();
		const_iterator begin()const;
		const_iterator end()const;
		reverse_iterator rbegin();
		reverse_iterator rend();
        
        //接口函数
		void push_front(const value_type& val);
		void pop_front();
		void push_back(const value_type& val);
		void pop_back();
		bool empty()const{ return head == tail; }
		size_type size()const;
		reference front(){ return (head.p->data); }
		reference back(){ return (tail.p->prev->data); }
		iterator insert(iterator position, const value_type& val);
		void insert(iterator position, size_type n, const value_type& val);//这个insert和下个insert为什么没有返回值
		template <class InputIterator>
		void insert(iterator position, InputIterator first, InputIterator last);
		iterator erase(iterator position);
		iterator erase(iterator first, iterator last);
		void swap(list& x);
		void clear();
		void splice(iterator position, list& x);
		void splice(iterator position, list& x, iterator i);
		void splice(iterator position, list& x, iterator first, iterator last);
		void remove(const value_type& val);
		template <class Predicate>
		void remove_if(Predicate pred);
		void unique();
		template <class BinaryPredicate>
		void unique(BinaryPredicate binary_pred);
		void merge(list& x);
		template <class Compare>
		void merge(list& x, Compare comp);
		void sort();
		template <class Compare>
		void sort(Compare comp);
		void reverse();

	private:
        //封装内部函数
		void ctorAux(size_type n, const value_type& val, std::true_type);
		template <class InputIterator>
		void ctorAux(InputIterator first, InputIterator last, std::false_type);
		nodePtr newNode(const T& val = T());
		void deleteNode(nodePtr p);
		void insert_aux(iterator position, size_type n, const T& val, std::true_type);
		template<class InputIterator>
		void insert_aux(iterator position, InputIterator first, InputIterator last, std::false_type);
		const_iterator changeIteratorToConstIterator(iterator& it)const;
	public:
        //作用域？
		template<class T>
		friend void swap(list<T>& x, list<T>& y);
		template <class T>
		friend bool operator== (const list<T>& lhs, const list<T>& rhs);
		template <class T>
		friend bool operator!= (const list<T>& lhs, const list<T>& rhs);
	};//end of List
}

#include "Detail\List.impl.h"
#endif