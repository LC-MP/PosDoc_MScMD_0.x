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

#ifndef _XABLAU_GRAPH_BINARY_TREE_H_
#define _XABLAU_GRAPH_BINARY_TREE_H_

#include <stdexcept>

#include "AbstractTree.h"
#include "BinaryTreeNode.h"

namespace xablau::graph
{
	template < typename DataType >
	class BinaryTree :
		public AbstractTree < DataType >
	{
	public:
		using Allocator = std::allocator < DataType >;
		using allocator_type = Allocator;
		using value_type = typename Allocator::value_type;
		using reference = typename Allocator::reference;
		using const_reference = typename Allocator::const_reference;
		using difference_type = typename Allocator::difference_type;
		using size_type = typename Allocator::size_type;
		using node_type = BinaryTreeNode < value_type >;

		class const_iterator;

		class iterator
		{
		private:
			friend BinaryTree < DataType >;

			std::optional < std::reference_wrapper < iterator > > lastElement = std::nullopt;
			std::optional < std::reference_wrapper < node_type > > node = std::nullopt;

			iterator(std::optional < std::reference_wrapper < node_type > > node, iterator &lastElement) :
				lastElement(lastElement),
				node(node)
			{
			}

		public:
			using value_type = typename Allocator::value_type;
			using reference = typename Allocator::reference;
			using pointer = typename Allocator::pointer;
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
					auto rightAndLeftmostChild = this->node.value().get().getRightChild();

					if (rightAndLeftmostChild)
					{
						while (rightAndLeftmostChild.value().get().hasLeftChild())
						{
							rightAndLeftmostChild = rightAndLeftmostChild.value().get().getLeftChild();
						}

						this->node = static_cast < node_type & > (rightAndLeftmostChild.value().get());

						return *this;
					}

					auto parent = this->node.value().get().getParentAndMyPosition();

					while (parent)
					{
						if (parent.value().second == 0)
						{
							this->node = static_cast < node_type & > (parent.value().first.get());

							return *this;
						}

						parent = parent.value().first.get().getParentAndMyPosition();
					}

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
					auto leftAndRightmostChild = this->node.value().get().getLeftChild();

					if (leftAndRightmostChild)
					{
						while (leftAndRightmostChild.value().get().hasRightChild())
						{
							leftAndRightmostChild = leftAndRightmostChild.value().get().getRightChild();
						}

						this->node = static_cast < node_type & > (leftAndRightmostChild.value().get());

						return *this;
					}

					auto parent = this->node.value().get().getParentAndMyPosition();

					while (parent)
					{
						if (parent.value().second == 1)
						{
							this->node = static_cast < node_type & > (parent.value().first.get());

							return *this;
						}

						parent = parent.value().first.get().getParentAndMyPosition();
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

			reference operator*() const
			{
				if (this->node)
				{
					return this->node.value().get().value;
				}

				throw std::logic_error("Attempt of getting the reference of an invalid iterator.");
			}

			pointer operator->() const
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
			friend BinaryTree < DataType >;

			std::optional < std::reference_wrapper < iterator > > lastElement = std::nullopt;
			std::optional < std::reference_wrapper < node_type > > node = std::nullopt;

			const_iterator(std::optional < std::reference_wrapper < node_type > > node, iterator &lastElement) :
				lastElement(lastElement),
				node(node)
			{
			}

		public:
			using value_type = typename Allocator::value_type;
			using const_reference = const typename Allocator::const_reference;
			using const_pointer = const typename Allocator::const_pointer;
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
					auto rightAndLeftmostChild = this->node.value().get().getRightChild();

					if (rightAndLeftmostChild)
					{
						while (rightAndLeftmostChild.value().get().hasLeftChild())
						{
							rightAndLeftmostChild = rightAndLeftmostChild.value().get().getLeftChild();
						}

						this->node = static_cast < node_type & > (rightAndLeftmostChild.value().get());

						return *this;
					}

					auto parent = this->node.value().get().getParentAndMyPosition();

					while (parent)
					{
						if (parent.value().second == 0)
						{
							this->node = static_cast < node_type & > (parent.value().first.get());

							return *this;
						}

						parent = parent.value().first.get().getParentAndMyPosition();
					}

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
					auto leftAndRightmostChild = this->node.value().get().getLeftChild();

					if (leftAndRightmostChild)
					{
						while (leftAndRightmostChild.value().get().hasRightChild())
						{
							leftAndRightmostChild = leftAndRightmostChild.value().get().getRightChild();
						}

						this->node = static_cast < node_type & > (leftAndRightmostChild.value().get());

						return *this;
					}

					auto parent = this->node.value().get().getParentAndMyPosition();

					while (parent)
					{
						if (parent.value().second == 1)
						{
							this->node = static_cast < node_type & > (parent.value().first.get());

							return *this;
						}

						parent = parent.value().first.get().getParentAndMyPosition();
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

			const_reference operator*() const
			{
				if (this->node)
				{
					return this->node.value().get().value;
				}

				throw std::logic_error("Attempt of getting the reference of an invalid iterator.");
			}

			const_pointer operator->() const
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
			if (node.size() == 0)
			{
				leafNodes.push_back(node);
			}

			else
			{
				auto leftChild = node.getLeftChild();

				if (leftChild)
				{
					BinaryTree::getLeafNodes(static_cast < const node_type & > (leftChild.value().get()), leafNodes);
				}

				auto rightChild = node.getRightChild();

				if (rightChild)
				{
					BinaryTree::getLeafNodes(static_cast < const node_type & > (rightChild.value().get()), leafNodes);
				}
			}
		}

		static void getNodes(
			const node_type &node,
			std::vector < std::reference_wrapper < const node_type > > &nodes)
		{
			nodes.push_back(node);

			auto leftChild = node.getLeftChild();

			if (leftChild)
			{
				BinaryTree::getNodes(static_cast < const node_type & > (leftChild.value().get()), nodes);
			}

			auto rightChild = node.getRightChild();

			if (rightChild)
			{
				BinaryTree::getNodes(static_cast < const node_type & > (rightChild.value().get()), nodes);
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

			auto leftChild = node.getLeftChild();

			if (leftChild)
			{
				BinaryTree::quantityOfLevels(
					static_cast < const node_type & > (leftChild.value().get()), currentLevel, quantityOfLevels);
			}

			auto rightChild = node.getRightChild();

			if (rightChild)
			{
				BinaryTree::quantityOfLevels(
					static_cast < const node_type & > (rightChild.value().get()), currentLevel, quantityOfLevels);
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

			auto leftChild = node.getLeftChild();

			if (leftChild)
			{
				BinaryTree::quantityOfNodesByLevel(
					static_cast < const node_type & > (leftChild.value().get()), currentLevel + 1, quantityOfNodesByLevel);
			}

			auto rightChild = node.getRightChild();

			if (rightChild)
			{
				BinaryTree::quantityOfNodesByLevel(
					static_cast < const node_type & > (rightChild.value().get()), currentLevel + 1, quantityOfNodesByLevel);
			}
		}

		static bool equal(
			const std::optional < std::reference_wrapper < const BinaryNode < DataType, true > > > &node1,
			const std::optional < std::reference_wrapper < const BinaryNode < DataType, true > > > &node2)
		{
			if (node1 && node2)
			{
				const auto &_node1 = node1.value().get();
				const auto &_node2 = node2.value().get();

				return
					_node1.value == _node2.value &&

					BinaryTree < DataType > ::equal(_node1.getLeftChild(), _node2.getLeftChild()) &&
					BinaryTree < DataType > ::equal(_node1.getRightChild(), _node2.getRightChild());
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

			path.push_back(0);

			const auto leftChild = node.getLeftChild();

			if (leftChild)
			{
				BinaryTree < DataType > ::find(
					static_cast < node_type & > (leftChild.value().get()),
					value,
					path,
					iterator);
			}

			if (iterator.node.has_value())
			{
				return;
			}

			path.back() = 1;

			const auto rightChild = node.getLeftChild();

			if (rightChild)
			{
				BinaryTree < DataType > ::find(
					static_cast < node_type & > (rightChild.value().get()),
					value,
					path,
					iterator);
			}

			if (iterator.node.has_value())
			{
				return;
			}

			path.pop_back();
		}

		void updateBegin()
		{
			if (this->root)
			{
				std::reference_wrapper < node_type > node = *(this->root.get());

				while (node.get().hasLeftChild())
				{
					node = static_cast < node_type & > (node.get().getLeftChild().value().get());
				}

				this->_begin.node = node.get();
			}

			else
			{
				this->_begin.node.reset();
			}
		}

		void updateLastElement()
		{
			if (this->root)
			{
				std::reference_wrapper < node_type > node = *(this->root.get());

				while (node.get().hasRightChild())
				{
					node = static_cast < node_type & > (node.get().getRightChild().value().get());
				}

				this->_lastElement.node = node.get();
			}

			else
			{
				this->_lastElement.node.reset();
			}
		}

		iterator eraseFindFirstIntersection(const_iterator begin, const_iterator end)
		{
			iterator intersection(std::nullopt, this->_lastElement);
			std::vector < std::uintptr_t > insertedNodes;

			--end;

			const auto find =
				[&insertedNodes] (const const_iterator &iterator) -> bool
				{
					return std::find(
						insertedNodes.cbegin(),
						insertedNodes.cend(),
						reinterpret_cast < std::uintptr_t > (&(iterator.node.value().get()))) != insertedNodes.cend();
				};

			const auto findAndInsert =
				[&find, &insertedNodes, &intersection] (const const_iterator &iterator) -> bool
				{
					if (iterator.node)
					{
						if (find(iterator))
						{
							intersection.node = iterator.node;

							return true;
						}

						insertedNodes.push_back(reinterpret_cast < std::uintptr_t > (&(iterator.node.value().get())));
					}

					return false;
				};

			const auto updateIterator =
				[] (const_iterator &iterator) -> void
				{
					if (iterator.node)
					{
						auto parent = iterator.node.value().get().getParent();

						if (parent)
						{
							iterator.node = static_cast < node_type & > (parent.value().get());
						}

						else
						{
							iterator.node.reset();
						}
					}
				};

			while (true)
			{
				if (findAndInsert(begin))
				{
					return intersection;
				}

				if (findAndInsert(end))
				{
					return intersection;
				}

				if (!begin.node.has_value() && !end.node.has_value())
				{
					throw std::logic_error("Both iterators are from different trees.");
				}

				updateIterator(begin);
				updateIterator(end);
			}
		}

		bool iteratorFromThisTree(const_iterator iterator)
		{
			return this->_lastElement == iterator.lastElement;
		}

		static size_t sizeSubtree(const node_type &binaryTreeNode)
		{
			return
				1 +
				(binaryTreeNode.hasLeftChild() ? BinaryTree < DataType > ::sizeSubtree(binaryTreeNode.getLeftChild().value().get()) : 0) +
				(binaryTreeNode.hasRightChild() ? BinaryTree < DataType > ::sizeSubtree(binaryTreeNode.getRightChild().value().get()) : 0);
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
		bool operator==(const BinaryTree < DataType > &tree) const
		{
			if (this->root && tree.root)
			{
				return BinaryTree < DataType > ::equal(*(this->root.get()), *(tree.root.get()));
			}

			if (!this->root && !this->root)
			{
				return true;
			}

			return false;
		}

		bool operator!=(const BinaryTree < DataType > &tree) const
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

		void swap(BinaryTree < DataType > &tree) noexcept
		{
			std::swap(this->_size, tree._size);
			std::swap(this->root, tree.root);
			std::swap(this->_begin, tree._begin);
			std::swap(this->_lastElement, tree._lastElement);
			std::swap(this->_end, tree._end);
		}

		Allocator get_allocator() const
		{
			return Allocator();
		}

		iterator insertRoot(const value_type &value)
		{
			this->root.reset(new node_type(value));

			this->_begin.node = *(this->root);
			this->_lastElement.node = this->_begin.node;
			this->_size = 1;

			return this->_begin;
		}

		iterator insertRoot(value_type &&value)
		{
			this->root.reset(new node_type(std::move(value)));

			this->_begin.node = *(this->root);
			this->_lastElement.node = this->_begin.node;
			this->_size = 1;

			return this->_begin;
		}

		iterator insertLeft(const_iterator iterator, const value_type &value)
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

				if (node.hasLeftChild())
				{
					const auto &currentLeftChild = static_cast < const node_type & > (node.getLeftChild().value().get());

					this->_size -= typename BinaryTree < DataType > ::sizeSubtree(currentLeftChild);
				}

				auto &newLeftChild = node.insertLeftChild(value);

				typename BinaryTree < DataType > ::iterator leftChildIterator(newLeftChild, this->_lastElement);

				this->updateBegin();
				++(this->_size);

				return leftChildIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insertLeft(const_iterator iterator, value_type &&value)
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

				if (node.hasLeftChild())
				{
					const auto &currentLeftChild = static_cast < const node_type & > (node.getLeftChild().value().get());

					this->_size -= typename BinaryTree < DataType > ::sizeSubtree(currentLeftChild);
				}

				auto &newLeftChild = node.insertLeftChild(std::move(value));

				typename BinaryTree < DataType > ::iterator leftChildIterator(newLeftChild, this->_lastElement);

				this->updateBegin();

				return leftChildIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insertRight(const_iterator iterator, const value_type &value)
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

				if (node.hasRightChild())
				{
					const auto &currentRightChild = static_cast < const node_type & > (node.getRightChild().value().get());

					this->_size -= typename BinaryTree < DataType > ::sizeSubtree(currentRightChild);
				}

				auto &newRightChild = node.insertRightChild(value);

				typename BinaryTree < DataType > ::iterator rightChildIterator(newRightChild, this->_lastElement);

				this->updateLastElement();

				return rightChildIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		iterator insertRight(const_iterator iterator, value_type &&value)
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

				if (node.hasRightChild())
				{
					const auto &currentRightChild = static_cast < const node_type & > (node.getRightChild().value().get());

					this->_size -= typename BinaryTree < DataType > ::sizeSubtree(currentRightChild);
				}

				auto &newRightChild = node.insertRightChild(std::move(value));

				typename BinaryTree < DataType > ::iterator rightChildIterator(newRightChild, this->_lastElement);

				this->updateLastElement();
				++(this->_size);

				return rightChildIterator;
			}

			throw std::logic_error("Attempt of getting the node of an invalid iterator");
		}

		virtual iterator erase(const_iterator iterator)
		{
			if (!this->iteratorFromThisTree(iterator))
			{
				throw std::logic_error("The iterator is not from this tree.");
			}

			if (!(iterator.node))
			{
				throw std::logic_error("Attempt of getting the node of an invalid iterator.");
			}

			if (this->root)
			{
				auto &node = iterator.node.value().get();
				auto parent = node.getParentAndMyPosition();

				if (!parent)
				{
					this->root.reset();

					this->_begin.node.reset();
					this->_lastElement.node.reset();
					this->_size = 0;

					return this->begin();
				}

				typename BinaryTree < DataType > ::iterator returnIterator(
					static_cast < node_type & > (parent.value().first.get()),
					this->_lastElement);

				this->_size -= typename BinaryTree < DataType > ::sizeSubtree(
					static_cast < const node_type & > (*std::get < 1 > (node.extractMyself()).get()));

				this->updateBegin();
				this->updateLastElement();

				if (parent.value().second == 1)
				{
					return ++returnIterator;
				}

				return returnIterator;
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

			auto iterator = BinaryTree < DataType > ::eraseFindFirstIntersection(begin, end);

			return BinaryTree < DataType > ::erase(iterator);
		}

		bool empty() const noexcept override
		{
			return this->root == nullptr;
		}

		bool binary() const noexcept override
		{
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

			BinaryTree::find(*(this->root.get()), value, path, iterator);

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

			BinaryTree < DataType > ::find(*(this->root.get()), value, path, iterator);

			return std::make_pair(std::move(iterator), std::move(path));
		}

		std::vector < std::reference_wrapper < const node_type > > getLeafNodes() const
		{
			std::vector < std::reference_wrapper < const node_type > > leafNodes;

			if (this->root)
			{
				BinaryTree < DataType > ::getLeafNodes(*(this->root.get()), leafNodes);
			}

			return leafNodes;
		}

		std::vector < std::reference_wrapper < const node_type > > getNodes() const
		{
			std::vector < std::reference_wrapper < const node_type > > nodes;

			if (this->root)
			{
				BinaryTree < DataType > ::getNodes(*(this->root.get()), nodes);
			}

			return nodes;
		}

		size_t quantityOfLevels() const override
		{
			size_t quantityOfLevels = 0;

			if (this->root)
			{
				BinaryTree < DataType > ::quantityOfLevels(*(this->root.get()), 0, quantityOfLevels);
			}

			return quantityOfLevels;
		}

		std::vector < size_t > quantityOfNodesByLevel() const override
		{
			std::vector < size_t > quantityOfNodesByLevel;

			if (this->root)
			{
				BinaryTree < DataType > ::quantityOfNodesByLevel(*(this->root.get()), 0, quantityOfNodesByLevel);
			}

			return quantityOfNodesByLevel;
		}

		BinaryTree() = default;

		BinaryTree(const BinaryTree < DataType > &) = default;

		BinaryTree(BinaryTree < DataType > &&) noexcept = default;

		~BinaryTree() override = default;
	};
}

#endif