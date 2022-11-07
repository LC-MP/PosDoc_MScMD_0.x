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

#ifndef _XABLAU_GRAPH_ABSTRACT_GRAPH_H_
#define _XABLAU_GRAPH_ABSTRACT_GRAPH_H_

#include <functional>
#include <map>
#include <optional>
#include <type_traits>

namespace xablau::graph
{
	template < typename DataType >
	class Connection
	{
	public:
		DataType weight;

		virtual bool operator==(const Connection < DataType > &connection) const noexcept
		{
			return this->weight == connection.weight;
		}

		virtual bool operator!=(const Connection < DataType > &connection) const noexcept
		{
			return this->weight != connection.weight;
		}

		friend std::ostream &operator<<(std::ostream &stream, const Connection < DataType > &connection)
		{
			stream << connection.weight;

			return stream;
		}

		Connection < DataType > &operator=(const Connection < DataType > &) = default;

		template < typename ConnectionDataType >
		Connection < DataType > &operator=(const Connection < ConnectionDataType > &connection)
		{
			this->weight = DataType(connection.weight);

			return *this;
		}

		Connection < DataType > &operator=(Connection < DataType > &&) noexcept = default;

		Connection() :
			weight(DataType(1))
		{
		}

		Connection(const Connection < DataType > &) = default;

		template < typename ConnectionDataType >
		Connection(const Connection < ConnectionDataType > &connection) :
			weight(DataType(connection.weight))
		{
		}

		Connection(Connection < DataType > &&) noexcept = default;

		Connection(const DataType &weight) :
			weight(weight)
		{
		}

		virtual ~Connection() noexcept = default;
	};

	template < typename ConnectionWeightDataType, typename NodeDataType, typename ConnectionDataType >
	requires
		std::is_floating_point < ConnectionWeightDataType > ::value &&
		std::is_base_of < Connection < ConnectionWeightDataType >, ConnectionDataType > ::value
	class AbstractGraph
	{
	public:
		virtual ConnectionWeightDataType degree(const NodeDataType &node) const = 0;

		virtual ConnectionWeightDataType averageDegree() const = 0;

		virtual ConnectionWeightDataType density() const = 0;

		virtual ConnectionWeightDataType degreeDistribution(const ConnectionWeightDataType degree) const = 0;

		virtual std::map < ConnectionWeightDataType, ConnectionWeightDataType > degreeDistribution() const = 0;

		virtual ConnectionWeightDataType statisticalMoment(const size_t order) const = 0;

		virtual ConnectionWeightDataType variance() const = 0;

		virtual ConnectionWeightDataType volume() const = 0;

		ConnectionWeightDataType complexityMeasure() const
		{
			return this->statisticalMoment(2) / this->statisticalMoment(1);
		}

		ConnectionWeightDataType ShannonEntropy() const
		{
			std::map < ConnectionWeightDataType, ConnectionWeightDataType > degreeDistribution = this->degreeDistribution();

			if (degreeDistribution.empty())
			{
				return ConnectionWeightDataType();
			}

			ConnectionWeightDataType acumulator = ConnectionWeightDataType();

			for (const auto &item : degreeDistribution)
			{
				acumulator += item.second * (item.second != ConnectionWeightDataType() ? std::log2(item.second) : ConnectionWeightDataType());
			}

			return -acumulator;
		}

		virtual bool connected() const = 0;

		virtual bool empty() const noexcept = 0;

		virtual size_t getNodesQuantity() const = 0;

		virtual size_t size() const noexcept = 0;

		virtual void clear() noexcept = 0;

		virtual bool insert(const NodeDataType &node) = 0;

		virtual std::optional < std::reference_wrapper < ConnectionDataType > >
			insert(const NodeDataType &node1, const NodeDataType &node2, const ConnectionDataType &connection) = 0;

		virtual bool erase(const NodeDataType &node) = 0;

		virtual bool erase(const NodeDataType &node1, const NodeDataType &node2) = 0;

		virtual bool contains(const NodeDataType &node) const = 0;

		virtual bool contains(const NodeDataType &node1, const NodeDataType &node2) const = 0;

		virtual std::optional < std::reference_wrapper < ConnectionDataType > >
			getConnection(const NodeDataType &node1, const NodeDataType &node2) = 0;

		virtual std::optional < std::reference_wrapper < const ConnectionDataType > >
			getConnection(const NodeDataType &node1, const NodeDataType &node2) const = 0;

		virtual size_t getConnectionsQuantity() const noexcept = 0;

		virtual size_t getConnectionsQuantity(const NodeDataType &node) const = 0;

		virtual ConnectionWeightDataType getWeight(const NodeDataType &node1, const NodeDataType &node2) const = 0;

		AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &operator=(
			const AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &) = default;

		AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &operator=(
			AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &&) noexcept = default;

		AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > () = default;

		AbstractGraph(const AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &) = default;

		AbstractGraph(AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &&) noexcept = default;

		virtual ~AbstractGraph() noexcept = default;
	};
}

#endif