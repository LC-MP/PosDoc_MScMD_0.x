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

#ifndef _XABLAU_GRAPH_NODE_H_
#define _XABLAU_GRAPH_NODE_H_

#include <compare>
#include <concepts>

namespace xablau::graph
{
	template < typename DataType >
		// TODO: Remove this
		// requires std::totally_ordered < DataType >
	class Node
	{
	public:
		DataType value = DataType();

		auto operator<=>(const Node < DataType > &) const = default;

		Node < DataType > &operator=(const Node < DataType > &) = default;

		Node < DataType > &operator=(Node < DataType > &&) noexcept = default;

		Node() = default;

		explicit Node(const DataType &value) :
			value(value)
		{
		}

		explicit Node(DataType &&value) noexcept :
			value(std::move(value))
		{
		}

		Node(const Node < DataType > &) = default;

		Node(Node < DataType > &&) noexcept = default;

		virtual ~Node() noexcept = default;
	};
}

namespace std
{
	template < typename DataType >
	struct hash < xablau::graph::Node < DataType > >
	{
		size_t operator()(const xablau::graph::Node < DataType > &node) const
		{
			return std::hash < DataType > ()(node.value);
		}
	};
}

#endif