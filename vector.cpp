#ifndef _VECTOR_CPP_
#define _VECTOR_CPP_

/*ʣ�����⣬��20�У�erase(iterator it,int len)��̬
��157�У�copy_backward��δ�Լ�д,��������ô�ǰ����ƣ���Ȼ���ܻḲ��һЩԪ�ء���176��Ϊʲôʹ��Integer��������204������
��153��std::false_type��ô�õġ���166����һ������,��186��ԭ�򡣵�200����ȡ����
��219�У�ΪʲôҪ��������operate==����ʾ����ʽ����278��push_back()��������251�У�������������
*/


namespace TinySTL{
    /************���죬���ƣ��������************/
	template<class T, class Alloc>
	vector<T, Alloc>::vector(const size_type n){
		allocateAndFillN(n, value_type());//value_type�൱��T()������ʱ�����൱��vector<int> v(10)��ֻ��һ�����������Ի���һ����ʼֵ
	}
	template<class T, class Alloc>
	vector<T, Alloc>::vector(const size_type n, const value_type& value){
		allocateAndFillN(n, value);
	}
	template<class T, class Alloc>
	template<class InputIterator>
	vector<T, Alloc>::vector(InputIterator first, InputIterator last){
		//����ָ������ּ������ĺ�����
		vector_aux(first, last, typename std::is_integral<InputIterator>::type());//?
	}

	template<class T, class Alloc>
	vector<T, Alloc>::vector(const vector& v){
		allocateAndCopy(v.start_, v.finish_);
	}
	template<class T, class Alloc>
	vector<T, Alloc>::vector(vector&& v){//ת�ƹ��캯��
		start_ = v.start_;
		finish_ = v.finish_;
		endOfStorage_ = v.endOfStorage_;
		v.start_ = v.finish_ = v.endOfStorage_ = 0;
	}

	template<class T, class Alloc>//����=
	vector<T, Alloc>& vector<T, Alloc>::operator = (const vector& v){
		if (this != &v){
			allocateAndCopy(v.start_, v.finish_);
		}
		return *this;
	}
	template<class T, class Alloc>
	vector<T, Alloc>& vector<T, Alloc>::operator = (vector&& v){
		if (this != &v){
			destroyAndDeallocateAll();
			start_ = v.start_;
			finish_ = v.finish_;
			endOfStorage_ = v.endOfStorage_;
			v.start_ = v.finish_ = v.endOfStorage_ = 0;
		}
		return *this;
	}

    template<class T, class Alloc>
	vector<T, Alloc>::~vector(){
		destroyAndDeallocateAll();
	}

	//*************���������������******************************
	template<class T, class Alloc>
	void vector<T, Alloc>::resize(size_type n, value_type val = value_type()){//�������ô�С
		if (n < size()){
			dataAllocator::destroy(start_ + n, finish_);
			finish_ = start_ + n;
		}
		else if (n > size() && n <= capacity()){
			auto lengthOfInsert = n - size();
			finish_ = TinySTL::uninitialized_fill_n(finish_, lengthOfInsert, val);
		}
		else if (n > capacity()){
			auto lengthOfInsert = n - size();
			T *newStart = dataAllocator::allocate(getNewCapacity(lengthOfInsert));//Ѱ��һ���´�С���ڴ�ռ�
			T *newFinish = TinySTL::uninitialized_copy(begin(), end(), newStart);
			newFinish = TinySTL::uninitialized_fill_n(newFinish, lengthOfInsert, val);

			destroyAndDeallocateAll();
			start_ = newStart;
			finish_ = newFinish;
			endOfStorage_ = start_ + n;
		}
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::reserve(size_type n){//������������
		if (n <= capacity())
			return;
		T *newStart = dataAllocator::allocate(n);
		T *newFinish = TinySTL::uninitialized_copy(begin(), end(), newStart);
		
        destroyAndDeallocateAll();
		start_ = newStart;
		finish_ = newFinish;
		endOfStorage_ = start_ + n;
	}

	//***************�޸���������ز���**************************
	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator position){//ɾ��positionλ��
		return erase(position, position + 1);//ת��Ϊ��һ����̬
	}
	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator first, iterator last){//ɾ��һ������
		difference_type lenOfTail = end() - last;//end()=finish_;��β���ľ���
		difference_type lenOfRemoved = last - first;//�Ƴ��ĳ���
		finish_ = finish_ - lenOfRemoved;//����β��
		for (; lenOfTail != 0; --lenOfTail){
			auto temp = (last - lenOfRemoved);//=��first��ʼ��һֱ������Ϊʣ�೤��lenOfTail�ĵ�ַ
			*temp = *(last++);//ͨ��last�ı�
		}
		return (first);//����first������
	}

	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::reallocateAndCopy(iterator position, InputIterator first, InputIterator last){//���ƽ���һ����������
		difference_type newCapacity = getNewCapacity(last - first);
		T *newStart = dataAllocator::allocate(newCapacity);
		T *newEndOfStorage = newStart + newCapacity;
        //uninitialized_copy(������㣬�����յ㣬���ڴ����)
		T *newFinish = TinySTL::uninitialized_copy(begin(), position, newStart);
		newFinish = TinySTL::uninitialized_copy(first, last, newFinish);
		newFinish = TinySTL::uninitialized_copy(position, end(), newFinish);
        //����ԭ���ռ䣬����������������
		destroyAndDeallocateAll();
		start_ = newStart;
		finish_ = newFinish;
		endOfStorage_ = newEndOfStorage;
	}

	template<class T, class Alloc>
	void vector<T, Alloc>::reallocateAndFillN(iterator position, const size_type& n, const value_type& val){//����n��val
		difference_type newCapacity = getNewCapacity(n);
		T *newStart = dataAllocator::allocate(newCapacity);
		T *newEndOfStorage = newStart + newCapacity;
		T *newFinish = TinySTL::uninitialized_copy(begin(), position, newStart);
		newFinish = TinySTL::uninitialized_fill_n(newFinish, n, val);
		newFinish = TinySTL::uninitialized_copy(position, end(), newFinish);

		destroyAndDeallocateAll();
		start_ = newStart;
		finish_ = newFinish;
		endOfStorage_ = newEndOfStorage;
	}

	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::insert_aux(iterator position,
		InputIterator first,
		InputIterator last,
		std::false_type){//�������뺯��������һ������
		difference_type locationLeft = endOfStorage_ - finish_; // ʣ��������С
		difference_type locationNeed = distance(first, last);//���볤�ȣ�iterator�º���

		if (locationLeft >= locationNeed){//�������
			if (finish_ - position > locationNeed){//���Ҫ���볤��С�ڴ�pos��β���ĳ���
				TinySTL::uninitialized_copy(finish_ - locationNeed, finish_, finish_);//�Ӻ���ǰ���ƣ��Ȱ�������һ����ת��
                std::copy_backward(position, finish_ - locationNeed, finish_);//�ٰ��м��ת�ƣ��������������յ�
				TinySTL::uninitialized_copy(first, last, position);//������Ԫ�أ����Ӧ�ÿ���ʹ��uninitialized_copy
			}
			else{
				iterator temp = TinySTL::uninitialized_copy(first + (finish_ - position), last, finish_);//�ȸ����м�һ�β���¼β��
				TinySTL::uninitialized_copy(position, finish_, temp);//��������沿�֣�����Ҫת�Ƶ�ԭ���䵽�����temp
                //�ƺ����ԴӺ���ǰ���ƴ�[pos,finish_)��[pos+locateNeed+finish,finish+locatedNeed)
				TinySTL::uninitialized_copy(first, first + (finish_ - position), position);//����ʣ�µĲ���
			}
			finish_ += locationNeed;//������β��
		}
		else{
			reallocateAndCopy(position, first, last);//ת��Ϊ���¿��ٸ��Ƶĺ���
		}
	}

	template<class T, class Alloc>
	template<class Integer>
	void vector<T, Alloc>::insert_aux(iterator position,
            Integer n, const value_type& value, std::true_type){//�������뺯��������n��val
		assert(n != 0);//���
		difference_type locationLeft = endOfStorage_ - finish_; 
		difference_type locationNeed = n;

		if (locationLeft >= locationNeed){//ʣ��ռ乻��
			auto tempPtr = end() - 1;//��һ����ΪҪȡֵ
			for (; tempPtr - position >= 0; --tempPtr){//����[position, finish_)��[pos+Need,finish+Need)
				//*(tempPtr + locationNeed) = *tempPtr;//Ϊʲô����
				construct(tempPtr + locationNeed, *tempPtr);//construct.h�ļ��µ�������
			}
			TinySTL::uninitialized_fill_n(position, n, value);//���n��val
			finish_ += locationNeed;
		}
		else{
			reallocateAndFillN(position, n, value);//ת��
		}
	}
	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::insert(iterator position, InputIterator first, InputIterator last){//�ӿڲ��뺯��1
		insert_aux(position, first, last, typename std::is_integral<InputIterator>::type());//��ȡ����?
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::insert(iterator position, const size_type& n, const value_type& val){//�ӿڲ��뺯��2
		insert_aux(position, n, val, typename std::is_integral<size_type>::type());//insert_aux�еڶ�������Ϊʲô����Ϊ��һ��n������
	}

	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(iterator position, const value_type& val){//�ӿڲ��뺯��3
		const auto index = position - begin();//len����
		insert(position, 1, val);
		return begin() + index;//��Ϊbegin()���ܸı�
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::push_back(const value_type& value){//β�巨
		insert(end(), value);
	}

    //*******************�߼��Ƚ���غ���**************************
	template<class T, class Alloc>
	bool vector<T, Alloc>::operator == (const vector& v)const{
		if (size() != v.size()){
			return false;
		}
		else{
			auto ptr1 = start_;
			auto ptr2 = v.start_;
			for (; ptr1 != finish_ && ptr2 != v.finish_; ++ptr1, ++ptr2){
				if (*ptr1 != *ptr2)
					return false;
			}
			return true;
		}
	}
	template<class T, class Alloc>
	bool vector<T, Alloc>::operator != (const vector& v)const{
		return !(*this == v);
	}
	template<class T, class Alloc>
	bool operator == (const vector<T, Alloc>& v1, const vector<T, Alloc>& v2){
		//return v1 == v2;
		return v1.operator==(v2);
	}
	template<class T, class Alloc>
	bool operator != (const vector<T, Alloc>& v1, const vector<T, Alloc>& v2){
		return !(v1 == v2);
	}

    //**************************************
	template<class T, class Alloc>
	void vector<T, Alloc>::shrink_to_fit(){//�����ڴ�
		//dataAllocator::deallocate(finish_, endOfStorage_ - finish_);
		//endOfStorage_ = finish_;
		T* t = (T*)dataAllocator::allocate(size());
		finish_ = TinySTL::uninitialized_copy(start_, finish_, t);
		dataAllocator::deallocate(start_, capacity());
		start_ = t;
		endOfStorage_ = finish_;
	}

	template<class T, class Alloc>
	void vector<T, Alloc>::clear(){//���
		dataAllocator::destroy(start_, finish_);//��û���ͷ�������ֻ��size=0;
		finish_ = start_;
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::swap(vector& v){//����
		if (this != &v){
			TinySTL::swap(start_, v.start_);
			TinySTL::swap(finish_, v.finish_);
			TinySTL::swap(endOfStorage_, v.endOfStorage_);
		}
	}

	template<class T, class Alloc>
	void vector<T, Alloc>::pop_back(){//βɾ��
		--finish_;
		dataAllocator::destroy(finish_);//�ǲ��Ǻ���һ��˳����ˣ�
	}
    


    //**************�ڴ���غ���******************
	template<class T, class Alloc>
	void vector<T, Alloc>::destroyAndDeallocateAll(){//���ٺ��ͷ�
		if (capacity() != 0){
			dataAllocator::destroy(start_, finish_);//typedef Alloc dataAllocator,���Ե��õ���Alloc.h�����destroy���ٲ���
			dataAllocator::deallocate(start_, capacity());//ͬ��,����Alloc�µ��ͷ�(��㣬���ȣ�capacity()Ϊ�����º���
		}
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::allocateAndFillN(const size_type n, const value_type& value){//�����ڴ�����
		start_ = dataAllocator::allocate(n);//
		TinySTL::uninitialized_fill_n(start_, n, value);//uninitializedFunctions.h�º���
		finish_ = endOfStorage_ = start_ + n;//+n�Ĳ�����
	}
	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::allocateAndCopy(InputIterator first, InputIterator last){//����͸���
		start_ = dataAllocator::allocate(last - first);//����������
		finish_ = TinySTL::uninitialized_copy(first, last, start_);
		endOfStorage_ = finish_;
	}
    //����vector_aux��̬
	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::vector_aux(InputIterator first, InputIterator last, std::false_type){
		allocateAndCopy(first, last);
	}
	template<class T, class Alloc>
	template<class Integer>
	void vector<T, Alloc>::vector_aux(Integer n, const value_type& value, std::true_type){
		allocateAndFillN(n, value);
	}

	template<class T, class Alloc>
	typename vector<T, Alloc>::size_type vector<T, Alloc>::getNewCapacity(size_type len)const{//��������
		//typename vector<T,Alloc>::size_type���߱�������size_type�����Ͳ��Ǳ���
        //ǰ��Ҫ����vector<T,Alloc>::��������Ϊsize_type�����ǳ�Ա��������Ա������Ҳ�����ǳ�Ա����
        //����ʵ���൱��size_type vector<T, Alloc>::getNewCapacity(size_type len)const;ֻ���ñ���������
        //����������+const������������������޸����е����ݳ�Ա
        size_type oldCapacity = endOfStorage_ - start_;//����size_type��������
		auto res = TinySTL::max(oldCapacity, len);
		size_type newCapacity = (oldCapacity != 0 ? (oldCapacity + res) : len);
		return newCapacity;
	}

}
#endif
