#ifndef _VECTOR_CPP_
#define _VECTOR_CPP_

/*剩余问题，第20行，erase(iterator it,int len)多态
第157行，copy_backward还未自己写,这个不能用从前向后复制，不然可能会覆盖一些元素。第176行为什么使用Integer？而不是204行做法
第153行std::false_type怎么用的。第166行另一种做法,第186行原因。第200行萃取技术
第219行，为什么要重载两个operate==，显示和隐式？第278行push_back()函数。第251行，采用哪种做法
*/


namespace TinySTL{
    /************构造，复制，析构相关************/
	template<class T, class Alloc>
	vector<T, Alloc>::vector(const size_type n){
		allocateAndFillN(n, value_type());//value_type相当于T()创建临时对象，相当于vector<int> v(10)，只有一个参数，所以会有一个初始值
	}
	template<class T, class Alloc>
	vector<T, Alloc>::vector(const size_type n, const value_type& value){
		allocateAndFillN(n, value);
	}
	template<class T, class Alloc>
	template<class InputIterator>
	vector<T, Alloc>::vector(InputIterator first, InputIterator last){
		//处理指针和数字间的区别的函数？
		vector_aux(first, last, typename std::is_integral<InputIterator>::type());//?
	}

	template<class T, class Alloc>
	vector<T, Alloc>::vector(const vector& v){
		allocateAndCopy(v.start_, v.finish_);
	}
	template<class T, class Alloc>
	vector<T, Alloc>::vector(vector&& v){//转移构造函数
		start_ = v.start_;
		finish_ = v.finish_;
		endOfStorage_ = v.endOfStorage_;
		v.start_ = v.finish_ = v.endOfStorage_ = 0;
	}

	template<class T, class Alloc>//重载=
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

	//*************和容器的容量相关******************************
	template<class T, class Alloc>
	void vector<T, Alloc>::resize(size_type n, value_type val = value_type()){//重新设置大小
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
			T *newStart = dataAllocator::allocate(getNewCapacity(lengthOfInsert));//寻找一块新大小的内存空间
			T *newFinish = TinySTL::uninitialized_copy(begin(), end(), newStart);
			newFinish = TinySTL::uninitialized_fill_n(newFinish, lengthOfInsert, val);

			destroyAndDeallocateAll();
			start_ = newStart;
			finish_ = newFinish;
			endOfStorage_ = start_ + n;
		}
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::reserve(size_type n){//重新设置容量
		if (n <= capacity())
			return;
		T *newStart = dataAllocator::allocate(n);
		T *newFinish = TinySTL::uninitialized_copy(begin(), end(), newStart);
		
        destroyAndDeallocateAll();
		start_ = newStart;
		finish_ = newFinish;
		endOfStorage_ = start_ + n;
	}

	//***************修改容器的相关操作**************************
	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator position){//删除position位置
		return erase(position, position + 1);//转化为另一个多态
	}
	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator first, iterator last){//删除一个区间
		difference_type lenOfTail = end() - last;//end()=finish_;距尾部的距离
		difference_type lenOfRemoved = last - first;//移除的长度
		finish_ = finish_ - lenOfRemoved;//更新尾部
		for (; lenOfTail != 0; --lenOfTail){
			auto temp = (last - lenOfRemoved);//=从first开始，一直到长度为剩余长度lenOfTail的地址
			*temp = *(last++);//通过last改变
		}
		return (first);//返回first迭代器
	}

	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::reallocateAndCopy(iterator position, InputIterator first, InputIterator last){//复制进来一个区间内容
		difference_type newCapacity = getNewCapacity(last - first);
		T *newStart = dataAllocator::allocate(newCapacity);
		T *newEndOfStorage = newStart + newCapacity;
        //uninitialized_copy(复制起点，复制终点，新内存起点)
		T *newFinish = TinySTL::uninitialized_copy(begin(), position, newStart);
		newFinish = TinySTL::uninitialized_copy(first, last, newFinish);
		newFinish = TinySTL::uninitialized_copy(position, end(), newFinish);
        //销毁原来空间，并更新三个迭代器
		destroyAndDeallocateAll();
		start_ = newStart;
		finish_ = newFinish;
		endOfStorage_ = newEndOfStorage;
	}

	template<class T, class Alloc>
	void vector<T, Alloc>::reallocateAndFillN(iterator position, const size_type& n, const value_type& val){//增加n个val
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
		std::false_type){//辅助插入函数：插入一段区间
		difference_type locationLeft = endOfStorage_ - finish_; // 剩余容量大小
		difference_type locationNeed = distance(first, last);//插入长度，iterator下函数

		if (locationLeft >= locationNeed){//如果够用
			if (finish_ - position > locationNeed){//如果要插入长度小于从pos到尾部的长度
				TinySTL::uninitialized_copy(finish_ - locationNeed, finish_, finish_);//从后向前复制，先把最后面的一部分转移
                std::copy_backward(position, finish_ - locationNeed, finish_);//再把中间的转移，第三个参数是终点
				TinySTL::uninitialized_copy(first, last, position);//插入新元素，这个应该可以使用uninitialized_copy
			}
			else{
				iterator temp = TinySTL::uninitialized_copy(first + (finish_ - position), last, finish_);//先复制中间一段并记录尾部
				TinySTL::uninitialized_copy(position, finish_, temp);//复制最后面部分，将需要转移的原区间到新起点temp
                //似乎可以从后向前复制从[pos,finish_)到[pos+locateNeed+finish,finish+locatedNeed)
				TinySTL::uninitialized_copy(first, first + (finish_ - position), position);//复制剩下的部分
			}
			finish_ += locationNeed;//更新新尾部
		}
		else{
			reallocateAndCopy(position, first, last);//转换为重新开辟复制的函数
		}
	}

	template<class T, class Alloc>
	template<class Integer>
	void vector<T, Alloc>::insert_aux(iterator position,
            Integer n, const value_type& value, std::true_type){//辅助插入函数，插入n个val
		assert(n != 0);//检查
		difference_type locationLeft = endOfStorage_ - finish_; 
		difference_type locationNeed = n;

		if (locationLeft >= locationNeed){//剩余空间够用
			auto tempPtr = end() - 1;//减一，因为要取值
			for (; tempPtr - position >= 0; --tempPtr){//复制[position, finish_)到[pos+Need,finish+Need)
				//*(tempPtr + locationNeed) = *tempPtr;//为什么出错？
				construct(tempPtr + locationNeed, *tempPtr);//construct.h文件下单个复制
			}
			TinySTL::uninitialized_fill_n(position, n, value);//添加n个val
			finish_ += locationNeed;
		}
		else{
			reallocateAndFillN(position, n, value);//转换
		}
	}
	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::insert(iterator position, InputIterator first, InputIterator last){//接口插入函数1
		insert_aux(position, first, last, typename std::is_integral<InputIterator>::type());//萃取技术?
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::insert(iterator position, const size_type& n, const value_type& val){//接口插入函数2
		insert_aux(position, n, val, typename std::is_integral<size_type>::type());//insert_aux中第二个参数为什么不设为上一行n的类型
	}

	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(iterator position, const value_type& val){//接口插入函数3
		const auto index = position - begin();//len长度
		insert(position, 1, val);
		return begin() + index;//因为begin()可能改变
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::push_back(const value_type& value){//尾插法
		insert(end(), value);
	}

    //*******************逻辑比较相关函数**************************
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
	void vector<T, Alloc>::shrink_to_fit(){//收缩内存
		//dataAllocator::deallocate(finish_, endOfStorage_ - finish_);
		//endOfStorage_ = finish_;
		T* t = (T*)dataAllocator::allocate(size());
		finish_ = TinySTL::uninitialized_copy(start_, finish_, t);
		dataAllocator::deallocate(start_, capacity());
		start_ = t;
		endOfStorage_ = finish_;
	}

	template<class T, class Alloc>
	void vector<T, Alloc>::clear(){//清空
		dataAllocator::destroy(start_, finish_);//并没有释放容量，只是size=0;
		finish_ = start_;
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::swap(vector& v){//交换
		if (this != &v){
			TinySTL::swap(start_, v.start_);
			TinySTL::swap(finish_, v.finish_);
			TinySTL::swap(endOfStorage_, v.endOfStorage_);
		}
	}

	template<class T, class Alloc>
	void vector<T, Alloc>::pop_back(){//尾删法
		--finish_;
		dataAllocator::destroy(finish_);//是不是和上一行顺序错了？
	}
    


    //**************内存相关函数******************
	template<class T, class Alloc>
	void vector<T, Alloc>::destroyAndDeallocateAll(){//销毁和释放
		if (capacity() != 0){
			dataAllocator::destroy(start_, finish_);//typedef Alloc dataAllocator,所以调用的是Alloc.h里面的destroy销毁操作
			dataAllocator::deallocate(start_, capacity());//同上,调用Alloc下的释放(起点，长度）capacity()为本类下函数
		}
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::allocateAndFillN(const size_type n, const value_type& value){//分配内存和填充
		start_ = dataAllocator::allocate(n);//
		TinySTL::uninitialized_fill_n(start_, n, value);//uninitializedFunctions.h下函数
		finish_ = endOfStorage_ = start_ + n;//+n的操作？
	}
	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::allocateAndCopy(InputIterator first, InputIterator last){//分配和复制
		start_ = dataAllocator::allocate(last - first);//迭代器操作
		finish_ = TinySTL::uninitialized_copy(first, last, start_);
		endOfStorage_ = finish_;
	}
    //两个vector_aux多态
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
	typename vector<T, Alloc>::size_type vector<T, Alloc>::getNewCapacity(size_type len)const{//增加容量
		//typename vector<T,Alloc>::size_type告诉编译器，size_type是类型不是变量
        //前面要包含vector<T,Alloc>::作用域，因为size_type可能是成员变量，成员函数，也可能是成员类型
        //所以实际相当于size_type vector<T, Alloc>::getNewCapacity(size_type len)const;只是让编译器明白
        //函数名后面+const表明不允许这个函数修改类中的数据成员
        size_type oldCapacity = endOfStorage_ - start_;//这里size_type就是类型
		auto res = TinySTL::max(oldCapacity, len);
		size_type newCapacity = (oldCapacity != 0 ? (oldCapacity + res) : len);
		return newCapacity;
	}

}
#endif
