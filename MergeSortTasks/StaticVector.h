
//src: https://raw.githubusercontent.com/Manu343726/raytracer/f028409d7c69d368ed5efddeae5b4eb5f51a1781/include/raytracer/StaticVector.hpp
#include <vector>
#include <type_traits>
#include <memory>
#include <utility>

	template<typename T, typename Allocator = std::allocator<T>>
	class StaticVector
	{
		using storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
		using vector  = std::vector<storage, Allocator>;
		using vectoriterator = typename vector::iterator;
		using Constvectoriterator = typename vector::const_iterator;

	public:
		explicit StaticVector(std::size_t maxSize) :
			_storage{ maxSize },
			_size{ 0 }
		{}

		StaticVector(const StaticVector&) = delete;
		StaticVector(StaticVector&&) = delete;
		StaticVector& operator=(const StaticVector&) = delete;
		StaticVector& operator=(StaticVector&&) = delete;

		std::size_t size() const
		{
			return _size;
		}

		template<typename... Args>
		bool emplace_back(Args&& ... args)
		{
			if (_size < _storage.size())
			{
				auto* storage = &_storage[_size++];
				new(storage) T{ std::forward<Args>(args)... };

				return true;
			}
			else
			{
				return false;
			}
		}

		template<typename ValueType, typename StorageIterator>
		class iterator_t
		{
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = ValueType;
			using reference_type = std::add_lvalue_reference_t<ValueType>;
			using pointer_type = std::add_pointer_t<ValueType>;

			explicit iterator_t(StorageIterator storageiterator) :
				_storageiterator{ storageiterator }
			{}

			reference_type operator*() const
			{
				return reinterpret_cast<reference_type>(*_storageiterator);
			}

			iterator_t& operator++()
			{
				++_storageiterator;
				return *this;
			}

			iterator_t operator++(int)
			{
				return { _storageiterator++ };
			}

			iterator_t& operator+=(std::ptrdiff_t distance)
			{
				_storageiterator += distance;
				return *this;
			}

			iterator_t& operator--()
			{
				--_storageiterator;
				return *this;
			}

			iterator_t operator--(int)
			{
				return { _storageiterator-- };
			}

			iterator_t& operator-=(std::ptrdiff_t distance)
			{
				_storageiterator -= distance;
				return *this;
			}

			friend bool operator==(iterator_t lhs, iterator_t rhs)
			{
				return lhs._storageiterator == rhs._storageiterator;
			}

			friend bool operator!=(iterator_t lhs, iterator_t rhs)
			{
				return !(lhs == rhs);
			}

			friend iterator_t operator+(iterator_t it, std::ptrdiff_t distance)
			{
				return it += distance;
			}

			friend iterator_t operator-(iterator_t it, std::ptrdiff_t distance)
			{
				return it -= distance;
			}

		private:
			StorageIterator _storageiterator;
		};

		using iterator = iterator_t<T, vectoriterator>;
		using const_iterator = iterator_t<const T, Constvectoriterator>;

		iterator begin()
		{
			return iterator{ _storage.begin() };
		}

		const_iterator begin() const
		{
			return const_iterator{ _storage.begin() };
		}

		iterator end()
		{
			return iterator{ _storage.end() };
		}

		const_iterator end() const
		{
			return const_iterator{ _storage.end() };
		}

		const_iterator cbegin() const
		{
			return const_iterator{ _storage.cbegin() };
		}

		const_iterator cend() const
		{
			return const_iterator{ _storage.cend() };
		}

		const T& operator[](std::size_t i) const
		{
			return *reinterpret_cast<const T*>(&_storage[i]);
		}

		T& operator[](std::size_t i)
		{
			return *reinterpret_cast<T*>(&_storage[i]);
		}

		~StaticVector()
		{
			for (std::size_t i = 0; i < _size; ++i)
			{
				reinterpret_cast<T*>(&_storage[i])->~T();
			}
		}

	private:
		vector _storage;
		std::size_t _size;
	};

