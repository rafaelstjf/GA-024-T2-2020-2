# Segundo trabalho de estrutura de dados

**Nome:**  Rafael de Souza Terra

## Sumário 
___
1. Estruturas de dados
2. Função de *Hashing*
3. Impressão
3. Testes

## Estruturas de dados
___
Para a implementação do trabalho a tabela *hash* foram utilizadas as três estruturas apresentadas abaixo:

```c
typedef struct occurrences{
    int line;
    struct occurrences *next;
} Occurrences;

typedef struct index_node{
    char *key;                    
    int num_occurrences;           
    Occurrences *occurrences_list; occurrences
    struct index_node *collisions; 
} Index_node;

struct index{
    char *text_file;
    Index_node **array;
    int num_keys;
    int table_size;
}; 
```
A tabela *Hash* consiste em um vetor de ponteiros do tipo `Index_node`, onde cada um desses ponteiros possui um ponteiro do tipo `Occurrences` onde é armazenado as linhas da ocorrência da palavra-chave no texto e um ponteiro do próprio `Index_node` para salvar as colisões geradas.

![Estrutura](/image.png)

## Função de *Hashing* 
___
Considerando as convenções adotadas no trabalho (palavras-chave de no máximo 16 caracteres) a função de *hashing* realiza o somatório do valor na tabela ASCII de cada letra multiplicado pela sua posição na cadeia de caracteres. No final é realizado a congruência desse valor com o tamanho da tabela.

![equação](./eq.png)

## Impressão
___

Para a impressão os valores foram armazenados em um vetor de cadeias de caracteres e ordenados utilizando o algoritmo *QuickSort*.

