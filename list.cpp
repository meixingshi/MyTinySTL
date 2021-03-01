#ifndef _LIST_IMPL_H_
#define _LIST_IMPL_H_

/*
list��һ��˫��ǻ�����head.prev=tail.next=NULL;��tail=NULL;
��97�г�������ת������,��129�У���187��
��275��unique�������󣬵�317���Ƿ�Ҫ��������this��=x��x�ÿյı�Ҫ�ԣ�
��323�У��޸�splice(fisrt,last)��splice(i)����splice(first,last)����i
��393�С�
*/

namespace TinySTL{
	namespace Detail{
        //���������
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

    //���졢���ơ���������
	template<class T>
	list<T>::list(){//������
		head.p = newNode();
		tail.p = head.p;
	}

	template<class T>
	list<T>::list(size_type n, const value_type& val = value_type()){//����n��Ĭ��ֵ������
		ctorAux(n, val, std::is_integral<value_type>());//��ȡ��Ҫ��value_type�Ƿ���������
	}

	template<class T>
	template <class InputIterator>
	list<T>::list(InputIterator first, InputIterator last){//�������䴴������
		ctorAux(first, last, std::is_integral<InputIterator>());//Ҫ��������Ƿ�����
	}

	template<class T>
	list<T>::list(const list& l){//��������
		head.p = newNode();
		tail.p = head.p;
		for (auto node = l.head.p; node != l.tail.p; node = node->next)
			push_back(node->data);
	}

	template<class T>
	list<T>& list<T>::operator = (const list& l){//����=
		if (this != &l){//��ʹ�ÿ������죬��Ϊ�����Ѿ��ж����ˣ�������������û�ж���ǰ����
			list(l).swap(*this);//�ȼ���temp=list(l);temp.swap(*this);
		}
		return *this;
	}

	template<class T>
	list<T>::~list(){//����
		for (; head != tail;){//�������ٸ����ڵ�
			auto temp = head++;
			nodeAllocator::destroy(temp.p);
			nodeAllocator::deallocate(temp.p);
		}
		nodeAllocator::deallocate(tail.p);
	}

    //��������غ���
	template<class T>
	typename list<T>::iterator list<T>::begin(){
		return head;
	}
	template<class T>
	typename list<T>::iterator list<T>::end(){
		return tail;
	}
	template<class T>
	typename list<T>::const_iterator list<T>::changeIteratorToConstIterator(iterator& it)const{//ת����������
        /*��Ҫת�������ݣ����������ݺ͵�������������
        ���������ݰ���nodeָ�룬listָ��*/
		using nodeP = Detail::node<const T>*;//�ȼ���typedef Detai::node<const T>* nodeP; 
		auto temp = (list<const T>*const)this;//ǿ��ת��Ϊָ�볣����ԭ��Ϊlist<T>*,����ת��Ϊlist<const T>*const
		auto ptr = it.p;//ȡ���������ݣ�typedef node<T>* nodePtr;nodePtr p;
		Detail::node<const T> node(ptr->data, (nodeP)(ptr->prev), (nodeP)(ptr->next), temp);//ǿ��ת��;ptr->data��������const
		return const_iterator(&node);//listIterator<const T>��������������
	}
	template<class T>
	typename list<T>::const_iterator list<T>::begin()const{
		auto temp = (list*const)this;//����Ϊʲô����list<const T>*const
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


    //�ӿں���
template<class T>
	void list<T>::push_front(const value_type& val){//ǰ��
		auto node = newNode(val);
		head.p->prev = node;
		node->next = head.p;
		head.p = node;
	}
	template<class T>
	void list<T>::pop_front(){//ǰɾ���Ƿ�Ҫ�пգ�
		auto oldNode = head.p;
		head.p = oldNode->next;
		head.p->prev = nullptr;
		deleteNode(oldNode);
	}
	template<class T>
	void list<T>::push_back(const value_type& val){//β��
		auto node = newNode();
		(tail.p)->data = val;
		(tail.p)->next = node;
		node->prev = tail.p;
		tail.p = node;
	}
	template<class T>
	void list<T>::pop_back(){//βɾ���Ƿ�Ҫ�п�
		auto newTail = tail.p->prev;
		newTail->next = nullptr;
		deleteNode(tail.p);
		tail.p = newTail;
	}

	template<class T>
	typename list<T>::size_type list<T>::size()const{//�󳤶�
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
		auto node = newNode(val);//����ǰ����
		auto prev = position.p->prev;//��¼posǰ���
		node->next = position.p;//�����½ڵ�ĺ����ڵ�
		node->prev = prev;//�����½ڵ��ǰ���
		prev->next = node;//����ǰ���ĺ���
		position.p->prev = node;//���ú����ǰ���
		return iterator(node);//��װΪ������
	}

    template<class T>
	void list<T>::insert(iterator position, size_type n, const value_type& val){//����n��ֵ
        nsert_aux(position, n, val, typename std::is_integral<InputIterator>::type());//������ȡ�����Ƿ�Ӧ��Ϊsize_type
    }

    template<class T>
    template<clas InputIterator>
    void list<T>::insert(iterator position,InputIterator first,InputIterator last){//��������
        insert_aux(position,first,last,typename std::is_integral<InputIterator>::type());
    }

    template<class T>
    typename list<T>::iterator list<T>::erase(iterator position){//����ָ��λ�ã�������nextλ�õĵ�����
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
    typename list<T>::iterator lsit<T>::erase(iterator firsr,iterator last){//��������
        typename list<T>::iterator res;
        for(;first!=last;){
            auto temp=first++;
            res=erase(temp);
        }
        return res;
    }

    template<class>
    void list<T>::clear(){//���
        erase(begin(),end());
    }

	template<class T>
	void list<T>::reverse(){//��ת����
		if (empty() || head.p->next == tail.p) return;//���������ֻ��һ��Ԫ��
		auto curNode = head.p;//����ͷ���
		head.p = tail.p->prev;//��ͷ����趨Ϊβ���tail.prev
		head.p->prev = nullptr;//�趨ͷ������
		do{
            auto nextNode=curNode->next;
            head.p->next->prev=curNode;
            cureNode->next=head.p->next;
            head.p->next=curNode;
            cureNode->prev=head.p;
            curNode=nextNode;
		} while (curNode != head.p);//�˹����У�β���û�з����仯
	}

	template<class T>
	void list<T>::remove(const value_type& val){//�Ƴ�ָ��ֵ���
		for (auto it = begin(); it != end();){
			if (*it == val)
				it = erase(it);//����Ƴ�����erase�ķ���ֵΪ��һ�����
			else
				++it;
		}
	}
	template<class T>
	template <class Predicate>//ν����ʽ
	void list<T>::remove_if(Predicate pred){//�Ƴ�ָ���������
		for (auto it = begin(); it != end();){
			if (pred(*it))
				it = erase(it);
			else
				++it;
		}
	}

	template<class T>
	void list<T>::swap(list& x){//����2������
		TinySTL::swap(head.p, x.head.p);//ʵ����Ҳ���ǽ������������ͷ����β���
		TinySTL::swap(tail.p, x.tail.p);
	}
	template<class T>
	void swap(list<T>& x, list<T>& y){
		x.swap(y);//����
	}

	template<class T>
	void list<T>::unique(){//ɾ�������������ظ�Ԫ�أ�������Ҫȥ�������������ظ�Ԫ�أ�����Ҫ����
		nodePtr curNode = head.p;//��Ȼ�е��������͸��õ���������
		while (curNode != tail.p){
			nodePtr nextNode = curNode->next;
			if (curNode->data == nextNode->data){
				if (nextNode == tail.p){//nextNode��Ȼ��Ԫ�أ���ô���ܵ���tail��
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
	void list<T>::unique(BinaryPredicate binary_pred){//��ν����ʽɾ������Ԫ��
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
	void list<T>::splice(iterator position, list& x){//��(list)x������Ԫ�ض�ƴ�ӵ�this��,ǰ������x��this��ͬ
		this->insert(position, x.begin(), x.end());
		x.head.p = x.tail.p;//���
	}

	template<class T>
	void list<T>::splice(iterator position, list& x, iterator first, iterator last){//��x��һ����ƴ�ӵ�ָ��λ��
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
	void list<T>::merge(list& x){//�ϲ�list��ǰ������list��������
		auto it1 = begin(), it2 = x.begin();
		while (it1 != end() && it2 != x.end()){
			if (*it1 <= *it2)
				++it1;
			else{
				auto temp = it2++;
				this->splice(it1, x, temp);//���������ò��뺯����õ�
			}
		}
		if (it1 == end()){
			this->splice(it1, x, it2, x.end());
		}
	}
	template<class T>
	template <class Compare>
	void list<T>::merge(list& x, Compare comp){//��ν����ʽ�ϲ�
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
	bool operator== (const list<T>& lhs, const list<T>& rhs){//����==
		auto node1 = lhs.head.p, node2 = rhs.head.p;
		for (; node1 != lhs.tail.p && node2 != rhs.tail.p; node1 = node1->next, node2 = node2->next){
			if (node1->data != node2->data)
				break;//����ֱ�ӷ���false
		}
		if (node1 == lhs.tail.p && node2 == rhs.tail.p)
			return true;
		return false;
	}

	template <class T>
	bool operator!= (const list<T>& lhs, const list<T>& rhs){//����!=
		return !(lhs == rhs);
	}

	template<class T>
	void list<T>::sort(){//����
		sort(TinySTL::less<T>());//Ĭ�ϵ�������
	}

	template<class T>
	template <class Compare>
	void list<T>::sort(Compare comp){//��ν����������
		if (empty() || head.p->next == tail.p)
			return;
        //���š�����ӶȲ���鲢����˿���ʹ�ù鲢����
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


        //�ڲ���װ����
	template<class T>
	void list<T>::insert_aux(iterator position, size_type n, const T& val, std::true_type){//����n��ֵ
		for (auto i = n; i != 0; --i){
			position = insert(position, val);//insert���ص��ǲ����Ľ��
		}
	}

	template<class T>
	template<class InputIterator>
	void list<T>::insert_aux(iterator position, InputIterator first, InputIterator last, std::false_type){//����һ������
		for (--last; first != last; --last){
			position = insert(position, *last);
		}
		insert(position, *last);
	}

	template<class T>
	typename list<T>::nodePtr list<T>::newNode(const T& val = T()){//������������list���½��
		nodePtr res = nodeAllocator::allocate();//�����ڴ�
		nodeAllocator::construct(res, Detail::node<T>(val, nullptr, nullptr, this));//�������
		return res;
	}

	template<class T>
	void list<T>::deleteNode(nodePtr p){//ɾ�����
		p->prev = p->next = nullptr;
		nodeAllocator::destroy(p);//����
		nodeAllocator::deallocate(p);//�ͷ�
	}

	template<class T>
	void list<T>::ctorAux(size_type n, const value_type& val, std::true_type){//����һ��n��val������
		head.p = newNode();//�ȴ���һ���������tail
		tail.p = head.p;
		while (n--)
			push_back(val);//β�巨����
	}
	template<class T>
	template <class InputIterator>
	void list<T>::ctorAux(InputIterator first, InputIterator last, std::false_type){//����һ�����������
		head.p = newNode();
		tail.p = head.p;
		for (; first != last; ++first)
			push_back(*first);
	}

}
#endif
