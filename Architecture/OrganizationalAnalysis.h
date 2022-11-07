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

#ifndef _ORGANIZATIONAL_ANALYSIS_H_
#define _ORGANIZATIONAL_ANALYSIS_H_

#include <charconv>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <thread>

#include "../File/File.h"
#include "../Graph/Digraph.h"

class OrganizationalAnalysis final
{
private:
	static inline constexpr float indirectlyInvolved = 1.0f;
	static inline constexpr float involved = 2.0f;
	static inline constexpr float directlyInvolved = 3.0f;
	static inline constexpr std::string_view _nameExpression = "([0-9]+)([ ]*-[ ]*)([^\r\n]+)";
	static inline constexpr std::string_view _groupExpression = "([^*][^\r\n]+)";
	static inline constexpr std::string_view _productInteractionExpression = "\t([0-9]+)([ ]*[\t]*)*";
	static inline constexpr std::string_view _personInChargeExpression = "\t([0-9]+)([ ]*[\t]*)*";
	static inline constexpr std::string_view _dependencyExpression = "\t\t([0-9]+)([ ]*[\t]*)*";
	static inline constexpr std::string_view _affiliationActivityExpression = "([0-9]+)[ ]*-[ ]*([^\r\n]+)";
	static inline constexpr std::string_view _affiliationProductWeightExpression = "\t([0-9]+)[ ]*,[ ]*([0-9]+)([ ]*[\t]*)*";

	void readAgents(const std::string &fileName)
	{
		std::string group;
		const std::regex groupExpression(OrganizationalAnalysis::_groupExpression.data());
		const std::regex roleExpression(OrganizationalAnalysis::_nameExpression.data());
		xablau::file::File < char > file(fileName);

		while (!file.eof())
		{
			std::string line;
			std::smatch match;

			file.read(line);

			if (std::regex_match(line, match, roleExpression))
			{
				size_t key{};
				const std::string stringKey = match[1].str();
				std::string role = match[3].str();

				std::from_chars(stringKey.data(), stringKey.data() + stringKey.size(), key);

				auto iterator = this->agents.descriptions.emplace(std::make_pair(key, Agent::Description())).first;
				iterator->second.role = std::move(role);
				iterator->second.group = group;
			}

			else if (std::regex_match(line, match, groupExpression))
			{
				group = match[0].str();
			}

			else if (!line.empty())
			{
				std::cout << "Unknown line in " + fileName + ": " << line << "\n";
			}
		}
	}

	void readActivities(const std::string &fileName)
	{
		size_t key{};
		std::string group;
		xablau::file::File < char > file(fileName);
		const std::regex groupExpression(OrganizationalAnalysis::_groupExpression.data());
		const std::regex activityExpression(OrganizationalAnalysis::_nameExpression.data());
		const std::regex personInChargeExpression(OrganizationalAnalysis::_personInChargeExpression.data());
		const std::regex dependencyExpression(OrganizationalAnalysis::_dependencyExpression.data());

		while (!file.eof())
		{
			std::string line;
			std::smatch match;

			file.read(line);

			if (std::regex_match(line, match, activityExpression))
			{
				const std::string stringKey = match[1].str();
				std::string name = match[3].str();

				std::from_chars(stringKey.data(), stringKey.data() + stringKey.size(), key);

				auto iterator = this->activities.descriptions.emplace(std::make_pair(key, Activity::Description())).first;
				iterator->second.name = std::move(name);
				iterator->second.group = group;
			}

			else if (std::regex_match(line, match, personInChargeExpression))
			{
				size_t personInCharge{};
				const std::string stringPersonInCharge = match[1].str();

				std::from_chars(stringPersonInCharge.data(), stringPersonInCharge.data() + stringPersonInCharge.size(), personInCharge);

				if (!this->agents.descriptions.contains(personInCharge))
				{
					std::string exception = "Could not find person ";
					exception.append(std::to_string(key));
					exception.append(".");

					throw std::runtime_error(exception);
				}

				this->activities.descriptions.at(key).peopleInCharge.insert(personInCharge);
			}

			else if (std::regex_match(line, match, dependencyExpression))
			{
				size_t dependency{};
				std::string stringDependency = match[1].str();

				std::from_chars(stringDependency.data(), stringDependency.data() + stringDependency.size(), dependency);

				if (key == dependency)
				{
					std::string exception = "Activity ";
					exception.append(std::to_string(key));
					exception.append(" is depending on itself.");

					throw std::runtime_error(exception);
				}

				this->activities.dependencies.insert(key, dependency);
			}

			else if (std::regex_match(line, match, groupExpression))
			{
				group = match[1].str();
			}

			else if (!line.empty())
			{
				std::cout << "Unknown line in " + fileName + ": " << line << "\n";
			}
		}

		for (const auto &[identification, description] : this->activities.descriptions)
		{
			this->activities.dependencies.insert(identification);
		}
	}

	void readProducts(const std::string &fileName)
	{
		size_t key{};
		std::string group;
		xablau::file::File < char > file(fileName);
		const std::regex groupExpression(OrganizationalAnalysis::_groupExpression.data());
		const std::regex productExpression(OrganizationalAnalysis::_nameExpression.data());
		const std::regex interactionExpression(OrganizationalAnalysis::_productInteractionExpression.data());

		while (!file.eof())
		{
			std::string line;
			std::smatch match;

			file.read(line);

			if (std::regex_match(line, match, productExpression))
			{
				const std::string stringKey = match[1].str();
				std::string name = match[3].str();

				std::from_chars(stringKey.data(), stringKey.data() + stringKey.size(), key);

				auto iterator = this->products.descriptions.emplace(std::make_pair(key, Product::Description())).first;
				iterator->second.name = std::move(name);
				iterator->second.group = group;
			}

			else if (std::regex_match(line, match, interactionExpression))
			{
				size_t index{};
				const std::string stringIndex = match[1].str();

				std::from_chars(stringIndex.data(), stringIndex.data() + stringIndex.size(), index);

				this->products.interactions.insert(key, index);
			}

			else if (std::regex_match(line, match, groupExpression))
			{
				group = match[1].str();
			}

			else if (!line.empty())
			{
				std::cout << "Unknown line in " + fileName + ": " << line << "\n";
			}
		}
	}

	void readAffiliations(const std::string &fileName)
	{
		size_t key{};
		xablau::file::File < char > file(fileName);
		const std::regex affiliationActivityExpression(OrganizationalAnalysis::_affiliationActivityExpression.data());
		const std::regex affiliationProductWeightExpression(OrganizationalAnalysis::_affiliationProductWeightExpression.data());

		while (!file.eof())
		{
			std::string line;
			std::smatch match;

			file.read(line);

			if (std::regex_match(line, match, affiliationActivityExpression))
			{
				const std::string stringKey = match[1].str();

				std::from_chars(stringKey.data(), stringKey.data() + stringKey.size(), key);

				if (!this->activities.descriptions.contains(key))
				{
					std::string exception = "Could not find activity ";
					exception.append(std::to_string(key));
					exception.append(".");

					throw std::runtime_error(exception);
				}

				this->affiliations.responsabilities.emplace(std::make_pair(key, std::map < size_t, float > ()));
			}

			else if (std::regex_match(line, match, affiliationProductWeightExpression))
			{
				size_t product{};
				float weight{};
				const std::string stringProduct = match[1].str();
				const std::string stringWeight = match[2].str();

				std::from_chars(stringProduct.data(), stringProduct.data() + stringProduct.size(), product);
				std::from_chars(stringWeight.data(), stringWeight.data() + stringWeight.size(), weight);

				if (!this->products.descriptions.contains(product))
				{
					std::string exception = "Could not find the product ";
					exception.append(std::to_string(key));
					exception.append(".");

					throw std::runtime_error(exception);
				}

				this->affiliations.responsabilities.at(key).emplace(std::make_pair(product, weight));
			}

			else if (!line.empty())
			{
				std::cout << "Unknown line in " + fileName + ": " << line << "\n";
			}
		}

		for (const auto &[identification, description] : this->activities.descriptions)
		{
			this->affiliations.responsabilities.emplace(std::make_pair(identification, std::map < size_t, float > ()));
		}
	}

	Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > createActivitiesMatrix(
		const std::unordered_map < size_t, size_t > &activityKeyToIndexMap) const
	{
		Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > activitiesMatrix;

		activitiesMatrix.setZero(
			this->activities.descriptions.size(),
			this->activities.descriptions.size());

		for (const auto &[identification1, description1] : this->activities.descriptions)
		{
			const auto conexoes = this->activities.dependencies.getConnections(identification1);

			if (!conexoes.has_value())
			{
				throw std::runtime_error("Could not find node " + std::to_string(identification1) + ".");
			}

			for (const auto &[identification2, description2] : this->activities.descriptions)
			{
				if (this->activities.dependencies.contains(identification1, identification2))
				{
					activitiesMatrix(
						activityKeyToIndexMap.at(identification1),
						activityKeyToIndexMap.at(identification2)) =
							conexoes.value().get().at(identification2).weight;
				}
			}
		}

		return activitiesMatrix;
	}

	Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > createProductsMatrix(
		const std::unordered_map < size_t, size_t > &productKeyToIndexMap) const
	{
		Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > productsMatrix;

		productsMatrix.setZero(
			this->products.descriptions.size(),
			this->products.descriptions.size());

		for (const auto &[identification1, description1] : this->products.descriptions)
		{
			for (const auto &[identification2, description2] : this->products.descriptions)
			{
				if (this->products.interactions.contains(identification1, identification2))
				{
					productsMatrix(
						productKeyToIndexMap.at(identification1),
						productKeyToIndexMap.at(identification2)) = 1;
				}
			}
		}

		return productsMatrix;
	}

	std::array < Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor >, 2 > createAffiliationsMatrices(
		const std::unordered_map < size_t, size_t > &activityKeyToIndexMap,
		const std::unordered_map < size_t, size_t > &productKeyToIndexMap) const
	{
		std::array < Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor >, 2 > affiliationsMatrices;

		affiliationsMatrices[0].setZero(
			this->activities.descriptions.size(),
			this->products.descriptions.size());

		affiliationsMatrices[1].setZero(
			this->activities.descriptions.size(),
			this->products.descriptions.size());

		for (const auto &[identification1, description1] : this->activities.descriptions)
		{
			auto iterator1 = this->affiliations.responsabilities.find(identification1);

			if (iterator1 == this->affiliations.responsabilities.cend())
			{
				throw std::runtime_error("Could not find the responsability of tge activity " + std::to_string(identification1) + ".");
			}

			for (const auto &[identification2, description2] : this->products.descriptions)
			{
				auto iterator2 = iterator1->second.find(identification2);
				
				if (iterator2 != iterator1->second.cend())
				{
					if (iterator2->second >= 1)
					{
						affiliationsMatrices[0](
							activityKeyToIndexMap.at(identification1),
							productKeyToIndexMap.at(identification2)) = 1;
					}

					if (iterator2->second >= 3)
					{
						affiliationsMatrices[1](
							activityKeyToIndexMap.at(identification1),
							productKeyToIndexMap.at(identification2)) = 1;
					}
				}
			}
		}

		return affiliationsMatrices;
	}

	static void createComparativeMatrixStep1(
		Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &comparativeMatrix,
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &strongPotentialMatrix,
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &activitiesMatrix)
	{
		for (Eigen::Index i = 0; i < comparativeMatrix.rows(); i++)
		{
			for (Eigen::Index j = 0; j < comparativeMatrix.cols(); j++)
			{
				if (i == j)
				{
					continue;
				}

				auto &celulaMatrizComparacaoFinal = comparativeMatrix(i, j);
				const auto celulaMatrizPotencialForte = strongPotentialMatrix(i, j);
				const auto celulaMatrizAtividades = activitiesMatrix(i, j);

				if (celulaMatrizPotencialForte > 0.0f && celulaMatrizAtividades == 0.0f)
				{
					celulaMatrizComparacaoFinal = OrganizationalAnalysis::indirectlyInvolved;
				}
			}
		}
	}

	static void createComparativeMatrixStep2(
		Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &finalComparativeMatrix,
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &totalPotentialMatrix,
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &activitiesMatrix)
	{
		for (Eigen::Index i = 0; i < finalComparativeMatrix.rows(); i++)
		{
			for (Eigen::Index j = 0; j < finalComparativeMatrix.cols(); j++)
			{
				if (i == j)
				{
					continue;
				}

				auto &finalComparativeMatrixCell = finalComparativeMatrix(i, j);
				const auto totalPotentialMatrixCell = totalPotentialMatrix(i, j);
				const auto activitiesMatrixCell = activitiesMatrix(i, j);

				if (activitiesMatrixCell != 0.0f)
				{
					if (totalPotentialMatrixCell != 0.0f)
					{
						finalComparativeMatrixCell = OrganizationalAnalysis::involved;
					}

					else
					{
						finalComparativeMatrixCell = OrganizationalAnalysis::directlyInvolved;
					}
				}
			}
		}
	}

	Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > eraseRedundancies(
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &strongPotentialMatrixWithRedundancies,
		const std::unordered_map < size_t, size_t > &activitiesToKeyIndexMap) const
	{
		auto strongPotentialMatrixWithoutRedundancies = strongPotentialMatrixWithRedundancies;

		for (Eigen::Index i = 0; i < strongPotentialMatrixWithoutRedundancies.rows(); i++)
		{
			for (Eigen::Index j = 0; j < strongPotentialMatrixWithoutRedundancies.cols(); j++)
			{
				auto &cell = strongPotentialMatrixWithoutRedundancies(i, j);

				if (cell != 0.0f)
				{
					const size_t key1 = activitiesToKeyIndexMap.at(static_cast < size_t > (i));
					const size_t key2 = activitiesToKeyIndexMap.at(static_cast < size_t > (j));
					const auto &peopleInCharge1 = this->activities.descriptions.at(key1).peopleInCharge;
					const auto &peopleInCharge2 = this->activities.descriptions.at(key2).peopleInCharge;

					bool isThereRedundancies = false;

					for (const auto personInCharge1 : peopleInCharge1)
					{
						for (const auto personInCharge2 : peopleInCharge2)
						{
							if (personInCharge1 == personInCharge2)
							{
								isThereRedundancies = true;

								break;
							}
						}

						if (isThereRedundancies)
						{
							break;
						}
					}

					if (isThereRedundancies)
					{
						cell = 0.0L;
					}
				}
			}
		}

		return strongPotentialMatrixWithoutRedundancies;
	}

	static void exportMatrix(
		const std::string &name,
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &matrix,
		const char separator)
	{
		xablau::file::File < char > file(name, std::ios_base::out | std::ios_base::trunc);

		for (Eigen::Index i = 0; i < matrix.rows(); i++)
		{
			for (Eigen::Index j = 0; j < matrix.cols(); j++)
			{
				if (matrix(i, j) != 0.0f)
				{
					file << matrix(i, j);
				}

				file << separator;
			}

			file << '\n';
		}
	}

	static std::vector < std::unordered_set < size_t > > identifyParallelizations(
		const xablau::graph::Digraph < float, size_t, xablau::graph::Connection < float > > &activitiesSequence)
	{
		const auto stronglyConnectedComponents = activitiesSequence.TarjanStronglyConnectedComponents();
		std::unordered_map < size_t, size_t > individualNodeToStronglyConnectedComponentMap;
		std::unordered_map < size_t, std::unordered_set < size_t > > stronglyConnectedComponentToIndividualNodeMap;
		std::random_device randomDevice;
		std::default_random_engine engine(randomDevice());
		const std::uniform_int_distribution < size_t > distribution(0, SIZE_MAX);

		for (
			const auto &[
				stronglyConnectedComponentIdentification,
				stronglyConnectedComponentElements] : stronglyConnectedComponents)
		{
			if (!individualNodeToStronglyConnectedComponentMap.contains(stronglyConnectedComponentIdentification))
			{
				size_t newIndex{};

				if (stronglyConnectedComponentElements.size() > 1)
				{
					do
					{
						newIndex = distribution(engine);
					} while (stronglyConnectedComponentToIndividualNodeMap.contains(newIndex));

					for (const auto index : stronglyConnectedComponentElements)
					{
						individualNodeToStronglyConnectedComponentMap.emplace(std::make_pair(index, newIndex));
					}
				}

				else
				{
					newIndex = stronglyConnectedComponentIdentification;

					individualNodeToStronglyConnectedComponentMap.emplace(std::make_pair(newIndex, newIndex));
				}

				stronglyConnectedComponentToIndividualNodeMap.emplace(std::make_pair(newIndex, stronglyConnectedComponentElements));
			}
		}

		xablau::graph::Digraph < float, size_t, xablau::graph::Connection < float > > mappedActivitiesSequence;

		for (const auto &[node, connections] : activitiesSequence.getConnections())
		{
			const size_t node1 = individualNodeToStronglyConnectedComponentMap.at(node);

			mappedActivitiesSequence.insert(node1);

			for (const auto &[connectedNode, connection] : connections)
			{
				const size_t node2 = individualNodeToStronglyConnectedComponentMap.at(connectedNode);

				if (node1 != node2)
				{
					mappedActivitiesSequence.insert(node1, node2);
				}
			}
		}

		std::vector < std::unordered_set < size_t > > finalActivitiesSequence;

		while (!mappedActivitiesSequence.empty())
		{
			const auto sourceNodes = mappedActivitiesSequence.getSourceNodes();

			finalActivitiesSequence.emplace_back(std::unordered_set < size_t > ());

			for (const auto sourceNode : sourceNodes)
			{
				for (const auto individualNode : stronglyConnectedComponentToIndividualNodeMap.at(sourceNode))
				{
					finalActivitiesSequence.back().insert(individualNode);
				}
			}

			for (const auto sourceNode : sourceNodes)
			{
				mappedActivitiesSequence.erase(sourceNode);
			}
		}

		return finalActivitiesSequence;
	}

	static void createReportNotConsideringRedundancies(
		const std::string &fileName,
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &comparativeMatrix,
		const std::unordered_map < size_t, size_t > &activityIndexToKeyMap)
	{
		xablau::file::File < char > report1(fileName, std::ios_base::out | std::ios_base::trunc);

		std::array < std::vector < size_t >, 3 > interactions1 =
		{
			std::vector < size_t > (comparativeMatrix.rows(), 0),
			std::vector < size_t > (comparativeMatrix.rows(), 0),
			std::vector < size_t > (comparativeMatrix.rows(), 0)
		};

		for (Eigen::Index i = 0; i < comparativeMatrix.rows(); i++)
		{
			for (Eigen::Index j = 0; j < comparativeMatrix.cols(); j++)
			{
				const auto cell1 = comparativeMatrix(i, j);

				if (cell1 == OrganizationalAnalysis::indirectlyInvolved)
				{
					interactions1[0][i]++;
				}

				else if (cell1 == OrganizationalAnalysis::involved)
				{
					interactions1[1][i]++;
				}

				else if (cell1 == OrganizationalAnalysis::directlyInvolved)
				{
					interactions1[2][i]++;
				}

				const auto cell2 = comparativeMatrix(j, i);

				if (cell2 == OrganizationalAnalysis::indirectlyInvolved)
				{
					interactions1[0][i]++;
				}

				else if (cell2 == OrganizationalAnalysis::involved)
				{
					interactions1[1][i]++;
				}

				else if (cell2 == OrganizationalAnalysis::directlyInvolved)
				{
					interactions1[2][i]++;
				}
			}

			report1 <<
				"Activity " << activityIndexToKeyMap.at(i) << "\n" <<
				"Attended interactions: " << interactions1[1][i] << "\n" <<
				"Unexpected interactions: " << interactions1[2][i] << "\n" <<
				"Unattended interactions: " << interactions1[0][i] << "\n\n";
		}

		report1 <<
			"Total of attended interactions: " << std::accumulate(interactions1[1].cbegin(), interactions1[1].cend(), size_t()) << "\n" <<
			"Total of unexpected interactions: " << std::accumulate(interactions1[2].cbegin(), interactions1[2].cend(), size_t()) << "\n" <<
			"Total of unattended interactions: " << std::accumulate(interactions1[0].cbegin(), interactions1[0].cend(), size_t());
	}
	
	static void createReportConsideringRedundancies(
		const std::string &fileName,
		const Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > &comparativeMatrix,
		const std::unordered_map < size_t, size_t > &activityIndexToKeyMap)
	{
		xablau::file::File < char > report2(fileName, std::ios_base::out | std::ios_base::trunc);

		std::array < std::vector < size_t >, 3 > interactions =
		{
			std::vector < size_t > (comparativeMatrix.rows(), size_t()),
			std::vector < size_t > (comparativeMatrix.rows(), size_t()),
			std::vector < size_t > (comparativeMatrix.rows(), size_t())
		};

		for (Eigen::Index i = 0; i < comparativeMatrix.rows(); i++)
		{
			for (Eigen::Index j = 0; j < comparativeMatrix.cols(); j++)
			{
				const auto cell1 = comparativeMatrix(i, j);

				if (cell1 == OrganizationalAnalysis::indirectlyInvolved)
				{
					interactions[0][i]++;
				}

				else if (cell1 == OrganizationalAnalysis::involved)
				{
					interactions[1][i]++;
				}

				else if (cell1 == OrganizationalAnalysis::directlyInvolved)
				{
					interactions[2][i]++;
				}

				const auto cell2 = comparativeMatrix(j, i);

				if (cell2 == OrganizationalAnalysis::indirectlyInvolved)
				{
					interactions[0][i]++;
				}

				else if (cell2 == OrganizationalAnalysis::involved)
				{
					interactions[1][i]++;
				}

				else if (cell2 == OrganizationalAnalysis::directlyInvolved)
				{
					interactions[2][i]++;
				}
			}

			report2 <<
				"Activity " << activityIndexToKeyMap.at(i) << "\n" <<
				"Attended interactions: " << interactions[1][i] << "\n" <<
				"Unexpected interactions: " << interactions[2][i] << "\n" <<
				"Unattended interactions: " << interactions[0][i] << "\n\n";
		}

		report2 <<
			"Total of attended interactions: " << std::accumulate(interactions[1].begin(), interactions[1].end(), size_t()) << "\n" <<
			"Total of unexpected interactions: " << std::accumulate(interactions[2].begin(), interactions[2].end(), size_t()) << "\n" <<
			"Total of unattended interactions: " << std::accumulate(interactions[0].begin(), interactions[0].end(), size_t());
	}

protected:
	struct Agent
	{
		struct Description
		{
			std::string group;
			std::string role;
		};

		std::map < size_t, Description > descriptions;
	};

	struct Activity
	{
		struct Description
		{
			std::string name;
			std::string group;
			std::unordered_set < size_t > peopleInCharge;
		};

		std::map < size_t, Activity::Description > descriptions;
		xablau::graph::Digraph < float, size_t, xablau::graph::Connection < float > > dependencies;
	};

	struct Product
	{
		struct Description
		{
			std::string name;
			std::string group;
		};

		std::map < size_t, Product::Description > descriptions;
		xablau::graph::Digraph < float, size_t, xablau::graph::Connection < float > > interactions;
	};

	struct Affiliation
	{
		std::map < size_t, std::map < size_t, float > > responsabilities;
	};

	Agent agents;
	Activity activities;
	Product products;
	Affiliation affiliations;

public:
	void read(
		const std::string &agentsFileName,
		const std::string &activitiesFileName,
		const std::string &productsFileName,
		const std::string &affiliationsFileName)
	{
		this->readAgents(agentsFileName);
		this->readActivities(activitiesFileName);
		this->readProducts(productsFileName);
		this->readAffiliations(affiliationsFileName);
	}

	std::vector < std::unordered_set < size_t > > identifyParallelizations() const
	{
		if (this->activities.dependencies.empty())
		{
			return std::vector < std::unordered_set < size_t > > ();
		}

		return OrganizationalAnalysis::identifyParallelizations(this->activities.dependencies.transpose());
	}

	std::unordered_map < size_t, size_t > identifyPriorities() const
	{
		if (this->activities.dependencies.empty())
		{
			return std::unordered_map < size_t, size_t > ();
		}

		const auto activitiesStraightSense =
			OrganizationalAnalysis::identifyParallelizations(this->activities.dependencies.transpose());

		auto activitiesReversedSense =
			OrganizationalAnalysis::identifyParallelizations(this->activities.dependencies);

		std::reverse(activitiesReversedSense.begin(), activitiesReversedSense.end());

		std::unordered_map < size_t, size_t > priorities;

		for (size_t i = 0; i < activitiesStraightSense.size(); i++)
		{
			for (const auto &straightActivity : activitiesStraightSense[i])
			{
				priorities.emplace(std::make_pair(straightActivity, i));
			}

			for (const auto &reversedActivity : activitiesReversedSense[i])
			{
				priorities.at(reversedActivity) = i - priorities.at(reversedActivity);
			}
		}

		return priorities;
	}

	void alignArchitectureProcessProductAndOrganization(
		const std::string &fileNameActivitiesMatrix,
		const std::string &fileNameWeakAffiliationsMatrix,
		const std::string &fileNameStrongAffiliationsMatrix,
		const std::string &fileNameProductsMatrix,
		const std::string &fileNameTotalPotentialMatrix,
		const std::string &fileNameStrongPotentialMatrixWithoutRedundancies,
		const std::string &fileNameStrongPotentialMatrixWithRedundancies,
		const std::string &fileNameComparativeMatrixWithoutRedundancies,
		const std::string &fileNameComparativeMatrixWithRedundancies,
		const std::string &fileNameReportNotConsideringRedundancies,
		const std::string &fileNameReportConsideringRedundancies,
		const char separator) const
	{
		std::unordered_map < size_t, size_t > activityKeyToIndexMap;
		std::unordered_map < size_t, size_t > activityIndexToKeyMap;
		std::unordered_map < size_t, size_t > productKeyToIndex;

		Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > productsMatrix;
		Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > activitiesMatrix;
		std::array < Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor >, 2 > affiliationsMatrices;
		std::array < Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor >, 2 > potentialMatrices;

		size_t index = 0;

		for (const auto &[identification, description] : this->activities.descriptions)
		{
			activityKeyToIndexMap.insert(std::make_pair(identification, index));
			activityIndexToKeyMap.insert(std::make_pair(index, identification));

			index++;
		}

		index = 0;

		for (const auto &[identification, description] : this->products.descriptions)
		{
			productKeyToIndex.insert(std::make_pair(identification, index));

			index++;
		}

		std::thread threadActivities(
			[this, &activitiesMatrix, &activityKeyToIndexMap] () -> void
			{
				activitiesMatrix = this->createActivitiesMatrix(activityKeyToIndexMap);
			});

		std::thread threadProducts(
			[this, &productsMatrix, &productKeyToIndex] () -> void
			{ 
				productsMatrix = this->createProductsMatrix(productKeyToIndex);
			});

		threadActivities.join();
		threadProducts.join();

		affiliationsMatrices = this->createAffiliationsMatrices(activityKeyToIndexMap, productKeyToIndex);

		potentialMatrices[0] = affiliationsMatrices[0] * productsMatrix * affiliationsMatrices[0].transpose();
		potentialMatrices[1] = affiliationsMatrices[1] * productsMatrix * affiliationsMatrices[1].transpose();

		auto strongPotentialMatrixWithoutRedundancies = this->eraseRedundancies(potentialMatrices[1], activityIndexToKeyMap);

		exportMatrix(fileNameActivitiesMatrix, activitiesMatrix, separator);
		exportMatrix(fileNameWeakAffiliationsMatrix, affiliationsMatrices[0], separator);
		exportMatrix(fileNameStrongAffiliationsMatrix, affiliationsMatrices[1], separator);
		exportMatrix(fileNameProductsMatrix, productsMatrix, separator);
		exportMatrix(fileNameTotalPotentialMatrix, potentialMatrices[0], separator);
		exportMatrix(fileNameStrongPotentialMatrixWithoutRedundancies, strongPotentialMatrixWithoutRedundancies, separator);
		exportMatrix(fileNameStrongPotentialMatrixWithRedundancies, potentialMatrices[1], separator);

		Eigen::Matrix < float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > comparativeMatrix;

		comparativeMatrix.setZero(activitiesMatrix.rows(), activitiesMatrix.cols());

		createComparativeMatrixStep1(comparativeMatrix, strongPotentialMatrixWithoutRedundancies, activitiesMatrix);
		createComparativeMatrixStep2(comparativeMatrix, potentialMatrices[0], activitiesMatrix);

		exportMatrix(fileNameComparativeMatrixWithoutRedundancies, comparativeMatrix, separator);

		createReportNotConsideringRedundancies(fileNameReportNotConsideringRedundancies, comparativeMatrix, activityIndexToKeyMap);

		comparativeMatrix.setZero(activitiesMatrix.rows(), activitiesMatrix.cols());

		createComparativeMatrixStep1(comparativeMatrix, potentialMatrices[1], activitiesMatrix);
		createComparativeMatrixStep2(comparativeMatrix, potentialMatrices[0], activitiesMatrix);

		exportMatrix(fileNameComparativeMatrixWithRedundancies, comparativeMatrix, separator);

		createReportConsideringRedundancies(fileNameReportConsideringRedundancies, comparativeMatrix, activityIndexToKeyMap);
	}

	void printAgents() const
	{
		std::cout << "Agent (identification, group, role):\n";

		for (const auto &[identification, description] : this->agents.descriptions)
		{
			std::cout << "\t" << identification << ", " << description.group << ", " << description.role << "\n";
		}

		std::cout << "\n";
	}

	void printActivities() const
	{
		std::cout << "Activities (identification, group, name, []people in charge):\n";

		for (const auto &[identification, description] : this->activities.descriptions)
		{
			std::cout << "\t" << identification << ", " << description.group << ", " << description.name << ",";

			for (const auto &item2 : description.peopleInCharge)
			{
				std::cout << " " << item2;
			}

			std::cout << "\n";
		}

		std::cout << "\nDependencies:\n";

		std::cout << this->activities.dependencies;

		std::cout << "\n\n";
	}

	void printProducts() const
	{
		std::cout << "Products (identification, group, name):" << "\n";

		for (const auto &[identification, description] : this->products.descriptions)
		{
			std::cout << "\t" << identification << ", " << description.group << ", " << description.name << "\n";
		}

		std::cout << "\nInteractions:\n";

		std::cout << this->products.interactions;

		std::cout << "\n\n";
	}

	void printAffiliations() const
	{
		std::cout << "Affiliations (activity : product, weight):" << "\n";

		for (const auto &[activityIdentification, description] : this->affiliations.responsabilities)
		{
			std::cout << activityIdentification << ":\n";

			for (const auto &[productIdentification, weight] : description)
			{
				std::cout << "\t" << productIdentification << ", " << weight << "\n";
			}
		}

		std::cout << "\n";
	}

	void print() const
	{
		this->printAgents();
		this->printActivities();
		this->printProducts();
		this->printAffiliations();
	}

	void exportAgents(const std::string &fileName, const char separator) const
	{
		xablau::file::File < char > arquivo(fileName, std::ios_base::out | std::ios_base::trunc);

		arquivo << "Identification" << separator << "Group" << separator << "Role\n";

		for (const auto &[identification, description] : this->agents.descriptions)
		{
			arquivo << identification;
			arquivo << separator;
			arquivo << description.group;
			arquivo << separator;
			arquivo << description.role;
			arquivo << '\n';
		}
	}

	void exportActivities(const std::string &fileName, const char separator) const
	{
		xablau::file::File < char > file(fileName, std::ios_base::out | std::ios_base::trunc);

		file << "Person in charge" << separator << "Group" << separator << "Activity" << separator;

		for (const auto & [identification, description] : this->activities.descriptions)
		{
			file << separator;
			file << identification;
		}

		file << '\n';

		for (const auto &[identification1, description1] : this->activities.descriptions)
		{
			for (const auto item : description1.peopleInCharge)
			{
				file << this->agents.descriptions.at(item).role;
				file << (separator == ',' ? ';' : ',') << ' ';
			}

			if (!description1.peopleInCharge.empty())
			{
				file.seekp(-2, std::ios_base::cur);
			}

			file << separator;

			file << description1.group;
			file << separator;

			file << description1.name;
			file << separator;

			file << identification1;
			file << separator;

			for (const auto &[identification2, description2] : this->activities.descriptions)
			{
				if (identification1 == identification2)
				{
					file << "####";
					
				}

				else
				{
					if (this->activities.dependencies.contains(identification1, identification2))
					{
						file << 1;
					}
				}

				file << separator;
			}

			file.seekp(-1, std::ios_base::cur);
			file << '\n';
		}
	}

	void exportProducts(const std::string &fileName, const char separator) const
	{
		xablau::file::File < char > file(fileName, std::ios_base::out | std::ios_base::trunc);

		file << "Group" << separator << "Product" << separator;

		for (const auto &[identification, description] : this->products.descriptions)
		{
			file << separator;
			file << identification;
		}

		file << '\n';

		for (const auto &[identification1, description1] : this->products.descriptions)
		{
			file << description1.group;
			file << separator;

			file << description1.name;
			file << separator;
			file << identification1;
			file << separator;

			for (const auto &[identification2, description2] : this->products.descriptions)
			{
				if (identification1 == identification2)
				{
					file << "####";

				}

				else
				{
					if (this->products.interactions.contains(identification1, identification2))
					{
						file << 1;
					}
				}

				file << separator;
			}

			file.seekp(-1, std::ios_base::cur);
			file << '\n';
		}
	}

	void exportAffiliations(const std::string &fileName, const char separator) const
	{
		xablau::file::File < char > arquivo(fileName, std::ios_base::out | std::ios_base::trunc);

		arquivo << "Descriptions" << separator;

		for (const auto &[identification, description] : this->products.descriptions)
		{
			arquivo << separator;
			arquivo << description.name;
		}

		arquivo << '\n' << separator << "Activities Indices/Product";

		for (const auto &[identification, description] : this->products.descriptions)
		{
			arquivo << separator;
			arquivo << identification;
		}

		arquivo << '\n';

		for (const auto &[identification1, description1] : this->affiliations.responsabilities)
		{
			arquivo << this->activities.descriptions.at(identification1).name;
			arquivo << separator;

			arquivo << identification1;
			arquivo << separator;

			auto iterator = description1.cbegin();

			for (const auto &[identification2, description2] : this->products.descriptions)
			{
				if (iterator != description1.cend() && iterator->first == identification2)
				{
					arquivo << iterator->second;

					++iterator;
				}

				arquivo << separator;
			}

			arquivo << '\n';
		}
	}

	void exportDatabase(
		const std::string &agentsFileName,
		const std::string &activitiesFileName,
		const std::string &productsFileName,
		const std::string &affiliationsFileName,
		const char separator) const
	{
		this->exportAgents(agentsFileName, separator);
		this->exportActivities(activitiesFileName, separator);
		this->exportProducts(productsFileName, separator);
		this->exportAffiliations(affiliationsFileName, separator);
	}

	~OrganizationalAnalysis() = default;
};

#endif