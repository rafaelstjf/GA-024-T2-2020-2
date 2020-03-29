#ifndef INDEX_H
#define INDEX_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
typedef struct index Index;
/**
 * le o arquivo key_file com as palavras chaves a serem usadas, 
 * o indice criado deve ser retornado em idx
 */
int index_createfrom(const char *key_file, const char *text_file, Index **idx);
/**
 * Devolve em occurrences um vetor de inteiros (alocado dinamicamente) contendo todas as
 * ocorrencias (linhas) da palavra-chave key armazenadas pelo indice remissivo apontado por idx.
 * O numero de entradas de occurences eh devolvido em num_occurrences
 */
int index_get(const Index *idx, const char *key, int **occurrences, int *num_occurrences);
/**
 * Inclui no indice a palavra-chave key, associando-a a todas as ocorrencias no arquivo de texto
 * usado na construcao do indice. Quando a palavra-chave ja existe atualiza todas as ocorrencias dela 
 * no texto)
 */
int index_put(Index *idx, const char *key);
/**
 * Imprime para o stdout o indice remissivo completo em ordem alfabetica, referenciadas por idx
 * o formato de saida eh: 
 * and: 4, 5, 6
 * be: 3
 * ...
 */
int index_print(const Index *idx);

/**
 * destrutor
*/
int index_destroy_hash(Index **idx);

#endif