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

#ifndef _XABLAU_GRAPH_DIGRAPH_H_
#define _XABLAU_GRAPH_DIGRAPH_H_

#include <atomic>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>
#include <type_traits>

#include "AbstractGraph.h"
#include "Graph.h"
#include "NaryTree.h"

namespace xablau::graph
{
	template <
		typename ConnectionWeightDataType,
		typename NodeDataType,
		typename ConnectionDataType = Connection < ConnectionWeightDataType > >
	class Digraph :
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

		void identifyCycles(
			const NodeDataType &targetNode,
			std::vector < NodeDataType > &visitedNodes,
			const size_t visitedNodesIndex,
			std::atomic < bool > &stop1,
			std::atomic < bool > &stop2,
			std::vector < std::vector < NodeDataType > > &cycles) const
		{
			if (stop1 || stop2)
			{
				return;
			}

			const auto endIterator = visitedNodes.begin() + visitedNodesIndex + 1;

			for (const auto &item : this->digraph.at(visitedNodes[visitedNodesIndex]))
			{
				if (item.first == targetNode)
				{
					cycles.emplace_back(std::vector < NodeDataType > ());
					cycles.back().assign(visitedNodes.begin(), endIterator);

					continue;
				}

				if (std::find(visitedNodes.begin(), endIterator, item.first) == endIterator)
				{
					visitedNodes[visitedNodesIndex + 1] = item.first;

					this->identifyCycles(targetNode, visitedNodes, visitedNodesIndex + 1, stop1, stop2, cycles);
				}
			}
		}

		std::vector < std::vector < NodeDataType > > identifyCyclesSelectCyclesGroup(
			const NodeDataType &node,
			const Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &copy) const
		{
			std::vector < std::vector < NodeDataType > > cycles1;
			std::vector < std::vector < NodeDataType > > cycles2;
			Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > transposedCopy = copy.transpose();

			std::vector < NodeDataType > visitedNodes1(copy.size());
			std::vector < NodeDataType > visitedNodes2(copy.size());

			visitedNodes1[0] = node;
			visitedNodes2[0] = node;

			std::atomic < bool > stop1(false);
			std::atomic < bool > stop2(false);

			std::thread thread1(
				[&copy, &node, &visitedNodes1, &stop1, &stop2, &cycles1] () -> void
				{
					copy.identifyCycles(node, visitedNodes1, 0, stop1, stop2, cycles1);

					if (!stop2)
					{
						stop1 = true;
					}
				});

			std::thread thread2(
				[&transposedCopy, &node, &visitedNodes2, &stop1, &stop2, &cycles2] () -> void
				{
					transposedCopy.identifyCycles(node, visitedNodes2, 0, stop1, stop2, cycles2);

					if (!stop1)
					{
						stop2 = true;
					}
				});

			thread1.join();
			thread2.join();

			if (stop1)
			{
				return cycles1;
			}

			std::for_each(cycles2.begin(), cycles2.end(),
				[] (std::vector < NodeDataType > &cycle) -> void
				{
					std::reverse(cycle.begin(), cycle.end());
					std::rotate(cycle.begin(), cycle.end() - 1, cycle.end());
				});

			return cycles2;
		}

		void TarjanStronglyConnectedComponents(
			const NodeDataType &node,
			const std::unordered_map < NodeDataType, ConnectionDataType > &neighbours,
			size_t &index,
			std::stack < NodeDataType, std::vector < NodeDataType > > &stack,
			std::vector < std::unordered_set < NodeDataType > > &stronglyConnectedComponents,
			std::unordered_map < NodeDataType, std::tuple < size_t, size_t, bool > > &nodeIndexLowlinkOnStack) const
		{
			auto &[_index, _lowlink, _onStack] = nodeIndexLowlinkOnStack[node];

			_index = index;
			_lowlink = index;
			_onStack = true;

			index++;

			stack.push(node);

			for (const auto &[neighbour, connection] : neighbours)
			{
				if (!nodeIndexLowlinkOnStack.contains(neighbour))
				{
					Digraph::TarjanStronglyConnectedComponents(
						neighbour, this->digraph.at(neighbour), index, stack, stronglyConnectedComponents, nodeIndexLowlinkOnStack);

					_lowlink = std::min(_lowlink, std::get < 1 > (nodeIndexLowlinkOnStack[neighbour]));
				}

				else if (std::get < 2 > (nodeIndexLowlinkOnStack[neighbour]))
				{
					_lowlink = std::min(_lowlink, std::get < 0 > (nodeIndexLowlinkOnStack[neighbour]));
				}
			}

			if (_lowlink == _index)
			{
				typename std::unordered_set < NodeDataType > ::iterator iterator{};

				stronglyConnectedComponents.emplace_back(std::unordered_set < NodeDataType > ());

				do
				{
					NodeDataType stronglyConnectedNode = std::move(stack.top());

					std::get < 2 > (nodeIndexLowlinkOnStack[stronglyConnectedNode]) = false;

					iterator = stronglyConnectedComponents.back().emplace(std::move(stronglyConnectedNode)).first;

					stack.pop();
				} while (*iterator != node);
			}
		}

		Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > convertToGraphSmallerWeight() const
		{
			Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > graph(this->size(), this->expectedConnectionsByNode);

			for (const auto &node : this->digraph)
			{
				for (const auto &connection : node.second)
				{
					if (connection.second.weight < this->getWeight(connection.first, node.first))
					{
						graph.insert(node.first, connection.first, connection.second);
					}

					else
					{
						graph.insert(connection.first, node.first, connection.second);
					}
				}
			}

			return graph;
		}

		Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > convertToGraphGreaterWeight() const
		{
			Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > grafo(this->size(), this->expectedConnectionsByNode);

			for (const auto &node : this->digraph)
			{
				for (const auto &connection : node.second)
				{
					if (connection.second.weight > this->getWeight(connection.first, node.first))
					{
						grafo.insert(node.first, connection.first, connection.second);
					}

					else
					{
						grafo.insert(connection.first, node.first, connection.second);
					}
				}
			}

			return grafo;
		}

	protected:
		ConnectionWeightDataType weightSum;
		std::unordered_map < NodeDataType, std::unordered_map < NodeDataType, ConnectionDataType > > digraph;
		std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > transposedGraph;
		std::unordered_map < NodeDataType, ConnectionWeightDataType > degrees;

		size_t expectedConnectionsByNode;

	public:
		enum class TreeGenerationModes
		{
			BreadthFirstSearch,
			DepthFirstSearch,
			Dijkstra
		};
		
		bool operator==(const Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &digrafo) const
		{
			return this->digraph == digrafo.digraph;
		}

		bool operator!=(const Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &digrafo) const
		{
			return this->digraph != digrafo.digraph;
		}

		ConnectionWeightDataType degree(const NodeDataType &node) const override
		{
			const auto iterator = this->degrees.find(node);

			if (iterator != this->degrees.cend())
			{
				return iterator->second;
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
			const size_t nodesQuantity = this->digraph.size();

			if (nodesQuantity < 2)
			{
				throw std::domain_error("There are less than two nodes in the graph.");
			}

			return this->weightSum / ConnectionWeightDataType(nodesQuantity * (nodesQuantity - 1));
		}

		ConnectionWeightDataType degreeDistribution(const ConnectionWeightDataType degree) const override
		{
			if (this->digraph.empty())
			{
				throw std::domain_error("There are no nodes in the digraph.");
			}

			ConnectionWeightDataType probability = ConnectionWeightDataType();

			for (const auto &item : this->digraph)
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
			if (this->empty())
			{
				throw std::domain_error("There are no nodes in the graph.");
			}

			std::map < ConnectionWeightDataType, ConnectionWeightDataType > probabilities;

			for (const auto &item : this->digraph)
			{
				++((probabilities.emplace(std::make_pair(this->degree(item.first), ConnectionWeightDataType()))).first->second);
			}

			for (auto &item : probabilities)
			{
				item.second /= ConnectionWeightDataType(this->size());
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
			return this->statisticalMoment(2) - std::pow(this->statisticalMoment(1), ConnectionWeightDataType(2));
		}

		ConnectionWeightDataType volume() const noexcept override
		{
			return this->weightSum;
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
			Eigen::Index counter = 0;
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

				for (const auto &item : this->digraph)
				{
					matrixIndices.insert(item.first);
				}

				for (const auto item : matrixIndices)
				{
					_graphIndexToMatrixIndex.emplace(std::pair < NodeDataType, Eigen::Index > (item, counter));
					_matrixIndexToGraphIndex.emplace(std::pair < Eigen::Index, NodeDataType > (counter, item));

					counter++;
				}
			}

			else
			{
				for (const auto &item : this->digraph)
				{
					_graphIndexToMatrixIndex.emplace(std::pair < NodeDataType, Eigen::Index > (item.first, counter));
					_matrixIndexToGraphIndex.emplace(std::pair < Eigen::Index, NodeDataType > (counter, item.first));

					counter++;
				}
			}

			std::for_each(this->digraph.cbegin(), this->digraph.cend(),
				[this, &_adjacencyMatrix, &_graphIndexToMatrixIndex] (
					const std::pair < NodeDataType, std::unordered_map < NodeDataType, ConnectionDataType > > &node1) -> void
				{
					const Eigen::Index rowIndex = _graphIndexToMatrixIndex.at(node1.first);

					for (const auto &node2 : node1.second)
					{
						const Eigen::Index columnIndex = _graphIndexToMatrixIndex.at(node2.first);

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

			if constexpr (!allowAutoConnections)
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
			std::optional < std::reference_wrapper < std::unordered_map < NodeDataType, Eigen::Index > > > graphIndexToMatrixIndex = std::nullopt,
			std::optional < std::reference_wrapper < std::unordered_map < Eigen::Index, NodeDataType > > > matrixIndexToGraphIndex = std::nullopt) const
		{
			std::unordered_map < NodeDataType, Eigen::Index > _graphIndexToMatrixIndex;
			std::unordered_map < Eigen::Index, NodeDataType > _matrixIndexToGraphIndex;

			auto _laplacianMatrix =
				this->adjacencyMatrix < allowAutoConnections, sparseMatrix, sortMatrixIndices, MatrixDataType, MatrixMajor, true > (
					_graphIndexToMatrixIndex, _matrixIndexToGraphIndex);

			auto diagonalMatrix = _laplacianMatrix.diagonal();

			std::for_each(_matrixIndexToGraphIndex.begin(), _matrixIndexToGraphIndex.end(),
				[this, &diagonalMatrix](const std::pair < Eigen::Index, NodeDataType > &pair) -> void
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
						diagonalElement -= this->getWeight(pair.second, pair.second);
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

		std::vector < std::vector < NodeDataType > > identifyCycles(const NodeDataType &node) const
		{
			if (this->digraph.contains(node))
			{
				std::unordered_set < NodeDataType > stronglyConnectedNode = this->TarjanStronglyConnectedComponents().at(node);

				Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > copy(this->size(), this->expectedConnectionsByNode);

				for (const auto &_node : stronglyConnectedNode)
				{
					copy.insert(_node);

					for (const auto &connection : this->digraph.at(_node))
					{
						if (stronglyConnectedNode.count(connection.first))
						{
							copy.insert(_node, connection.first);
						}
					}
				}

				return this->identifyCyclesSelectCyclesGroup(node, copy);
			}

			return std::vector < std::vector < NodeDataType > > ();
		}

		std::unordered_map < NodeDataType, std::vector < std::vector < NodeDataType > > > identifyCycles() const
		{
			std::unordered_map < NodeDataType, std::vector < std::vector < NodeDataType > > > cycles(this->getNodesQuantity());
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > stronglyConnectedNodes = this->TarjanStronglyConnectedComponents();

			for (const auto &stronglyConnectedNode : stronglyConnectedNodes)
			{
				const NodeDataType &node = stronglyConnectedNode.first;

				Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > copy(this->size(), this->expectedConnectionsByNode);

				for (const auto &_node : stronglyConnectedNode.second)
				{
					copy.insert(_node);

					for (const auto &connection : this->digraph.at(_node))
					{
						if (stronglyConnectedNode.second.count(connection.first))
						{
							copy.insert(_node, connection.first);
						}
					}
				}

				cycles.emplace(std::make_pair(node, this->identifyCyclesSelectCyclesGroup(node, copy)));
			}

			return cycles;
		}

		std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > TarjanStronglyConnectedComponents() const
		{
			size_t index = 0;
			std::stack < NodeDataType, std::vector < NodeDataType > > stack;
			std::vector < std::unordered_set < NodeDataType > > stronglyConnectedComponents;
			std::unordered_map < NodeDataType, std::tuple < size_t, size_t, bool > > nodeIndexLowlinkOnStack;

			for (const auto &node : this->digraph)
			{
				if (!nodeIndexLowlinkOnStack.contains(node.first))
				{
					this->TarjanStronglyConnectedComponents(
						node.first, node.second, index, stack, stronglyConnectedComponents, nodeIndexLowlinkOnStack);
				}
			}

			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > _stronglyConnectedNodes(this->getNodesQuantity());

			for (const auto &stronglyConnectedComponent : stronglyConnectedComponents)
			{
				for (const auto &node : stronglyConnectedComponent)
				{
					_stronglyConnectedNodes[node] = stronglyConnectedComponent;
				}
			}

			return _stronglyConnectedNodes;
		}

		template < bool greaterWeight = true >
		Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > convertToGraph() const
		{
			if constexpr(greaterWeight)
			{
				return this->convertToGraphGreaterWeight();
			}

			else
			{
				return this->convertToGraphSmallerWeight();
			}
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

		template <
			bool sparseMatrix,
			bool pathReconstruction = false,
			bool sortMatrixIndices = false,
			typename MatrixDataType = EigenDataType,
			Eigen::StorageOptions MatrixMajor = Eigen::StorageOptions::RowMajor >
		auto FloydWarshall(
			std::optional < std::reference_wrapper < std::unordered_map < NodeDataType, Eigen::Index > > > graphIndexToMatrixIndex = std::nullopt,
			std::optional < std::reference_wrapper < std::unordered_map < Eigen::Index, NodeDataType > > > matrixIndexToGraphIndex = std::nullopt)
		{
			Eigen::Index counter = 0;
			const size_t matrixSize = this->size();

			using DistanceMatrixType =
				typename std::conditional < sparseMatrix,
					Eigen::SparseMatrix < MatrixDataType, MatrixMajor, std::ptrdiff_t >,
					Eigen::Matrix < MatrixDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > > ::type;

			using NodeMatrixType =
				typename std::conditional < sparseMatrix,
					Eigen::SparseMatrix < NodeDataType, MatrixMajor, std::ptrdiff_t >,
					Eigen::Matrix < NodeDataType, Eigen::Dynamic, Eigen::Dynamic, MatrixMajor > > ::type;

			DistanceMatrixType distanceMatrix{};
			NodeMatrixType nextNode{};

			if constexpr (sparseMatrix)
			{
				distanceMatrix.reserve(this->expectedConnectionsByNode * this->getNodesQuantity());

				if constexpr (pathReconstruction)
				{
					nextNode.reserve(this->expectedConnectionsByNode * this->getNodesQuantity());
				}
			}

			else
			{
				distanceMatrix.setConstant(matrixSize, matrixSize, MatrixDataType());

				if constexpr (pathReconstruction)
				{
					nextNode.setConstant(matrixSize, matrixSize, NodeDataType());
				}
			}

			std::unordered_map < NodeDataType, Eigen::Index > _graphIndexToMatrixIndex(matrixSize);
			std::unordered_map < Eigen::Index, NodeDataType > _matrixIndexToGraphIndex(matrixSize);

			if constexpr (sortMatrixIndices)
			{
				std::set < NodeDataType > matrixIndices;

				for (const auto &node : this->digraph)
				{
					matrixIndices.insert(node.first);
				}

				for (const auto &item : matrixIndices)
				{
					_graphIndexToMatrixIndex.insert(std::pair < NodeDataType, Eigen::Index > (item, counter));
					_matrixIndexToGraphIndex.insert(std::pair < Eigen::Index, NodeDataType > (counter, item));

					counter++;
				}
			}

			else
			{
				for (const auto &node : this->digraph)
				{
					_graphIndexToMatrixIndex.insert(std::pair < NodeDataType, Eigen::Index > (node.first, counter));
					_matrixIndexToGraphIndex.insert(std::pair < Eigen::Index, NodeDataType > (counter, node.first));

					counter++;
				}
			}

			for (const auto &node1 : this->digraph)
			{
				const Eigen::Index matrixIndex1 = _graphIndexToMatrixIndex[node1.first];

				for (const auto &node2 : node1.second)
				{
					const Eigen::Index matrixIndex2 = _graphIndexToMatrixIndex[node2.first];

					if constexpr (sparseMatrix)
					{
						distanceMatrix.insert(matrixIndex1, matrixIndex2) = node2.second.weight;

						if constexpr (pathReconstruction)
						{
							nextNode.insert(matrixIndex1, matrixIndex2) = node2.first;
						}
					}

					else
					{
						distanceMatrix(matrixIndex1, matrixIndex2) = node2.second.weight;

						if constexpr (pathReconstruction)
						{
							nextNode(matrixIndex1, matrixIndex2) = node2.first;
						}
					}
				}

				if constexpr (sparseMatrix)
				{
					distanceMatrix.coeffRef(matrixIndex1, matrixIndex1) = MatrixDataType();

					if constexpr (pathReconstruction)
					{
						nextNode.coeffRef(matrixIndex1, matrixIndex1) = node1.first;
					}
				}

				else
				{
					distanceMatrix(matrixIndex1, matrixIndex1) = MatrixDataType();

					if constexpr (pathReconstruction)
					{
						nextNode(matrixIndex1, matrixIndex1) = node1.first;
					}
				}
			}

			for (Eigen::Index k = 0; k < matrixSize; k++)
			{
				for (Eigen::Index i = 0; i < matrixSize; i++)
				{
					if constexpr (sparseMatrix)
					{
						const auto distanceIK = distanceMatrix.coeff(i, k);

						for (Eigen::Index j = 0; j < matrixSize; j++)
						{
							if (distanceMatrix.coeff(i, j) > distanceIK + distanceMatrix.coeff(k, j))
							{
								distanceMatrix.coeffRef(i, j) = distanceIK + distanceMatrix.coeff(k, j);

								if constexpr (pathReconstruction)
								{
									nextNode.coeffRef(i, j) = nextNode.coeffRef(i, k);
								}
							}
						}
					}

					else
					{
						const auto distanceIK = distanceMatrix(i, k);

						for (Eigen::Index j = 0; j < matrixSize; j++)
						{
							if (distanceMatrix(i, j) > distanceIK + distanceMatrix(k, j))
							{
								distanceMatrix(i, j) = distanceIK + distanceMatrix(k, j);

								if constexpr (pathReconstruction)
								{
									nextNode(i, j) = nextNode(i, k);
								}
							}
						}
					}
				}
			}

			if (graphIndexToMatrixIndex)
			{
				graphIndexToMatrixIndex.value().get() = std::move(_graphIndexToMatrixIndex);
			}

			if (matrixIndexToGraphIndex)
			{
				matrixIndexToGraphIndex.value().get() = std::move(_matrixIndexToGraphIndex);
			}

			if constexpr (pathReconstruction)
			{
				return std::make_pair(std::move(distanceMatrix), std::move(nextNode));
			}

			else
			{
				return distanceMatrix;
			}
		}

		void breadthFirstSearch(
			const NodeDataType &currentNode,
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > &next,
			std::unordered_set < NodeDataType > &visitedNodes) const
		{
			std::vector < NodeDataType > queue;

			next[currentNode].reserve(this->expectedConnectionsByNode);

			visitedNodes.insert(currentNode);

			for (const auto &[neighbour, connection] : this->digraph.at(currentNode))
			{
				if (!visitedNodes.contains(neighbour))
				{
					next[currentNode].insert(neighbour);

					queue.push_back(neighbour);
				}
			}

			for (const auto &nextNode : queue)
			{
				this->breadthFirstSearch(nextNode, next, visitedNodes);
			}
		}

		std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > breadthFirstSearch(const NodeDataType &origin) const
		{
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > next(this->size());
			std::unordered_set < NodeDataType > visitedNodes(this->size());

			this->breadthFirstSearch(origin, next, visitedNodes);
			
			return next;
		}

		void depthFirstSearch(
			const NodeDataType &currentNode,
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > &next,
			std::unordered_set < NodeDataType > &visitedNodes) const
		{
			next[currentNode].reserve(this->expectedConnectionsByNode);

			visitedNodes.insert(currentNode);

			for (const auto &[neighbour, connection] : this->digraph.at(currentNode))
			{
				if (!visitedNodes.contains(neighbour))
				{
					next[currentNode].insert(neighbour);

					this->breadthFirstSearch(neighbour, next, visitedNodes);
				}
			}
		}

		std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > depthFirstSearch(const NodeDataType &origin) const
		{
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > next(this->size());
			std::unordered_set < NodeDataType > visitedNodes(this->size());

			this->breadthFirstSearch(origin, next, visitedNodes);
			
			return next;
		}

		template < NaryTreeNodeContainerType ContainerType, TreeGenerationModes TreeGenerationMode >
		NaryTree < NodeDataType, ContainerType > generateTree(const NodeDataType &origin) const
		{
			std::unordered_map < NodeDataType, std::unordered_set < NodeDataType > > next;

			if constexpr (TreeGenerationMode == TreeGenerationModes::BreadthFirstSearch)
			{
				next = this->breadthFirstSearch(origin);
			}

			else if constexpr (TreeGenerationMode == TreeGenerationModes::DepthFirstSearch)
			{
				next = this->depthFirstSearch(origin);
			}

			else if constexpr (TreeGenerationMode == TreeGenerationModes::Dijkstra)
			{
				auto [_previous, _next] = this->DijkstraSinglePreviousAndNextNode(origin);

				next = std::move(_next);
			}

			std::queue < typename NaryTree < NodeDataType, ContainerType > ::iterator > queue;
			NaryTree < NodeDataType, ContainerType > tree;

			queue.push(tree.insertRoot(origin));

			while (!queue.empty())
			{
				const auto &currentNodeInTree = queue.front();
				const auto currentNodeInGraph = *currentNodeInTree;

				if (next.contains(currentNodeInGraph))
				{
					const auto &connections = next.at(currentNodeInGraph);

					for (const auto &connection : connections)
					{
						queue.emplace(tree.insertBack(currentNodeInTree, connection));
					}
				}

				queue.pop();
			}

			return tree;
		}

		bool connected() const override
		{
			return this->convertToGraph().connected();
		}

		std::vector < Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > > dismemberDisconnectedNodes() const noexcept
		{
			Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > graph(this->size(), this->expectedConnectionsByNode);

			for (const auto &item1 : this->digraph)
			{
				for (const auto &item2 : item1.second)
				{
					graph.insert(item1.first, item2.first);
				}
			}

			std::vector < Graph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > > connectedGraphs =
				graph.dismemberDisconnectedNodes();

			std::vector < Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > > connectedDigraphs(
				connectedGraphs.size(), Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > (
					this->size(), this->expectedConnectionsByNode));

			for (size_t i = 0; i < connectedDigraphs.size(); i++)
			{
				const auto nos = connectedGraphs[i].getNodes();

				for (const auto &no : nos)
				{
					for (const auto &connection : this->digraph.at(no))
					{
						connectedDigraphs[i].insert(no, connection.first, connection.second);
					}
				}
			}

			return connectedDigraphs;
		}

		Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > transpose() const
		{
			Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > digraph(this->size(), this->expectedConnectionsByNode);

			for (const auto &item1 : this->digraph)
			{
				digraph.insert(item1.first);

				for (const auto &item2 : item1.second)
				{
					digraph.insert(item2.first, item1.first, item2.second);
				}
			}

			return digraph;
		}

		bool empty() const noexcept override
		{
			return this->digraph.empty();
		}

		size_t getNodesQuantity() const noexcept override
		{
			return this->digraph.size();
		}

		size_t size() const noexcept override
		{
			return this->digraph.size();
		}

		void clear() noexcept override
		{
			this->weightSum = ConnectionWeightDataType();
			this->digraph.clear();
			this->transposedGraph.clear();
			this->degrees.clear();
		}

		bool insert(const NodeDataType &node) override
		{
			if (!this->digraph.insert(std::make_pair(
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

			const auto pair = this->digraph.at(node1).insert(std::make_pair(node2, connection));
			
			if (!pair.second)
			{
				return std::nullopt;
			}

			auto &insertedConnection = pair.first->second;

			this->transposedGraph.at(node2).insert(node1);

			this->degrees.at(node1) += weight;

			this->weightSum += weight;

			return insertedConnection;
		}

		bool erase(const NodeDataType &node) override
		{
			if (this->contains(node))
			{
				auto begin = this->transposedGraph.at(node).begin();
				auto end = this->transposedGraph.at(node).end();
				
				std::for_each(begin, end,
					[this, node](const NodeDataType &item) -> void
					{
						auto &connections = this->digraph.at(item);

						this->weightSum -= connections.at(node).weight;

						connections.erase(node);
					});

				for (const auto &item : this->digraph.at(node))
				{
					this->weightSum -= item.second.weight;

					this->transposedGraph.at(item.first).erase(node);
				}

				this->digraph.erase(node);
				this->transposedGraph.erase(node);
				this->degrees.erase(node);

				return true;
			}

			return false;
		}

		bool erase(const NodeDataType &node1, const NodeDataType &node2) override
		{
			const auto iterator1 = this->digraph.find(node1);

			if (iterator1 != this->digraph.end())
			{
				const auto &connections = iterator1->second;
				const auto iterator2 = connections.find(node2);

				if (iterator2 != connections.end())
				{
					const ConnectionWeightDataType weight = iterator2->second.weight;

					this->weightSum -= weight;

					this->degrees.at(node1) -= weight;

					this->digraph.at(node1).erase(node2);

					this->transposedGraph.at(node2).erase(node1);

					return true;
				}
			}

			return false;
		}

		bool contains(const NodeDataType &node) const override
		{
			return this->digraph.contains(node);
		}

		bool contains(const NodeDataType &node1, const NodeDataType &node2) const override
		{
			const auto iterator = this->digraph.find(node1);

			if (iterator != this->digraph.end())
			{
				const auto &connections = iterator->second;

				return connections.find(node2) != connections.end();
			}

			return false;
		}

		std::optional < std::reference_wrapper < ConnectionDataType > >
			getConnection(const NodeDataType &node1, const NodeDataType &node2) override
		{
			const auto iterator1 = this->digraph.find(node1);

			if (iterator1 != this->digraph.end())
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
			const auto iterator1 = this->digraph.find(node1);

			if (iterator1 != this->digraph.end())
			{
				const auto &connections = iterator1->second;
				const auto iterator2 = connections.find(node2);

				if (iterator2 != connections.end())
				{
					return iterator2->second;
				}
			}

			return std::nullopt;
		}

		const std::unordered_map < NodeDataType, std::unordered_map < NodeDataType, ConnectionDataType > > &getConnections() const noexcept
		{
			return this->digraph;
		}

		std::optional < std::reference_wrapper < const std::unordered_map < NodeDataType, ConnectionDataType > > >
			getConnections(const NodeDataType &node) const
		{
			const auto iterator = this->digraph.find(node);

			if (iterator != this->digraph.end())
			{
				return iterator->second;
			}

			return std::nullopt;
		}

		size_t getConnectionsQuantity() const noexcept override
		{
			size_t connectionsQuantity = 0;

			for (const auto &item : this->digraph)
			{
				connectionsQuantity += item.second.size();
			}

			return connectionsQuantity;
		}

		size_t getConnectionsQuantity(const NodeDataType &node) const override
		{
			const auto iterator = this->digraph.find(node);

			if (iterator != this->digraph.cend())
			{
				return iterator->second.size();
			}

			return 0;
		}

		ConnectionWeightDataType getWeight(const NodeDataType &node1, const NodeDataType &node2) const override
		{
			const auto iterator1 = this->digraph.find(node1);

			if (iterator1 != this->digraph.cend())
			{
				const auto &connections = iterator1->second;
				const auto iterator2 = connections.find(node2);

				if (iterator2 != connections.cend())
				{
					return iterator2->second.weight;
				}
			}

			return ConnectionWeightDataType();
		}

		std::unordered_set < NodeDataType > getNodes() const
		{
			std::unordered_set < NodeDataType > nodes(this->digraph.size());

			for (const auto &item : this->digraph)
			{
				nodes.insert(item.first);
			}

			return nodes;
		}

		std::unordered_set < NodeDataType > getSourceNodes() const
		{
			std::unordered_set < NodeDataType > sourceNodes;

			for (const auto &item : this->digraph)
			{
				if (this->transposedGraph.at(item.first).empty())
				{
					sourceNodes.insert(item.first);
				}
			}

			return sourceNodes;
		}

		std::unordered_set < NodeDataType > getSinkNodes() const
		{
			std::unordered_set < NodeDataType > sinkNodes;

			for (const auto &item : this->digraph)
			{
				if (item.second.empty())
				{
					sinkNodes.insert(item.first);
				}
			}

			return sinkNodes;
		}

		friend std::ostream &operator<<(std::ostream &stream, const Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &digraph)
		{
			size_t accumulator1 = 0;
			const size_t nodesQuantity = digraph.size();

			stream << "*->* (\n";

			for (const auto &node : digraph.digraph)
			{
				stream << "* " << node.first << " ->\n";

				size_t accumulator2 = 0;
				const size_t connectionsQuantity = node.second.size();

				for (const auto &connections : node.second)
				{
					stream << "\t * " << connections.first << ": " << connections.second;

					accumulator2++;

					if (accumulator2 != connectionsQuantity)
					{
						stream << "\n";
					}
				}

				accumulator1++;

				if (accumulator1 != nodesQuantity)
				{
					stream << "\n";
				}
			}

			stream << " )";

			return stream;
		}

		Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &operator=(
			const Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &) = default;

		Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &operator=(
			Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &&) noexcept = default;

		Digraph() :
			weightSum(ConnectionWeightDataType()),
			expectedConnectionsByNode(0)
		{
		}

		Digraph(const Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &) = default;

		Digraph(Digraph < ConnectionWeightDataType, NodeDataType, ConnectionDataType > &&) noexcept = default;

		Digraph(const size_t expectedNodes, const size_t expectedConnectionsByNode) :
			weightSum(ConnectionWeightDataType()),
			digraph(expectedNodes),
			transposedGraph(expectedNodes),
			degrees(expectedNodes),
			expectedConnectionsByNode(expectedConnectionsByNode)
		{
		}

		~Digraph() noexcept override = default;
	};
}

#endif