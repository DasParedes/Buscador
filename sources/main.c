#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "../headers/main.h"

#ifndef ABP_H 
#include "../headers/abp.h"
#endif

#ifndef LDE_H
#include "../headers/lde.h"
#endif

#ifndef TIPOCONSULTA_H
#include "../headers/tipoconsulta.h"
#endif

#ifndef DEBUG_MESSAGES
#define DEBUG_MESSAGES

#define DEBUG(msg) printf("DEBUG: %s\n", msg);
#define ERROR(msg) printf("ERROR: %s\n", msg);
#define WARN(msg) printf("WARNING: %s\n", msg);
#endif

char *parse_string(char *str);

int main(int args, char *argc[]){
	struct timeval t_inicio, t_fim_insert, t_fim_programa;
	gettimeofday(&t_inicio, NULL);

	char *entrada, *operacoes, *saida;
	FILE *file_entrada, *file_operacoes;
	int buffer_size = 255;
	char buffer[buffer_size];

	// Estruturas
	// Guarda os locais onde foram feitas as consultas
	abp_node *abp_locais = NULL;
	lde_descritor *des_lde_cidades = inicializa_lde_descritor();

	// guarda as consultas que foram realizadas
	// guarda os termos que foram procurados
	abp_node *abp_termos = NULL;
	//LDE_Node *lista_termos = inicializa_LDE_Node();
	lde_descritor *des_lde_termos = inicializa_lde_descritor();
	
	abp_node *abp_consultas = NULL; // consultas em todo o arquivo
	//lde_descritor *des_lde_consultas = inicializa_lde_descritor(); // consultas em todo o arquivo

	if(args < 3){
		ERROR("Does not have the right number of parameters\n");
		return 1;
	}

	entrada = argc[1];
	operacoes = argc[2];
	saida = argc[3];

	printf("Nome do arquivo de entrada: %s\n", entrada);
	printf("Nome do arquivo de operações: %s\n", operacoes);
	printf("Nome do arquivo de saida: %s\n", saida);
	
	// Open the files
	file_entrada = fopen(entrada, "r");
	file_operacoes = fopen(operacoes, "r");

	// Verifing if the files exist
	if(!file_entrada){
		ERROR("The file of input does not exists\n");
		fclose(file_entrada);
		return 1;
	} else
	if (!file_operacoes){
		ERROR("The file of operations does not exists");
		fclose(file_operacoes);
		return 1;
	}
	
	// reading the lines of the file of input
	printf("\nReading for file of input:\n");
	
	int i=0, MAX = 10;
	while(!feof(file_entrada) && i++ < MAX){
		TipoConsulta *nodo_consulta = inicializa_nodo_consulta();
		LDE_Node *termos = inicializa_LDE_Node();
			
		nodo_consulta->termos = termos;

		fgets(buffer, buffer_size, file_entrada);

		// setting the strtok to get the locality and the terms between the tokens ";"
		char *locality = strtok(buffer, ";");
		char *token = strtok(NULL, ";");

		printf("Parsing string\n");
		parse_string(locality);
		printf("Procurando na abp\n");
		// tenta inserir, contando quando tenta inserir o mesmo nodo e devolve o nodo inserido
		abp_node *abp_localidade = inserir_abp_node(&abp_locais, locality);
		if(abp_localidade->lde_cidade == NULL){
			LDE_Node *lde_cidade = insere_info_lde_inicio(des_lde_cidades, abp_localidade->chave);
			// arrumar, transformar em ponteiros;
			// facilita pra achar a cidade pelo nome ou as cidade mais ou menos pesquisadas
			abp_localidade->lde_cidade = lde_cidade;
		}
		else {
			// atualiza a frequencia da cidade atual
			abp_localidade->lde_cidade->frequencia = abp_localidade->frequencia;
			// ordenar cidade mais pesquisadas
		}
		
		//abp_cidade->node = nodo_consulta->termos;
		printf("ABP pronta\n");

		while( token != NULL ){
			parse_string(token);

			printf("\t\tLDE - Inserindo na consulta\n");
			insere_info_LDE_Node_alf(&termos, token);

			printf("\tABP - Inserindo na abp_termos\n");
			abp_node *termo = inserir_abp_node(&abp_termos, token);
			if(termo->node == NULL) printf("Novo termo inserido a lista\n");
			else printf("termo: chave: %s\n", termo->chave);

			printf("\tLDE - Inserindo na lista de termos do arquivo\n");
			// ordem alfabetica
			//LDE_Node *lde_atual = insere_info_LDE_Node_alf(&lista_termos, token);

			// ordem de frequencia
			LDE_Node *lde_atual = insere_termo_lde(des_lde_termos, termo->node, token);			
			termo->node = lde_atual;
			printf("nodo inserido: %s\n", termo->node->info);

			token = strtok(NULL, ";");
		}

		printf("%s: termos: ", abp_localidade->chave);
		imprime_lde_linha(termos);
		printf("lista_termos: ");
		
		// insere consulta(lde de termos) na abp de consultas do arquivo
		printf("\nInserindo consulta na abp\n");
		abp_node *abp_novo_node = abp_insere_consulta2(&abp_consultas, termos);
		if(abp_novo_node->consultas == NULL) printf("node vazio");
		/*
		printf("Inserindo consulta na lde\n");
		LDE_Node *lde_atual = insere_consulta_lde(des_lde_consultas, abp_novo_node->consultas, termos);
		abp_novo_node->consultas = lde_atual;
		*/

		printf("Consulta inserida: \n");
		imprime_lde_consultas(abp_novo_node->consultas);
		printf("Lista geral: ");
		imprime_lde_linha(des_lde_termos->inicio);
		printf("\n\n");

	}

	gettimeofday(&t_fim_insert, NULL);

	printf("\nlista_termos do arquivo: \n");
	imprime_lde(des_lde_termos->inicio);

	printf("\nLista_termos inverso: \n");
	printf("Termos mais procurados no arquivo\n");
	imprime_inverso_lde(des_lde_termos->fim);

	printf("\nabp_termos: \n");
	imprime_abp(abp_termos, 0);

	printf("\nabp de consultas do arquivo\n");
	imprime_abp_consulta(abp_consultas, 0);

	// Testando a procura na abp pelo termo
	printf("\nProcurando e imprimindo lista a partir do termo '%s'\n", "avl");
	abp_node *procura_termo = abp_procurar_chave(abp_termos, "avl");
	if(procura_termo != NULL)
		imprime_lde(procura_termo->node);

	//printf("Imprime inverso:\n");
	//imprime_inverso_lde(lista_termos);
	//LDE_Node *consulta = consultas[0];
	//printf("consulta[0]: %s\n", consulta->info);
	printf("\nLocais:\n");
	imprime_abp(abp_locais, 0);

	printf("\nLocais por ordem de acesso crescente\n");
	imprime_lde(des_lde_cidades->inicio);
	printf("Fim: %s", des_lde_cidades->fim->info);

	printf("\nTestando procura abp cidade procurando '%s'\n", "salvador");
	abp_node *retorno_procurar = abp_procurar_chave(abp_locais, "salvador");
	if(retorno_procurar != NULL)
		printf("Chave retorno: %s\n", retorno_procurar->chave);

	printf("\nProcurando termo '%s': \n", "russia");
	abp_node *retorno_termo = abp_procurar_chave(abp_termos, "russia");
	if(retorno_termo != NULL)
		printf("chave %s [%d]\n", retorno_termo->chave, retorno_termo->frequencia);

	// closing the files
	fclose(file_operacoes);
	fclose(file_entrada);

	gettimeofday(&t_fim_programa, NULL);
	//printf("tempos: %d - %d - %d", t_inicio, t_fim_insert, t_fim_programa);
	//printf("Tempo de insercao: %ds\n", t_fim_insert - t_inicio);
	//printf("tempo de operacao: %ds\n", t_fim_programa - t_fim_insert);
	return 0;
}



// parsing the string receives,
// all the characters to lower case
// if end the string with lineFeed, removes the lineFeed
char *parse_string(char *str){
	int len = strlen(str);
	int i;
	for(i=0; i<len; i++){
		//printf("\n[%d]: %c->",i, str[i]);
		
		str[i] = tolower(str[i]);
		switch(str[i]){
			case 10: // 10 = \n = Line Feed
				str[i] = 0;
				break;
			case -61:  // acentuação
				// case 'Á': 2 chars: [-61] e [-127]
				if(str[i+1] == -127){
					// replaces the first character
					str[i] = 'a';
					// shift the rest of the string 1 pra esquerda
					strncpy( (char*)(str+i+1),(char*)(str+i+2), len-i);
				} else // case 'ã': 2 chars [-61] e [-93]
				if(str[i+1] == -93){
					// replaces the first character
					str[i] = 'a';
					// shift the rest of the string 1 pra esquerda
					strncpy( (char*)(str+i+1),(char*)(str+i+2), len-i);
				}
				break;
		}
		//printf("%1c[%d]\n",str[i],str[i]);
	}
	return str;
}