

#ifndef __WHEEL_VECTOR
#define __WHEEL_VECTOR

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>

using std::ptrdiff_t;
using std::pair;
using std::copy_n;
using std::initializer_list;
using std::uninitialized_copy;
using std::uninitialized_fill;
using std::uninitialized_fill_n;
using std::move_backward;
using std::copy;
using std::copy_backward;
namespace wheel
{
	template <class T, class Allocator = std::allocator<T>>
	class vector
	{
	public:
		using value_type = T;
		using allocator_type = Allocator;
		using reference = T&;
		using const_reference = const reference;
		using iterator = T*;
		using const_iterator = const T*;
		using size_type = size_t;
		typedef typename allocator_type::difference_type difference_type;
		typedef typename allocator_type::pointer pointer;
		typedef typename allocator_type::const_pointer const_pointer;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


		iterator _allocate_and_fill(size_type n, const T& value);

		void _fill_and_initialize(size_type n, const T& value);

		vector() noexcept;
		explicit vector(const allocator_type&);
		explicit vector(size_type n);

		explicit vector(size_type n, const allocator_type&);
		vector(size_type n, const value_type& value);

		vector(const vector& x);

		vector(vector&& x) noexcept;

		vector(initializer_list<value_type> il);

		vector(initializer_list<value_type> il, const allocator_type& a);

		~vector();

		vector& operator=(const vector& x);
		vector& operator=(vector&& rhs) noexcept;
		vector& operator=(initializer_list<value_type> il);
		template <class InputIterator>
		void assign(InputIterator first, InputIterator last);
		void assign(size_type n, const value_type& u);
		void assign(initializer_list<value_type> il);

		// refinment
		iterator begin() noexcept { return _begin; }
		const_iterator begin() const noexcept { return static_cast<const_iterator>(_begin); }
		iterator end() noexcept { return _end; }
		const_iterator end() const noexcept { return static_cast<const_iterator>(_end); }

		reverse_iterator rbegin() noexcept;
		const_reverse_iterator rbegin() const noexcept;
		reverse_iterator rend() noexcept;
		const_reverse_iterator rend() const noexcept;

		const_iterator cbegin() const noexcept;
		const_iterator cend() const noexcept { _end; }
		const_reverse_iterator crbegin() const noexcept;
		const_reverse_iterator crend() const noexcept;

		size_type size() const noexcept { return static_cast<size_type>(end() - begin()); }

		size_type max_size() const noexcept { return _capacity; }

		size_type capacity() const noexcept { return _capacity; }

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
		void _check_capacity()
		{
			if (end() == _capacity)
				_reallocate();
		}
		void _reallocate()
		{
			// allocated space is not enough
			const size_type old_size = size();
			const size_type curr_size = old_size != 0 ? (old_size << 1) : 1;
			iterator curr_begin = alloc.allocate(curr_size);
			iterator curr_end = curr_begin;
			iterator elem = begin();
			for (size_t i = 0; i != size(); i++, ++curr_end, ++elem)
			{
				alloc.construct(curr_end, std::move(*elem));
			}
			_free();
			_begin = curr_begin;
			_end = curr_end;
			_capacity = _begin + curr_size;
		}

		void push_back(const value_type& x);
		void push_back(value_type&& x);

		template <class... Args>
		reference emplace_back(Args&&... args); // reference in C++17
		void pop_back();

		template <class... Args>
		iterator emplace(const_iterator position, Args&&... args);

		iterator insert(const_iterator position, const value_type& x);
		iterator insert(const_iterator position, value_type&& x);
		void insert(const_iterator position, size_type n, const value_type& x);
		template <class InputIterator>
		iterator insert(const_iterator position, InputIterator first, InputIterator last);
		iterator insert(const_iterator position, initializer_list<value_type> il);

		iterator erase(const_iterator position)
		{
			if (position + 1 != end())
				std::copy(position + 1, end(), position);
			--_end;
			alloc.destroy(_end);
			return position;
		}

		iterator erase(iterator first, iterator last)
		{
			difference_type n = last - first;
			difference_type old_size = _end - _begin;
			_end = copy_n(last, _end - last, first);
			for (auto it = end(); n > 0; n--, ++it)
			{
				alloc.destroy(it);
			}
		}

		void clear() noexcept { erase(begin(), end()); }

		void resize(size_type sz);
		void resize(size_type sz, const value_type& c);

	protected:
		void _free()
		{
			if (begin() != nullptr)
			{
				for (auto p = end(); p != begin();)
					alloc.destroy(--p);
				alloc.deallocate(_begin, _capacity - _begin);
			}
		}

		void insert_aux(const_iterator position, const value_type&);
		iterator insert_aux(const_iterator position, size_type n, const value_type& value);
		template <typename BidIterator>
		void insertFromIterator(const_iterator position,
		                        BidIterator first,
		                        BidIterator last,
		                        std::bidirectional_iterator_tag);
		template <typename InputIterator>
		void insertFromIterator(const_iterator position,
		                        InputIterator first,
		                        InputIterator last,
		                        std::input_iterator_tag);
		template <typename InputIterator>
		void insert_aux(const_iterator position, InputIterator first, InputIterator last, std::false_type);

	private:
		std::pair<iterator, iterator> alloc_copy(const_iterator begin, const_iterator end)
		{
			auto data = alloc.allocate(end - begin);
			return {data, uninitialized_copy(begin, end, data)};
		}
		iterator _begin;
		iterator _end;
		iterator _capacity;
		allocator_type alloc;
	}; // end_of_class vector

	// some auxiliary function

	template <typename T, typename Allocator>
	typename vector<T, Allocator>::iterator vector<T, Allocator>::_allocate_and_fill(size_type n, const T& value)
	{
		iterator result = alloc.allocate(n);
		std::uninitialized_fill_n(result, n, value);
		return result;
	}

	template <typename T, typename Allocator>
	void vector<T, Allocator>::_fill_and_initialize(size_type n, const T& value)
	{
		_begin = _allocate_and_fill(n, value);
		_end = _begin + n;
		_capacity = _end;
	}
	// a bundle of contructors
	template <typename T, typename Allocator>
	vector<T, Allocator>::vector() noexcept : _begin(nullptr), _end(nullptr), _capacity(nullptr){};

	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(size_type n)
	{
		_fill_and_initialize(n, T());
	}

	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(const allocator_type& allocator)
	    : _begin(nullptr), _end(nullptr), _capacity(nullptr), alloc(allocator)
	{
	}

	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(size_type n, const allocator_type& allocator) : alloc(allocator)
	{
		_fill_and_initialize(n, T());
	}

	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(size_type n, const value_type& value)
	{
		_fill_and_initialize(n, value);
	}

	template <typename T, typename Allocator>
	vector<T, Allocator>::vector(const vector& x)
	{
		auto data = alloc_copy(x.begin(), x.end());
		_begin = data.first;
		_end = _capacity = data.second;
	}

	template <typename T, typename Allocator>
	vector<T, Allocator>::vector(vector&& x) noexcept : _begin(x._begin), _end(x._end), _capacity(x._capacity)
	{
		x._begin = x._end = x._capacity = nullptr;
		alloc = x.alloc;
	}

	template <typename T, typename Allocator>
	vector<T, Allocator>::vector(initializer_list<value_type> il)
	{
		_begin = alloc.allocate(il.size());
		_end = std::copy(il.begin(), il.end(), begin());
		_capacity = _end;
	}

	template <typename T, typename Allocator>
	vector<T, Allocator>::vector(initializer_list<value_type> il, const allocator_type& a) : alloc(a)
	{
		_begin = a.allocate(il.size());
		_end = std::copy(il.begin(), il.end(), begin());
		_capacity = _end;
	}
	// destructor
	template <typename T, typename Allocator>
	vector<T, Allocator>::~vector()
	{
		for (auto it = begin(); it != end(); ++it)
			alloc.destroy(it);
		alloc.deallocate(begin(), _capacity - begin());
	}
	// assignment function
	template <typename T, typename Allocator>
	vector<T, Allocator>& vector<T, Allocator>::operator=(const vector& x)
	{
		auto data = alloc_copy(x.begin(), x.end());
		_free();
		_begin = data.first;
		_end = _capacity = data.second;
		return *this;
	}

	template <typename T, typename Allocator>
	vector<T, Allocator>& vector<T, Allocator>::operator=(vector&& rhs) noexcept
	{
		if (this != &rhs) // in case of rhs is self
		{
			_free();
			_begin = rhs._begin;
			_end = rhs._end;
			_capacity = rhs._capacity;
			rhs._begin = rhs._end = rhs._capacity = nullptr;
		}
		return *this;
	}

	template <typename T, typename Allocator>
	vector<T, Allocator>& vector<T, Allocator>::operator=(initializer_list<value_type> il)
	{
		_free();
		auto data = alloc_copy(il.begin(), il.end());
		_begin = data.first;
		_end = _capacity = data.second;
		return *this;
	}

	// push_back
	template <typename T, typename Allocator>
	void vector<T, Allocator>::push_back(const value_type& x)
	{
		_check_capacity();
		alloc.construct(end(), x);
		++_end;
	}

	template <typename T, typename Allocator>
	void vector<T, Allocator>::push_back(
	    value_type&& x) // refinment!!! we should use uninitialized_move to improve performance
	{
		_check_capacity();
		alloc.construct(end(), x);
		++_end;
	}
	template <typename T, typename Allocator>
	// auxiliary function
	void vector<T, Allocator>::insert_aux(const_iterator position, const value_type& value)
	{
		iterator desPosi = const_cast<iterator>(position);
		if (_end != _capacity) // have enough space
		{
			const T* pValue = &value;
			if (pValue >= desPosi && pValue < _end)
				++pValue; // beacuse if pvalue within range of des and end,their position will incremet

			::new (static_cast<void*>(_end)) value_type(*(_end - 1)); // placement new
			std::move_backward(desPosi, _end - 1, _end);
			*desPosi = *pValue; // if we don't change this value,the value is wrong
			++_end;
		}
		else // don't have enough space
		{
			//_reallocate();
			const auto nPosi = position - _begin;
			const auto old_size = _capacity - _begin;
			const auto curr_size = (old_size > 0) ? old_size * 2 : 1;
			const auto curr_begin = alloc.allocate(curr_size);
			::new (static_cast<void*>(curr_begin + nPosi)) value_type(value); // the value we want insert
			iterator curr_end = uninitialized_copy(_begin, desPosi, curr_begin);
			curr_end =
			    uninitialized_copy(desPosi, _end, ++curr_end); // incremet cuur_end because we already insert the value
			_free();                                           // deallocate old memory
			_begin = curr_begin;
			_end = curr_end;
			_capacity = _begin + curr_size;
		}
	}
	template <typename T, typename Allocator>
	// auxiliary function
	typename vector<T, Allocator>::iterator vector<T, Allocator>::insert_aux(const_iterator position,
	                                                                         size_type n,
	                                                                         const value_type& value)
	{
		iterator destPosition = const_cast<value_type*>(position);
		if (n <= size_type(_capacity - _end))
		{ // have enough memory
			size_type nExtra = static_cast<size_type>(_end - position);
			if (n < nExtra)
			{
				uninitialized_copy(_end - n, _end, _end);
				std::move_backward(destPosition, _end - n, _end);
				std::uninitialized_fill_n(destPosition, n, value);
			}
			else
			{
				uninitialized_fill(_end, _end + n - nExtra, value);
				uninitialized_copy(destPosition, _end, _end + (n - nExtra));
				uninitialized_fill(destPosition, _end, value);
			}
			_end += n;
		}
		else
		{ // memory don't enough ,we need _reallocate
			const size_type pre_size = _end - _begin;
			const size_type grow_size = (pre_size) < 0 ? 1 : pre_size * 2;
			const size_type curr_size = (grow_size > (pre_size + n)) ? grow_size : pre_size + n;
			const iterator curr_begin = alloc.allocate(curr_size);
			iterator curr_end = uninitialized_copy(_begin, destPosition, curr_begin);
			iterator tmpdes = curr_end;
			uninitialized_fill_n(curr_end, n, value);
			curr_end = uninitialized_copy(destPosition, _end, curr_end + n);
			_free();
			_begin = curr_begin;
			_end = curr_end;
			_capacity = curr_begin + curr_size;
			destPosition = tmpdes;
		}
		return destPosition;
	}
	template <typename T, typename Allocator>
	template <typename InputIterator>
	void vector<T, Allocator>::insert_aux(const_iterator position,
	                                      InputIterator first,
	                                      InputIterator last,
	                                      std::false_type)
	{
		using itercate = typename std::iterator_traits<InputIterator>::iterator_category;
		insertFromIterator(position, first, last, itercate());
	}

	template <typename T, typename Allocator>
	template <typename InputIterator>
	void vector<T, Allocator>::insertFromIterator(const_iterator position,
	                                              InputIterator first,
	                                              InputIterator last,
	                                              std::input_iterator_tag) // inputIterator only support incremet
	{
		for (; first != last; ++first, ++position)
		{
			position = insert(position, *first);
		}
	}
	template <typename T, typename Allocator>
	template <typename BidIterator>
	void vector<T, Allocator>::insertFromIterator(const_iterator position,
	                                              BidIterator first,
	                                              BidIterator last,
	                                              std::bidirectional_iterator_tag)

	{
		iterator desPosition = const_cast<iterator>(position);
		const size_type n = static_cast<size_type>((last - first));
		if (n <= static_cast<size_type>(_capacity - _end))
		{
			const size_type nExtra = (_end - desPosition);
			if (n < nExtra) //[first,last) are entrely within initilalized memory
			{
				uninitialized_copy(_end - n, _end, _end);
				move_backward(desPosition, _end - n, _end);
				copy(first, last, desPosition);
			}
			else
			{
				BidIterator tmp = first; // tmp
				std::advance(tmp, nExtra);
				uninitialized_copy(tmp, last, _end); // n-nExtra
				uninitialized_copy(desPosition, _end, _end + (n - nExtra));
				copy_backward(first, tmp, _end);
			}
			_end += n;
		}
		else
		{
			const size_type pre_size = _end - _begin;
			const size_type grow_size = (pre_size) < 0 ? 1 : pre_size * 2;
			const size_type curr_size = (grow_size > (pre_size + n)) ? grow_size : pre_size + n;
			const iterator curr_begin = alloc.allocate(curr_size);

			iterator curr_end = uninitialized_copy(_begin, desPosition, curr_begin);
			curr_end = uninitialized_copy(first, last, curr_end);
			curr_end = uninitialized_copy(desPosition, _end, curr_end);
			_free();
			_begin = curr_begin;
			_end = curr_end;
			_capacity = _begin + curr_size;
		}
	}

	template <typename T, typename Allocator>
	typename vector<T, Allocator>::iterator vector<T, Allocator>::insert(const_iterator position, const value_type& x)
	{
		const ptrdiff_t n = position - _begin;
		if ((_end == _capacity) || (position != _end))
			insert_aux(position, x);
		else
		{
			::new (static_cast<void*>(_end)) value_type(x);
			++_end;
		}
		return _begin + n;
	}

	template <typename T, typename Allocator>
	typename vector<T, Allocator>::iterator vector<T, Allocator>::insert(const_iterator position, value_type&& x)
	{
		// return emplace(position, std::move(value));//todp
	}

	template <typename T, typename Allocator>
	void vector<T, Allocator>::insert(const_iterator position, size_type n, const value_type& x)
	{

		std::cout << "insert happens";
		insert_aux(position, n, x);
	}
	template <typename T, typename Allocator>
	template <class InputIterator>
	typename vector<T, Allocator>::iterator vector<T, Allocator>::insert(const_iterator position,
	                                                                     InputIterator first,
	                                                                     InputIterator last)
	{
		insert_aux(position, first, last, std::false_type());
	}
	template <typename T, typename Allocator>
	typename vector<T, Allocator>::iterator vector<T, Allocator>::insert(const_iterator position,
	                                                                     initializer_list<value_type> il)
	{
		const difference_type n = position - _end;
		insert_aux(position, il.begin(), il.end(), std::false_type());
		return _begin + n;
	}
} // namespace wheel

#endif