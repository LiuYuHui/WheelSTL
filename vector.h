

#ifndef __WHEEL_VECTOR
#define __WHEEL_VECTOR

#include <memory>
#include <algorithm>
#include <iterator>
#include <initializer_list>
#include <utility>

using std::pair;
using std::copy_n;
using std::initializer_list;
using std::uninitialized_copy;
// using std::iterator_traits;
namespace wheel {
template <class T, class Allocator = std::allocator<T>>
class vector {
 public:
  typedef T value_type;
  typedef Allocator allocator_type;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  vector() noexcept : _begin(nullptr), _end(nullptr), end_of_storage(nullptr){};

  // explicit vector(const alloc&);
  iterator _allocate_and_fill(size_type n, const T& value) {
    iterator result = alloc.allocate(n);
    std::uninitialized_fill_n(result, n, value);
    return result;
  }

  void _fill_and_initialize(size_type n, const T& value) {
    _begin = _allocate_and_fill(n, value);
    _end = _begin + n;
    end_of_storage = _end;
  }

  explicit vector(size_type n) { _fill_and_initialize(n, T()); }

  explicit vector(size_type n, const allocator_type&) {
    _fill_and_initialize(n, T());
  }
  vector(size_type n, const value_type& value) {
    _fill_and_initialize(n, value);
  }

  vector(const vector& x) {
    auto data = alloc_copy(x.begin(), x.end());
    _begin = data.first;
    _end = end_of_storage = data.second;
  }

  vector(vector&& x) noexcept : _begin(x._begin),
                                _end(x._end),
                                end_of_storage(x.end_of_storage) {
    x._begin = x._end = x.end_of_storage = nullptr;
  }

  vector(initializer_list<value_type> il) {
    _begin = alloc.allocate(il.size());
    _end = std::copy(il.begin(), il.end(), _begin());
    end_of_storage = _end;
  }

  vector(initializer_list<value_type> il, const allocator_type& a) {
    _begin = a.allocate(il.size());
    _end = std::copy(il.begin(), il.end(), begin());
    end_of_storage = _end;
  }

  ~vector() {
    for (auto it = begin(); it != end(); ++it) alloc.destroy(it);
    alloc.deallocate(begin(), end_of_storage - begin());
  }
  vector& operator=(const vector& x);
  // vector& operator=(vector&& x) noexcept(
  // alloc::propagate_on_container_move_assignment::value ||
  // alloc::is_always_equal::value);  // C++17
  // vector& operator=(initializer_list<value_type> il);
  template <class InputIterator>
  void assign(InputIterator first, InputIterator last);
  void assign(size_type n, const value_type& u);
  void assign(initializer_list<value_type> il);

  iterator begin() noexcept { return _begin; }
  const_iterator begin() const noexcept {
    return static_cast<const_iterator>(_begin);
  }
  iterator end() noexcept { return _end; }
  const_iterator end() const noexcept {
    return static_cast<const_iterator>(_end);
  }

  reverse_iterator rbegin() noexcept;
  const_reverse_iterator rbegin() const noexcept;
  reverse_iterator rend() noexcept;
  const_reverse_iterator rend() const noexcept;

  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;
  const_reverse_iterator crbegin() const noexcept;
  const_reverse_iterator crend() const noexcept;

  size_type size() const noexcept {
    return static_cast<size_type>(end() - begin());
  }

  size_type max_size() const noexcept { return end_of_storage; }

  size_type capacity() const noexcept { return end_of_storage; }

  bool empty() const noexcept { return size() == 0; }

  void reserve(size_type n);
  void shrink_to_fit() noexcept;

  reference operator[](size_type n) { return *(begin() + n); }
  const_reference operator[](size_type n) const;
  reference at(size_type n);
  const_reference at(size_type n) const;

  reference front() { return *(begin()); }
  const_reference front() const;
  reference back() { return *(end() - 1); }
  const_reference back() const;
  void _check_capacity() {
    if (end() == end_of_storage) _reallocate();
  }
  void _reallocate() {
    {  // allocated space is not enough
      const size_type old_size = size();
      const size_type curr_size = old_size != 0 ? (old_size << 1) : 1;
      iterator curr_begin = alloc.allocate(curr_size);
      iterator curr_end = curr_begin;
      iterator elem = begin();
      for (size_t i = 0; i != size(); i++, ++curr_end, ++elem) {
        alloc.construct(curr_end, std::move(*elem));
      }
      for (auto p = end(); p != begin();) alloc.destroy(--p);
      alloc.deallocate(_begin, end_of_storage - _begin);
      _begin = curr_begin;
      _end = curr_end;
      end_of_storage = _begin + curr_size;
    }
  }

  void push_back(const value_type& x) {
    _check_capacity();
    alloc.construct(end(), x);
    ++_end;
  }
  void push_back(value_type&& x) {
    _check_capacity();
    alloc.construct(end(), x);
    ++_end;
  }

  template <class... Args>
  reference emplace_back(Args&&... args);  // reference in C++17
  void pop_back();

  template <class... Args>
  iterator emplace(const_iterator position, Args&&... args);
  iterator insert(const_iterator position, const value_type& x);
  iterator insert(const_iterator position, value_type&& x);
  iterator insert(const_iterator position, size_type n, const value_type& x);
  template <class InputIterator>
  iterator insert(const_iterator position, InputIterator first,
                  InputIterator last);
  iterator insert(const_iterator position, initializer_list<value_type> il);

  iterator erase(const_iterator position) {
    if (position + 1 != end()) std::copy(position + 1, end(), position);
    --_end;
    alloc.destroy(_end);
    return position;
  }

  iterator erase(iterator first, iterator last) {
    difference_type n = last - first;
    // iterator b = first, e = last;
    difference_type old_size = _end - _begin;
    _end = copy_n(last, _end - last, first);
    // _begin = _end - old_size;
    for (auto it = end(); n > 0; n--, ++it) {
      alloc.destroy(it);
    }
  }

  void clear() noexcept { erase(begin(), end()); }

  void resize(size_type sz);
  void resize(size_type sz, const value_type& c);

 private:
  std::pair<iterator, iterator> alloc_copy(const_iterator begin,
                                           const_iterator end) {
    auto data = alloc.allocate(end - begin);
    return {data, uninitialized_copy(begin, end, data)};
  }
  iterator _begin;
  iterator _end;
  iterator end_of_storage;
  allocator_type alloc;
};
}

#endif