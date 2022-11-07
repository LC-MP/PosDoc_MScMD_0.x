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

#ifndef _XABLAU_GRAPH_BINARY_TREE_NODE_H_
#define _XABLAU_GRAPH_BINARY_TREE_NODE_H_

#include "BinaryNode.h"

#include <memory>

namespace xablau::graph
{
	template < typename DataType >
	class BinaryTreeNode :
		public BinaryNode < DataType, true >
	{
	private:
		void updateChildren()
		{
			if (this->leftChild)
			{
				this->leftChild->parent = *this;
			}

			if (this->rightChild)
			{
				this->rightChild->parent = *this;
			}
		}

	protected:
		std::optional < std::reference_wrapper < BinaryTreeNode < DataType > > > parent = std::nullopt;
		std::unique_ptr < BinaryTreeNode < DataType > > leftChild = nullptr;
		std::unique_ptr < BinaryTreeNode < DataType > > rightChild = nullptr;

	public:
		BinaryTreeNode < DataType > &insertLeftChild(const DataType &value) override
		{
			this->leftChild.reset(new BinaryTreeNode < DataType > (value));

			this->leftChild->parent = *this;

			return *(this->leftChild);
		}

		BinaryTreeNode < DataType > &insertRightChild(const DataType &value) override
		{
			this->rightChild.reset(new BinaryTreeNode < DataType > (value));

			this->rightChild->parent = *this;

			return *(this->rightChild);
		}

		BinaryTreeNode < DataType > &insertLeftChild(DataType &&value) override
		{
			this->leftChild.reset(new BinaryTreeNode < DataType > (std::move(value)));

			this->leftChild->parent = *this;

			return *(this->leftChild);
		}

		BinaryTreeNode < DataType > &insertRightChild(DataType &&value) override
		{
			this->rightChild.reset(new BinaryTreeNode < DataType > (std::move(value)));

			this->rightChild->parent = *this;

			return *(this->rightChild);
		}

		BinaryTreeNode < DataType > &insertLeftChild(const BinaryNode < DataType, true > &child) override
		{
			this->leftChild.reset(new BinaryTreeNode < DataType > (child));

			this->leftChild->parent = *this;

			return *(this->leftChild);
		}

		BinaryTreeNode < DataType > &insertRightChild(const BinaryNode < DataType, true > &child) override
		{
			this->rightChild.reset(new BinaryTreeNode < DataType > (child));

			this->rightChild->parent = *this;

			return *(this->rightChild);
		}

		BinaryTreeNode < DataType > &insertLeftChild(BinaryNode < DataType, true > &&child) override
		{
			auto extractedNode = std::get < 1 > (child.extractMyself());

			this->leftChild.reset(new BinaryTreeNode < DataType > (std::move(*(extractedNode.get()))));

			this->leftChild->parent = *this;

			return *(this->leftChild);
		}

		BinaryTreeNode < DataType > &insertRightChild(BinaryNode < DataType, true > &&child) override
		{
			auto extractedNode = std::get < 1 > (child.extractMyself());

			this->rightChild.reset(new BinaryTreeNode < DataType > (std::move(*(extractedNode.get()))));

			this->rightChild->parent = *this;

			return *(this->rightChild);
		}

		void eraseLeftChild() override
		{
			this->leftChild.reset();
		}

		void eraseRightChild() override
		{
			this->rightChild.reset();
		}

		[[nodiscard]] size_t size() const noexcept override
		{
			return
				static_cast < size_t > (this->leftChild != nullptr) +
				static_cast < size_t > (this->rightChild != nullptr);
		}

		void clear() noexcept override
		{
			this->eraseLeftChild();
			this->eraseRightChild();
		}

		[[nodiscard]] bool hasLeftChild() const override
		{
			return this->leftChild != nullptr;
		}

		[[nodiscard]] bool hasRightChild() const override
		{
			return this->rightChild != nullptr;
		}

		[[nodiscard]] bool hasParent() const override
		{
			return this->parent.has_value();
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const BinaryNode < DataType, true > > > getLeftChild() const override
		{
			if (this->leftChild)
			{
				return *(this->leftChild.get());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const BinaryNode < DataType, true > > > getRightChild() const override
		{
			if (this->rightChild)
			{
				return *(this->rightChild.get());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < const BinaryNode < DataType, true > > > getParent() const override
		{
			if (this->parent)
			{
				return this->parent.value().get();
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional <
			std::pair < std::reference_wrapper < const BinaryNode < DataType, true > >, size_t > > getParentAndMyPosition() const override
		{
			if (this->parent)
			{
				std::pair < std::reference_wrapper < const BinaryNode < DataType, true > >, size_t > pair(
					this->parent.value().get(),
					0);

				auto rightChild = pair.first.get().getRightChild();

				if (rightChild && &(rightChild.value().get()) == this)
				{
					pair.second = 1;
				}

				return pair;
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < BinaryNode < DataType, true > > > getLeftChild() override
		{
			if (this->leftChild)
			{
				return *(this->leftChild.get());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < BinaryNode < DataType, true > > > getRightChild() override
		{
			if (this->rightChild)
			{
				return *(this->rightChild.get());
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional < std::reference_wrapper < BinaryNode < DataType, true > > > getParent() override
		{
			if (this->parent)
			{
				return this->parent.value().get();
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional <
			std::pair < std::reference_wrapper < BinaryNode < DataType, true > >, size_t > > getParentAndMyPosition() override
		{
			if (this->parent)
			{
				std::pair < std::reference_wrapper < BinaryNode < DataType, true > >, size_t > pair(
					this->parent.value().get(),
					0);

				auto rightChild = pair.first.get().getRightChild();

				if (rightChild && &(rightChild.value().get()) == this)
				{
					pair.second = 1;
				}

				return pair;
			}

			return std::nullopt;
		}

		std::tuple < std::optional < size_t >, std::shared_ptr < BinaryNode < DataType, true > > > extractMyself() override
		{
			if (this->parent)
			{
				auto &parent = this->parent.value().get();
				auto leftChild = parent.getLeftChild();

				if (leftChild && &(leftChild.value().get()) == this)
				{
					return std::make_tuple(0, parent.extractLeftChild());
				}

				return std::make_tuple(1, parent.extractRightChild());
			}

			return std::make_tuple(
				std::nullopt,
				std::shared_ptr < BinaryNode < DataType, true > > (
					new BinaryTreeNode < DataType > (std::move(*this))));
		}

		std::shared_ptr < BinaryNode < DataType, true > > extractLeftChild() override
		{
			if (this->leftChild)
			{
				this->leftChild->parent.reset();

				std::shared_ptr < BinaryNode < DataType, true > > pointer(this->leftChild.release());

				return pointer;
			}

			return nullptr;
		}

		std::shared_ptr < BinaryNode < DataType, true > > extractRightChild() override
		{
			if (this->rightChild)
			{
				this->rightChild->parent.reset();

				std::shared_ptr < BinaryNode < DataType, true > > pointer(this->rightChild.release());

				return pointer;
			}

			return nullptr;
		}

		BinaryTreeNode < DataType > &operator=(const BinaryTreeNode < DataType > &node)
		{
			this->value = node.value;

			this->leftChild.reset(node.leftChild ? new BinaryTreeNode < DataType > (*(node.leftChild.get())) : nullptr);
			this->rightChild.reset(node.rightChild ? new BinaryTreeNode < DataType > (*(node.rightChild.get())) : nullptr);

			this->updateChildren();

			return *this;
		}

		BinaryTreeNode < DataType > &operator=(BinaryTreeNode < DataType > &&node) noexcept
		{
			this->value = std::move(node.value);

			this->leftChild = std::move(node.leftChild);
			this->rightChild = std::move(node.rightChild);

			this->updateChildren();

			node.extractMyself();

			return *this;
		}

		BinaryTreeNode() = default;

		explicit BinaryTreeNode(const DataType &data) :
			BinaryNode < DataType, true > (data)
		{
		}

		explicit BinaryTreeNode(DataType &&data) :
			BinaryNode < DataType, true > (std::move(data))
		{
		}

		BinaryTreeNode(const BinaryNode < DataType, true > &node) :
			BinaryNode < DataType, true > (node.value),

			leftChild(node.hasLeftChild() ? new BinaryTreeNode < DataType > (node.getLeftChild().value().get()) : nullptr),
			rightChild(node.hasRightChild() ? new BinaryTreeNode < DataType > (node.getRightChild().value().get()) : nullptr)
		{
			this->updateChildren();
		}

		BinaryTreeNode(BinaryNode < DataType, true > &&node) noexcept :
			BinaryNode < DataType, true > (std::move(node.value))
		{
			auto _node = dynamic_cast < BinaryTreeNode < DataType > * > (&node);

			if (_node)
			{
				this->leftChild = std::move(_node->leftChild);
				this->rightChild = std::move(_node->rightChild);
			}

			else
			{
				if (node.hasLeftChild())
				{
					this->leftChild.reset(new BinaryTreeNode < DataType > (std::move(node.getLeftChild().value().get())));
				}

				if (node.hasRightChild())
				{
					this->rightChild.reset(new BinaryTreeNode < DataType > (std::move(node.getRightChild().value().get())));
				}
			}

			this->updateChildren();

			node.extractMyself();
		}

		BinaryTreeNode(const BinaryTreeNode < DataType > &node) :
			BinaryNode < DataType, true > (node.value),

			leftChild(node.leftChild ? new BinaryTreeNode < DataType > (*(node.leftChild.get())) : nullptr),
			rightChild(node.rightChild ? new BinaryTreeNode < DataType > (*(node.rightChild.get())) : nullptr)
		{
			this->updateChildren();
		}

		BinaryTreeNode(BinaryTreeNode < DataType > &&node) noexcept :
			BinaryNode < DataType, true > (std::move(node.value)),

			leftChild(std::move(node.leftChild)),
			rightChild(std::move(node.rightChild))
		{
			this->updateChildren();

			node.extractMyself();
		}

		~BinaryTreeNode() noexcept override = default;
	};
}

#endif