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

#ifndef _XABLAU_GRAPH_NARY_NODE_H_
#define _XABLAU_GRAPH_NARY_NODE_H_

#include <functional>
#include <optional>

#include "NaryNodeParent.h"
#include "Node.h"

namespace xablau::graph
{
	template < typename DataType, bool HasParent >
	class NaryNode :
		public Node < DataType >,
		public NaryNodeParent < NaryNode < DataType, HasParent >, HasParent >
	{
	public:
		virtual NaryNode < DataType, HasParent > &insertChild(const DataType &value, const size_t position) = 0;

		virtual NaryNode < DataType, HasParent > &insertBackChild(const DataType &value) = 0;

		virtual NaryNode < DataType, HasParent > &insertFrontChild(const DataType &value) = 0;

		virtual NaryNode < DataType, HasParent > &insertChild(DataType &&value, const size_t position) = 0;

		virtual NaryNode < DataType, HasParent > &insertBackChild(DataType &&value) = 0;

		virtual NaryNode < DataType, HasParent > &insertFrontChild(DataType &&value) = 0;

		virtual NaryNode < DataType, HasParent > &insertChild(const NaryNode < DataType, HasParent > &child, const size_t position) = 0;

		virtual NaryNode < DataType, HasParent > &insertBackChild(const NaryNode < DataType, HasParent > &child) = 0;

		virtual NaryNode < DataType, HasParent > &insertFrontChild(const NaryNode < DataType, HasParent > &child) = 0;

		virtual void invalidateBackChild() = 0;

		virtual void invalidateFrontChild() = 0;

		virtual void invalidateChild(const size_t position) = 0;

		virtual void eraseBackChild() = 0;

		virtual void eraseFrontChild() = 0;

		virtual void eraseChild(const size_t position) = 0;

		virtual size_t validSize() const noexcept = 0;

		virtual bool empty() const noexcept = 0;

		virtual size_t size() const noexcept = 0;

		virtual void clear() noexcept = 0;

		virtual bool hasValidChild(const size_t position) const = 0;

		virtual bool hasChild(const size_t position) const = 0;

		virtual bool hasBackChild() const = 0;

		virtual bool hasFrontChild() const = 0;

		virtual std::vector < std::reference_wrapper < const NaryNode < DataType, HasParent > > > getValidChildren() const = 0;

		virtual std::vector < std::reference_wrapper < NaryNode < DataType, HasParent > > > getValidChildren() = 0;

		virtual std::vector < std::optional < std::reference_wrapper < const NaryNode < DataType, HasParent > > > > getChildren() const = 0;

		virtual std::vector < std::optional < std::reference_wrapper < NaryNode < DataType, HasParent > > > > getChildren() = 0;

		virtual std::optional < std::reference_wrapper < const NaryNode < DataType, HasParent > > > getChild(const size_t position) const = 0;

		virtual std::optional < std::reference_wrapper < const NaryNode < DataType, HasParent > > > getBackChild() const = 0;

		virtual std::optional < std::reference_wrapper < const NaryNode < DataType, HasParent > > > getFrontChild() const = 0;

		virtual std::optional < std::reference_wrapper < NaryNode < DataType, HasParent > > > getChild(const size_t position) = 0;

		virtual std::optional < std::reference_wrapper < NaryNode < DataType, HasParent > > > getBackChild() = 0;

		virtual std::optional < std::reference_wrapper < NaryNode < DataType, HasParent > > > getFrontChild() = 0;

		virtual std::shared_ptr < NaryNode < DataType, HasParent > > extractAndInvalidateChild(const size_t position) = 0;

		virtual std::shared_ptr < NaryNode < DataType, HasParent > > extractAndEraseChild(const size_t position) = 0;

		virtual std::shared_ptr < NaryNode < DataType, HasParent > > extractAndInvalidateBackChild() = 0;

		virtual std::shared_ptr < NaryNode < DataType, HasParent > > extractAndEraseBackChild() = 0;

		virtual std::shared_ptr < NaryNode < DataType, HasParent > > extractAndInvalidateFrontChild() = 0;

		virtual std::shared_ptr < NaryNode < DataType, HasParent > > extractAndEraseFrontChild() = 0;

		NaryNode < DataType, HasParent > &operator=(const NaryNode < DataType, HasParent > &) = default;

		NaryNode < DataType, HasParent > &operator=(NaryNode < DataType, HasParent > &&) noexcept = default;

		NaryNode() = default;

		explicit NaryNode(const DataType &data) :
			Node < DataType > (data)
		{
		}

		explicit NaryNode(DataType &&data) :
			Node < DataType > (std::move(data))
		{
		}

		NaryNode(const NaryNode < DataType, HasParent > &) = default;

		NaryNode(NaryNode < DataType, HasParent > &&) noexcept = default;

		~NaryNode() noexcept override = default;
	};
}

#endif