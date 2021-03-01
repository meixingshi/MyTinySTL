#ifndef _LIST_H_
#define _LIST_H_

/*��16�С���33�С���49�С���72�У���73��
��112��
*/

#include "Allocator.h"
#include "Iterator.h"
#include "ReverseIterator.h"
#include "UninitializedFunctions.h"

#include <type_traits>

namespace TinySTL{
	template<class T>
	class list;
	namespace Detail{//ΪʲôҪ����һ�������ռ�
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
		struct listIterator :public iterator<bidirectional_iterator_tag, T>{//�ⲿ��publicʲô��˼��
			template<class T>
			friend class list;//list����ʹ��listIterator
		public:
			typedef node<T>* nodePtr;
			nodePtr p;
		public:
			explicit listIterator(nodePtr ptr = nullptr) :p(ptr){}

			listIterator& operator++();//ǰ׺
			listIterator operator++(int);//��׺
			listIterator& operator --();
			listIterator operator --(int);
			T& operator *(){ return p->data; }//������
			T* operator ->(){ return &(operator*()); }
            /*
            T* operator ->(){ return &(operator*()); }
            ��������ʱ������
            list<T>::iterator it=s.begin();
            it->data;
            �൱�ڵ��� it.operator->()->data;
            ���ȵ���operator*()���ص���data�����ã�Ȼ���dataȡ��ַ����������ĵ�ַʵ����Ҳ�ǽṹ��ĵ�ַ��
            �������⣺���˵it->�൱�ڷ���it.operator->()����ʱ���ص���һ����ַ�������->�Ӻζ���
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
		friend struct listIterator;//��Ϊ��Ԫ��
	private:
		typedef allocator<Detail::node<T>> nodeAllocator;//��������
		typedef Detail::node<T> *nodePtr;//�ǲ��Ǵ��ˣ�Ӧ����typedef Detail::node<T>* nodePtr;
	public:
		typedef T value_type;
		typedef Detail::listIterator<T> iterator;//����������󡣵����������--����һ��ָ�������node�ṹ��ĵ�ַ
		typedef Detail::listIterator<const T> const_iterator;
		typedef reverse_iterator_t<iterator> reverse_iterator;//��ת������
		typedef T& reference;
		typedef size_t size_type;
	private:
		iterator head;
		iterator tail;
	public:
        //���졢���ơ���������
		list();
		explicit list(size_type n, const value_type& val = value_type());//��ʽ��ʼ��
		template <class InputIterator>
		list(InputIterator first, InputIterator last);
		list(const list& l);
		list& operator = (const list& l);
		~list();

        //��������غ���
		iterator begin();
		iterator end();
		const_iterator begin()const;
		const_iterator end()const;
		reverse_iterator rbegin();
		reverse_iterator rend();
        
        //�ӿں���
		void push_front(const value_type& val);
		void pop_front();
		void push_back(const value_type& val);
		void pop_back();
		bool empty()const{ return head == tail; }
		size_type size()const;
		reference front(){ return (head.p->data); }
		reference back(){ return (tail.p->prev->data); }
		iterator insert(iterator position, const value_type& val);
		void insert(iterator position, size_type n, const value_type& val);//���insert���¸�insertΪʲôû�з���ֵ
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
        //��װ�ڲ�����
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
        //������
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