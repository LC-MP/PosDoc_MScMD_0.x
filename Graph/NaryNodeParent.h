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

#ifndef _XABLAU_GRAPH_NARY_NODE_PARENT_H_
#define _XABLAU_GRAPH_NARY_NODE_PARENT_H_

#include <functional>
#include <optional>
#include <tuple>

namespace xablau::graph
{
	template < typename NodeType, bool HasParent >
	class NaryNodeParent
	{
	public:
		virtual NodeType &insertChild(NodeType &&child, const size_t position) = 0;

		virtual NodeType &insertBackChild(NodeType &&child) = 0;

		virtual NodeType &insertFrontChild(NodeType &&child) = 0;

		virtual bool hasParent() const = 0;

		virtual std::optional < std::reference_wrapper < const NodeType > > getParent() const = 0;

		virtual std::optional < std::reference_wrapper < NodeType > > getParent() = 0;

		virtual std::optional < std::reference_wrapper < const NodeType > > getLeftValidSibling() const = 0;

		virtual std::optional < std::reference_wrapper < const NodeType > > getRightValidSibling() const = 0;

		virtual std::optional < std::reference_wrapper < NodeType > > getLeftValidSibling() = 0;

		virtual std::optional < std::reference_wrapper < NodeType > > getRightValidSibling() = 0;

		virtual std::optional < std::pair < std::reference_wrapper < const NodeType >, size_t > > getParentAndMyValidPosition() const = 0;

		virtual std::optional < std::pair < std::reference_wrapper < NodeType >, size_t > > getParentAndMyValidPosition() = 0;

		virtual std::optional < std::pair < std::reference_wrapper < const NodeType >, size_t > > getParentAndMyPosition() const = 0;

		virtual std::optional < std::pair < std::reference_wrapper < NodeType >, size_t > > getParentAndMyPosition() = 0;

		virtual std::tuple < std::optional < size_t >, std::shared_ptr < NodeType > > extractMyselfAndInvalidateMeOnParent() = 0;

		virtual std::tuple < std::optional < size_t >, std::shared_ptr < NodeType > > extractMyselfAndEraseMeOnParent() = 0;

		virtual ~NaryNodeParent() noexcept = default;
	};

	template < typename NodeType >
	class NaryNodeParent < NodeType, false >
	{
	};
}

#endif