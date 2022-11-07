// MIT License
//
// Copyright (c) 2020 Jean Amaro <jean.amaro@outlook.com.br> & Lucas Melchiori Pereira <lc.melchiori@gmail.com>
// Copyright (c) 2022 Jean Amaro <jean.amaro@outlook.com.br> & Lucas Melchiori Pereira <lc.melchiori@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _XABLAU_FILE_FILE_H_
#define _XABLAU_FILE_FILE_H_

#include <array>
#include <chrono>
#include <deque>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <functional>
#include <ios>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace xablau::file
{
	template < typename CharType, typename Traits = std::char_traits < CharType > >
	class File :
		public std::basic_fstream < CharType, Traits >
	{
	private:
		template < typename DataType, typename Container >
		class DerivedStack :
			public std::stack < DataType, Container >
		{
		public:
			const Container &getContainer()
			{
				return this->c;
			}

			explicit DerivedStack(const std::stack < DataType, Container > &stack) :
				std::stack < DataType, Container > (stack)
			{
			}
		};

		template < typename DataType, typename Container >
		class DerivedQueue :
			public std::queue < DataType, Container >
		{
		public:
			const Container &getContainer()
			{
				return this->c;
			}

			explicit DerivedQueue(const std::queue < DataType, Container > &stack) :
				std::queue < DataType, Container > (stack)
			{
			}
		};

		template < typename DataType, typename Container, typename Compare >
		class DerivedPriorityQueue :
			public std::priority_queue < DataType, Container, Compare >
		{
		public:
			const Container &getContainer()
			{
				return this->c;
			}

			explicit DerivedPriorityQueue(const std::priority_queue < DataType, Container, Compare > &priorityQueue) :
				std::priority_queue < DataType, Container, Compare > (priorityQueue)
			{
			}
		};

		static constexpr auto _offsetUntil =
			[] (const typename File::char_type character) -> bool
			{
				return
					character != '\n' &&
					character != '\r' &&
					character != '\0' &&
					character != std::char_traits < typename File::char_type > ::eof();
			};

	protected:
		std::filesystem::path _fileName;

	public:
		using std::basic_fstream < CharType, Traits > ::read;
		using std::basic_fstream < CharType, Traits > ::write;

		static bool exists(const std::filesystem::path &file)
		{
			return std::filesystem::exists(file);
		}

		std::streamoff offsetUntil(
			const std::function < bool(const typename File::char_type) > &expression = File < CharType, Traits > ::_offsetUntil)
		{
			std::streamoff offset{};

			while (true)
			{
				typename File::char_type character{};

				this->get(character);

				if (!this->good())
				{
					break;
				}

				if (expression(character))
				{
					this->unget();

					break;
				}

				offset += sizeof(typename File::char_type);
			}

			return offset;
		}

		template < typename DataType >
		requires (sizeof(CharType) == 1)
		File &read(DataType &data)
		{
			this->read(reinterpret_cast < typename File::char_type * > (&data), sizeof(DataType));

			return *this;
		}

		File &read(
			std::basic_string < typename File::char_type > &string,
			const std::function < bool(const CharType) > &delimiter =
			[] (const CharType character) -> bool
			{
				return
					character == '\n' ||
					character == '\r' ||
					character == '\0' ||
					character == std::char_traits < CharType > ::eof();
			})
		{
			while (true)
			{
				typename File::char_type character{};

				this->get(character);

				if (!this->good() || character == typename File::char_type() || delimiter(character))
				{
					break;
				}

				string.push_back(character);
			}

			return *this;
		}

		template < typename DataType, size_t Size >
		File &read(std::array < DataType, Size > &array)
		{
			for (size_t i = 0; i < Size; i++)
			{
				this->read(array[i]);
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &read(std::vector < DataType, Allocator > &vector)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			vector.reserve(quantityOfElements + vector.size());

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				vector.emplace_back(std::move(data));
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &read(std::deque < DataType, Allocator > &deque)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				deque.emplace_back(std::move(data));
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &read(std::forward_list < DataType, Allocator > &forwardList)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			std::vector < DataType, Allocator > stackContainer;

			stackContainer.reserve(quantityOfElements);

			std::stack < DataType, std::vector < DataType, Allocator > > stack(std::move(stackContainer));

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				stack.emplace(std::move(data));
			}

			while (!stack.empty())
			{
				forwardList.emplace_front(std::move(stack.top()));

				stack.pop();
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &read(std::list < DataType, Allocator > &list)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				list.emplace_back(std::move(data));
			}

			return *this;
		}

		template < typename DataType1, typename DataType2 >
		File &read(std::pair < DataType1, DataType2 > &pair)
		{
			this->read(pair.first);
			this->read(pair.second);

			return *this;
		}

		template < typename DataType, typename LessThan, typename Allocator >
		File &read(std::set < DataType, LessThan, Allocator > &set)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				set.emplace(std::move(data));
			}

			return *this;
		}

		template < typename DataType, typename LessThan, typename Allocator >
		File &read(std::multiset < DataType, LessThan, Allocator > &multiset)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				multiset.emplace(std::move(data));
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename LessThan, typename Allocator >
		File &read(std::map < DataType1, DataType2, LessThan, Allocator > &map)
		{
			size_t quantityOfPairs{};

			this->read(quantityOfPairs);

			for (size_t i = 0; i < quantityOfPairs; i++)
			{
				std::pair < DataType1, DataType2 > pair;

				this->read(pair);

				map.emplace(std::move(pair));
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename LessThan, typename Allocator >
		File &read(std::multimap < DataType1, DataType2, LessThan, Allocator > &multimap)
		{
			size_t quantityOfPairs{};

			this->read(quantityOfPairs);

			for (size_t i = 0; i < quantityOfPairs; i++)
			{
				std::pair < DataType1, DataType2 > pair;

				this->read(pair);

				multimap.emplace(std::move(pair));
			}

			return *this;
		}

		template < typename DataType, typename Hasher, typename EqualTo, typename Allocator >
		File &read(std::unordered_set < DataType, Hasher, EqualTo, Allocator > &unorderedSet)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			unorderedSet.reserve(unorderedSet.size() + quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType pair{};

				this->read(pair);

				unorderedSet.emplace(std::move(pair));
			}

			return *this;
		}

		template < typename DataType, typename Hasher, typename EqualTo, typename Allocator >
		File &read(std::unordered_multiset < DataType, Hasher, EqualTo, Allocator > &unorderedMultiset)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			unorderedMultiset.reserve(unorderedMultiset.size() + quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType dado{};

				this->read(dado);

				unorderedMultiset.emplace(std::move(dado));
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename Hasher, typename EqualTo, typename Allocator >
		File &read(std::unordered_map < DataType1, DataType2, Hasher, EqualTo, Allocator > &unorderedMap)
		{
			size_t quantityOfPairs{};

			this->read(quantityOfPairs);

			unorderedMap.reserve(unorderedMap.size() + quantityOfPairs);

			for (size_t i = 0; i < quantityOfPairs; i++)
			{
				std::pair < DataType1, DataType2 > pair;

				this->read(pair);

				unorderedMap.emplace(std::move(pair));
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename Hasher, typename EqualTo, typename Allocator >
		File &read(std::unordered_multimap < DataType1, DataType2, Hasher, EqualTo, Allocator > &unorderedMultimap)
		{
			size_t quantityOfPairs{};

			this->read(quantityOfPairs);

			for (size_t i = 0; i < quantityOfPairs; i++)
			{
				std::pair < DataType1, DataType2 > pair;

				this->read(pair);

				unorderedMultimap.emplace(pair);
			}

			return *this;
		}

		template < typename DataType, typename Container >
		File &read(std::stack < DataType, Container > &stack)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				stack.emplace(std::move(data));
			}

			return *this;
		}

		template < typename DataType, typename Container >
		File &read(std::queue < DataType, Container > &queue)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				queue.emplace(std::move(data));
			}

			return *this;
		}

		template < typename DataType, typename Container, typename Compare >
		File &read(std::priority_queue < DataType, Container, Compare > &priorityQueue)
		{
			size_t quantityOfElements{};

			this->read(quantityOfElements);

			for (size_t i = 0; i < quantityOfElements; i++)
			{
				DataType data{};

				this->read(data);

				priorityQueue.emplace(std::move(data));
			}

			return *this;
		}

		template < typename Rep, typename Period >
		File &read(std::chrono::duration < Rep, Period > &duration)
		{
			Rep _duration{};

			this->read(_duration);

			duration = std::chrono::duration < Rep, Period > (_duration);

			return *this;
		}

		template < typename TipoDado >
		requires (sizeof(CharType) == 1)
		File &write(const TipoDado &data)
		{
			this->write(reinterpret_cast < const typename File::char_type * > (&data), sizeof(TipoDado));

			return *this;
		}

		template < bool WriteNullCharacter >
		File &write(const std::basic_string < typename File::char_type > &string)
		{
			for (const auto character : string)
			{
				this->put(character);
			}

			if constexpr (WriteNullCharacter)
			{
				this->put(typename File::char_type());
			}

			return *this;
		}

		template < typename DataType, size_t Size >
		File &write(const std::array < DataType, Size > &array)
		{
			for (const auto &data : array)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &write(const std::vector < DataType, Allocator > &vector)
		{
			this->write(vector.size());

			for (const auto &data : vector)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &write(const std::deque < DataType, Allocator > &deque)
		{
			this->write(deque.size());

			for (const auto &data : deque)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &write(const std::forward_list < DataType, Allocator > &forwardList)
		{
			this->write(std::distance(forwardList.cbegin(), forwardList.cend()));

			for (const auto &data : forwardList)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename Allocator >
		File &write(const std::list < DataType, Allocator > &list)
		{
			this->write(list.size());

			for (const auto &data : list)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType1, typename DataType2 >
		File &write(const std::pair < DataType1, DataType2 > &pair)
		{
			this->write(pair.first);
			this->write(pair.second);

			return *this;
		}

		template < typename DataType, typename LessThan, typename Allocator >
		File &write(const std::set < DataType, LessThan, Allocator > &set)
		{
			this->write(set.size());

			for (const auto &data : set)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename LessThan, typename Allocator >
		File &write(const std::multiset < DataType, LessThan, Allocator > &multiset)
		{
			this->write(multiset.size());

			for (const auto &data : multiset)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename LessThan, typename Allocator >
		File &write(const std::map < DataType1, DataType2, LessThan, Allocator > &mapa)
		{
			this->write(mapa.size());

			for (const auto &pair : mapa)
			{
				this->write(pair);
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename LessThan, typename Allocator >
		File &write(const std::multimap < DataType1, DataType2, LessThan, Allocator > &multimap)
		{
			this->write(multimap.size());

			for (const auto &pair : multimap)
			{
				this->write(pair);
			}

			return *this;
		}

		template < typename DataType, typename Hasher, typename EqualTo, typename Allocator >
		File &write(const std::unordered_set < DataType, Hasher, EqualTo, Allocator > &unorderedSet)
		{
			this->write(unorderedSet.size());

			for (const DataType &data : unorderedSet)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename Hasher, typename EqualTo, typename Allocator >
		File &write(const std::unordered_multiset < DataType, Hasher, EqualTo, Allocator > &unorderedMultiset)
		{
			this->write(unorderedMultiset.size());

			for (const DataType &data : unorderedMultiset)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename Hasher, typename EqualTo, typename Allocator >
		File &write(const std::unordered_map < DataType1, DataType2, Hasher, EqualTo, Allocator > &unorderedMap)
		{
			this->write(unorderedMap.size());

			for (const auto &pair : unorderedMap)
			{
				this->write(pair);
			}

			return *this;
		}

		template < typename DataType1, typename DataType2, typename Hasher, typename EqualTo, typename Allocator >
		File &write(const std::unordered_multimap < DataType1, DataType2, Hasher, EqualTo, Allocator > &unorderedMultimap)
		{
			this->write(unorderedMultimap.size());

			for (const auto &pair : unorderedMultimap)
			{
				this->write(pair);
			}

			return *this;
		}

		template < typename DataType, typename Container >
		File &write(const std::stack < DataType, Container > &stack)
		{
			DerivedStack < DataType, Container > _stack(stack);
			const Container &stackContainer = _stack.getContainer();

			this->write(stack.size());

			for (const auto &data : stackContainer)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename Container >
		File &write(const std::queue < DataType, Container > &queue)
		{
			DerivedQueue < DataType, Container > _queue(queue);
			const Container &queueContainer = _queue.getContainer();

			this->write(queue.size());

			for (const auto &data : queueContainer)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename DataType, typename Container, typename Compare >
		File &write(const std::priority_queue < DataType, Container, Compare > &priorityQueue)
		{
			DerivedPriorityQueue < DataType, Container, Compare > _priorityQueue(priorityQueue);
			const Container &queueContainer = _priorityQueue.getContainer();

			this->write(priorityQueue.size());

			for (const auto &data : queueContainer)
			{
				this->write(data);
			}

			return *this;
		}

		template < typename Rep, typename Period >
		File &write(const std::chrono::duration < Rep, Period > &duration)
		{
			this->write(duration.count());

			return *this;
		}

		const std::filesystem::path &getFileName() const noexcept
		{
			return this->_fileName;
		}

		std::uintmax_t getFileSize() const
		{
			return std::filesystem::file_size(this->_fileName);
		}

		File() = default;

		explicit File(
			const char *fileName,
			std::ios_base::openmode openMode = std::ios_base::in | std::ios_base::out) :

			std::basic_fstream < CharType > (fileName, openMode),

			_fileName(fileName)
		{
		}

		explicit File(
			const std::filesystem::path::value_type *fileName,
			std::ios_base::openmode openMode = std::ios_base::in | std::ios_base::out) :

			std::basic_fstream < CharType > (fileName, openMode),

			_fileName(fileName)
		{
		}

		explicit File(
			const std::string &fileName,
			std::ios_base::openmode openMode = std::ios_base::in | std::ios_base::out) :

			std::basic_fstream < CharType > (fileName, openMode),

			_fileName(fileName)
		{
		}

		explicit File(
			const std::filesystem::path &fileName,
			std::ios_base::openmode openMode = std::ios_base::in | std::ios_base::out) :

			std::basic_fstream < CharType > (fileName, openMode),

			_fileName(fileName)
		{
		}

		File(const File &) = delete;
		File(File &&) noexcept = default;
		File &operator=(const File &) = delete;
		File &operator=(File &&) noexcept = default;
		~File() noexcept override = default;
	};
}

#endif