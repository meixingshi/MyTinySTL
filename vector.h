#ifndef MYTINYSTL_VECTOR_H_
#define MYTINYSTL_VECTOR_H_

// vectorģ����

// notes:
//
// �쳣��֤��
// mystl::vecotr<T> ��������쳣��֤�����ֺ������쳣��֤���������º�����ǿ�쳣��ȫ��֤��
//   * emplace
//   * emplace_back
//   * push_back
// �� std::is_nothrow_move_assignable<T>::value == true ʱ�����º���Ҳ����ǿ�쳣��֤��
//   * reserve
//   * resize
//   * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

/*��72��noexceptû����ȫ��⣬��82����ȡ��������85�У���104�У���109��110��
��176�У���332�С���361�С���394��,��476�У���821�С�
���vectorʵ�ֵĺܺã�ע��ֻ�ܳ������ף������ڲ�������ѧϰhttps://blog.csdn.net/meixingshi/category_10846333.html
*/

namespace mystl
{
//����Ƿ���max��min�������ͻ
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
  //Ƕ�����Ͷ���
  typedef mystl::allocator<T>                      allocator_type;
  typedef mystl::allocator<T>                      data_allocator;

  typedef typename allocator_type::value_type      value_type;
  typedef typename allocator_type::pointer         pointer;
  typedef typename allocator_type::const_pointer   const_pointer;
  typedef typename allocator_type::reference       reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type       size_type;
  typedef typename allocator_type::difference_type difference_type;
  //���������
  typedef value_type*                              iterator;
  typedef const value_type*                        const_iterator;
  typedef mystl::reverse_iterator<iterator>        reverse_iterator;
  typedef mystl::reverse_iterator<const_iterator>  const_reverse_iterator;

  allocator_type get_allocator() { return data_allocator(); }//��

private:
  iterator begin_;//vector�ײ�λ��    
  iterator end_;//vectorβ��λ��      
  iterator cap_;//vector�������λ��    
public:
  //���졢���ơ���������ֵ
  //�����������춼ת��Ϊtry_init()
    vector() noexcept//��ʾ����ľ��Բ��ᷢ���쳣
  { try_init(); }

  explicit vector(size_type n)//��ʽ����
  { fill_init(n, value_type()); }

  vector(size_type n, const value_type& value)
  { fill_init(n, value); }
  //����������ת��Ϊrange_init()
  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>//ͨ��enable_if������ȡ��������
  vector(Iter first, Iter last)
  {
    MYSTL_DEBUG(!(last < first));//���˳��Ϊ�β�ֱ��ʹ��assert��
    range_init(first, last);
  }

  vector(const vector& rhs)
  {
    range_init(rhs.begin_, rhs.end_);
  }
  //ת�ƿ���
  vector(vector&& rhs) noexcept//�������Ѹ�ֵ�����ڳ�ʼ����
    :begin_(rhs.begin_),
    end_(rhs.end_),
    cap_(rhs.cap_)
  {
    rhs.begin_ = nullptr;
    rhs.end_ = nullptr;
    rhs.cap_ = nullptr;
  }
  //ͨ��ilist����
  vector(std::initializer_list<value_type> ilist)//������
  {
    range_init(ilist.begin(), ilist.end());
  }
  //���ظ�ֵ
  vector& operator=(const vector& rhs);
  vector& operator=(vector&& rhs) noexcept;//����ΪʲôҪ��noexcept

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
  //��������غ���
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

  //�ӿ���غ���
  bool      empty()    const noexcept//�п�
  { return begin_ == end_; }
  size_type size()     const noexcept//����
  { return static_cast<size_type>(end_ - begin_); }//static_cast������ptrdiff_t
  size_type max_size() const noexcept//�������ܲ�����vector�������
  { return static_cast<size_type>(-1) / sizeof(T); }
  size_type capacity() const noexcept//����
  { return static_cast<size_type>(cap_ - begin_); }
  void      reserve(size_type n);
  void      shrink_to_fit();
  //����[]��at����
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
  //Ϊʲôat����Ҫ�ӻط���,��[]����
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

  reference front()//ͷԪ��
  {
    MYSTL_DEBUG(!empty());
    return *begin_;
  }
  const_reference front() const
  {
    MYSTL_DEBUG(!empty());
    return *begin_;
  }
  reference back()//ĩԪ��
  {
    MYSTL_DEBUG(!empty());
    return *(end_ - 1);
  }
  const_reference back() const
  {
    MYSTL_DEBUG(!empty());
    return *(end_ - 1);
  }

  pointer       data()       noexcept { return begin_; }//��ֵ����
  const_pointer data() const noexcept { return begin_; }

  
  
  void assign(size_type n, const value_type& value)//��ԭ����0λ�ÿ�ʼ��ֵ�������ԭ��Ԫ��
  { fill_assign(n, value); }

  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>
  void assign(Iter first, Iter last)//β������
  {
    MYSTL_DEBUG(!(last < first));
    copy_assign(first, last, iterator_category(first));
  }
  
  void assign(std::initializer_list<value_type> il)//β����һ��
  { copy_assign(il.begin(), il.end(), mystl::forward_iterator_tag{}); }

  
  template <class... Args>
  iterator emplace(const_iterator pos, Args&& ...args);//�����

  template <class... Args>
  void emplace_back(Args&& ...args);//

  
  void push_back(const value_type& value);
  void push_back(value_type&& value)//ת��β��
  { emplace_back(mystl::move(value)); }

  void pop_back();

  
  iterator insert(const_iterator pos, const value_type& value);
  iterator insert(const_iterator pos, value_type&& value)
  { return emplace(pos, mystl::move(value)); }//ת�Ʋ���

  iterator insert(const_iterator pos, size_type n, const value_type& value)
  {
    MYSTL_DEBUG(pos >= begin() && pos <= end());
    return fill_insert(const_cast<iterator>(pos), n, value);
  }

  template <class Iter, typename std::enable_if<
    mystl::is_input_iterator<Iter>::value, int>::type = 0>
  void     insert(const_iterator pos, Iter first, Iter last)//��������
  {
    MYSTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
    copy_insert(const_cast<iterator>(pos), first, last);
  }

  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);
  void     clear() { erase(begin(), end()); }//���

  void     resize(size_type new_size) { return resize(new_size, value_type()); }//Ĭ������
  void     resize(size_type new_size, const value_type& value);

  void     reverse() { mystl::reverse(begin(), end()); }//��ת

  void     swap(vector& rhs) noexcept;//����

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
vector<T>& vector<T>::operator=(const vector& rhs)//���ظ�ֵ
{
  if (this != &rhs)
  {
    const auto len = rhs.size();
    if (len > capacity())//����������������ʱ�����ٽ���
    { 
      vector tmp(rhs.begin(), rhs.end());
      swap(tmp);
    }
    else if (size() >= len)//�����ռ�õ��ڴ����len
    {
      auto i = mystl::copy(rhs.begin(), rhs.end(), begin());//��������ַ���ص�������¸���
      data_allocator::destroy(i, end_);//�������Ŀռ�
      end_ = begin_ + len;
    }
    else//������������ռ�õĲ�������Ҫ����ռ�ÿ��еĿռ�
    { 
      mystl::copy(rhs.begin(), rhs.begin() + size(), begin_);//�����θ���
      mystl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);
      cap_ = end_ = begin_ + len;//������Ϊʲôcap_ҲҪ�任���ƺ�û�б�Ҫ��������һ�������û������
    }
  }
  return *this;
}
//ת�Ƹ�ֵ
template <class T>
vector<T>& vector<T>::operator=(vector&& rhs) noexcept
{
  destroy_and_recover(begin_, end_, cap_ - begin_);//Ҫ��ԭ���������ڴ������ͷ�
  begin_ = rhs.begin_;
  end_ = rhs.end_;
  cap_ = rhs.cap_;
  rhs.begin_ = nullptr;
  rhs.end_ = nullptr;
  rhs.cap_ = nullptr;
  return *this;
}

template <class T>
void vector<T>::reserve(size_type n)//������������
{//ֻ�����������������������������ִ��
  if (capacity() < n)
  {
    THROW_LENGTH_ERROR_IF(n > max_size(),
                          "n can not larger than max_size() in vector<T>::reserve(n)");
    const auto old_size = size();
    auto tmp = data_allocator::allocate(n);//���������ڴ�
    mystl::uninitialized_move(begin_, end_, tmp);//ת��Ԫ��
    data_allocator::deallocate(begin_, cap_ - begin_);//Ϊʲô������destroy_and_recover
    begin_ = tmp;
    end_ = tmp + old_size;
    cap_ = begin_ + n;
  }
}

template <class T>
void vector<T>::shrink_to_fit()//��������
{
  if (end_ < cap_)
  {
    reinsert(size());
  }
}

template <class T>
template <class ...Args>
typename vector<T>::iterator
vector<T>::emplace(const_iterator pos, Args&& ...args)//����һ��Ԫ�ز�����������ǳ�������
{
  MYSTL_DEBUG(pos >= begin() && pos <= end());
  iterator xpos = const_cast<iterator>(pos);//ȥ��const
  const size_type n = xpos - begin_;
  if (end_ != cap_ && xpos == end_)//��������������Ҳ����λ����ĩλ
  {
    data_allocator::construct(mystl::address_of(*end_), mystl::forward<Args>(args)...);
    ++end_;
  }
  else if (end_ != cap_)
  {
    auto new_end = end_;
    data_allocator::construct(mystl::address_of(*end_), *(end_ - 1));//��ת��ĩλ
    ++new_end;
    mystl::copy_backward(xpos, end_ - 1, end_);//�ٴӺ��渴��ǰ���
    *xpos = value_type(mystl::forward<Args>(args)...);//�����posλ��
    end_ = new_end;//Դ����û�и���end_
  }
  else
  {
    reallocate_emplace(xpos, mystl::forward<Args>(args)...);//����������
  }
  return begin() + n;//���ز���λ��
}

template <class T>
template <class ...Args>
void vector<T>::emplace_back(Args&& ...args)//emplace_back��push_back��ͬ
{
  if (end_ < cap_)
  {//emplace���ô���Ĳ���ԭ�ع���һ�����󣬶�push_back�������һ������
    data_allocator::construct(mystl::address_of(*end_), mystl::forward<Args>(args)...);
    ++end_;
  }
  else
  {
    reallocate_emplace(end_, mystl::forward<Args>(args)...);//��̬����
  }
}

template <class T>
void vector<T>::push_back(const value_type& value)//β�巨
{
  if (end_ != cap_)
  {
    data_allocator::construct(mystl::address_of(*end_), value);
    ++end_;
  }
  else
  {
    reallocate_insert(end_, value);//��̬����
  }
}

template <class T>
void vector<T>::pop_back()//βɾ��
{
  MYSTL_DEBUG(!empty());
  data_allocator::destroy(end_ - 1);
  --end_;
}

template <class T>
typename vector<T>::iterator
vector<T>::insert(const_iterator pos, const value_type& value)//ָ��λ�ò���
{//����ͬemplace
  MYSTL_DEBUG(pos >= begin() && pos <= end());
  iterator xpos = const_cast<iterator>(pos);
  const size_type n = pos - begin_;
  if (end_ != cap_ && xpos == end_)//���ã�β��
  {
    data_allocator::construct(mystl::address_of(*end_), value);
    ++end_;
  }
  else if (end_ != cap_)//�����ã��м����
  {//�ֶ���
    auto new_end = end_;
    data_allocator::construct(mystl::address_of(*end_), *(end_ - 1));
    ++new_end;
    auto value_copy = value;      mystl::copy_backward(xpos, end_ - 1, end_);
    *xpos = mystl::move(value_copy);
    end_ = new_end;
  }
  else
  {//��������
    reallocate_insert(xpos, value);
  }
  return begin_ + n;
}

template <class T>
typename vector<T>::iterator
vector<T>::erase(const_iterator pos)//����
{
  MYSTL_DEBUG(pos >= begin() && pos < end());
  iterator xpos = begin_ + (pos - begin());//Ҫ���ص�λ�ã�Ϊʲô��ֱ�ӵ���pos��
  mystl::move(xpos + 1, end_, xpos);//�ƶ�
  data_allocator::destroy(end_ - 1);
  --end_;
  return xpos;
}

template <class T>
typename vector<T>::iterator
vector<T>::erase(const_iterator first, const_iterator last)//��������
{
  MYSTL_DEBUG(first >= begin() && last <= end() && !(last < first));
  const auto n = first - begin();
  iterator r = begin_ + (first - begin());
  data_allocator::destroy(mystl::move(r + (last - first), end_, r), end_);//���ƶ�����,Ȼ�������ٶ��������
  end_ = end_ - (last - first);
  return begin_ + n;
}

template <class T>
void vector<T>::resize(size_type new_size, const value_type& value)//���¶����С
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
void vector<T>::swap(vector<T>& rhs) noexcept//����
{
  if (this != &rhs)//ͨ��������ַ��ʵ��
  {
    mystl::swap(begin_, rhs.begin_);
    mystl::swap(end_, rhs.end_);
    mystl::swap(cap_, rhs.cap_);
  }
}

/*****************************************************************************************/

template <class T>
void vector<T>::try_init() noexcept//Ĭ�Ϲ���
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
void vector<T>::init_space(size_type size, size_type cap)//ָ�������ʹ�С����ʼvector
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
fill_init(size_type n, const value_type& value)//n��val��ʼ��
{//�������16��size=cap��������16,�����16����Ԥ��
  const size_type init_size = mystl::max(static_cast<size_type>(16), n);//���ٷ���16��
  init_space(n, init_size);//�ȳ�ʼ���ռ䣬�ٸ�ֵ
  mystl::uninitialized_fill_n(begin_, n, value);
}

template <class T>
template <class Iter>
void vector<T>::
range_init(Iter first, Iter last)//��Χ��ʼ��
{//����ռ�
  const size_type init_size = mystl::max(static_cast<size_type>(last - first),
                                         static_cast<size_type>(16));
  init_space(static_cast<size_type>(last - first), init_size);
  mystl::uninitialized_copy(first, last, begin_);//����
}

template <class T>
void vector<T>::
destroy_and_recover(iterator first, iterator last, size_type n)//���ٺ��ͷ�
{
  data_allocator::destroy(first, last);//����
  data_allocator::deallocate(first, n);//�ͷ�
}

template <class T>
typename vector<T>::size_type 
vector<T>::
get_new_cap(size_type add_size)//��ȡ���ڴ��С
{
  const auto old_size = capacity();
  THROW_LENGTH_ERROR_IF(old_size > max_size() - add_size,
                        "vector<T>'s size too big");
  if (old_size > max_size() - old_size / 2)//�Ѿ�������ڴ��2/3
  {
    return old_size + add_size > max_size() - 16
      ? old_size + add_size : old_size + add_size + 16;//���������
  }
  const size_type new_size = old_size == 0
    ? mystl::max(add_size, static_cast<size_type>(16))//���ԭ��СΪ0��һ�����ٿ���16��
    : mystl::max(old_size + old_size / 2, old_size + add_size);//����һ����߸���
  return new_size;
}

template <class T>
void vector<T>::
fill_assign(size_type n, const value_type& value)//assgin�ľ���ʵ��
{//�����ԭ��vector��Ԫ��
  if (n > capacity())//�����������
  {
    vector tmp(n, value);
    swap(tmp);
  }
  else if (n > size())
  {//�ֶ����
    mystl::fill(begin(), end(), value);
    end_ = mystl::uninitialized_fill_n(end_, n - size(), value);
  }
  else
  {
    erase(mystl::fill_n(begin_, n, value), end_);//����䣬Ȼ����������
  }
}

template <class T>
template <class IIter>
void vector<T>::
copy_assign(IIter first, IIter last, input_iterator_tag)//�������䣬�������ԭ��Ԫ��
{
  auto cur = begin_;
  for (; first != last && cur != end_; ++first, ++cur)
  {
    *cur = *first;
  }
  if (first == last)//���ԭ�������
  {
    erase(cur, end_);//����ʣ���
  }
  else
  {
    insert(end_, first, last);//ԭ����С������ʣ�µ�
  }
}

template <class T>
template <class FIter>
void vector<T>::
copy_assign(FIter first, FIter last, forward_iterator_tag)//�������䣬�������ԭ��Ԫ��
{//�������������
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
reallocate_emplace(iterator pos, Args&& ...args)//end_<cap_��Ҫ���������ڴ�
{
  const auto new_size = get_new_cap(1);//��̬�����ڴ�
  auto new_begin = data_allocator::allocate(new_size);//���������ڴ�
  auto new_end = new_begin;
  try
  {//������ת��
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
  destroy_and_recover(begin_, end_, cap_ - begin_);//���ٺ��ͷ�ԭ����
  begin_ = new_begin;
  end_ = new_end;
  cap_ = new_begin + new_size;
}

template <class T>
void vector<T>::reallocate_insert(iterator pos, const value_type& value)
{//��reallocate_emplace������ͬ��������Ȼ���ڴ������һ���Ƕ���һ�������Ƕ������
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
fill_insert(iterator pos, size_type n, const value_type& value)//insert��ʵ�ʵ���
{
  if (n == 0)
    return pos;
  const size_type xpos = pos - begin_;
  const value_type value_copy = value;    
  if (static_cast<size_type>(cap_ - end_) >= n){//ʣ�µ���������
    const size_type after_elems = end_ - pos;
    auto old_end = end_;
    if (after_elems > n)//��β���λ�õ�β���ľ������n
    {
      mystl::uninitialized_copy(end_ - n, end_, end_);//���ƶ�һ����Ԫ�ص�û��ʹ�õ�һ�οռ�
      end_ += n;
      mystl::move_backward(pos, old_end - n, old_end);//���ƶ�ʣ�µ���Ҫ�ƶ���Ԫ��
      mystl::uninitialized_fill_n(pos, n, value_copy);//����
    }
    else
    {//��β���λ�õ�β���ľ���С��n
      end_ = mystl::uninitialized_fill_n(end_, n - after_elems, value_copy);//�Ȳ���һ����Ԫ��
      end_ = mystl::uninitialized_move(pos, old_end, end_);//���ƶ�ԭ�ռ���Ҫ�ƶ��ĵ������
      mystl::uninitialized_fill_n(pos, after_elems, value_copy);//����ʣ�µ�
    }
  }
  else//ʣ�µ�����������
  {     
    const auto new_size = get_new_cap(n);
    auto new_begin = data_allocator::allocate(new_size);
    auto new_end = new_begin;
    try
    {//�����临��
      new_end = mystl::uninitialized_move(begin_, pos, new_begin);
      new_end = mystl::uninitialized_fill_n(new_end, n, value);
      new_end = mystl::uninitialized_move(pos, end_, new_end);
    }
    catch (...)
    {
      destroy_and_recover(new_begin, new_end, new_size);
      throw;
    }
    data_allocator::deallocate(begin_, cap_ - begin_);//����ԭ����
    begin_ = new_begin;
    end_ = new_end;
    cap_ = begin_ + new_size;
  }
  return begin_ + xpos;
}

template <class T>
template <class IIter>
void vector<T>::
copy_insert(iterator pos, IIter first, IIter last)//���Ʋ���
{//ΪʲôҪ�ֿ��ƶ���ֱ���Ⱥ��Ʋ�����
  if (first == last)
    return;
  const auto n = mystl::distance(first, last);//�������
  if ((cap_ - end_) >= n){//��������
    const auto after_elems = end_ - pos;
    auto old_end = end_;
    if (after_elems > n)//ͬ����n��val
    {//��[pos,end)����Ϊ���Σ�[pos,pos+n),[pos+n,end)
      end_ = mystl::uninitialized_copy(end_ - n, end_, end_);//�ȸ���[pos+n,end)��end֮��,������end
      mystl::move_backward(pos, old_end - n, old_end);//����oldendΪ�յ��ƶ�[pos,pos+n)
      mystl::uninitialized_copy(first, last, pos);//����
    }
    else
    {//����mid��[first,last)���仮��Ϊ����[first,after),[after,last)
      auto mid = first;//����һ��mid������������
      mystl::advance(mid, after_elems);//�����iter��ʲô���ͣ�������davance������mid�����ô���
      end_ = mystl::uninitialized_copy(mid, last, end_);//���ƶ�[a,l)��end_֮��,������end_
      end_ = mystl::uninitialized_move(pos, old_end, end_);//���ƶ�[a,l)��end_֮�󣬸���end_
      mystl::uninitialized_copy(first, mid, pos);//�ƶ�[f,a)��pos_֮�󣬸���end_
    }
  }
  else{//�ڴ治�����ˣ���Ҫ���»�ȡ����
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
void vector<T>::reinsert(size_type size)//���²��룬shrink_to_fit��ʵ�ʵ���
{//���⣬�������Ĳ���������ԭsize�أ�
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
  data_allocator::deallocate(begin_, cap_ - begin_);//ԭ����
  begin_ = new_begin;
  end_ = begin_ + size;
  cap_ = begin_ + size;
}

/*****************************************************************************************/
//���������
template <class T>
bool operator==(const vector<T>& lhs, const vector<T>& rhs)
{//���ȱȶԴ�С
  return lhs.size() == rhs.size() &&//�����С��ȣ�ȷʵֻ��Ҫ����������������
    mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class T>
bool operator<(const vector<T>& lhs, const vector<T>& rhs)//����<
{
  return mystl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), lhs.end());
}

template <class T>
bool operator!=(const vector<T>& lhs, const vector<T>& rhs)//���أ�=
{
  return !(lhs == rhs);
}

template <class T>
bool operator>(const vector<T>& lhs, const vector<T>& rhs)//����<
{
  return rhs < lhs;
}

template <class T>
bool operator<=(const vector<T>& lhs, const vector<T>& rhs)//����<=
{
  return !(rhs < lhs);
}

template <class T>
bool operator>=(const vector<T>& lhs, const vector<T>& rhs)//����!=
{
  return !(lhs < rhs);
}

template <class T>
void swap(vector<T>& lhs, vector<T>& rhs)//����
{
  lhs.swap(rhs);
}

} #endif 
