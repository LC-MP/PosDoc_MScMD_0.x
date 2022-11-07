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

#ifndef _XABLAU_GRAPH_NARY_TREE_H_
#define _XABLAU_GRAPH_NARY_TREE_H_

#include <stdexcept>

#include "AbstractTree.h"
#include "NaryTreeNode.h"

namespace xablau::graph
{
	template < typename DataType, NaryTreeNodeContainerType ContainerType >
	class NaryTree :
		public AbstractTree < DataType >
	{
	public:
		using node_type = NaryTreeNode < DataType, ContainerType >;

		class const_iterator;

		class iterator
		{
		private:
			friend NaryTree < DataType, ContainerType >;

			std::optional < std::reference_wrapper < iterator > > lastElement = std::nullopt;
			std::optional < std::reference_wrapper < node_type > > node = std::nullopt;

			iterator(std::optional < std::reference_wrapper < node_type > > node, iterator &lastElement) :
				lastElement(lastElement),
				node(node)
			{
			}

		public:
			using iterator_category = std::bidirectional_iterator_tag;

			bool operator==(const const_iterator &iterator) const
			{
				if (this->node.has_value() && iterator.node.has_value())
				{
					return &(this->node.value().get()) == &(iterator.node.value().get());
				}

				if (!this->node.has_value() && !iterator.node.has_value())
				{
					return true;
				}

				return false;
			}

			bool operator!=(const const_iterator &iterator) const
			{
				return !(*this == iterator);
			}

			bool operator==(const iterator &iterator) const
			{
				if (this->node.has_value() && iterator.node.has_value())
				{
					return &(this->node.value().get()) == &(iterator.node.value().get());
				}

				if (!this->node.has_value() && !iterator.node.has_value())
				{
					return true;
				}

				return false;
			}

			bool operator!=(const iterator &iterator) const
			{
				return !(*this == iterator);
			}

			iterator &operator=(const iterator &) = default;

			iterator &operator=(iterator &&) noexcept = default;

			iterator &operator++()
			{
				if (this->node)
				{
					auto &_node1 = this->node.value().get();

					if (!_node1.empty())
					{
						this->node = static_cast < node_type & > (_node1.getFrontChild().value().get());

						return *this;
					}

					std::optional < std::reference_wrapper < NaryNode < DataType, true > > > _node2 = this->node;

					do
					{
						auto rightSibling = _node2.value().get().getRightValidSibling();

						if (rightSibling.has_value())
						{
							this->node = static_cast < node_type & > (rightSibling.value().get());

							return *this;
						}

						_node2 = _node2.value().get().getParent();
					} while (_node2.has_value());

					this->node.reset();

					return *this;
				}

				throw std::out_of_range("Increment of an invalid iterator.");
			}

			iterator operator++(int)
			{
				iterator _iterator(*this);

				operator++();

				return _iterator;
			}

			iterator &operator--()
			{
				if (this->node)
				{
					auto leftSibling = this->node.value().get().getLeftValidSibling();

					if (leftSibling.has_value())
					{
						auto rightmostNode = leftSibling;

						while (rightmostNode.value().get().hasBackChild())
						{
							rightmostNode = rightmostNode.value().get().getBackChild();
						}

						this->node = static_cast < node_type & > (rightmostNode.value().get());

						return *this;
					}

					auto parent = this->node.value().get().getParentAndMyPosition();

					if (parent.has_value())
					{
						this->node = static_cast < node_type & > (parent.value().first.get());

						return *this;
					}

					throw std::out_of_range("Decrement of an invalid iterator.");
				}

				if (this->lastElement.has_value() && this->lastElement.value().get().node.has_value())
				{
					this->node = this->lastElement.value().get().node;

					return *this;
				}

				throw std::out_of_range("Decrement of an invalid iterator.");
			}

			iterator operator--(int)
			{
				iterator _iterator(*this);

				operator--();

				return _iterator;
			}

			DataType operator*() const
			{
				if (this->node)
				{
					return this->node.value().get().value;
				}

				throw std::logic_error("Attempt of getting the reference of an invalid iterator.");
			}

			DataType *operator->() const
			{
				if (this->node)
				{
					return &(this->node.value().get().value);
				}

				throw std::logic_error("Attempt of getting the pointer of an invalid iterator.");
			}

			std::optional < std::reference_wrapper < const node_type > > getNode()
			{
				return this->node;
			}

			void swap(iterator &iterator) noexcept
			{
				std::swap(this->node, iterator.node);
				std::swap(this->lastElement, iterator.lastElement);
			}

			constexpr iterator() = default;

			iterator(const iterator &) = default;

			iterator(iterator &&) noexcept = default;

			~iterator() = default;
		};

		class const_iterator
		{
		private:
			friend NaryTree < DataType, ContainerType >;

			std::optional < std::reference_wrapper < iterator > > lastElement = std::nullopt;
			std::optional < std::reference_wrapper < node_type > > node = std::nullopt;

			const_iterator(std::optional < std::reference_wrapper < node_type > > node, iterator &lastElement) :
				lastElement(lastElement),
				node(node)
			{
			}

		public:
			using iterator_category = std::bidirectional_iterator_tag;

			bool operator==(const const_iterator &iterator) const
			{
				if (this->node.has_value() && iterator.node.has_value())
				{
					return &(this->node.value().get()) == &(iterator.node.value().get());
				}

				if (!this->node.has_value() && !iterator.node.has_value())
				{
					return true;
				}

				return false;
			}

			bool operator!=(const const_iterator &iterator) const
			{
				return !(*this == iterator);
			}

			bool operator==(const iterator &iterator) const
			{
				if (this->node.has_value() && iterator.node.has_value())
				{
					return &(this->node.value().get()) == &(iterator.node.value().get());
				}

				if (!this->node.has_value() && !iterator.node.has_value())
				{
					return true;
				}

				return false;
			}

			bool operator!=(const iterator &iterator) const
			{
				return !(*this == iterator);
			}

			const_iterator &operator=(const const_iterator &) = default;

			const_iterator &operator=(const_iterator &&) noexcept = default;

			const_iterator &operator=(const iterator &iterator)
			{
				this->node = iterator.node;

				return *this;
			}

			const_iterator &operator++()
			{
				if (this->node)
				{
					auto &_node1 = this->node.value().get();

					if (!_node1.empty())
					{
						this->node = static_cast < node_type & > (_node1.getFrontChild().value().get());

						return *this;
					}

					std::optional < std::reference_wrapper < NaryNode < DataType, true > > > _node2 = this->node;

					do
					{
						auto rightSibling = _node2.value().get().getRightValidSibling();

						if (rightSibling.has_value())
						{
							this->node = static_cast < node_type & > (rightSibling.value().get());

							return *this;
						}

						_node2 = _node2.value().get().getParent();
					} while (_node2.has_value());

					this->node.reset();

					return *this;
				}

				throw std::out_of_range("Increment of an invalid iterator.");
			}

			const_iterator operator++(int)
			{
				const_iterator _iterator(*this);

				operator++();

				return _iterator;
			}

			const_iterator &operator--()
			{
				if (this->node)
				{
					auto leftSibling = this->node.value().get().getLeftValidSibling();

					if (leftSibling.has_value())
					{
						auto rightmostNode = leftSibling;

						while (rightmostNode.value().get().hasBackChild())
						{
							rightmostNode = rightmostNode.value().get().getBackChild();
						}

						this->node = static_cast < node_type & > (rightmostNode.value().get());

						return *this;
					}

					auto parent = this->node.value().get().getParentAndMyPosition();

					if (parent.has_value())
					{
						this->node = static_cast < node_type & > (parent.value().first.get());

						return *this;
					}

					throw std::out_of_range("Decrement of an invalid iterator.");
				}

				if (this->lastElement.has_value() && this->lastElement.value().get().node.has_value())
				{
					this->node = this->lastElement.value().get().node;

					return *this;
				}

				throw std::out_of_range("Decrement of an invalid iterator.");
			}

			const_iterator operator--(int)
			{
				const_iterator _iterator(*this);

				operator--();

				return _iterator;
			}

			const DataType &operator*() const
			{
				if (this->node)
				{
					return this->node.value().get().value;
				}

				throw std::logic_error("Attempt of getting the reference of an invalid iterator.");
			}

			const DataType *operator->() const
			{
				if (this->node)
				{
					return &(this->node.value().get().value);
				}

				throw std::logic_error("Attempt of getting the pointer of an invalid iterator.");
			}

			std::optional < std::reference_wrapper < const node_type > > getNode()
			{
				return this->node;
			}

			void swap(const_iterator &iterator) noexcept
			{
				std::swap(this->node, iterator.node);
				std::swap(this->lastElement, iterator.lastElement);
			}

			constexpr const_iterator() = default;

			const_iterator(const iterator &iterator) :
				lastElement(iterator.lastElement),
				node(iterator.node)
			{
			}

			const_iterator(const const_iterator &) = default;

			const_iterator(const_iterator &&) noexcept = default;

			~const_iterator() = default;
		};

		using reverse_iterator = std::reverse_iterator < iterator >;
		using const_reverse_iterator = std::reverse_iterator < const_iterator >;

	private:
		static void getLeafNodes(
			const node_type &node,
			std::vector < std::reference_wrapper < const node_type > > &leafNodes)
		{
			if (node.empty())
			{
				leafNodes.emplace_back(node);
			}

			else
			{
				const auto children = node.getValidChildren();

				for (const auto &child : children)
				{
					NaryTree::getLeafNodes(static_cast < const node_type & > (child.get()), leafNodes);
				}
			}
		}

		static void getNodes(
			const node_type &node,
			std::vector < std::reference_wrapper < const node_type > > &nodes)
		{
			nodes.emplace_back(node);

			const auto children = node.getValidChildren();

			for (const auto &child : children)
			{
				NaryTree::getNodes(static_cast < const node_type & > (child.get()), nodes);
			}
		}

		static void quantityOfLevels(
			const node_type &node,
			const size_t currentLevel,
			size_t &quantityOfLevels)
		{
			if (currentLevel + 1 > quantityOfLevels)
			{
				quantityOfLevels = currentLevel + 1;
			}

			const auto children = node.getValidChildren();

			for (const auto &child : children)
			{
				NaryTree::quantityOfLevels(
					static_cast < const node_type & > (child.get()), currentLevel, quantityOfLevels);
			}
		}

		static void quantityOfNodesByLevel(
			const node_type &node,
			const size_t currentLevel,
			std::vector < size_t > &quantityOfNodesByLevel)
		{
			if (quantityOfNodesByLevel.size() < currentLevel + 1)
			{
				quantityOfNodesByLevel.resize(currentLevel + 1, 0);
			}

			++(quantityOfNodesByLevel[currentLevel]);

			const auto children = node.getValidChildren();

			for (const auto &child : children)
			{
				NaryTree::quantityOfNodesByLevel(
					static_cast < const node_type & > (child.get()), currentLevel + 1, quantityOfNodesByLevel);
			}
		}

		static bool equal(
			const std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > &node1,
			const std::optional < std::reference_wrapper < const NaryNode < DataType, true > > > &node2)
		{
			if (node1 && node2)
			{
				const auto &_node1 = node1.value().get();
				const auto &_node2 = node2.value().get();

				if (_node1.size() != _node2.size())
				{
					return false;
				}

				const auto equalChildren =
					[] (const NaryNode < DataType, true > &node1, const NaryNode < DataType, true > &node2) -> bool
					{
						auto children1 = node1.getChildren();
						auto children2 = node2.getChildren();

						for (size_t i = 0; i < children1.size(); i++)
						{
							if (!NaryTree < DataType, ContainerType > ::equal(children1[i], children2[i]))
							{
								return false;
							}
						}

						return true;
					};

				return _node1.value == _node2.value && equalChildren(_node1, _node2);
			}

			if (!node1 && !node2)
			{
				return true;
			}

			return false;
		}

		template < typename IteratorType >
		static void find(
			node_type &node,
			const DataType &value,
			std::vector < size_t > &path,
			IteratorType &iterator)
		{
			if (node.value == value)
			{
				iterator.node = node;

				return;
			}

			const auto children = node.getValidChildren();

			if (!children.empty())
			{
				path.push_back(0);

				for (size_t i = 0; i < children.size(); i++)
				{
					path.back() = i;

					NaryTree < DataType, ContainerType > ::find(
						static_cast < node_type & > (children[i].get()),
						value,
						path,
						iterator);

					if (iterator.node.has_value())
					{
						return;
					}
				}

				path.pop_back();
			}
		}

		void updateLastElement()
		{
			if (this->root)
			{
				std::reference_wrapper < node_type > node = *(this->root.get());

				while (node.get().hasBackChild())
				{
					node = static_cast < node_type & > (node.get().getBackChild().value().get());
				}

				this->_lastElement.node = node.get();
			}

			else
			{
				this->_lastElement.node.reset();
			}
		}

		std::vector < const_iterator > upwardChain(const_iterator end)
		{
			std::vector < const_iterator > verticalChain;

			--end;

			while (end != this->_begin)
			{
				verticalChain.push_back(end);

				auto parent = end.node.value().get().getParent();

				if (parent)
				{
					end.node = static_cast < node_type & > (parent.value().get());
				}
			}

			verticalChain.push_back(end);

			return verticalChain;
		}

		bool iteratorFromThisTree(const_iterator iterator)
		{
			return this->_lastElement == iterator.lastElement;
		}

		static size_t sizeSubtree(const node_type &naryTreeNode)
		{
			size_t counter = 0;
			const auto children = naryTreeNode.getValidChildren();

			for (const auto &child : children)
			{
				counter += sizeSubtree(child.get());
			}

			return 1 + counter;
		}

		static bool binary(const node_type &naryTreeNode)
		{
			const auto children = naryTreeNode.getValidChildren();

			if (children.size() > 2)
			{
				return false;
			}

			for (const auto &child : children)
			{
				if (!NaryTree < DataType, ContainerType > ::binary(static_cast < const node_type & > (child.get())))
				{
					return false;
				}
			}

			return true;
		}

		static void print(std::ostream &stream, const node_type &naryTreeNode, const size_t offset)
		{
			for (size_t i = 0; i < offset; i++)
			{
				stream << '\t';
			}

			stream << "* " << naryTreeNode.value << " ->\n";

			const auto children = naryTreeNode.getValidChildren();

			for (const auto &child : children)
			{
				NaryTree::print(stream, static_cast < const node_type & > (child.get()), offset + 1);
			}
		}

	protected:
		size_t _size = 0;
		std::shared_ptr < node_type > root = nullptr;
		mutable iterator _lastElement;
		iterator _begin = iterator(std::nullopt, _lastElement);
		iterator _end = iterator(std::nullopt, _lastElement);

		static std::optional < std::reference_wrapper < node_type > > &getNode(iterator &iterator)
		{
			return iterator.node;
		}

		static std::optional < std::reference_wrapper < node_type > > &getNode(const_iterator &iterator)
		{
			return iterator.node;
		}

		static std::optional < std::reference_wrapper < iterator > > &getLastElement(iterator &iterator)
		{
			return iterator.lastElement;
		}

		static std::optional < std::reference_wrapper < iterator > > &getLastElement(const_iterator &iterator)
		{
			return iterator.lastElement;
		}

	public:
		bool operator==(const NaryTree < DataType, ContainerType > &tree) const
		{
			if (this->root && tree.root)
			{
				return NaryTree < DataType, ContainerType > ::equal(*(this->root.get()), *(tree.root.get()));
			}

			if (!this->root && !tree.root)
			{
				return true;
			}

			return false;
		}

		bool operator!=(const NaryTree < DataType, ContainerType > &tree) const
		{
			return !(*this == tree);
		}

		iterator begin()
		{
			return this->_begin;
		}

		const_iterator begin() const
		{
			return this->_begin;
		}

		const_iterator cbegin() const
		{
			return this->_begin;
		}

		iterator end()
		{
			return this->_end;
		}

		const_iterator end() const
		{
			return this->_end;
		}

		const_iterator cend() const
		{
			return this->_end;
		}

		reverse_iterator rbegin()
		{
			return std::reverse_iterator(this->_end);
		}

		const_reverse_iterator rbegin() const
		{
			return std::reverse_iterator(this->_end);
		}

		const_reverse_iterator crbegin() const
		{
			return std::reverse_iterator(this->_end);
		}

		reverse_iterator rend()
		{
			return std::reverse_iterator(this->_begin);
		}

		const_reverse_iterator rend() const
		{
			return std::reverse_iterator(this->_begin);
		}

		const_reverse_iterator crend() const
		{
			return std::reverse_iterator(this->_begin);
		}

		void swap(NaryTree < DataType, ContainerType > &tree) noexcept
		{
			std::swap(this->_size, tree._size);
			std::swap(this->root, tree.root);
			std::swap(this->_begin, tree._begin);
			std::swap(this->_lastElement, tree._lastElement);
			std::swap(this->_end, tree._end);
		}

		iterator insertRoot(const DataType &value)
		{
			this->root.reset(new node_type(value));

			this->_begin.node = *(this->root);
			this->_lastElement.node = this->_begin.node;
			this->_size = 1;

			return this->_begin;
		}

		iterator insertRoot(DataType &&value)
		{
			this->root.reset(new node_type(std::move(value)));

			this->_begin.node = *(this->root);
			this->_lastElement.node = this->_begin.node;
			this->_size = 1;

			return this->_begin;
		}

		iterator insert(const_iterator iterator, const DataType &value, const size_t position)
		{
			if (!this->iteratorFromThisTree(iterator))
			{
				throw std::logic_error("The iterator is not from this tree.");
			}

			if (this->empty())
			{
				if (iterator == this->_end)
				{
					return this->insertRoot(value);
				}

				throw std::logic_error("Tree is empty and the iterator has a dangling reference.");
			}

			if (iterator.node)
			{
				auto &node = iterator.node.value().get();

				if (node.hasChild(position))
				{
					const auto &currentChild = static_cast < const node_type & > (node.getChild(position).value().get());

					this->_size -= typename NaryTree < DataType, ContainerType > ::sizeSubtree(currentChild);
				}

				auto &newChild = node.insertChild(value, position);

				typename NaryTree < DataType, ContainerType > ::iterator childIterator(newChild, this->_lastElement);

				++(this->_size);

				return childIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insert(const_iterator iterator, DataType &&value, const size_t position)
		{
			if (!this->iteratorFromThisTree(iterator))
			{
				throw std::logic_error("The iterator is not from this tree.");
			}

			if (this->empty())
			{
				if (iterator == this->_end)
				{
					return this->insertRoot(std::move(value));
				}

				throw std::logic_error("Tree is empty and the iterator has a dangling reference.");
			}

			if (iterator.node)
			{
				auto &node = iterator.node.value().get();

				if (node.hasChild(position))
				{
					const auto &currentChild = static_cast < const node_type & > (node.getChild(position).value().get());

					this->_size -= typename NaryTree < DataType, ContainerType > ::sizeSubtree(currentChild);
				}

				auto &newChild = node.insertChild(std::move(value), position);

				typename NaryTree < DataType, ContainerType > ::iterator childIterator(newChild, this->_lastElement);

				++(this->_size);

				return childIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insertBack(const_iterator iterator, const DataType &value)
		{
			if (!this->iteratorFromThisTree(iterator))
			{
				throw std::logic_error("The iterator is not from this tree.");
			}

			if (this->empty())
			{
				if (iterator == this->_end)
				{
					return this->insertRoot(value);
				}

				throw std::logic_error("Tree is empty and the iterator has a dangling reference.");
			}

			if (iterator.node)
			{
				auto &node = iterator.node.value().get();
				auto &newChild = node.insertBackChild(value);

				typename NaryTree < DataType, ContainerType > ::iterator childIterator(newChild, this->_lastElement);

				++(this->_size);

				return childIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insertBack(const_iterator iterator, DataType &&value)
		{
			if (!this->iteratorFromThisTree(iterator))
			{
				throw std::logic_error("The iterator is not from this tree.");
			}

			if (this->empty())
			{
				if (iterator == this->_end)
				{
					return this->insertRoot(std::move(value));
				}

				throw std::logic_error("Tree is empty and the iterator has a dangling reference.");
			}

			if (iterator.node)
			{
				auto &node = iterator.node.value().get();
				auto &newChild = node.insertBackChild(std::move(value));

				typename NaryTree < DataType, ContainerType > ::iterator childIterator(newChild, this->_lastElement);

				++(this->_size);

				return childIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insertFront(const_iterator iterator, const DataType &value)
		{
			if (!this->iteratorFromThisTree(iterator))
			{
				throw std::logic_error("The iterator is not from this tree.");
			}

			if (this->empty())
			{
				if (iterator == this->_end)
				{
					return this->insertRoot(value);
				}

				throw std::logic_error("Tree is empty and the iterator has a dangling reference.");
			}

			if (iterator.node)
			{
				auto &node = iterator.node.value().get();
				auto &newChild = node.insertFrontChild(value);

				typename NaryTree < DataType, ContainerType > ::iterator childIterator(newChild, this->_lastElement);

				++(this->_size);

				return childIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insertFront(const_iterator iterator, DataType &&value)
		{
			if (!this->iteratorFromThisTree(iterator))
			{
				throw std::logic_error("The iterator is not from this tree.");
			}

			if (this->empty())
			{
				if (iterator == this->_end)
				{
					return this->insertRoot(std::move(value));
				}

				throw std::logic_error("Tree is empty and the iterator has a dangling reference.");
			}

			if (iterator.node)
			{
				auto &node = iterator.node.value().get();
				auto &newChild = node.insertFrontChild(std::move(value));

				typename NaryTree < DataType, ContainerType > ::iterator childIterator(newChild, this->_lastElement);

				++(this->_size);

				return childIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		virtual iterator erase(const_iterator iterator)
		{
			if (!(iterator.node.has_value()))
			{
				throw std::logic_error("Attempt of getting the node of an invalid iterator.");
			}

			if (this->root)
			{
				auto &node = iterator.node.value().get();

				if (!(node.hasParent()))
				{
					this->root.reset();

					this->_begin.node.reset();
					this->_lastElement.node.reset();
					this->_size = 0;

					return this->begin();
				}

				typename NaryTree < DataType, ContainerType > ::iterator returnIterator(node, this->_lastElement);

				--returnIterator;

				this->_size -= typename NaryTree < DataType, ContainerType > ::sizeSubtree(
					static_cast < const node_type & > (*std::get < 1 > (node.extractMyselfAndEraseMeOnParent()).get()));

				this->updateLastElement();

				return ++returnIterator;
			}

			throw std::out_of_range("Attempt to erase an element of an empty tree.");
		}

		virtual iterator erase(const_iterator begin, const_iterator end)
		{
			if (!this->iteratorFromThisTree(begin) || !this->iteratorFromThisTree(end))
			{
				throw std::logic_error("begin, end or both are not from this tree.");
			}

			if (begin == end)
			{
				iterator iterator{};

				iterator.node = end.node;
				iterator.lastElement = end.lastElement;

				return iterator;
			}

			const auto upwardChain = this->upwardChain(end);
			auto iteratorUpwardChain = std::find(upwardChain.cbegin(), upwardChain.cend(), begin);

			while (iteratorUpwardChain != upwardChain.cend())
			{
				begin = NaryTree < DataType, ContainerType > ::erase(begin);

				iteratorUpwardChain = std::find(upwardChain.cbegin(), upwardChain.cend(), begin);
			}

			return NaryTree < DataType, ContainerType > ::erase(*iteratorUpwardChain);
		}

		bool empty() const noexcept override
		{
			return this->root == nullptr;
		}

		bool binary() const override
		{
			if (this->root)
			{
				return NaryTree < DataType, ContainerType > ::binary(*(this->root));
			}

			return true;
		}

		size_t size() const noexcept override
		{
			return this->_size;
		}

		void clear() noexcept override
		{
			this->_size = 0;
			this->root.reset();
			this->_begin.node.reset();
			this->_lastElement.node.reset();
		}

		size_t count(const DataType &value) const override
		{
			size_t counter = 0;
			
			for (auto iterator = this->cbegin(); iterator != this->cend(); ++iterator)
			{
				if (*iterator == value)
				{
					counter++;
				}
			}

			return counter;
		}

		bool contains(const DataType &value) const override
		{
			return std::find(this->cbegin(), this->cend(), value) != this->cend();
		}

		virtual std::pair < const_iterator, std::vector < size_t > > find(const DataType &value) const
		{
			const_iterator iterator(std::nullopt, this->_lastElement);

			if (this->root == nullptr)
			{
				return std::make_pair(iterator, std::vector < size_t > ());
			}

			std::vector < size_t > path;

			NaryTree < DataType, ContainerType > ::find(*(this->root.get()), value, path, iterator);

			return std::make_pair(std::move(iterator), std::move(path));
		}

		virtual std::pair < iterator, std::vector < size_t > > find(const DataType &value)
		{
			iterator iterator(std::nullopt, this->_lastElement);
			
			if (this->root == nullptr)
			{
				return std::make_pair(iterator, std::vector < size_t > ());
			}

			std::vector < size_t > path;

			NaryTree < DataType, ContainerType > ::find(*(this->root.get()), value, path, iterator);

			return std::make_pair(std::move(iterator), std::move(path));
		}

		std::vector < std::reference_wrapper < const node_type > > getLeafNodes() const
		{
			std::vector < std::reference_wrapper < const node_type > > leafNodes;

			if (this->root)
			{
				NaryTree < DataType, ContainerType > ::getLeafNodes(*(this->root.get()), leafNodes);
			}

			return leafNodes;
		}

		std::vector < std::reference_wrapper < const node_type > > getNodes() const
		{
			std::vector < std::reference_wrapper < const node_type > > nodes;

			if (this->root)
			{
				NaryTree < DataType, ContainerType > ::getNodes(*(this->root.get()), nodes);
			}

			return nodes;
		}

		size_t quantityOfLevels() const override
		{
			size_t quantityOfLevels = 0;

			if (this->root)
			{
				NaryTree < DataType, ContainerType > ::quantityOfLevels(*(this->root.get()), 0, quantityOfLevels);
			}

			return quantityOfLevels;
		}

		std::vector < size_t > quantityOfNodesByLevel() const override
		{
			std::vector < size_t > quantityOfNodesByLevel;

			if (this->root)
			{
				NaryTree < DataType, ContainerType > ::quantityOfNodesByLevel(*(this->root.get()), 0, quantityOfNodesByLevel);
			}

			return quantityOfNodesByLevel;
		}

		friend std::ostream &operator<<(std::ostream &stream, const NaryTree < DataType, ContainerType > &tree)
		{
			stream << "*->* (\n";

			if (tree.root != nullptr)
			{
				NaryTree::print(stream, *(tree.root), 0);
			}

			stream << " )";

			return stream;
		}

		NaryTree() = default;

		NaryTree(const NaryTree < DataType, ContainerType > &) = default;

		NaryTree(NaryTree < DataType, ContainerType > &&) noexcept = default;

		~NaryTree() override = default;
	};
}

#endif