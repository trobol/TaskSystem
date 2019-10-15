#pragma once
#include <array>

//src https://github.com/palotasb/static_vector/blob/master/include/palotasb/static_vector.hpp
template <typename T, std::size_t Capacity>
struct StaticVector {

	// Value type equal to T
	using value_type = T;
	// std::size_t without including a header just for this name
	using size_type = std::size_t;
	// std::ptrdiff_t without including a header just for this name
	using difference_type = std::ptrdiff_t;
	// Reference type is a regular reference, not a proxy
	using reference = value_type &;
	using const_reference = const value_type &;
	// Pointer is a regular pointer
	using pointer = value_type *;
	using const_pointer = const value_type*;
	// Iterator is a regular pointer
	using iterator = pointer;
	using const_iterator = const_pointer;
	// Reverse iterator is what the STL provides for reverse iterating pointers
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	// The static capacity of the static_vector
	static const size_type static_capacity = Capacity;



	StaticVector() noexcept : m_size(0) {}

	StaticVector(size_type count, const_reference value) //
		noexcept(noexcept(value_type(value)))
		: m_size(count) {
		std::uninitialized_fill(begin(), end(), value);
	}


	void push_back(const value_type& value) {
		if (full())
			throw std::out_of_range("size()");
		new (storage_end()) value_type(value);
		m_size++;
	}
	void push_back(value_type&& value) {
		if (full())
			throw std::out_of_range("size()");
		new (storage_end()) value_type(std::move(value));
		m_size++;
	}

private:
	// The array providing the inline storage for the elements.
	std::array<storage_type, static_capacity> m_data = {};
	// The current occupied size of the static_vector
	size_type m_size = 0;

	// Get data by index, used for convenience instead of (*this)[index]
	// Note that as opposed to data(), these return a `reference`, not `pointer`
	reference data(size_t index) noexcept {
		return *reinterpret_cast<pointer>(&m_data[index]);
	}

	const_reference data(size_t index) const noexcept {
		return *reinterpret_cast<const_pointer>(&m_data[index]);
	}
};