#ifndef _LIST_IMPL_H_
#define _LIST_IMPL_H_

/*
list是一个双向非环链表，head.prev=tail.next=NULL;且tail=NULL;
第97行常迭代器转换问题,第129行，第187行
第275行unique函数错误，第317行是否要补充条件this！=x。x置空的必要性？
第323行，修改splice(fisrt,last)和splice(i)。让splice(first,last)调用i
第393行。
*/

namespace TinySTL{
	namespace Detail{
        //重载运算符
		template<class T>
		listIterator<T>& listIterator<T>::operator++(){
			p = p->next;
			return *this;
		}
		template<class T>
		listIterator<T> listIterator<T>::operator++(int){
			auto res = *this;
			++*this;
			return res;
		}
		template<class T>
		listIterator<T>& listIterator<T>::operator --(){
			p = p->prev;
			return *this;
		}
		template<class T>
		listIterator<T> listIterator<T>::operator --(int){
			auto res = *this;
			--*this;
			return res;
		}
		template<class T>
		bool operator ==(const listIterator<T>& lhs, const listIterator<T>& rhs){
			return lhs.p == rhs.p;
		}
		template<class T>
		bool operator !=(const listIterator<T>& lhs, const listIterator<T>& rhs){
			return !(lhs == rhs);
		}
	}//end of Detail namespace

    //构造、复制、析构函数
	template<class T>
	list<T>::list(){//空链表
		head.p = newNode();
		tail.p = head.p;
	}

	template<class T>
	list<T>::list(size_type n, const value_type& val = value_type()){//构建n个默认值的链表
		ctorAux(n, val, std::is_integral<value_type>());//萃取，要求value_type是泛整数类型
	}

	template<class T>
	template <class InputIterator>
	list<T>::list(InputIterator first, InputIterator last){//根据区间创建链表
		ctorAux(first, last, std::is_integral<InputIterator>());//要求迭代器非泛整型
	}

	template<class T>
	list<T>::list(const list& l){//拷贝构造
		head.p = newNode();
		tail.p = head.p;
		for (auto node = l.head.p; node != l.tail.p; node = node->next)
			push_back(node->data);
	}

	template<class T>
	list<T>& list<T>::operator = (const list& l){//重载=
		if (this != &l){//不使用拷贝构造，因为这里已经有对象了，拷贝构造是在没有对象前提下
			list(l).swap(*this);//等价于temp=list(l);temp.swap(*this);
		}
		return *this;
	}

	template<class T>
	list<T>::~list(){//析构
		for (; head != tail;){//依次销毁各个节点
			auto temp = head++;
			nodeAllocator::destroy(temp.p);
			nodeAllocator::deallocate(temp.p);
		}
		nodeAllocator::deallocate(tail.p);
	}

    //迭代器相关函数
	template<class T>
	typename list<T>::iterator list<T>::begin(){
		return head;
	}
	template<class T>
	typename list<T>::iterator list<T>::end(){
		return tail;
	}
	template<class T>
	typename list<T>::const_iterator list<T>::changeIteratorToConstIterator(iterator& it)const{//转换常迭代器
        /*需要转换的内容：迭代器内容和迭代器本身类型
        迭代器内容包括node指针，list指针*/
		using nodeP = Detail::node<const T>*;//等价于typedef Detai::node<const T>* nodeP; 
		auto temp = (list<const T>*const)this;//强制转换为指针常量，原本为list<T>*,这里转换为list<const T>*const
		auto ptr = it.p;//取迭代器内容，typedef node<T>* nodePtr;nodePtr p;
		Detail::node<const T> node(ptr->data, (nodeP)(ptr->prev), (nodeP)(ptr->next), temp);//强制转换;ptr->data本来就是const
		return const_iterator(&node);//listIterator<const T>迭代器本身类型
	}
	template<class T>
	typename list<T>::const_iterator list<T>::begin()const{
		auto temp = (list*const)this;//这里为什么不用list<const T>*const
		return changeIteratorToConstIterator(temp->head);
	}
	template<class T>
	typename list<T>::const_iterator list<T>::end()const{
		auto temp = (list*const)this;
		return changeIteratorToConstIterator(temp->tail);
	}
	template<class T>
	typename list<T>::reverse_iterator list<T>::rbegin(){
		return reverse_iterator(tail);
	}
	template<class T>
	typename list<T>::reverse_iterator list<T>::rend(){
		return reverse_iterator(head);
	}


    //接口函数
template<class T>
	void list<T>::push_front(const value_type& val){//前插
		auto node = newNode(val);
		head.p->prev = node;
		node->next = head.p;
		head.p = node;
	}
	template<class T>
	void list<T>::pop_front(){//前删，是否要判空？
		auto oldNode = head.p;
		head.p = oldNode->next;
		head.p->prev = nullptr;
		deleteNode(oldNode);
	}
	template<class T>
	void list<T>::push_back(const value_type& val){//尾插
		auto node = newNode();
		(tail.p)->data = val;
		(tail.p)->next = node;
		node->prev = tail.p;
		tail.p = node;
	}
	template<class T>
	void list<T>::pop_back(){//尾删，是否要判空
		auto newTail = tail.p->prev;
		newTail->next = nullptr;
		deleteNode(tail.p);
		tail.p = newTail;
	}

	template<class T>
	typename list<T>::size_type list<T>::size()const{//求长度
		size_type length = 0;
		for (auto h = head; h != tail; ++h)
			++length;
		return length;
	}

template<class T>
	typename list<T>::iterator list<T>::insert(iterator position, const value_type& val){
		if (position == begin()){
			push_front(val);
			return begin();
		}else if (position == end()){
			auto ret = position;
			push_back(val);
			return ret;
		}
		auto node = newNode(val);//更新前后结点
		auto prev = position.p->prev;//记录pos前结点
		node->next = position.p;//设置新节点的后续节点
		node->prev = prev;//设置新节点的前结点
		prev->next = node;//设置前结点的后结点
		position.p->prev = node;//设置后结点的前结点
		return iterator(node);//封装为迭代器
	}

    template<class T>
	void list<T>::insert(iterator position, size_type n, const value_type& val){//插入n个值
        nsert_aux(position, n, val, typename std::is_integral<InputIterator>::type());//错误萃取内容是否应该为size_type
    }

    template<class T>
    template<clas InputIterator>
    void list<T>::insert(iterator position,InputIterator first,InputIterator last){//插入区间
        insert_aux(position,first,last,typename std::is_integral<InputIterator>::type());
    }

    template<class T>
    typename list<T>::iterator list<T>::erase(iterator position){//擦除指定位置，并返回next位置的迭代器
        if(position==head){
            pop_front();
            return head;
        }
        else{
            auto prev=position.p-prev;
            prev->next=position.p->next;
            postion.p->next->prev=prev;
            deleteNode(position.p);
            return iterator(prev->next);
        }
    }
    template<class T>
    typename list<T>::iterator lsit<T>::erase(iterator firsr,iterator last){//擦除区间
        typename list<T>::iterator res;
        for(;first!=last;){
            auto temp=first++;
            res=erase(temp);
        }
        return res;
    }

    template<class>
    void list<T>::clear(){//清空
        erase(begin(),end());
    }

	template<class T>
	void list<T>::reverse(){//反转链表
		if (empty() || head.p->next == tail.p) return;//空链表或者只有一个元素
		auto curNode = head.p;//保存头结点
		head.p = tail.p->prev;//将头结点设定为尾结点tail.prev
		head.p->prev = nullptr;//设定头结点结束
		do{
            auto nextNode=curNode->next;
            head.p->next->prev=curNode;
            cureNode->next=head.p->next;
            head.p->next=curNode;
            cureNode->prev=head.p;
            curNode=nextNode;
		} while (curNode != head.p);//此过程中，尾结点没有发生变化
	}

	template<class T>
	void list<T>::remove(const value_type& val){//移除指定值结点
		for (auto it = begin(); it != end();){
			if (*it == val)
				it = erase(it);//如果移除，则erase的返回值为下一个结点
			else
				++it;
		}
	}
	template<class T>
	template <class Predicate>//谓语表达式
	void list<T>::remove_if(Predicate pred){//移除指定条件结点
		for (auto it = begin(); it != end();){
			if (pred(*it))
				it = erase(it);
			else
				++it;
		}
	}

	template<class T>
	void list<T>::swap(list& x){//交换2个链表
		TinySTL::swap(head.p, x.head.p);//实际上也就是交换两个链表的头结点和尾结点
		TinySTL::swap(tail.p, x.tail.p);
	}
	template<class T>
	void swap(list<T>& x, list<T>& y){
		x.swap(y);//重载
	}

	template<class T>
	void list<T>::unique(){//删除链表中相邻重复元素，因此如果要去除链表中所有重复元素，首先要排序
		nodePtr curNode = head.p;//既然有迭代器，就该用迭代器操作
		while (curNode != tail.p){
			nodePtr nextNode = curNode->next;
			if (curNode->data == nextNode->data){
				if (nextNode == tail.p){//nextNode既然有元素，怎么可能等于tail？
					curNode->next = nullptr;
					tail.p = curNode;
				}
				else{
					curNode->next = nextNode->next;
					nextNode->next->prev = curNode;
				}
				deleteNode(nextNode);
			}
			else{
				curNode = nextNode;
			}
		}
	}
	template<class T>
	template <class BinaryPredicate>
	void list<T>::unique(BinaryPredicate binary_pred){//按谓语表达式删除相邻元素
		nodePtr curNode = head.p;
		while (curNode != tail.p){
			nodePtr nextNode = curNode->next;
			if (binary_pred(curNode->data, nextNode->data)){
				if (nextNode == tail.p){
					curNode->next = nullptr;
					tail.p = curNode;
				}
				else{
					curNode->next = nextNode->next;
					nextNode->next->prev = curNode;
				}
				deleteNode(nextNode);
			}
			else{
				curNode = nextNode;
			}
		}
	}

	template<class T>
	void list<T>::splice(iterator position, list& x){//把(list)x的所有元素都拼接到this上,前提条件x和this不同
		this->insert(position, x.begin(), x.end());
		x.head.p = x.tail.p;//清空
	}

	template<class T>
	void list<T>::splice(iterator position, list& x, iterator first, iterator last){//将x的一部分拼接到指定位置
		if (first.p == last.p) return;
		auto tailNode = last.p->prev;
		if (x.head.p == first.p){
			x.head.p = last.p;
			x.head.p->prev = nullptr;
		}
		else{
			first.p->prev->next = last.p;
			last.p->prev = first.p->prev;
		}
		if (position.p == head.p){
			first.p->prev = nullptr;
			tailNode->next = head.p;
			head.p->prev = tailNode;
			head.p = first.p;
		}
		else{
			position.p->prev->next = first.p;
			first.p->prev = position.p->prev;
			tailNode->next = position.p;
			position.p->prev = tailNode;
		}
	}
	template<class T>
	void list<T>::splice(iterator position, list& x, iterator i){
		auto next = i;
		this->splice(position, x, i, ++next);
	}

	template<class T>
	void list<T>::merge(list& x){//合并list，前提两个list均已排序
		auto it1 = begin(), it2 = x.begin();
		while (it1 != end() && it2 != x.end()){
			if (*it1 <= *it2)
				++it1;
			else{
				auto temp = it2++;
				this->splice(it1, x, temp);//不合理，调用插入函数会好点
			}
		}
		if (it1 == end()){
			this->splice(it1, x, it2, x.end());
		}
	}
	template<class T>
	template <class Compare>
	void list<T>::merge(list& x, Compare comp){//按谓语表达式合并
		auto it1 = begin(), it2 = x.begin();
		while (it1 != end() && it2 != x.end()){
			if (comp(*it2, *it1)){
				auto temp = it2++;
				this->splice(it1, x, temp);
			}
			else
				++it1;
		}
		if (it1 == end()){
			this->splice(it1, x, it2, x.end());
		}
	}
	template <class T>
	bool operator== (const list<T>& lhs, const list<T>& rhs){//重载==
		auto node1 = lhs.head.p, node2 = rhs.head.p;
		for (; node1 != lhs.tail.p && node2 != rhs.tail.p; node1 = node1->next, node2 = node2->next){
			if (node1->data != node2->data)
				break;//可以直接返回false
		}
		if (node1 == lhs.tail.p && node2 == rhs.tail.p)
			return true;
		return false;
	}

	template <class T>
	bool operator!= (const list<T>& lhs, const list<T>& rhs){//重载!=
		return !(lhs == rhs);
	}

	template<class T>
	void list<T>::sort(){//排序
		sort(TinySTL::less<T>());//默认递增排序
	}

	template<class T>
	template <class Compare>
	void list<T>::sort(Compare comp){//含谓语条件排序
		if (empty() || head.p->next == tail.p)
			return;
        //快排。最坏复杂度不如归并，因此可以使用归并排序
		list<T> carry;
		list<T> counter[64];
		int fill = 0;
		while (!empty()){
			carry.splice(carry.begin(), *this, begin());
			int i = 0;
			while (i < fill && !counter[i].empty()){
				counter[i].merge(carry, comp);
				carry.swap(counter[i++]);
			}
			carry.swap(counter[i]);
			if (i == fill)
				++fill;
		}
		for (int i = 0; i != fill; ++i){
			counter[i].merge(counter[i - 1], comp);
		}
		swap(counter[fill - 1]);
	}


        //内部封装函数
	template<class T>
	void list<T>::insert_aux(iterator position, size_type n, const T& val, std::true_type){//插入n个值
		for (auto i = n; i != 0; --i){
			position = insert(position, val);//insert返回的是插入后的结点
		}
	}

	template<class T>
	template<class InputIterator>
	void list<T>::insert_aux(iterator position, InputIterator first, InputIterator last, std::false_type){//插入一个区间
		for (--last; first != last; --last){
			position = insert(position, *last);
		}
		insert(position, *last);
	}

	template<class T>
	typename list<T>::nodePtr list<T>::newNode(const T& val = T()){//创建属于容器list的新结点
		nodePtr res = nodeAllocator::allocate();//分配内存
		nodeAllocator::construct(res, Detail::node<T>(val, nullptr, nullptr, this));//建立结点
		return res;
	}

	template<class T>
	void list<T>::deleteNode(nodePtr p){//删除结点
		p->prev = p->next = nullptr;
		nodeAllocator::destroy(p);//销毁
		nodeAllocator::deallocate(p);//释放
	}

	template<class T>
	void list<T>::ctorAux(size_type n, const value_type& val, std::true_type){//建立一个n个val的链表
		head.p = newNode();//先创建一个结束结点tail
		tail.p = head.p;
		while (n--)
			push_back(val);//尾插法建立
	}
	template<class T>
	template <class InputIterator>
	void list<T>::ctorAux(InputIterator first, InputIterator last, std::false_type){//建立一个区间的链表
		head.p = newNode();
		tail.p = head.p;
		for (; first != last; ++first)
			push_back(*first);
	}

}
#endif
