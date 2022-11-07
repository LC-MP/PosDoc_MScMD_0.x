#include "../Architecture/OrganizationalAnalysis.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#if defined(_WIN32) || defined(_WIN64)
#include <crtdbg.h>
#endif
#endif

int main()
{
#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	try
	{
		OrganizationalAnalysis sistema;

		sistema.read("Agentes.txt", "Atividades.txt", "Produtos.txt", "Afiliações.txt");
		//sistema.print();
		sistema.exportDatabase(
			"Agentes.csv",
			"Atividades.csv",
			"Produtos.csv",
			"Afiliações.csv",
			';');

		sistema.alignArchitectureProcessProductAndOrganization(
			"MatrizAtividades.csv",
			"MatrizAfiliaçõesFracas.csv",
			"MatrizAfiliaçõesFortes.csv",
			"MatrizProdutos.csv",
			"MatrizPotencialTotal.csv",
			"MatrizPotencialForteSemRedundância.csv",
			"MatrizPotencialForteComRedundância.csv",
			"MatrizComparacaoSemRedundância.csv",
			"MatrizComparacaoComRedundância.csv",
			"RelatórioSemRedundância.txt",
			"RelatórioComRedundância.txt",
			';');

		const auto parallelizations = sistema.identifyParallelizations();

		size_t counter = 0;

		for (size_t i = 0; i < parallelizations.size(); i++)
		{
			for (size_t atividade : parallelizations[i])
			{
				counter++;

				std::cout << atividade << " ";
			}

			std::cout << '\n';
		}

		std::cout << '\n' << "Foram registradas " << counter << " atividades em " <<
			parallelizations.size() << " tempos." << '\n' << '\n';

		const auto prioridades = sistema.identifyPriorities();

		std::map < size_t, size_t > prioridadesOrdenadas;

		for (const auto &prioridade : prioridades)
		{
			prioridadesOrdenadas.insert(prioridade);
		}

		std::cout << "Prioridades:" << '\n';

		for (const auto &prioridade : prioridadesOrdenadas)
		{
			std::cout << prioridade.first << ": " << prioridade.second << '\n';
		}
	}
	catch (std::exception &excecao)
	{
		std::cerr << '\n' << excecao.what() << '\n';
	}

	return 0;
}