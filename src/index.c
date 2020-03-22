#include "index.h"
static const int false = 1;
static const int true = 0;
#define M 128
typedef struct occurrences
{
    int line;
    struct occurrences *next;
} Occurrences;
typedef struct index_node
{
    char *key; //palavra-chave
    int num_ocorrences;
    Occurrences *occurrences_list; //ocorrencias
} Index_node;
struct index
{
    char *text_file; //nome do arquivo de texto
    Index_node **array;
};
static char *replace_char(char *input, char find, char replace)
{
    /*funcao para trocar um caractere. 
Fonte: https://stackoverflow.com/questions/28637882/c-replace-one-character-in-an-char-array-by-another */

    char *output = (char *)malloc(strlen(input));

    for (int i = 0; i < strlen(input); i++)
    {
        if (input[i] == find)
            output[i] = replace;
        else
            output[i] = input[i];
    }

    output[strlen(input)] = '\0';

    return output;
}

static int index_hashing_funct(const char *key)
{
    if (key)
    {
        //depois eu penso em uma funcao maneira
        int result = 0;
        int off = 0;
        int i = 0;
        while (key[i] != '\0')
        {
            result = 4 * result + key[i];
            //printf("valor de key[i]: %d\n", key[i]);
            i++;
        }
        return (result % M);
    }
    else
        return -1;
}
int index_readFile(char **strstream, const char *file_name)
{
    FILE *input;
    input = fopen(file_name, "r");
    if (!input)
    {
        fprintf(stderr, "Failed to open the file!\n");
        return false;
    }
    else
    {
        if (*strstream)
            free(*strstream);
        unsigned int buffer_size = 256;
        *strstream = calloc(1, 1); //aloca e inicia tudo com 0
        char buffer[buffer_size];
        while (fgets(buffer, buffer_size, input)) // read from stdin
        {
            (*strstream) = realloc((*strstream), strlen(*strstream) + 1 + strlen(buffer));
            if (!(*strstream))
            {
                return false;
            }
            strcat((*strstream), buffer);
        }
        fclose(input);
        return true;
    }
}
int index_createfrom(const char *key_file, const char *text_file, Index **idx)
{
    char *search = "\n";
    char *token;
    char *strstream;
    //le o key_file
    if (index_readFile(&strstream, key_file) == false)
        return false;
    printf("CHAVES: \n%s\n", strstream);
    token = strtok(strstream, search); //divide tudo em linhas;
    if (token)
    {
        (*idx) = (Index *)malloc(sizeof(Index));
        (*idx)->text_file = (char *)malloc((strlen(text_file) + 1) * sizeof(char));
        strcpy((*idx)->text_file, text_file);
        (*idx)->array = malloc(M * sizeof(Index_node *)); //alocacao dinamica de um array de ponteiros de index_node
        while (token)
        {
            int index = index_hashing_funct(token);
            printf("Token: %s \tIndice: %d\n", token, index);
            if (index >= 0)
            {
                if (!(*idx)->array[index])
                {
                    (*idx)->array[index] = (Index_node *)malloc(sizeof(Index_node));
                    (*idx)->array[index]->key = token;
                    (*idx)->array[index]->num_ocorrences = 0;
                }
                else
                {
                    //ja existe elemento nesse indice
                }
            }
            else
            {
                //algum erro na criacao do indice
                free((*idx)->text_file);
                free((*idx)->array);
                printf("deu erro na criacao do indice!\n");
                return false;
            }
            token = strtok(NULL, search);
        }
    }
    else
        return false; //arquivo de entrada vazio
                      //le texto
    if (index_readFile(&strstream, text_file) == false)
        return false;
    replace_char(strstream, '\n', ' ');
    char *search_temp = " ";
    printf("Texto:\n%s\n", strstream);
    token = strtok(strstream, search_temp); //divide tudo em linhas;
    if (token)
    {
        int line = 1;
        while (token)
        {
            int token_len = strlen(token);
            //if ((token[token_len - 1] >= 33 && token[token_len - 1] <= 47) || (token[token_len - 1] >= 58 && token[token_len - 1] <= 64))
            //  token[token_len - 1] = '\0';
            int index = index_hashing_funct(token);
            printf("Token-2: %s \tIndice: %d\n", token, index);
            if (index >= 0 && (*idx)->array[index])
            {
                if (!(*idx)->array[index]->occurrences_list)
                {
                    (*idx)->array[index]->occurrences_list = (Occurrences *)malloc(sizeof(Occurrences));
                    (*idx)->array[index]->occurrences_list->line = line;
                    (*idx)->array[index]->occurrences_list->next = NULL;
                    (*idx)->array[index]->num_ocorrences++;
                }
                else
                {
                    Occurrences *it = (*idx)->array[index]->occurrences_list;
                    while (it->next)
                    {
                        it = it->next;
                    }
                    Occurrences *c = (Occurrences *)malloc(sizeof(Occurrences));
                    c->line = line;
                    c->next = NULL;
                    it->next = c;
                    (*idx)->array[index]->num_ocorrences++;
                }
            }
            token = strtok(NULL, search_temp);
        }
        return true;
    }
    else
        return false; //arquivo de entrada vazio
                      //le texto
}
int index_get(const Index *idx, const char *key, int **occurrences, int *num_occurrences)
{
    if (idx)
    {
        int index = index_hashing_funct(key);
        if (idx->array[index])
        {
            num_occurrences = &(idx->array[index]->num_ocorrences); //talvez esteja errado
            if (!occurrences)
                free(occurrences);
            (*occurrences) = malloc(sizeof(int) * idx->array[index]->num_ocorrences);
            Occurrences *it = idx->array[index]->occurrences_list;
            unsigned int i = 0;
            while (it)
            {
                (*occurrences)[i] = it->line;
                it = it->next;
            }
            return true;
        }
    }
    return false;
}
int index_put(Index *idx, const char *key)
{
    if (idx)
    {
        int index_key = index_hashing_funct(key);
        if (!idx->array[index_key])
        {
            idx->array[index_key] = (Index_node *)malloc(sizeof(Index_node));
            strcpy(idx->array[index_key]->key, key);
            idx->array[index_key]->num_ocorrences = 0;
        }
        else //le o texto de novo
        {
            char *strstream;
            char *token;
            char *search = "\n";
            if (index_readFile(&strstream, idx->text_file) == false)
                return false;
            replace_char(strstream, '\n', ' ');
            char *search_temp = " ";
            token = strtok(strstream, search_temp); //divide tudo em linhas;
            if (token)
            {
                int line = 1;
                while (token)
                {

                    int token_len = strlen(token);
                    if ((token[token_len - 1] >= 33 && token[token_len - 1] <= 47) || (token[token_len - 1] >= 58 && token[token_len - 1] <= 64))
                        token[token_len - 1] = '\0';
                    int index = index_hashing_funct(token);
                    if (index >= 0 && index == index_key)
                    {
                        if (!idx->array[index]->occurrences_list)
                        {
                            idx->array[index]->occurrences_list = (Occurrences *)malloc(sizeof(Occurrences));
                            idx->array[index]->occurrences_list->line = line;
                            idx->array[index]->occurrences_list->next = NULL;
                            idx->array[index]->num_ocorrences++;
                        }
                        else
                        {
                            Occurrences *it = idx->array[index]->occurrences_list;
                            while (it->next)
                            {
                                it = it->next;
                            }
                            Occurrences *c = (Occurrences *)malloc(sizeof(Occurrences));
                            c->line = line;
                            c->next = NULL;
                            it->next = c;
                            idx->array[index]->num_ocorrences++;
                        }
                    }
                    token = strtok(NULL, search);
                }
                return true;
            }
            else
                return false; //arquivo de entrada vazio
        }
    }
    return false;
}
int index_print(const Index *idx)
{
    if (idx)
    {
        for (unsigned int i = 0; i < M; i++)
        {
            if (idx->array[i])
            {
                printf("%s: ", idx->array[i]->key);
                Occurrences *it = idx->array[i]->occurrences_list;
                while (it && it->next)
                {
                    printf("%d, ", it->line);
                    it = it->next;
                }
                if (it)
                    printf("%d\n", it->line);
                else
                    printf("\n");
            }
        }
        return true;
    }
    else
        return false;
}
int index_destroy_hash(Index **idx)
{
    if (!idx)
        return true;
    else
    {
        //destroy
    }
}