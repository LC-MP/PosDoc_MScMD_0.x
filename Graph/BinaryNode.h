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

#ifndef _XABLAU_GRAPH_BINARY_NODE_H_
#define _XABLAU_GRAPH_BINARY_NODE_H_

#include <functional>
#include <optional>

#include "BinaryNodeParent.h"
#include "Node.h"

namespace xablau::graph
{
	template < typename DataType, bool HasParent >
	class BinaryNode :
		public Node < DataType >,
		public BinaryNodeParent < BinaryNode < DataType, HasParent >, HasParent >
	{
	public:
		virtual BinaryNode < DataType, HasParent > &insertLeftChild(const DataType &value) = 0;

		virtual BinaryNode < DataType, HasParent > &insertRightChild(const DataType &value) = 0;

		virtual BinaryNode < DataType, HasParent > &insertLeftChild(DataType &&value) = 0;

		virtual BinaryNode < DataType, HasParent > &insertRightChild(DataType &&value) = 0;

		virtual BinaryNode < DataType, HasParent > &insertLeftChild(const BinaryNode < DataType, HasParent > &child) = 0;

		virtual BinaryNode < DataType, HasParent > &insertRightChild(const BinaryNode < DataType, HasParent > &child) = 0;

		virtual void eraseLeftChild() = 0;

		virtual void eraseRightChild() = 0;

		virtual size_t size() const noexcept = 0;

		virtual void clear() noexcept = 0;

		virtual bool hasLeftChild() const = 0;

		virtual bool hasRightChild() const = 0;

		virtual std::optional < std::reference_wrapper < const BinaryNode < DataType, HasParent > > > getLeftChild() const = 0;

		virtual std::optional < std::reference_wrapper < const BinaryNode < DataType, HasParent > > > getRightChild() const = 0;

		virtual std::optional < std::reference_wrapper < BinaryNode < DataType, HasParent > > > getLeftChild() = 0;

		virtual std::optional < std::reference_wrapper < BinaryNode < DataType, HasParent > > > getRightChild() = 0;

		virtual std::shared_ptr < BinaryNode < DataType, HasParent > > extractLeftChild() = 0;

		virtual std::shared_ptr < BinaryNode < DataType, HasParent > > extractRightChild() = 0;

		BinaryNode < DataType, HasParent > &operator=(const BinaryNode < DataType, HasParent > &) = default;

		BinaryNode < DataType, HasParent > &operator=(BinaryNode < DataType, HasParent > &&) noexcept = default;

		BinaryNode() = default;

		explicit BinaryNode(const DataType &data) :
			Node < DataType > (data)
		{
		}

		explicit BinaryNode(DataType &&data) noexcept :
			Node < DataType > (std::move(data))
		{
		}

		BinaryNode(const BinaryNode < DataType, HasParent > &) = default;

		BinaryNode(BinaryNode < DataType, HasParent > &&) noexcept = default;

		~BinaryNode() noexcept override = default;
	};
}

#endif