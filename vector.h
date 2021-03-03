#ifndef MYTINYSTL_VECTOR_H_
#define MYTINYSTL_VECTOR_H_

// vector模板类

// notes:
//
// 异常保证：
// mystl::vecotr<T> 满足基本异常保证，部分函数无异常保证，并对以下函数做强异常安全保证：
//   * emplace
//   * emplace_back
//   * push_back
// 当 std::is_nothrow_move_assignable<T>::value == true 时，以下函数也满足强异常保证：
//   * reserve
//   * resize
//   * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

/*第72行noexcept没有完全理解，第82行萃取技术，第85行，第104行，第109、110行
第176行，第332行、第361行、第394行,第476行，第821行。
这个vector实现的很好，注释只能初步明白，后续在博客深入学习https://blog.csdn.net/meixingshi/category_10846333.html
*/

namespace mystl
{
//检测是否有max和min，避免冲突
#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif 
#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif 
template <class T>
class vector
{
  static_assert(!std::is_same<bool, T>::value, "vector<bool> is abandoned in mystl");
public:
  //嵌套类型定义
  typedef mystl::allocator<T>                      allocator_type;
  typedef mystl::allocator<T>                      data_allocator;

  typedef typename allocator_type::value_type      value_type;
  typedef typename allocator_type::pointer         pointer;
  typedef typename allocator_type::const_pointer   const_pointer;
  typedef typename allocator_type::reference       reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type       size_type;
  typedef typename allocator_type::difference_type difference_type;
  //定义迭代器
  typedef value_type*                              iterator;
  typedef const value_type*                        const_iterator;
  typedef mystl::reverse_iterator<iterator>        reverse_iterator;
  typedef mystl::reverse_iterator<const_iterator>  const_reverse_iterator;

  allocator_type get_allocator() { return data_allocator(); }//？

private:
  iterator begin_;//vector首部位置    
  iterator end_;//vector尾部位置      
  iterator cap_;//vector容量最后位置    
public:
  //构造、复制、析构、赋值
  //下面三个构造都转化为try_init()
    vector() noexcept//表示下面的绝对不会发生异常
  { try_init(); }

  explicit vector(size_type n)//显式构造
  { fill_init(n, value_type()); }

  vector(size_type n, const value_type& value)
  { fill_init(n, value); }
  //下面两个都转化为range_init()
  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>//通过enable_if控制萃取，不明白
  vector(Iter first, Iter last)
  {
    MYSTL_DEBUG(!(last < first));//检查顺序，为何不直接使用assert？
    range_init(first, last);
  }

  vector(const vector& rhs)
  {
    range_init(rhs.begin_, rhs.end_);
  }
  //转移拷贝
  vector(vector&& rhs) noexcept//简练，把赋值语句放在初始化里
    :begin_(rhs.begin_),
    end_(rhs.end_),
    cap_(rhs.cap_)
  {
    rhs.begin_ = nullptr;
    rhs.end_ = nullptr;
    rhs.cap_ = nullptr;
  }
  //通过ilist构造
  vector(std::initializer_list<value_type> ilist)//不明白
  {
    range_init(ilist.begin(), ilist.end());
  }
  //重载赋值
  vector& operator=(const vector& rhs);
  vector& operator=(vector&& rhs) noexcept;//这里为什么要用noexcept

  vector& operator=(std::initializer_list<value_type> ilist)
  {
    vector tmp(ilist.begin(), ilist.end());
    swap(tmp);
    return *this;
  }
  
  ~vector()
  { 
    destroy_and_recover(begin_, end_, cap_ - begin_);
    begin_ = end_ = cap_ = nullptr;
  }

public:
  //迭代器相关函数
  iterator               begin()         noexcept 
  { return begin_; }
  const_iterator         begin()   const noexcept
  { return begin_; }
  iterator               end()           noexcept
  { return end_; }
  const_iterator         end()     const noexcept 
  { return end_; }

  reverse_iterator       rbegin()        noexcept 
  { return reverse_iterator(end()); }
  const_reverse_iterator rbegin()  const noexcept
  { return const_reverse_iterator(end()); }
  reverse_iterator       rend()          noexcept
  { return reverse_iterator(begin()); }
  const_reverse_iterator rend()    const noexcept 
  { return const_reverse_iterator(begin()); }

  const_iterator         cbegin()  const noexcept 
  { return begin(); }
  const_iterator         cend()    const noexcept
  { return end(); }
  const_reverse_iterator crbegin() const noexcept
  { return rbegin(); }
  const_reverse_iterator crend()   const noexcept
  { return rend(); }

  //接口相关函数
  bool      empty()    const noexcept//判空
  { return begin_ == end_; }
  size_type size()     const noexcept//长度
  { return static_cast<size_type>(end_ - begin_); }//static_cast代替了ptrdiff_t
  size_type max_size() const noexcept//电脑所能产生的vector最大容量
  { return static_cast<size_type>(-1) / sizeof(T); }
  size_type capacity() const noexcept//容量
  { return static_cast<size_type>(cap_ - begin_); }
  void      reserve(size_type n);
  void      shrink_to_fit();
  //重载[]，at函数
  reference operator[](size_type n)
  {
    MYSTL_DEBUG(n < size());
    return *(begin_ + n);
  }
  const_reference operator[](size_type n) const
  {
    MYSTL_DEBUG(n < size());
    return *(begin_ + n);
  }
  //为什么at函数要扔回返回,而[]不用
  reference at(size_type n)
  {
    THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
    return (*this)[n];
  }
  const_reference at(size_type n) const
  {
    THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
    return (*this)[n];
  }

  reference front()//头元素
  {
    MYSTL_DEBUG(!empty());
    return *begin_;
  }
  const_reference front() const
  {
    MYSTL_DEBUG(!empty());
    return *begin_;
  }
  reference back()//末元素
  {
    MYSTL_DEBUG(!empty());
    return *(end_ - 1);
  }
  const_reference back() const
  {
    MYSTL_DEBUG(!empty());
    return *(end_ - 1);
  }

  pointer       data()       noexcept { return begin_; }//奇怪的设计
  const_pointer data() const noexcept { return begin_; }

  
  
  void assign(size_type n, const value_type& value)//从原容器0位置开始赋值，会清除原有元素
  { fill_assign(n, value); }

  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>
  void assign(Iter first, Iter last)//尾插区间
  {
    MYSTL_DEBUG(!(last < first));
    copy_assign(first, last, iterator_category(first));
  }
  
  void assign(std::initializer_list<value_type> il)//尾插另一个
  { copy_assign(il.begin(), il.end(), mystl::forward_iterator_tag{}); }

  
  template <class... Args>
  iterator emplace(const_iterator pos, Args&& ...args);//变参数

  template <class... Args>
  void emplace_back(Args&& ...args);//

  
  void push_back(const value_type& value);
  void push_back(value_type&& value)//转移尾插
  { emplace_back(mystl::move(value)); }

  void pop_back();

  
  iterator insert(const_iterator pos, const value_type& value);
  iterator insert(const_iterator pos, value_type&& value)
  { return emplace(pos, mystl::move(value)); }//转移插入

  iterator insert(const_iterator pos, size_type n, const value_type& value)
  {
    MYSTL_DEBUG(pos >= begin() && pos <= end());
    return fill_insert(const_cast<iterator>(pos), n, value);
  }

  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>
  void     insert(const_iterator pos, Iter first, Iter last)//插入区间
  {
    MYSTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
    copy_insert(const_cast<iterator>(pos), first, last);
  }

  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);
  void     clear() { erase(begin(), end()); }//清空

  void     resize(size_type new_size) { return resize(new_size, value_type()); }//默认收缩
  void     resize(size_type new_size, const value_type& value);

  void     reverse() { mystl::reverse(begin(), end()); }//反转

  void     swap(vector& rhs) noexcept;//交换

private:
  
  void      try_init() noexcept;

  void      init_space(size_type size, size_type cap);

  void      fill_init(size_type n, const value_type& value);
  template <class Iter>
  void      range_init(Iter first, Iter last);

  void      destroy_and_recover(iterator first, iterator last, size_type n);

  size_type get_new_cap(size_type add_size);

  
  void      fill_assign(size_type n, const value_type& value);

  template <class IIter>
  void      copy_assign(IIter first, IIter last, input_iterator_tag);

  template <class FIter>
  void      copy_assign(FIter first, FIter last, forward_iterator_tag);

  
  template <class... Args>
  void      reallocate_emplace(iterator pos, Args&& ...args);
  void      reallocate_insert(iterator pos, const value_type& value);

  
  iterator  fill_insert(iterator pos, size_type n, const value_type& value);
  template <class IIter>
  void      copy_insert(iterator pos, IIter first, IIter last);

  
  void      reinsert(size_type size);
};

/*****************************************************************************************/

template <class T>
vector<T>& vector<T>::operator=(const vector& rhs)//重载赋值
{
  if (this != &rhs)
  {
    const auto len = rhs.size();
    if (len > capacity())//容量不够：构造临时对象，再交换
    { 
      vector tmp(rhs.begin(), rhs.end());
      swap(tmp);
    }
    else if (size() >= len)//如果已占用的内存大于len
    {
      auto i = mystl::copy(rhs.begin(), rhs.end(), begin());//在两个地址有重叠的情况下复制
      data_allocator::destroy(i, end_);//清除残余的空间
      end_ = begin_ + len;
    }
    else//容量够，现在占用的不够，需要继续占用空闲的空间
    { 
      mystl::copy(rhs.begin(), rhs.begin() + size(), begin_);//分两段复制
      mystl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);
      cap_ = end_ = begin_ + len;//不明白为什么cap_也要变换，似乎没有必要收缩，上一种情况就没有收缩
    }
  }
  return *this;
}
//转移赋值
template <class T>
vector<T>& vector<T>::operator=(vector&& rhs) noexcept
{
  destroy_and_recover(begin_, end_, cap_ - begin_);//要把原来的区间内存销毁释放
  begin_ = rhs.begin_;
  end_ = rhs.end_;
  cap_ = rhs.cap_;
  rhs.begin_ = nullptr;
  rhs.end_ = nullptr;
  rhs.cap_ = nullptr;
  return *this;
}

template <class T>
void vector<T>::reserve(size_type n)//重新申请容量
{//只有重新申请的容量大于已有容量才执行
  if (capacity() < n)
  {
    THROW_LENGTH_ERROR_IF(n > max_size(),
                          "n can not larger than max_size() in vector<T>::reserve(n)");
    const auto old_size = size();
    auto tmp = data_allocator::allocate(n);//重新申请内存
    mystl::uninitialized_move(begin_, end_, tmp);//转移元素
    data_allocator::deallocate(begin_, cap_ - begin_);//为什么不调用destroy_and_recover
    begin_ = tmp;
    end_ = tmp + old_size;
    cap_ = begin_ + n;
  }
}

template <class T>
void vector<T>::shrink_to_fit()//收缩容量
{
  if (end_ < cap_)
  {
    reinsert(size());
  }
}

template <class T>
template <class ...Args>
typename vector<T>::iterator
vector<T>::emplace(const_iterator pos, Args&& ...args)//插入一个元素参数，传入的是常迭代器
{
  MYSTL_DEBUG(pos >= begin() && pos <= end());
  iterator xpos = const_cast<iterator>(pos);//去除const
  const size_type n = xpos - begin_;
  if (end_ != cap_ && xpos == end_)//如果还有容量，且插入的位置是末位
  {
    data_allocator::construct(mystl::address_of(*end_), mystl::forward<Args>(args)...);
    ++end_;
  }
  else if (end_ != cap_)
  {
    auto new_end = end_;
    data_allocator::construct(mystl::address_of(*end_), *(end_ - 1));//先转移末位
    ++new_end;
    mystl::copy_backward(xpos, end_ - 1, end_);//再从后面复制前面的
    *xpos = value_type(mystl::forward<Args>(args)...);//最后复制pos位置
    end_ = new_end;//源代码没有更新end_
  }
  else
  {
    reallocate_emplace(xpos, mystl::forward<Args>(args)...);//容量不够了
  }
  return begin() + n;//返回插入位置
}

template <class T>
template <class ...Args>
void vector<T>::emplace_back(Args&& ...args)//emplace_back和push_back不同
{
  if (end_ < cap_)
  {//emplace是用传入的参数原地构建一个对象，而push_back传入的是一个对象
    data_allocator::construct(mystl::address_of(*end_), mystl::forward<Args>(args)...);
    ++end_;
  }
  else
  {
    reallocate_emplace(end_, mystl::forward<Args>(args)...);//动态增长
  }
}

template <class T>
void vector<T>::push_back(const value_type& value)//尾插法
{
  if (end_ != cap_)
  {
    data_allocator::construct(mystl::address_of(*end_), value);
    ++end_;
  }
  else
  {
    reallocate_insert(end_, value);//动态增长
  }
}

template <class T>
void vector<T>::pop_back()//尾删法
{
  MYSTL_DEBUG(!empty());
  data_allocator::destroy(end_ - 1);
  --end_;
}

template <class T>
typename vector<T>::iterator
vector<T>::insert(const_iterator pos, const value_type& value)//指定位置插入
{//基本同emplace
  MYSTL_DEBUG(pos >= begin() && pos <= end());
  iterator xpos = const_cast<iterator>(pos);
  const size_type n = pos - begin_;
  if (end_ != cap_ && xpos == end_)//够用，尾插
  {
    data_allocator::construct(mystl::address_of(*end_), value);
    ++end_;
  }
  else if (end_ != cap_)//不够用，中间插入
  {//分段移
    auto new_end = end_;
    data_allocator::construct(mystl::address_of(*end_), *(end_ - 1));
    ++new_end;
    auto value_copy = value;      mystl::copy_backward(xpos, end_ - 1, end_);
    *xpos = mystl::move(value_copy);
    end_ = new_end;
  }
  else
  {//不够用了
    reallocate_insert(xpos, value);
  }
  return begin_ + n;
}

template <class T>
typename vector<T>::iterator
vector<T>::erase(const_iterator pos)//擦除
{
  MYSTL_DEBUG(pos >= begin() && pos < end());
  iterator xpos = begin_ + (pos - begin());//要返回的位置，为什么不直接等于pos？
  mystl::move(xpos + 1, end_, xpos);//移动
  data_allocator::destroy(end_ - 1);
  --end_;
  return xpos;
}

template <class T>
typename vector<T>::iterator
vector<T>::erase(const_iterator first, const_iterator last)//擦除区间
{
  MYSTL_DEBUG(first >= begin() && last <= end() && !(last < first));
  const auto n = first - begin();
  iterator r = begin_ + (first - begin());
  data_allocator::destroy(mystl::move(r + (last - first), end_, r), end_);//先移动区间,然后再销毁多余的区间
  end_ = end_ - (last - first);
  return begin_ + n;
}

template <class T>
void vector<T>::resize(size_type new_size, const value_type& value)//重新定义大小
{
  if (new_size < size())
  {
    erase(begin() + new_size, end());
  }
  else
  {
    insert(end(), new_size - size(), value);
  }
}

template <class T>
void vector<T>::swap(vector<T>& rhs) noexcept//交换
{
  if (this != &rhs)//通过交换地址来实现
  {
    mystl::swap(begin_, rhs.begin_);
    mystl::swap(end_, rhs.end_);
    mystl::swap(cap_, rhs.cap_);
  }
}

/*****************************************************************************************/

template <class T>
void vector<T>::try_init() noexcept//默认构造
{
  try
  {
    begin_ = data_allocator::allocate(16);
    end_ = begin_;
    cap_ = begin_ + 16;
  }
  catch (...)
  {
    begin_ = nullptr;
    end_ = nullptr;
    cap_ = nullptr;
  }
}

template <class T>
void vector<T>::init_space(size_type size, size_type cap)//指定容量和大小，初始vector
{
  try
  {
    begin_ = data_allocator::allocate(cap);
    end_ = begin_ + size;
    cap_ = begin_ + cap;
  }
  catch (...)
  {
    begin_ = nullptr;
    end_ = nullptr;
    cap_ = nullptr;
    throw;
  }
}

template <class T>
void vector<T>::
fill_init(size_type n, const value_type& value)//n个val初始化
{//如果大于16，size=cap，不大于16,则分配16个做预留
  const size_type init_size = mystl::max(static_cast<size_type>(16), n);//至少分配16个
  init_space(n, init_size);//先初始化空间，再赋值
  mystl::uninitialized_fill_n(begin_, n, value);
}

template <class T>
template <class Iter>
void vector<T>::
range_init(Iter first, Iter last)//范围初始化
{//先求空间
  const size_type init_size = mystl::max(static_cast<size_type>(last - first),
                                         static_cast<size_type>(16));
  init_space(static_cast<size_type>(last - first), init_size);
  mystl::uninitialized_copy(first, last, begin_);//复制
}

template <class T>
void vector<T>::
destroy_and_recover(iterator first, iterator last, size_type n)//销毁和释放
{
  data_allocator::destroy(first, last);//销毁
  data_allocator::deallocate(first, n);//释放
}

template <class T>
typename vector<T>::size_type 
vector<T>::
get_new_cap(size_type add_size)//获取新内存大小
{
  const auto old_size = capacity();
  THROW_LENGTH_ERROR_IF(old_size > max_size() - add_size,
                        "vector<T>'s size too big");
  if (old_size > max_size() - old_size / 2)//已经到最大内存的2/3
  {
    return old_size + add_size > max_size() - 16
      ? old_size + add_size : old_size + add_size + 16;//分情况增长
  }
  const size_type new_size = old_size == 0
    ? mystl::max(add_size, static_cast<size_type>(16))//如果原大小为0，一次至少开辟16个
    : mystl::max(old_size + old_size / 2, old_size + add_size);//增长一半或者更多
  return new_size;
}

template <class T>
void vector<T>::
fill_assign(size_type n, const value_type& value)//assgin的具体实现
{//会清除原来vector的元素
  if (n > capacity())//如果大于容量
  {
    vector tmp(n, value);
    swap(tmp);
  }
  else if (n > size())
  {//分段填充
    mystl::fill(begin(), end(), value);
    end_ = mystl::uninitialized_fill_n(end_, n - size(), value);
  }
  else
  {
    erase(mystl::fill_n(begin_, n, value), end_);//先填充，然后擦除多余的
  }
}

template <class T>
template <class IIter>
void vector<T>::
copy_assign(IIter first, IIter last, input_iterator_tag)//复制区间，并会清除原有元素
{
  auto cur = begin_;
  for (; first != last && cur != end_; ++first, ++cur)
  {
    *cur = *first;
  }
  if (first == last)//如果原区间更大
  {
    erase(cur, end_);//擦除剩余的
  }
  else
  {
    insert(end_, first, last);//原区间小，插入剩下的
  }
}

template <class T>
template <class FIter>
void vector<T>::
copy_assign(FIter first, FIter last, forward_iterator_tag)//复制区间，并会清除原有元素
{//正向迭代器即可
  const size_type len = mystl::distance(first, last);
  if (len > capacity())
  {
    vector tmp(first, last);
    swap(tmp);
  }
  else if (size() >= len)
  {
    auto new_end = mystl::copy(first, last, begin_);
    data_allocator::destroy(new_end, end_);
    end_ = new_end;
  }
  else
  {
    auto mid = first;
    mystl::advance(mid, size());
    mystl::copy(first, mid, begin_);
    auto new_end = mystl::uninitialized_copy(mid, last, end_);
    end_ = new_end;
  }
}

template <class T>
template <class ...Args>
void vector<T>::
reallocate_emplace(iterator pos, Args&& ...args)//end_<cap_需要重新申请内存
{
  const auto new_size = get_new_cap(1);//动态增长内存
  auto new_begin = data_allocator::allocate(new_size);//重新申请内存
  auto new_end = new_begin;
  try
  {//分区间转移
    new_end = mystl::uninitialized_move(begin_, pos, new_begin);
    data_allocator::construct(mystl::address_of(*new_end), mystl::forward<Args>(args)...);
    ++new_end;
    new_end = mystl::uninitialized_move(pos, end_, new_end);
  }
  catch (...)
  {
    data_allocator::deallocate(new_begin, new_size);
    throw;
  }
  destroy_and_recover(begin_, end_, cap_ - begin_);//销毁和释放原来的
  begin_ = new_begin;
  end_ = new_end;
  cap_ = new_begin + new_size;
}

template <class T>
void vector<T>::reallocate_insert(iterator pos, const value_type& value)
{//与reallocate_emplace基本相同，区别仍然在于传入的是一个是对象，一个可以是对象参数
  const auto new_size = get_new_cap(1);
  auto new_begin = data_allocator::allocate(new_size);
  auto new_end = new_begin;
  const value_type& value_copy = value;
  try
  {
    new_end = mystl::uninitialized_move(begin_, pos, new_begin);
    data_allocator::construct(mystl::address_of(*new_end), value_copy);
    ++new_end;
    new_end = mystl::uninitialized_move(pos, end_, new_end);
  }
  catch (...)
  {
    data_allocator::deallocate(new_begin, new_size);
    throw;
  }
  destroy_and_recover(begin_, end_, cap_ - begin_);
  begin_ = new_begin;
  end_ = new_end;
  cap_ = new_begin + new_size;
}

template <class T>
typename vector<T>::iterator 
vector<T>::
fill_insert(iterator pos, size_type n, const value_type& value)//insert的实际调用
{
  if (n == 0)
    return pos;
  const size_type xpos = pos - begin_;
  const value_type value_copy = value;    
  if (static_cast<size_type>(cap_ - end_) >= n){//剩下的容量够用
    const size_type after_elems = end_ - pos;
    auto old_end = end_;
    if (after_elems > n)//如何插入位置到尾部的距离大于n
    {
      mystl::uninitialized_copy(end_ - n, end_, end_);//先移动一部分元素到没有使用的一段空间
      end_ += n;
      mystl::move_backward(pos, old_end - n, old_end);//再移动剩下的需要移动的元素
      mystl::uninitialized_fill_n(pos, n, value_copy);//插入
    }
    else
    {//如何插入位置到尾部的距离小于n
      end_ = mystl::uninitialized_fill_n(end_, n - after_elems, value_copy);//先插入一部分元素
      end_ = mystl::uninitialized_move(pos, old_end, end_);//再移动原空间需要移动的到最后面
      mystl::uninitialized_fill_n(pos, after_elems, value_copy);//插入剩下的
    }
  }
  else//剩下的容量不够用
  {     
    const auto new_size = get_new_cap(n);
    auto new_begin = data_allocator::allocate(new_size);
    auto new_end = new_begin;
    try
    {//分区间复制
      new_end = mystl::uninitialized_move(begin_, pos, new_begin);
      new_end = mystl::uninitialized_fill_n(new_end, n, value);
      new_end = mystl::uninitialized_move(pos, end_, new_end);
    }
    catch (...)
    {
      destroy_and_recover(new_begin, new_end, new_size);
      throw;
    }
    data_allocator::deallocate(begin_, cap_ - begin_);//销毁原来的
    begin_ = new_begin;
    end_ = new_end;
    cap_ = begin_ + new_size;
  }
  return begin_ + xpos;
}

template <class T>
template <class IIter>
void vector<T>::
copy_insert(iterator pos, IIter first, IIter last)//复制插入
{//为什么要分开移动？直接先后移不好吗？
  if (first == last)
    return;
  const auto n = mystl::distance(first, last);//计算距离
  if ((cap_ - end_) >= n){//容量够用
    const auto after_elems = end_ - pos;
    auto old_end = end_;
    if (after_elems > n)//同插入n个val
    {//把[pos,end)划分为两段，[pos,pos+n),[pos+n,end)
      end_ = mystl::uninitialized_copy(end_ - n, end_, end_);//先复制[pos+n,end)到end之后,并更新end
      mystl::move_backward(pos, old_end - n, old_end);//再以oldend为终点移动[pos,pos+n)
      mystl::uninitialized_copy(first, last, pos);//插入
    }
    else
    {//先用mid把[first,last)区间划分为两段[first,after),[after,last)
      auto mid = first;//定义一个mid用来划分区间
      mystl::advance(mid, after_elems);//不清楚iter是什么类型，所以用davance调整，mid是引用传参
      end_ = mystl::uninitialized_copy(mid, last, end_);//先移动[a,l)到end_之后,并更新end_
      end_ = mystl::uninitialized_move(pos, old_end, end_);//再移动[a,l)到end_之后，更新end_
      mystl::uninitialized_copy(first, mid, pos);//移动[f,a)到pos_之后，更新end_
    }
  }
  else{//内存不够用了，需要重新获取容量
    const auto new_size = get_new_cap(n);
    auto new_begin = data_allocator::allocate(new_size);
    auto new_end = new_begin;
    try
    {
      new_end = mystl::uninitialized_move(begin_, pos, new_begin);
      new_end = mystl::uninitialized_copy(first, last, new_end);
      new_end = mystl::uninitialized_move(pos, end_, new_end);
    }
    catch (...)
    {
      destroy_and_recover(new_begin, new_end, new_size);
      throw;
    }
    data_allocator::deallocate(begin_, cap_ - begin_);
    begin_ = new_begin;
    end_ = new_end;
    cap_ = begin_ + new_size;
  }
}

template <class T>
void vector<T>::reinsert(size_type size)//重新插入，shrink_to_fit的实际调用
{//问题，如果传入的参数不等于原size呢？
  auto new_begin = data_allocator::allocate(size);
  try
  {
    mystl::uninitialized_move(begin_, end_, new_begin);
  }
  catch (...)
  {
    data_allocator::deallocate(new_begin, size);
    throw;
  }
  data_allocator::deallocate(begin_, cap_ - begin_);//原来的
  begin_ = new_begin;
  end_ = begin_ + size;
  cap_ = begin_ + size;
}

/*****************************************************************************************/
//重载运算符
template <class T>
bool operator==(const vector<T>& lhs, const vector<T>& rhs)
{//首先比对大小
  return lhs.size() == rhs.size() &&//如果大小相等，确实只需要传入三个变量即可
    mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class T>
bool operator<(const vector<T>& lhs, const vector<T>& rhs)//重载<
{
  return mystl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), lhs.end());
}

template <class T>
bool operator!=(const vector<T>& lhs, const vector<T>& rhs)//重载！=
{
  return !(lhs == rhs);
}

template <class T>
bool operator>(const vector<T>& lhs, const vector<T>& rhs)//重载<
{
  return rhs < lhs;
}

template <class T>
bool operator<=(const vector<T>& lhs, const vector<T>& rhs)//重载<=
{
  return !(rhs < lhs);
}

template <class T>
bool operator>=(const vector<T>& lhs, const vector<T>& rhs)//重载!=
{
  return !(lhs < rhs);
}

template <class T>
void swap(vector<T>& lhs, vector<T>& rhs)//交换
{
  lhs.swap(rhs);
}

} #endif 
