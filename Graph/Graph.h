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

#ifndef _XABLAU_GRAPH_GRAPH_H_
#define _XABLAU_GRAPH_GRAPH_H_

#include <bit>
#include <climits>
#include <execution>
#include <mutex>
#include <random>
#include <set>
#include <thread>
#include <unordered_set>
#include <eigen3/Eigen/Eigenvalues>
#include <eigen3/Eigen/Sparse>

#include "AbstractGraph.h"

template < class _Ty >
[[nodiscard]] constexpr _Ty rotr(_Ty _Val, int _Rotation) noexcept;

template <class _Ty>
[[nodiscard]] constexpr _Ty rotl(const _Ty _Val, const int _Rotation) noexcept
{
	constexpr auto _Digits = std::numeric_limits<_Ty>::digits;
	const auto _Remainder = _Rotation % _Digits;

	if (_Remainder > 0) {
		return static_cast<_Ty>(
			static_cast<_Ty>(_Val << _Remainder) | static_cast<_Ty>(_Val >> (_Digits - _Remainder)));
	}

	if (_Remainder == 0)
	{
		return _Val;
	}

	return rotr(_Val, -_Remainder);
}

template < class _Ty >
[[nodiscard]] constexpr _Ty rotr(const _Ty _Val, const int _Rotation) noexcept
{
	constexpr auto _Digits = std::numeric_limits<_Ty>::digits;
	const auto _Remainder = _Rotation % _Digits;

	if (_Remainder > 0) {
		return static_cast<_Ty>(
			static_cast<_Ty>(_Val >> _Remainder) | static_cast<_Ty>(_Val << (_Digits - _Remainder)));
	}

	if (_Remainder == 0)
	{
		return _Val;
	}

	return rotl(_Val, -_Remainder);
}

namespace xablau::graph
{
	template <
		typename ConnectionWeightDataType,
		typename NodeDataType,
		typename ConnectionDataType = Connection < ConnectionWeightDataType > >
	class Graph :
		public AbstractGraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType >
	{
	private:
		enum class DijkstraModes
		{
			AllPreviousAndNextNodes,
			AllPreviousAndNextNodesAndDistance,
			SinglePreviousAndNextNode,
			SinglePreviousAndNextNodeAndDistance,
			SinglePath
		};

		using EigenDataType = typename std::conditional < sizeof(ConnectionWeightDataType) <= sizeof(float), float, double > ::type;

		struct PairHash
		{
			size_t operator()(const std::pair < NodeDataType, NodeDataType > &pair) const
			{
				return
					std::hash < NodeDataType > ()(pair.first) ^
					/*std::*/rotl(std::hash < NodeDataType > ()(pair.second), 1);
			}
		};

		static size_t tripleSum(const size_t number) noexcept
		{
			return (number + 1) * (number / 2) + (number % 2 ? (number + 1) / 2 : 0);
		}

		template < DijkstraModes mode, typename Conteiner >
		void genericDijkstra(
			const NodeDataType &origin,
			Conteiner &previous,
			const NodeDataType &destiny = NodeDataType(),
			std::optional < std::reference_wrapper < ConnectionWeightDataType > > smallestDistance = std::nullopt) const
		{
			if constexpr (mode == DijkstraModes::SinglePath)
			{
				smallestDistance.value().get() = std::numeric_limits < ConnectionWeightDataType > ::max();
			}

			if (this->empty())
			{
				return;
			}

			NodeDataType searchIndex = origin;
			std::vector < std::pair < NodeDataType, ConnectionWeightDataType > > distances;

			distances.reserve(this->size());

			constexpr auto orderingFunction =
				[] (const std::pair < NodeDataType, ConnectionWeightDataType > &a,
					const std::pair < NodeDataType, ConnectionWeightDataType > &b) -> bool
				{
					return a.second < b.second;
				};

			const auto searchFunction =
				[&searchIndex] (const std::pair < NodeDataType, ConnectionWeightDataType > &a) -> bool
				{
					return a.first == searchIndex;
				};

			for (const auto &item : this->digraph)
			{
				distances.emplace_back(std::make_pair(
					item.first, static_cast < ConnectionWeightDataType > (std::numeric_limits < ConnectionWeightDataType > ::max())));
			}

			std::find_if(distances.begin(), distances.end(), searchFunction)->second = ConnectionWeightDataType();

			previous.reserve(this->size() * this->expectedConnectionsByNode);

			if constexpr (mode == DijkstraModes::AllPreviousAndNextNodes)
			{
				previous.insert(std::make_pair(origin, std::unordered_set < NodeDataType > ()));
			}

			else if constexpr (mode == DijkstraModes::AllPreviousAndNextNodesAndDistance)
			{
				previous.insert(std::make_pair(origin,
					std::make_pair(std::unordered_set < NodeDataType > (), ConnectionWeightDataType())));
			}

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sort(distances.rbegin(), distances.rend(), orderingFunction);

				const std::pair < NodeDataType, ConnectionWeightDataType > &bestNode = distances.back();

				if (bestNode.second == std::numeric_limits < ConnectionWeightDataType > ::max())
				{
					break;
				}

				for (const auto &neighbour : this->digraph.at(bestNode.first))
				{
					searchIndex = neighbour.first;
					const ConnectionWeightDataType currentDistance = distances.back().second + neighbour.second.weight;
					const auto search = std::find_if(distances.begin(), distances.end(), searchFunction);

					if constexpr (
						mode == DijkstraModes::SinglePreviousAndNextNode ||
						mode == DijkstraModes::SinglePreviousAndNextNodeAndDistance ||
						mode == DijkstraModes::SinglePath)
					{
						if (search != distances.end() && currentDistance < search->second)
						{
							search->second = currentDistance;

							if constexpr (mode == DijkstraModes::SinglePreviousAndNextNodeAndDistance)
							{
								previous[neighbour.first] = std::make_pair(bestNode.first, currentDistance);
							}

							else
							{
								previous[neighbour.first] = bestNode.first;
							}
						}
					}

					else
					{
						if (search != distances.end())
						{
							if (currentDistance < search->second)
							{
								search->second = currentDistance;

								if constexpr (mode == DijkstraModes::AllPreviousAndNextNodes)
								{
									(previous[neighbour.first] = std::unordered_set < NodeDataType > ()).insert(bestNode.first);
								}

								else if constexpr (mode == DijkstraModes::AllPreviousAndNextNodesAndDistance)
								{
									(previous[neighbour.first] = std::make_pair(std::unordered_set < NodeDataType > (), currentDistance)).
										first.insert(bestNode.first);
								}
							}

							else if (currentDistance == search->second)
							{
								if constexpr (mode == DijkstraModes::AllPreviousAndNextNodes)
								{
									previous[neighbour.first].insert(bestNode.first);
								}

								else if constexpr (mode == DijkstraModes::AllPreviousAndNextNodesAndDistance)
								{
									previous[neighbour.first].first.insert(bestNode.first);
								}
							}
						}
					}
				}

				if constexpr (mode == DijkstraModes::SinglePath)
				{
					if (bestNode.first == destiny)
					{
						smallestDistance.value().get() = bestNode.second;

						break;
					}
				}

				distances.pop_back();
			}
		}

		template < DijkstraModes mode, typename Conteiner1, typename Conteiner2 >
		static void DijkstraCreateNext(Conteiner1 &previous, Conteiner2 &next)
		{
			if constexpr (mode == DijkstraModes::AllPreviousAndNextNodes)
			{
				for (const auto &[node1, otherNodes] : previous)
				{
					for (const auto &node2 : otherNodes)
					{
						next[node2].insert(node1);
					}
				}
			}

			else if constexpr (mode == DijkstraModes::AllPreviousAndNextNodesAndDistance)
			{
				for (const auto &[node1, otherNodesAndWeight] : previous)
				{
					for (const auto &node2 : otherNodesAndWeight.first)
					{
						next[node2][node1] = otherNodesAndWeight.second;
					}
				}
			}

			else if constexpr (mode == DijkstraModes::SinglePreviousAndNextNode)
			{
				for (const auto &[node1, node2] : previous)
				{
					next[node2].insert(node1);
				}
			}

			else if constexpr (mode == DijkstraModes::SinglePreviousAndNextNodeAndDistance)
			{
				for (const auto &[node1, node2AndWeight] : previous)
				{
					next[node2AndWeight.first][node1] = node2AndWeight.second;
				}
			}
		}

		std::unordered_set < NodeDataType > connectedNodes(const NodeDataType &node) const
		{
			std::queue < NodeDataType > queue;
			std::unordered_set < NodeDataType > connectedNodes;

			queue.push(node);
			connectedNodes.insert(queue.front());

			while (!queue.empty())
			{
				for (const auto &item : this->graph.at(queue.front()))
				{
					if (connectedNodes.insert(item.first).second)
					{
						queue.push(item.first);
					}
				}

				queue.pop();
			}

			return connectedNodes;
		}

	protected:
		ConnectionWeightDataType weightSum = ConnectionWeightDataType();
		std::unordered_map < NodeDataType, std::unordered_map < NodeDataType, ConnectionDataType > > graph;
		std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > transposedGraph;
		std::unordered_map < NodeDataType, ConnectionWeightDataType > degrees;

		size_t expectedConnectionsByNode = 0;

	public:
		bool operator==(const Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &graph) const
		{
			return this->graph == graph.graph;
		}

		bool operator!=(const Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &graph) const
		{
			return this->graph != graph.graph;
		}

		ConnectionWeightDataType degree(const NodeDataType &node) const override
		{
			const auto degreeIterator = this->degrees.find(node);

			if (degreeIterator != this->degrees.cend())
			{
				const auto &connections = this->graph.at(node);
				const auto graphIterator = connections.find(node);

				return degreeIterator->second + (graphIterator != connections.cend() ? graphIterator->second.weight : ConnectionWeightDataType());
			}

			return ConnectionWeightDataType();
		}

		ConnectionWeightDataType averageDegree() const override
		{
			const size_t nodesQuantity = this->size();

			if (nodesQuantity == 0)
			{
				throw std::domain_error("There are no nodes in the graph.");
			}

			return this->weightSum / ConnectionWeightDataType(nodesQuantity);
		}

		ConnectionWeightDataType density() const override
		{
			const size_t nodesQuantity = this->graph.size();

			if (nodesQuantity < 2)
			{
				throw std::domain_error("There are less than two nodes in the graph.");
			}

			return this->weightSum / ConnectionWeightDataType(nodesQuantity * (nodesQuantity - 1));
		}

		ConnectionWeightDataType degreeDistribution(const ConnectionWeightDataType degree) const override
		{
			if (this->graph.empty())
			{
				throw std::domain_error("There are no nodes in the graph.");
			}

			ConnectionWeightDataType probability = ConnectionWeightDataType();

			for (const auto &item : this->graph)
			{
				if (degree == this->degree(item.first))
				{
					++probability;
				}
			}

			return probability / ConnectionWeightDataType(this->size());
		}

		std::map < ConnectionWeightDataType, ConnectionWeightDataType > degreeDistribution() const override
		{
			if (this->graph.empty())
			{
				throw std::domain_error("There are no nodes in the graph.");
			}

			std::map < ConnectionWeightDataType, ConnectionWeightDataType > probabilities;

			for (const auto &item : this->graph)
			{
				++((probabilities.emplace(std::make_pair(this->degree(item.first), ConnectionWeightDataType()))).first->second);
			}

			for (auto &item : probabilities)
			{
				item.second /= ConnectionWeightDataType(this->graph.size());
			}

			return probabilities;
		}

		ConnectionWeightDataType statisticalMoment(const size_t order) const override
		{
			ConnectionWeightDataType result = ConnectionWeightDataType();
			std::map < ConnectionWeightDataType, ConnectionWeightDataType > degreeDistribution = this->degreeDistribution();

			for (const auto &item : degreeDistribution)
			{
				result += std::pow(item.first, ConnectionWeightDataType(order)) * item.second;
			}

			return result;
		}

		ConnectionWeightDataType variance() const override
		{
			return this->statisticalMoment(2) -
				std::pow(this->statisticalMoment(1), ConnectionWeightDataType(2));
		}

		ConnectionWeightDataType volume() const noexcept override
		{
			return this->weightSum / ConnectionWeightDataType(2);
		}

		size_t getTrianglesQuantity() const
		{
			size_t trianglesQuantity = 0;

			for (const auto &item : this->graph)
			{
				const auto &connections = item.second;

				if (connections.size() >= 2)
				{
					for (auto i = connections.begin(), j = ++(connections.begin());
						j != connections.end();
						j = ++i, ++j)
					{
						if (i->first == item.first)
						{
							continue;
						}

						if (j->first == item.first)
						{
							++j;
						}

						while (j != connections.end())
						{
							if (this->graph.at(i->first).count(j->first))
							{
								trianglesQuantity++;
							}

							++j;
						}
					}
				}
			}

			return trianglesQuantity / 3;
		}

		size_t getTriplesQuantity() const
		{
			size_t triplesQuantity = 0;

			for (const auto &item : this->graph)
			{
				const NodeDataType &node = item.first;

				const size_t uniqueConnectionsQuantity =
					this->getConnectionsQuantity(node) - (this->graph.at(node).contains(node) ? 1 : 0);

				if (uniqueConnectionsQuantity)
				{
					triplesQuantity += this->tripleSum(uniqueConnectionsQuantity - 1);
				}
			}

			return triplesQuantity;
		}

		// Local analysis. Works only on weightless graphs.
		ConnectionWeightDataType clusteringCoefficient(const NodeDataType &node) const noexcept
		{
			const size_t nodesQuantity = this->getConnectionsQuantity(node);

			if (nodesQuantity < 2)
			{
				return ConnectionWeightDataType();
			}

			size_t trianglesQuantity = 0;

			const auto iterator = this->graph.find(node);

			if (iterator != this->graph.end() && iterator->second.size() >= 2)
			{
				for (auto i = iterator->second.begin(), j = ++iterator->second.begin();
					j != iterator->second.end();
					j = ++i, ++j)
				{
					if (i->first == iterator->first)
					{
						continue;
					}

					if (j->first == iterator->first)
					{
						++j;
					}

					while (j != iterator->second.end())
					{
						if (this->graph.at(i->first).count(j->first))
						{
							trianglesQuantity++;
						}

						++j;
					}
				}
			}

			return (trianglesQuantity * ConnectionWeightDataType(2)) / (nodesQuantity * (nodesQuantity - 1));
		}

		// Global analysis. Works only on weightless graphs.
		ConnectionWeightDataType clusteringCoefficient() const noexcept
		{
			size_t trianglesQuantity{};
			size_t triplesQuantity{};

			std::thread threadQuantidadeTriangulos(
				[this, &trianglesQuantity] () -> void
				{
					trianglesQuantity = this->getTrianglesQuantity();
				});

			std::thread threadQuantidadeTriplas(
				[this, &triplesQuantity] () -> void
				{
					triplesQuantity = this->getTriplesQuantity();
				});

			threadQuantidadeTriangulos.join();
			threadQuantidadeTriplas.join();

			if (triplesQuantity)
			{
				return (ConnectionWeightDataType(3) * ConnectionWeightDataType(trianglesQuantity)) /
					ConnectionWeightDataType(triplesQuantity);
			}

			return ConnectionWeightDataType();
		}

		// Global analysis. Works only on weightless graphs.
		ConnectionWeightDataType averageClusteringCoefficient() const noexcept
		{
			if (this->graph.empty())
			{
				return ConnectionWeightDataType();
			}

			ConnectionWeightDataType averageCoefficient = ConnectionWeightDataType();

			for (const auto &item : this->graph)
			{
				averageCoefficient += this->clusteringCoefficient(item.first);
			}

			return averageCoefficient / ConnectionWeightDataType(this->size());
		}

		// https://journals.aps.org/pre/pdf/10.1103/PhysRevE.69.026113
		// Weights cannot be negative.
		template < typename CommunityDataType >
		ConnectionWeightDataType modularity(
			const std::unordered_map < CommunityDataType, std::unordered_set < NodeDataType > > &communitiesClassification) const
		{
			if (this->weightSum == ConnectionWeightDataType())
			{
				throw std::domain_error("There are no connections in the graph.");
			}

			std::unordered_set < NodeDataType > insertedNodes;

			for (const auto &item1 : communitiesClassification)
			{
				for (const auto &item2 : item1.second)
				{
					if (this->graph.contains(item2))
					{
						throw std::runtime_error("There is a node in \"communitiesClassification\" not present in the graph.");
					}

					insertedNodes.insert(item2);
				}
			}

			if (insertedNodes.size() != this->graph.size())
			{
				throw std::runtime_error("Not every node in the graph is present in \"communitiesClassification\".");
			}

			// this->weightSum is already multiplied by two.
			const ConnectionWeightDataType inverseOfTwoTimesOfWeightSum = ConnectionWeightDataType(1) / this->weightSum;
			ConnectionWeightDataType sum = ConnectionWeightDataType();
			std::mutex mutex;

			std::for_each(std::execution::par, communitiesClassification.cbegin(), communitiesClassification.cend(),
				[this, &sum, &mutex, inverseOfTwoTimesOfWeightSum] (
					const std::pair < CommunityDataType, std::unordered_set < NodeDataType > > &pair) noexcept -> void
				{
					ConnectionWeightDataType weightSum = ConnectionWeightDataType();
					ConnectionWeightDataType degreeSum = ConnectionWeightDataType();

					for (const auto &item1 : pair.second)
					{
						degreeSum += this->degree(item1);

						for (const auto &item2 : pair.second)
						{
							weightSum += this->getWeight(item1, item2);
						}
					}

					weightSum *= inverseOfTwoTimesOfWeightSum;
					degreeSum *= inverseOfTwoTimesOfWeightSum;

					std::unique_lock < std::mutex > lock(mutex);

					sum += weightSum - degreeSum * degreeSum;
				});

			return sum;
		}

		template <
			bool allowAutoConnections,
			bool sparseMatrix,
			bool sortMatrixIndices = false,
			typename MatrixDataType = EigenDataType,
			Eigen::StorageOptions MatrixMajor = Eigen::StorageOptions::RowMajor,
			bool useWeightInsteadOfConnection = true >
		auto adjacencyMatrix(
			std::optional < std::reference_wrapper < std::unordered_map < NodeDataType, Eigen::Index > > > graphIndexToMatrixIndex = std::nullopt,
			std::optional < std::reference_wrapper < std::unordered_map < Eigen::Index, NodeDataType > > > matrixIndexToGraphIndex = std::nullopt) const
		{
			size_t counter = 0;
			const size_t matrixSize = this->size();

			using MatrixType =
				typename std::conditional < sparseMatrix,
					Eigen::SparseMatrix < MatrixDataType, MatrixMajor, std::ptrdiff_t >,
					Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > > ::type;

			MatrixType _adjacencyMatrix{};

			if constexpr (sparseMatrix)
			{
				_adjacencyMatrix.reserve(this->expectedConnectionsByNode * this->getNodesQuantity());
			}

			else
			{
				_adjacencyMatrix.setConstant(matrixSize, matrixSize, MatrixDataType());
			}

			std::unordered_map < NodeDataType, Eigen::Index > _graphIndexToMatrixIndex(matrixSize);
			std::unordered_map < Eigen::Index, NodeDataType > _matrixIndexToGraphIndex(matrixSize);

			if constexpr (sortMatrixIndices)
			{
				std::set < NodeDataType > matrixIndices;

				for (const auto &item : this->graph)
				{
					matrixIndices.insert(item.first);
				}

				for (const auto item : matrixIndices)
				{
					_graphIndexToMatrixIndex.insert(std::pair < NodeDataType, Eigen::Index > (item, counter));
					_matrixIndexToGraphIndex.insert(std::pair < Eigen::Index, NodeDataType > (counter, item));

					counter++;
				}
			}

			else
			{
				for (const auto &item : this->graph)
				{
					_graphIndexToMatrixIndex.insert(std::pair < NodeDataType, Eigen::Index > (item.first, counter));
					_matrixIndexToGraphIndex.insert(std::pair < Eigen::Index, NodeDataType > (counter, item.first));

					counter++;
				}
			}

			std::for_each(this->graph.cbegin(), this->graph.cend(),
				[this, &_adjacencyMatrix, &_graphIndexToMatrixIndex] (
					const std::pair < NodeDataType, std::unordered_map < NodeDataType, ConnectionDataType > > &node1) -> void
				{
					const size_t rowIndex = _graphIndexToMatrixIndex.at(node1.first);

					for (const auto &node2 : node1.second)
					{
						const size_t columnIndex = _graphIndexToMatrixIndex.at(node2.first);

						if constexpr (sparseMatrix)
						{
							if constexpr (useWeightInsteadOfConnection)
							{
								_adjacencyMatrix.insert(rowIndex, columnIndex) = MatrixDataType(node2.second.weight);
							}

							else
							{
								_adjacencyMatrix.insert(rowIndex, columnIndex) = MatrixDataType(node2.second);
							}
						}

						else
						{
							if constexpr (useWeightInsteadOfConnection)
							{
								_adjacencyMatrix(rowIndex, columnIndex) = MatrixDataType(node2.second.weight);
							}

							else
							{
								_adjacencyMatrix(rowIndex, columnIndex) = MatrixDataType(node2.second);
							}
						}
					}
				});

			if constexpr (allowAutoConnections)
			{
				_adjacencyMatrix.diagonal() *= MatrixDataType(2);
			}

			else
			{
				_adjacencyMatrix.diagonal().setConstant(MatrixDataType());
			}

			if (graphIndexToMatrixIndex)
			{
				graphIndexToMatrixIndex.value().get() = std::move(_graphIndexToMatrixIndex);
			}

			if (matrixIndexToGraphIndex)
			{
				matrixIndexToGraphIndex.value().get() = std::move(_matrixIndexToGraphIndex);
			}

			return _adjacencyMatrix;
		}

		template <
			bool allowAutoConnections,
			bool sparseMatrix,
			bool sortMatrixIndices = false,
			typename MatrixDataType = EigenDataType,
			Eigen::StorageOptions MatrixMajor = Eigen::StorageOptions::RowMajor >
		auto laplacianMatrix(
			std::optional < std::reference_wrapper < std::unordered_map < NodeDataType, size_t > > > graphIndexToMatrixIndex = std::nullopt,
			std::optional < std::reference_wrapper < std::unordered_map < size_t, NodeDataType > > > matrixIndexToGraphIndex = std::nullopt) const
		{
			std::unordered_map < NodeDataType, size_t > _graphIndexToMatrixIndex;
			std::unordered_map < size_t, NodeDataType > _matrixIndexToGraphIndex;

			auto _laplacianMatrix = this->adjacencyMatrix < allowAutoConnections, sparseMatrix, sortMatrixIndices, MatrixDataType, MatrixMajor > (
				_graphIndexToMatrixIndex, _matrixIndexToGraphIndex);

			auto diagonalMatrix = _laplacianMatrix.diagonal();

			std::for_each(_matrixIndexToGraphIndex.begin(), _matrixIndexToGraphIndex.end(),
				[this, &diagonalMatrix](const std::pair < size_t, NodeDataType > &pair) -> void
				{
					std::reference_wrapper < MatrixDataType > _diagonalElement;

					if constexpr (sparseMatrix)
					{
						_diagonalElement = diagonalMatrix.coeffRef(pair.first);
					}

					else
					{
						_diagonalElement = diagonalMatrix(pair.first);
					}
					
					auto &diagonalElement = _diagonalElement.get();

					diagonalElement -= this->degrees.at(pair.second);

					if constexpr (!allowAutoConnections)
					{
						diagonalElement -= 2 * this->getWeight(pair.second, pair.second);
					}
				});

			_laplacianMatrix *= MatrixDataType(-1);

			if (graphIndexToMatrixIndex)
			{
				graphIndexToMatrixIndex.value().get() = std::move(_graphIndexToMatrixIndex);
			}

			if (matrixIndexToGraphIndex)
			{
				matrixIndexToGraphIndex.value().get() = std::move(_matrixIndexToGraphIndex);
			}

			return _laplacianMatrix;
		}

		// https://pdfs.semanticscholar.org/a339/cd39d351147ffa00234a524b4f87e9fc46e8.pdf
		template < typename MatrixDataType = EigenDataType, Eigen::StorageOptions MatrixMajor >
		void mutualInformation(
			Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > &conditionalProbabilityDistribution,
			Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > &normalizedJointDistribution,
			ConnectionWeightDataType &entropyX,
			ConnectionWeightDataType &entropyY,
			ConnectionWeightDataType &mutualInformation)
		{
			if (this->empty())
			{
				conditionalProbabilityDistribution.setZero();
				normalizedJointDistribution.setZero();
				entropyX = 0;
				entropyY = 0;
				mutualInformation = 0;

				return;
			}

			if (!this->connected())
			{
				throw std::domain_error("The graph is not connected.");
			}

			ConnectionWeightDataType accumulator = ConnectionWeightDataType();
			std::unordered_map < size_t, NodeDataType > matrixIndexToGraphIndex;
			std::unordered_map < NodeDataType, size_t > graphIndexToMatrixIndex;

			auto laplacianMatrix =
				this->laplacianMatrix < false, false, false, MatrixDataType, MatrixMajor > (graphIndexToMatrixIndex, matrixIndexToGraphIndex);

			std::vector < ConnectionWeightDataType > normalizedDegrees(laplacianMatrix.rows());

			for (size_t i = 0; i < normalizedDegrees.size(); i++)
			{
				const NodeDataType &nodeIndex = matrixIndexToGraphIndex.at(i);
				const ConnectionWeightDataType degree = this->degrees.at(nodeIndex) - this->getWeight(nodeIndex, nodeIndex);

				normalizedDegrees[i] = degree;
				accumulator += degree;
			}

			std::for_each(normalizedDegrees.begin(), normalizedDegrees.end(),
				[accumulator] (ConnectionWeightDataType &normalizedDegree) -> void
				{
					normalizedDegree /= accumulator;
					normalizedDegree = ConnectionWeightDataType(1) / normalizedDegree;
				});

			for (size_t i = 0; i < laplacianMatrix.rows(); i++)
			{
				for (size_t j = 0; j < laplacianMatrix.cols(); j++)
				{
					laplacianMatrix(i, j) *= -normalizedDegrees[j];
				}
			}

			Eigen::EigenSolver < Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > > solver;

			solver.compute(laplacianMatrix, false);

			MatrixDataType smallest = std::numeric_limits < MatrixDataType > ::max();
			MatrixDataType secondSmallest = std::numeric_limits < MatrixDataType > ::max();

			for (long long int i = 0; i < solver.eigenvalues().size(); i++)
			{
				const auto absoluteValue = std::abs(solver.eigenvalues()(i));

				if (absoluteValue < smallest)
				{
					secondSmallest = smallest;
					smallest = absoluteValue;
				}
			}

			laplacianMatrix /= secondSmallest;

			auto eigenvalues = solver.compute(laplacianMatrix, true).eigenvalues();
			auto eigenvectors = solver.eigenvectors();

			eigenvectors.transposeInPlace();

			for (size_t i = 0; i < eigenvalues.size(); i++)
			{
				eigenvalues(i) = std::exp(eigenvalues(i));
			}

			decltype(eigenvalues) _eigenvalues{};

			_eigenvalues.setZero(eigenvalues.rows(), eigenvalues.rows());
			_eigenvalues.diagonal() = eigenvalues.col(0);

			conditionalProbabilityDistribution = (eigenvectors * _eigenvalues) * eigenvectors.inverse();

			decltype(eigenvalues) _degreeDistribution{};

			_degreeDistribution.setZero(conditionalProbabilityDistribution.rows(), conditionalProbabilityDistribution.cols());

			for (Eigen::Index i = 0; i < conditionalProbabilityDistribution.rows(); i++)
			{
				_degreeDistribution(i, i) = 1.0 / normalizedDegrees[i];
			}

			normalizedJointDistribution = conditionalProbabilityDistribution * _degreeDistribution;

			Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > marginalX;
			Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > marginalY;
			Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > logaritmicMarginalX;
			Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > logaritmicMarginalY;

			marginalX.setZero(1, normalizedJointDistribution.cols());
			marginalY.setZero(normalizedJointDistribution.rows(), 1);
			logaritmicMarginalX.setZero(1, normalizedJointDistribution.cols());
			logaritmicMarginalY.setZero(normalizedJointDistribution.rows(), 1);

			for (size_t i = 0; i < normalizedJointDistribution.rows(); i++)
			{
				marginalX(i) = normalizedJointDistribution.col(i).sum();
				marginalY(i) = normalizedJointDistribution.row(i).sum();

				logaritmicMarginalX(i) = std::log2(marginalX(i));
				logaritmicMarginalY(i) = std::log2(marginalY(i));
			}

			marginalX.transposeInPlace();
			marginalY.transposeInPlace();
			
			entropyX = -((logaritmicMarginalX * marginalX)(0));
			entropyY = -((marginalY * logaritmicMarginalY)(0));

			Eigen::Map < Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > >
				columnMatrixJointDistribution(
					normalizedJointDistribution.data(),
					normalizedJointDistribution.rows(),
					normalizedJointDistribution.cols());

			Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor >
				columnMatrixLogaritmicJointDistribution;

			columnMatrixLogaritmicJointDistribution.
				setZero(normalizedJointDistribution.rows() * normalizedJointDistribution.cols(), 1);

			for (size_t i = 0; i < columnMatrixJointDistribution.rows(); i++)
			{
				columnMatrixLogaritmicJointDistribution(i) =
					std::log2(columnMatrixJointDistribution(i));
			}

			columnMatrixJointDistribution.transposeInPlace();

			mutualInformation = entropyX + entropyY +
				(columnMatrixJointDistribution * columnMatrixLogaritmicJointDistribution)(0);
		}

		auto DijkstraAllPreviousAndNextNodes(const NodeDataType &origin) const
		{
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > previous;
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > next;

			this->genericDijkstra < DijkstraModes::AllPreviousAndNextNodes > (origin, previous);

			this->DijkstraCreateNext < DijkstraModes::AllPreviousAndNextNodes > (previous, next);

			return std::make_pair(previous, next);
		}

		auto DijkstraAllPreviousAndNextNodesAndDistance(const NodeDataType &origin) const
		{
			std::unordered_map < NodeDataType, std::pair < std::unordered_set < NodeDataType >, ConnectionWeightDataType > > previous;
			std::unordered_map < NodeDataType, std::unordered_map < NodeDataType, ConnectionWeightDataType > > next;

			this->genericDijkstra < DijkstraModes::AllPreviousAndNextNodesAndDistance > (origin, previous);

			this->DijkstraCreateNext < DijkstraModes::AllPreviousAndNextNodesAndDistance > (previous, next);

			return std::make_pair(previous, next);
		}

		auto DijkstraSinglePreviousAndNextNode(const NodeDataType &origin) const
		{
			std::unordered_map < NodeDataType, NodeDataType > previous;
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > next;

			this->genericDijkstra < DijkstraModes::SinglePreviousAndNextNode > (origin, previous);

			this->DijkstraCreateNext < DijkstraModes::SinglePreviousAndNextNode > (previous, next);

			return std::make_pair(previous, next);
		}

		auto DijkstraSinglePreviousAndNextNodeAndDistance(const NodeDataType &origin) const
		{
			std::unordered_map < NodeDataType, std::pair < NodeDataType, ConnectionWeightDataType > > previous;
			std::unordered_map < NodeDataType, std::unordered_map < NodeDataType, ConnectionWeightDataType > > next;

			this->genericDijkstra < DijkstraModes::SinglePreviousAndNextNodeAndDistance > (origin, previous);

			this->DijkstraCreateNext < DijkstraModes::SinglePreviousAndNextNodeAndDistance > (previous, next);

			return std::make_pair(previous, next);
		}

		template < bool createPath >
		auto DijkstraSinglePath(const NodeDataType &origin, const NodeDataType &destiny) const
		{
			std::vector < NodeDataType > path;
			
			if (!this->contains(origin) || !this->contains(destiny))
			{
				if constexpr (createPath)
				{
					return std::make_pair(std::numeric_limits < ConnectionWeightDataType > ::max(), std::vector < NodeDataType > ());
				}

				else
				{
					return std::numeric_limits < ConnectionWeightDataType > ::max();
				}
			}

			if (origin == destiny)
			{
				if constexpr (createPath)
				{
					path.push_back(destiny);

					return std::make_pair(ConnectionWeightDataType(), path);
				}

				else
				{
					return ConnectionWeightDataType();
				}
			}

			ConnectionWeightDataType smallestDistance = ConnectionWeightDataType();

			std::unordered_map < NodeDataType, NodeDataType > previous;

			this->genericDijkstra < DijkstraModes::SinglePath > (origin, previous, destiny, smallestDistance);

			if constexpr (createPath)
			{
				NodeDataType current = destiny;

				while (true)
				{
					const auto iterator = previous.find(current);

					path.push_back(current);

					if (iterator != previous.cend())
					{
						current = iterator->second;
					}

					else
					{
						break;
					}
				}

				std::reverse(path.begin(), path.end());

				return std::make_pair(smallestDistance, path);
			}

			else
			{
				return smallestDistance;
			}
		}

		bool connected() const override
		{
			if (this->graph.empty())
			{
				return true;
			}

			return this->connectedNodes(this->graph.begin()->first).size() == this->size();
		}

		std::vector < Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > > dismemberDisconnectedNodes() const
		{
			std::vector < Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > > connectedGraphs;

			for (const auto &item1 : this->graph)
			{
				bool newGraph = true;

				for (size_t i = 0; i < connectedGraphs.size(); i++)
				{
					if (connectedGraphs[i].contains(item1.first))
					{
						newGraph = false;

						break;
					}
				}

				if (newGraph)
				{
					std::unordered_set < NodeDataType > nodes = this->connectedNodes(item1.first);
					Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > graph(this->size(), this->expectedConnectionsByNode);

					for (const auto &item2 : nodes)
					{
						for (const auto &item3 : this->getConnections(item2.first).value().get())
						{
							graph.insert(item2.first, item3.first, item3.second);
						}
					}

					connectedGraphs.emplace_back(std::move(graph));
				}
			}

			return connectedGraphs;
		}

		std::unordered_map < NodeDataType, ConnectionWeightDataType > randomWalk() const
		{
			if (this->graph.empty())
			{
				return std::unordered_map < NodeDataType, ConnectionWeightDataType > ();
			}

			size_t contador = 0;
			std::unordered_map < NodeDataType, size_t > graphIndexToMatrixIndex;
			std::unordered_map < size_t, NodeDataType > matrixIndexToGraphIndex;

			for (const auto &item : this->graph)
			{
				matrixIndexToGraphIndex.insert(std::make_pair(contador, item.first));
				graphIndexToMatrixIndex.insert(std::make_pair(item.first, contador));

				contador++;
			}

			const size_t matrixSize = this->graph.size();
			Eigen::Matrix < EigenDataType, Eigen::Dynamic, Eigen::Dynamic > matrix;

			matrix.resize(matrixSize, matrixSize);

			std::for_each(std::execution::par_unseq, this->graph.cbegin(), this->graph.cend(),
				[&matrix, &graphIndexToMatrixIndex] (
					const std::pair < NodeDataType, std::unordered_map < NodeDataType, ConnectionDataType > > &item1) -> void
				{
					for (const auto &item2 : item1.second)
					{
						matrix(graphIndexToMatrixIndex.at(item1.first), graphIndexToMatrixIndex.at(item2.first)) =
							EigenDataType(1) / EigenDataType(item2.second.weight);
					}
				});

			const Eigen::SelfAdjointEigenSolver < Eigen::MatrixXf > eigenSolver(matrix, Eigen::DecompositionOptions::ComputeEigenvectors);

			const auto &eigenvalues = eigenSolver.eigenvalues();
			const auto &eigenvectors = eigenSolver.eigenvectors();

			const size_t greatestAbsoluteEigenvalue =
				std::abs(eigenvalues(0)) > std::abs(eigenvalues(matrixSize - 1)) ? 0 : matrixSize - 1;

			std::unordered_map < NodeDataType, ConnectionWeightDataType > centralityValues;

			for (size_t i = 0; i < matrixSize; i++)
			{
				centralityValues.insert(
					std::make_pair(
						matrixIndexToGraphIndex.at(i),
						std::abs(ConnectionWeightDataType(eigenvectors.col(greatestAbsoluteEigenvalue)(i)))));
			}

			return centralityValues;
		}

		Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > shuffle() const
		{
			std::random_device randomDevice;
			std::default_random_engine engine(randomDevice());
			std::unordered_map < NodeDataType, NodeDataType > indicesMap;
			std::vector < NodeDataType > indices;

			indices.reserve(this->graph.size());

			for (const auto &node : this->graph)
			{
				indices.push_back(node.first);
			}

			std::shuffle(indices.begin(), indices.end(), engine);

			size_t counter = 0;

			for (const auto &node : this->graph)
			{
				indicesMap.insert(std::pair < NodeDataType, NodeDataType > (node.first, indices[counter++]));
			}

			Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > newGraph(this->size(), this->expectedConnectionsByNode);

			for (const auto &node1 : this->graph)
			{
				for (const auto &node2 : node1.second)
				{
					newGraph.insert(indicesMap.at(node1.first), indicesMap.at(node2.first), node2.second);
				}
			}

			return newGraph;
		}

		bool empty() const noexcept override
		{
			return this->graph.empty();
		}

		size_t getNodesQuantity() const noexcept override
		{
			return this->graph.size();
		}

		size_t size() const noexcept override
		{
			return this->graph.size();
		}

		void clear() noexcept override
		{
			this->weightSum = ConnectionWeightDataType();
			this->graph.clear();
			this->transposedGraph.clear();
			this->degrees.clear();
		}
		
		bool insert(const NodeDataType &node) override
		{
			if (!this->graph.insert(std::make_pair(
				node, std::unordered_map < NodeDataType, ConnectionDataType > (this->expectedConnectionsByNode))).second)
			{
				return false;
			}

			this->transposedGraph.insert(std::make_pair(node, std::unordered_set < NodeDataType > (this->expectedConnectionsByNode)));
			this->degrees.insert(std::make_pair(node, ConnectionWeightDataType()));

			return true;
		}

		std::optional < std::reference_wrapper < ConnectionDataType > >
			insert(const NodeDataType &node1, const NodeDataType &node2, const ConnectionDataType &connection = ConnectionDataType()) override
		{
			const ConnectionWeightDataType weight = connection.weight;

			if (weight == ConnectionWeightDataType())
			{
				return std::nullopt;
			}

			this->insert(node1);
			this->insert(node2);

			const auto pair = this->graph.at(node1).insert(std::make_pair(node2, connection));
			
			if (!pair.second)
			{
				return std::nullopt;
			}

			auto &insertedConnection = pair.first->second;

			this->graph.at(node2).insert(std::make_pair(node1, connection));

			this->transposedGraph.at(node2).insert(node1);
			this->transposedGraph.at(node1).insert(node2);

			this->degrees.at(node1) += weight;
			this->degrees.at(node2) += weight;

			this->weightSum += 2 * weight;

			return insertedConnection;
		}

		bool erase(const NodeDataType &node) override
		{
			if (this->contains(node))
			{
				auto begin = this->transposedGraph.at(node).begin();
				auto end = this->transposedGraph.at(node).end();
				
				std::for_each(begin, end,
					[this, node] (const NodeDataType &item) -> void
					{
						auto &connections = this->graph.at(item);

						this->weightSum -= connections.at(node).weight * 2;

						connections.erase(node);
					});

				for (const auto &item : this->graph.at(node))
				{
					this->transposedGraph.at(item.first).erase(node);
				}

				this->graph.erase(node);
				this->transposedGraph.erase(node);
				this->degrees.erase(node);

				return true;
			}

			return false;
		}

		bool erase(const NodeDataType &node1, const NodeDataType &node2) override
		{
			const auto iterator1 = this->graph.find(node1);

			if (iterator1 != this->graph.end())
			{
				const auto &connections = iterator1->second;
				const auto iterator2 = connections.find(node2);

				if (iterator2 != connections.end())
				{
					const ConnectionWeightDataType weight = iterator2->second.weight;

					this->weightSum -= 2 * weight;

					this->degrees.at(node1) -= weight;
					this->degrees.at(node2) -= weight;

					this->graph.at(node1).erase(node2);
					this->transposedGraph.at(node2).erase(node1);

					this->graph.at(node2).erase(node1);
					this->transposedGraph.at(node1).erase(node2);

					return true;
				}
			}

			return false;
		}

		bool contains(const NodeDataType &node) const override
		{
			return this->graph.contains(node);
		}

		bool contains(const NodeDataType &node1, const NodeDataType &node2) const override
		{
			const auto iterador = this->graph.find(node1);

			if (iterador != this->graph.cend())
			{
				const auto &conexoes = iterador->second;

				return conexoes.contains(node2);
			}

			return false;
		}

		std::optional < std::reference_wrapper < ConnectionDataType > >
			getConnection(const NodeDataType &node1, const NodeDataType &node2) override
		{
			const auto iterator1 = this->graph.find(node1);

			if (iterator1 != this->graph.end())
			{
				auto &connections = iterator1->second;
				const auto iterator2 = connections.find(node2);

				if (iterator2 != connections.end())
				{
					return iterator2->second;
				}
			}

			return std::nullopt;
		}

		std::optional < std::reference_wrapper < const ConnectionDataType > >
			getConnection(const NodeDataType &node1, const NodeDataType &node2) const override
		{
			const auto iterator1 = this->graph.find(node1);

			if (iterator1 != this->graph.cend())
			{
				const auto &connections = iterator1->second;
				const auto iterator2 = connections.find(node2);

				if (iterator2 != connections.cend())
				{
					return iterator2->second;
				}
			}

			return std::nullopt;
		}

		const std::unordered_map < NodeDataType, std::unordered_map < NodeDataType, ConnectionDataType > > &getConnections() const noexcept
		{
			return this->graph;
		}

		std::optional < std::reference_wrapper < const std::unordered_map < NodeDataType, ConnectionDataType > > >
			getConnections(const NodeDataType &no) const
		{
			const auto iterador = this->graph.find(no);

			if (iterador != this->graph.cend())
			{
				return iterador->second;
			}

			return std::nullopt;
		}

		size_t getConnectionsQuantity() const noexcept override
		{
			size_t connectionsQuantity = 0;

			for (const auto &item : this->graph)
			{
				connectionsQuantity += item.second.size();
			}

			return connectionsQuantity;
		}

		size_t getConnectionsQuantity(const NodeDataType &no) const override
		{
			const auto iterator = this->graph.find(no);

			if (iterator != this->graph.cend())
			{
				return iterator->second.size();
			}

			return 0;
		}

		ConnectionWeightDataType getWeight(const NodeDataType &no1, const NodeDataType &no2) const override
		{
			const auto iterator1 = this->graph.find(no1);

			if (iterator1 != this->graph.cend())
			{
				const auto &connections = iterator1->second;
				const auto iterator2 = connections.find(no2);

				if (iterator2 != connections.cend())
				{
					return iterator2->second.weight;
				}
			}

			return ConnectionWeightDataType();
		}

		std::unordered_set < NodeDataType > getNodes() const
		{
			std::unordered_set < NodeDataType > nodes(this->size());

			for (const auto &item : this->graph)
			{
				nodes.insert(item.first);
			}

			return nodes;
		}

		friend std::ostream &operator<<(std::ostream &stream, const Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &graph)
		{
			size_t accumulator1 = 0;
			const size_t nodesQuantity = graph.size();

			stream << "*--* (" << std::endl;

			for (const auto &nodes : graph.graph)
			{
				stream << "* " << nodes.first << " --" << std::endl;

				size_t accumulator2 = 0;
				const size_t connectionsQuantity = nodes.second.size();

				for (const auto &connections : nodes.second)
				{
					stream << "\t * " << connections.first << ": " << connections.second;

					accumulator2++;

					if (accumulator2 != connectionsQuantity)
					{
						stream << std::endl;
					}
				}

				accumulator1++;

				if (accumulator1 != nodesQuantity)
				{
					stream << std::endl;
				}
			}

			stream << " )";

			return stream;
		}

		Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &operator=(
			const Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &) = default;

		Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &operator=(
			Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &&) noexcept = default;

		Graph() = default;

		Graph(const Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &) = default;

		Graph(Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &&) noexcept = default;

		Graph(const size_t expectedNodes, const size_t expectedConnectionsByNode = expectedNodes) :
			graph(expectedNodes),
			transposedGraph(expectedNodes),
			degrees(expectedNodes),
			expectedConnectionsByNode(expectedConnectionsByNode)
		{
		}

		~Graph() noexcept override = default;
	};
}

#endif