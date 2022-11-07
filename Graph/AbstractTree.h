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

#ifndef _XABLAU_GRAPH_ABSTRACT_TREE_H_
#define _XABLAU_GRAPH_ABSTRACT_TREE_H_

#include <vector>

namespace xablau::graph
{
	template < typename DataType >
	class AbstractTree
	{
	public:
		virtual bool empty() const noexcept = 0;

		virtual size_t size() const noexcept = 0;

		virtual size_t quantityOfLevels() const = 0;

		virtual std::vector < size_t > quantityOfNodesByLevel() const = 0;

		virtual bool binary() const = 0;

		virtual void clear() noexcept = 0;

		virtual size_t count(const DataType &value) const = 0;

		virtual bool contains(const DataType &value) const = 0;

		AbstractTree() = default;
		AbstractTree(const AbstractTree < DataType > &) = default;
		AbstractTree(AbstractTree < DataType > &&) noexcept = default;
		AbstractTree &operator=(const AbstractTree < DataType > &) = default;
		AbstractTree &operator=(AbstractTree < DataType > &&) noexcept = default;

		virtual ~AbstractTree() noexcept = default;
	};
}

#endif