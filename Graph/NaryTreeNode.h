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

#ifndef _XABLAU_GRAPH_NARY_TREE_NODE_H_
#define _XABLAU_GRAPH_NARY_TREE_NODE_H_

#include "NaryNode.h"

#include <deque>
#include <list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace xablau::graph
{
	enum class NaryTreeNodeContainerType
	{
		Vector,
		Deque,
		List
	};

	template < typename DataType, NaryTreeNodeContainerType ContainerType >
	class NaryTreeNode :
		public NaryNode < DataType, true >
	{
	protected:
		using Container =
			typename std::conditional < ContainerType == NaryTreeNodeContainerType::Vector,
				std::vector < std::unique_ptr < NaryTreeNode < DataType, ContainerType > > >,

			typename std::conditional < ContainerType == NaryTreeNodeContainerType::Deque,
				std::deque < std::unique_ptr < NaryTreeNode < DataType, ContainerType > > >,
				std::list < std::unique_ptr < NaryTreeNode < DataType, ContainerType > > > >::type > ::type;

		std::optional < std::reference_wrapper < NaryTreeNode < DataType, ContainerType > > > parent = std::nullopt;
		Container children;

	private:
		typename Container::iterator calculateIterator(size_t position)
		{
			if constexpr (ContainerType == NaryTreeNodeContainerType::List)
			{
				auto iterator = this->children.begin();

				for (size_t i = 0; i < position; i++)
				{
					++iterator;
				}

				return iterator;
			}

			else
			{
				return this->children.begin() + position;
			}
		}

		typename Container::const_iterator calculateConstIterator(size_t position) const
		{
			if constexpr (ContainerType == NaryTreeNodeContainerType::List)
			{
				auto iterator = this->children.cbegin();

				for (size_t i = 0; i < position; i++)
				{
					++iterator;
				}

				return iterator;
			}

			else
			{
				return this->children.cbegin() + position;
			}
		}

		void updateChildren()
		{
			for (auto &child : this->children)
			{
				if (child)
				{
					child->parent = *this;
				}
			}
		}

	public:
		NaryTreeNode < DataType, ContainerType > &insertChild(const DataType &value, const size_t position) override
		{
			if (position > this->size())
			{
				throw std::out_of_range("Argument \"position\" is greater than \"size\".");
			}

			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (value));

			return *(this->children.emplace(this->calculateIterator(position), std::move(pointer))->get());
		}

		NaryTreeNode < DataType, ContainerType > &insertBackChild(const DataType &value) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (value));

			return *(this->children.emplace_back(std::move(pointer)).get());
		}

		NaryTreeNode < DataType, ContainerType > &insertFrontChild(const DataType &value) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (value));

			if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
			{
				return *(this->children.emplace(this->children.begin(), std::move(pointer))->get());
			}

			else
			{
				return *(this->children.emplace_front(std::move(pointer)).get());
			}
		}

		NaryTreeNode < DataType, ContainerType > &insertChild(DataType &&value, const size_t position) override
		{
			if (position > this->size())
			{
				throw std::out_of_range("Argument \"position\" is greater than \"size\".");
			}

			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (std::move(value)));

			return *(this->children.emplace(this->calculateIterator(position), std::move(pointer))->get());
		}

		NaryTreeNode < DataType, ContainerType > &insertBackChild(DataType &&value) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (std::move(value)));

			return *(this->children.emplace_back(std::move(pointer)).get());
		}

		NaryTreeNode < DataType, ContainerType > &insertFrontChild(DataType &&value) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (std::move(value)));

			if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
			{
				return *(this->children.emplace(this->children.begin(), std::move(pointer))->get());
			}

			else
			{
				return *(this->children.emplace_front(std::move(pointer)).get());
			}
		}

		NaryTreeNode < DataType, ContainerType > &insertChild(const NaryNode < DataType, true > &child, const size_t position) override
		{
			if (position > this->size())
			{
				throw std::out_of_range("Argument \"position\" is greater than \"size\".");
			}

			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (child));

			return *(this->children.emplace(this->calculateIterator(position), std::move(pointer))->get());
		}

		NaryTreeNode < DataType, ContainerType > &insertBackChild(const NaryNode < DataType, true > &child) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (child));

			return *(this->children.emplace_back(std::move(pointer)).get());
		}

		NaryTreeNode < DataType, ContainerType > &insertFrontChild(const NaryNode < DataType, true > &child) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (child));

			if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
			{
				return *(this->children.emplace(this->children.begin(), std::move(pointer))->get());
			}

			else
			{
				return *(this->children.emplace_front(std::move(pointer)).get());
			}
		}

		NaryTreeNode < DataType, ContainerType > &insertChild(NaryNode < DataType, true > &&child, const size_t position) override
		{
			if (position > this->size())
			{
				throw std::out_of_range("Argument \"position\" is greater than \"size\".");
			}

			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (std::move(child)));

			return *(this->children.emplace(this->calculateIterator(position), std::move(pointer))->get());
		}

		NaryTreeNode < DataType, ContainerType > &insertBackChild(NaryNode < DataType, true > &&child) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (std::move(child)));

			return *(this->children.emplace_back(std::move(pointer)).get());
		}

		NaryTreeNode < DataType, ContainerType > &insertFrontChild(NaryNode < DataType, true > &&child) override
		{
			std::unique_ptr < NaryTreeNode < DataType, ContainerType > > pointer(
				new NaryTreeNode < DataType, ContainerType > (std::move(child)));

			if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
			{
				return *(this->children.emplace(this->children.begin(), std::move(pointer))->get());
			}

			else
			{
				return *(this->children.emplace_front(std::move(pointer)).get());
			}
		}

		void invalidateBackChild() override
		{
			if (!this->children.empty())
			{
				this->children.back().reset();

				return;
			}

			throw std::out_of_range("There is not a single child to invalidate.");
		}

		void invalidateFrontChild() override
		{
			if (!this->children.empty())
			{
				this->children.front().reset();

				return;
			}

			throw std::out_of_range("There is not a single child to invalidate.");
		}

		void invalidateChild(const size_t position) override
		{
			if (position < this->size())
			{
				this->calculateIterator(position)->reset();
			}

			throw std::out_of_range("Argument \"position\" is greater or equal to \"size\".");
		}

		void eraseBackChild() override
		{
			if (!this->children.empty())
			{
				this->children.pop_back();

				return;
			}

			throw std::out_of_range("There is not a single child to remove.");
		}

		void eraseFrontChild() override
		{
			if (!this->children.empty())
			{
				if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
				{
					this->children.erase(this->children.begin());
				}

				else
				{
					this->children.pop_front();
				}

				return;
			}

			throw std::out_of_range("There is not a single child to remove.");
		}

		void eraseChild(const size_t position) override
		{
			if (position < this->size())
			{
				this->children.erase(this->calculateIterator(position));
			}

			throw std::out_of_range("Argument \"position\" is greater or equal to \"size\".");
		}

		[[nodiscard]] bool empty() const noexcept override
		{
			return this->children.empty();
		}

		[[nodiscard]] size_t validSize() const noexcept override
		{
			size_t validElements = 0;

			for (const auto &pointer : this->children)
			{
				if (pointer != nullptr)
				{
					validElements++;
				}
			}

			return validElements;
		}

		[[nodiscard]] size_t size() const noexcept override
		{
			return this->children.size();
		}

		void clear() noexcept override
		{
			this->children.clear();
		}

		[[nodiscard]] bool hasValidChild(const size_t position) const override
		{
			return this->hasChild(position) && *(this->calculateConstIterator(position)) != nullptr;
		}

		[[nodiscard]] bool hasChild(const size_t position) const override
		{
			return position < this->size();
		}

		[[nodiscard]] bool hasBackChild() const override
		{
			return !this->empty() && this->children.back() != nullptr;
		}

		[[nodiscard]] bool hasFrontChild() const override
		{
			return !this->empty() && this->children.front() != nullptr;
		}

		[[nodiscard]] bool hasParent() const override
		{
			return this->parent.has_value();
		}

		[[nodiscard]] std::vector < std::reference_wrapper < const NaryNode < DataType, true > > > getValidChildren() const override
		{
			std::vector < std::reference_wrapper < const NaryNode < DataType, true > > > _children;

			_children.reserve(this->size());

			for (auto iterator = this->children.cbegin(); iterator != this->children.cend(); ++iterator)
			{
				if (*iterator)
				{
					_children.emplace_back(*(iterator->get()));
				}
			}

			return _children;
		}

		[[nodiscard]] std::vector < std::reference_wrapper < NaryNode < DataType, true > > > getValidChildren() override
		{
			std::vector < std::reference_wrapper < NaryNode < DataType, true > > > _children;

			_children.reserve(this->size());

			for (auto iterator = this->children.begin(); iterator != this->children.end(); ++iterator)
			{
				if (*iterator)
				{
					_children.emplace_back(*(iterator->get()));
				}
			}

			return _children;
		}

		[[nodiscard]] std::vector < std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > > getChildren() const override
		{
			std::vector < std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > > _children;

			_children.reserve(this->size());

			for (auto iterator = this->children.cbegin(); iterator != this->children.cend(); ++iterator)
			{
				if (*iterator)
				{
					_children.emplace_back(*(iterator->get()));
				}

				else
				{
					_children.emplace_back(std::nullopt);
				}
			}

			return _children;
		}

		[[nodiscard]] std::vector < std::optional < std::reference_wrapper < NaryNode < DataType, true > > > > getChildren() override
		{
			std::vector < std::optional < std::reference_wrapper < NaryNode < DataType, true > > > > _children;

			_children.reserve(this->size());

			for (auto iterator = this->children.begin(); iterator != this->children.end(); ++iterator)
			{
				if (*iterator)
				{
					_children.emplace_back(*(iterator->get()));
				}

				else
				{
					_children.emplace_back(std::nullopt);
				}
			}

			return _children;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > getParent() const override
		{
			if (this->parent)
			{
				return this->parent.value().get();
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < NaryNode < DataType, true > > > getParent() override
		{
			if (this->parent.has_value())
			{
				return this->parent.value().get();
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > getLeftValidSibling() const override
		{
			auto parent = this->getParentAndMyPosition();

			if (parent)
			{
				const auto &_parent = static_cast < const NaryTreeNode < DataType, ContainerType > & > (parent.value().first.get());

				for (auto iterator = std::reverse_iterator(_parent.calculateConstIterator(parent.value().second));
					iterator != _parent.children.crend();
					++iterator)
				{
					if (*iterator)
					{
						return *(iterator->get());
					}
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > getRightValidSibling() const override
		{
			auto parent = this->getParentAndMyPosition();

			if (parent)
			{
				const auto &_parent = static_cast < const NaryTreeNode < DataType, ContainerType > & > (parent.value().first.get());

				for (auto iterator = ++(_parent.calculateConstIterator(parent.value().second));
					iterator != _parent.children.cend();
					++iterator)
				{
					if (*iterator)
					{
						return *(iterator->get());
					}
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional <
			std::pair < std::reference_wrapper < const NaryNode < DataType, true > >, size_t > > getParentAndMyValidPosition() const override
		{
			if (this->parent)
			{
				std::pair < std::reference_wrapper < const NaryNode < DataType, true > >, size_t > pair(
					this->parent.value().get(),
					0);

				size_t counter = 0;
				const auto &_parent = static_cast < const NaryTreeNode < DataType, ContainerType > & > (pair.first.get());

				for (const auto &sibling : _parent.children)
				{
					if (sibling.get() == this)
					{
						pair.second = counter;

						return pair;
					}

					if (sibling.get() != nullptr)
					{
						counter++;
					}
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional <
			std::pair < std::reference_wrapper < const NaryNode < DataType, true > >, size_t > > getParentAndMyPosition() const override
		{
			if (this->parent)
			{
				std::pair < std::reference_wrapper < const NaryNode < DataType, true > >, size_t > pair(
					this->parent.value().get(),
					0);

				size_t counter = 0;
				const auto &parent = static_cast < const NaryTreeNode < DataType, ContainerType > & > (pair.first.get());

				for (const auto &sibling : parent.children)
				{
					if (sibling.get() == this)
					{
						pair.second = counter;

						return pair;
					}

					counter++;
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > getChild(const size_t position) const override
		{
			if (this->hasValidChild(position))
			{
				return *(this->calculateConstIterator(position)->get());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > getBackChild() const override
		{
			if (!this->children.empty() && this->children.back() != nullptr)
			{
				return *(this->children.back());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > getFrontChild() const override
		{
			if (!this->children.empty() && this->children.front() != nullptr)
			{
				return *(this->children.front());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < NaryNode < DataType, true > > > getLeftValidSibling() override
		{
			auto parent = this->getParentAndMyPosition();

			if (parent)
			{
				auto &_parent = static_cast < NaryTreeNode < DataType, ContainerType > & > (parent.value().first.get());

				for (auto iterator = std::reverse_iterator(_parent.calculateIterator(parent.value().second));
					iterator != _parent.children.crend();
					++iterator)
				{
					if (*iterator)
					{
						return *(iterator->get());
					}
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < NaryNode < DataType, true > > > getRightValidSibling() override
		{
			auto parent = this->getParentAndMyPosition();

			if (parent)
			{
				auto &_parent = static_cast < NaryTreeNode < DataType, ContainerType > & > (parent.value().first.get());

				for (auto iterator = ++(_parent.calculateConstIterator(parent.value().second));
					iterator != _parent.children.cend();
					++iterator)
				{
					if (*iterator)
					{
						return *(iterator->get());
					}
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional <
			std::pair < std::reference_wrapper < NaryNode < DataType, true > >, size_t > > getParentAndMyValidPosition() override
		{
			if (this->parent)
			{
				std::pair < std::reference_wrapper < NaryNode < DataType, true > >, size_t > pair(
					this->parent.value().get(),
					0);

				size_t counter = 0;
				const auto &_parent = static_cast < const NaryTreeNode < DataType, ContainerType > & > (pair.first.get());

				for (const auto &sibling : _parent.children)
				{
					if (sibling.get() == this)
					{
						pair.second = counter;

						return pair;
					}

					if (sibling.get() != nullptr)
					{
						counter++;
					}
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional <
			std::pair < std::reference_wrapper < NaryNode < DataType, true > >, size_t > > getParentAndMyPosition() override
		{
			if (this->parent)
			{
				std::pair < std::reference_wrapper < NaryNode < DataType, true > >, size_t > pair(
					this->parent.value().get(),
					0);

				size_t counter = 0;
				auto &parent = static_cast < NaryTreeNode < DataType, ContainerType > & > (pair.first.get());

				for (auto &sibling : parent.children)
				{
					if (sibling.get() == this)
					{
						pair.second = counter;

						return pair;
					}

					counter++;
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < NaryNode < DataType, true > > > getChild(const size_t position) override
		{
			if (this->hasValidChild(position))
			{
				return *(this->calculateIterator(position)->get());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < NaryNode < DataType, true > > > getBackChild() override
		{
			if (!this->children.empty() && this->children.back() != nullptr)
			{
				return *(this->children.back());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < NaryNode < DataType, true > > > getFrontChild() override
		{
			if (!this->children.empty() && this->children.front() != nullptr)
			{
				return *(this->children.front());
			}

			return std::nullopt;
		}

		std::tuple < std::optional < size_t >, std::shared_ptr < NaryNode < DataType, true > > > extractMyselfAndInvalidateMeOnParent() override
		{
			if (this->parent)
			{
				size_t counter = 0;
				auto &parent = static_cast < NaryTreeNode < DataType, ContainerType > & > (this->parent.value().get());

				for (const auto &sibling : parent.children)
				{
					if (sibling.get() == this)
					{
						return std::make_tuple(counter, parent.extractAndInvalidateChild(counter));
					}

					counter++;
				}
			}

			return std::make_tuple(
				std::nullopt,
				std::shared_ptr < NaryNode < DataType, true > > (
					new NaryTreeNode < DataType, ContainerType > (std::move(*this))));
		}

		std::tuple < std::optional < size_t >, std::shared_ptr < NaryNode < DataType, true > > > extractMyselfAndEraseMeOnParent() override
		{
			if (this->parent)
			{
				size_t counter = 0;
				auto &parent = static_cast < NaryTreeNode < DataType, ContainerType > & > (this->parent.value().get());

				for (const auto &sibling : parent.children)
				{
					if (sibling.get() == this)
					{
						return std::make_tuple(counter, parent.extractAndEraseChild(counter));
					}

					counter++;
				}
			}

			return std::make_tuple(
				std::nullopt,
				std::shared_ptr < NaryNode < DataType, true > > (
					new NaryTreeNode < DataType, ContainerType > (std::move(*this))));
		}

		std::shared_ptr < NaryNode < DataType, true > > extractAndInvalidateChild(const size_t position) override
		{
			if (position < this->size())
			{
				std::shared_ptr < NaryNode < DataType, true > > pointer(this->calculateIterator(position)->release());

				return pointer;
			}

			return nullptr;
		}

		std::shared_ptr < NaryNode < DataType, true > > extractAndEraseChild(const size_t position) override
		{
			if (position < this->size())
			{
				auto iterator = this->calculateIterator(position);
				std::shared_ptr < NaryNode < DataType, true > > pointer(iterator->release());

				this->children.erase(iterator);

				return pointer;
			}

			return nullptr;
		}

		std::shared_ptr < NaryNode < DataType, true > > extractAndInvalidateBackChild() override
		{
			if (!this->children.empty())
			{
				std::shared_ptr < NaryNode < DataType, true > > pointer(this->children.back().release());

				return pointer;
			}

			return nullptr;
		}

		std::shared_ptr < NaryNode < DataType, true > > extractAndEraseBackChild() override
		{
			if (!this->children.empty())
			{
				std::shared_ptr < NaryNode < DataType, true > > pointer(this->children.back().release());

				this->eraseBackChild();

				return pointer;
			}

			return nullptr;
		}

		std::shared_ptr < NaryNode < DataType, true > > extractAndInvalidateFrontChild() override
		{
			if (!this->children.empty())
			{
				std::shared_ptr < NaryNode < DataType, true > > pointer(this->children.front().release());

				return pointer;
			}

			return nullptr;
		}

		std::shared_ptr < NaryNode < DataType, true > > extractAndEraseFrontChild() override
		{
			if (!this->children.empty())
			{
				std::shared_ptr < NaryNode < DataType, true > > pointer(this->children.front().release());

				this->eraseFrontChild();

				return pointer;
			}

			return nullptr;
		}

		NaryTreeNode < DataType, ContainerType > &operator=(const NaryTreeNode < DataType, ContainerType > &node)
		{
			if (this == &node)
			{
				return *this;
			}

			this->clear();

			if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
			{
				this->children.reserve(node.size());
			}

			this->value = node.value;

			for (const auto &child : node.children)
			{
				this->children.emplace_back(child ? new NaryTreeNode < DataType, ContainerType > (*(child.get())) : nullptr);
			}

			this->updateChildren();

			return *this;
		}

		NaryTreeNode < DataType, ContainerType > &operator=(NaryTreeNode < DataType, ContainerType > &&node) noexcept
		{
			this->value = std::move(node.value);

			this->children = std::move(node.children);

			this->updateChildren();

			node.extractMyselfAndInvalidateMeOnParent();

			return *this;
		}

		NaryTreeNode() = default;

		explicit NaryTreeNode(const DataType &data) :
			NaryNode < DataType, true > (data)
		{
		}

		explicit NaryTreeNode(DataType &&data) :
			NaryNode < DataType, true > (std::move(data))
		{
		}

		NaryTreeNode(const NaryNode < DataType, true > &node) :
			NaryNode < DataType, true > (node.value)
		{
			auto children = node.getChildren();

			for (const auto &child : children)
			{
				if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
				{
					this->children.reserve(node.size());
				}

				if (child.has_value())
				{
					this->children.emplace_back(new NaryTreeNode < DataType, ContainerType > (child.value().get()));
				}

				else
				{
					this->children.emplace_back(nullptr);
				}
			}
			
			this->updateChildren();
		}

		NaryTreeNode(NaryNode < DataType, true > &&node) :
			NaryNode < DataType, true > (std::move(node.value))
		{
			auto _node = dynamic_cast < NaryTreeNode < DataType, ContainerType > * > (&node);

			if (_node)
			{
				this->children = std::move(_node->children);
			}

			else
			{
				if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
				{
					this->children.reserve(node.size());
				}

				auto children = node.getChildren();

				for (const auto &child : children)
				{
					if (child.has_value())
					{
						this->children.emplace_back(new NaryTreeNode < DataType, ContainerType > (child.value().get()));
					}

					else
					{
						this->children.emplace_back(nullptr);
					}
				}
			}

			this->updateChildren();

			node.extractMyselfAndInvalidateMeOnParent();
		}

		NaryTreeNode(const NaryTreeNode < DataType, ContainerType > &node) :
			NaryNode < DataType, true > (node.value)
		{
			const auto &children = node.children;

			for (const auto &child : children)
			{
				if constexpr (ContainerType == NaryTreeNodeContainerType::Vector)
				{
					this->children.reserve(node.size());
				}

				if (child)
				{
					this->children.emplace_back(new NaryTreeNode < DataType, ContainerType > (*(child.get())));
				}

				else
				{
					this->children.emplace_back(nullptr);
				}
			}

			this->updateChildren();
		}

		NaryTreeNode(NaryTreeNode < DataType, ContainerType > &&node) noexcept :
			NaryNode < DataType, true > (std::move(node.value)),

			children(std::move(node.children))
		{
			this->updateChildren();

			node.extractMyselfAndInvalidateMeOnParent();
		}

		~NaryTreeNode() noexcept override = default;
	};
}

#endif