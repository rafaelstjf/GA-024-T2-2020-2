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
    struct index_node *collisions; //colisoes
} Index_node;
struct index
{
    char *text_file; //nome do arquivo de texto
    Index_node **array;
};
static char *substring(char *source, int beg, int end)
{
    char *sub = (char *)malloc(sizeof(char) * (end - beg + 1));
    int j = 0;
    for (int i = beg; i < end; i++)
    {
        sub[j] = source[i];
        j++;
    }
    sub[j] = '\0';
    return sub;
}
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
        int factor = 4;
        while (key[i] != '\0')
        {
            result = factor * result + key[i];
            i++;
        }
        return (result % M);
    }
    else
        return -1;
}
/**
 * Opens a file and saves its contents in a char*
 */

static int index_readfile(char **strstream, const char *file_name)
{
    FILE *input;
    input = fopen(file_name, "r");
    unsigned int buffer_size = 256;
    if (!input)
    {
        fprintf(stdout, "Failed to open the file!\n");
        return false;
    }
    else
    {
        printf("to aqui\n");
        *strstream = calloc(1, 1); //aloca e inicia tudo com 0
        char buffer[buffer_size];
        while (fgets(buffer, buffer_size, input)) // read from stdin
        {
            (*strstream) = realloc((*strstream), strlen(*strstream) + 1 + strlen(buffer));
            if (!strstream)
            {
                return false;
            }
            strcat((*strstream), buffer);
        }
        fclose(input);
        printf("Texto:\n%s\n", *strstream);
        return true;
    }
}
static int index_addkeys(const char *key_file, Index **idx)
{
    char *search = "\n";
    char *token;
    char *strstream;
    if (index_readfile(&strstream, key_file) == false)
        return false;
    printf("CHAVES: \n%s\n", strstream);
    token = strtok(strstream, search);
    if (!token || !(*idx))
        return false;
    while (token)
    {
        int index = index_hashing_funct(token);
        printf("Token: %s \tIndice: %d\n", token, index);
        if (index >= 0)
        {
            if (!(*idx)->array[index])
            {
                printf("criando no\n");
                (*idx)->array[index] = (Index_node *)malloc(sizeof(Index_node));
                (*idx)->array[index]->key = token;
                (*idx)->array[index]->occurrences_list = NULL;
                (*idx)->array[index]->collisions = NULL;
                (*idx)->array[index]->num_ocorrences = 0;
            }
            else
            {
                //cria colisao
                Index_node *it = (*idx)->array[index];
                Index_node *ant = NULL;
                while (it && (strcmp(it->key, token) != 0))
                {
                    ant = it;
                    it = it->collisions;
                }
                if (!it)
                {
                    //elemento ainda nao existe na lista
                    ant->collisions = (Index_node *)malloc(sizeof(Index_node));
                    ant->collisions->key = token;
                    ant->collisions->occurrences_list = NULL;
                    ant->collisions->collisions = NULL;
                    ant->collisions->num_ocorrences = 0;
                }
            }
        }
        token = strtok(NULL, search);
    }
    return true;
}
static int index_addtext(const char *text_file, Index **idx, int clean)
{
    if (!idx)
        return false;
    if (clean == true)
    {
        for (unsigned int i = 0; i < M; i++)
        {
            Index_node *it = (*idx)->array[i];
            while (it)
            {
                Occurrences *ant = NULL, *it_o = it->occurrences_list;
                while (it_o)
                {
                    ant = it_o;
                    it_o = it_o->next;
                    //free(ant);
                    ant = NULL;
                }
                it = it->collisions;
            }
        }
    }
    char *search = "\n";
    char *strstream;
    char *token;
    char *token_space;
    int beg;
    if (index_readfile(&strstream, text_file) == false)
        return false;
    token = strtok(strstream, search); //divide tudo em linhas;
    if (!token)
        return false;
    int line = 1;
    while (token)
    {
        token_space = strchr(token, ' ');
        beg = 0;
        int next_line = false;
        while (next_line == false)
        {
            char *sub;
            if (!token_space)
            {
                sub = substring(token, beg, strlen(token));
                next_line = true;
            }
            else
                sub = substring(token, beg, token_space - token);
            beg = token_space - token + 1;
            //int token_len = strlen(tk);
            int index = index_hashing_funct(sub);
            printf("Token-2: %s\tIndice: %d\n", sub, index);
            if (index >= 0 && (*idx)->array[index] != NULL)
            {
                Index_node *it = (*idx)->array[index];
                while (it && strcmp(it->key, sub) != 0)
                {
                    it = it->collisions;
                }
                if (it)
                {
                    if (!it->occurrences_list)
                    {
                        printf("to na delicinha da chave: %s\n", sub);

                        it->occurrences_list = (Occurrences *)malloc(sizeof(Occurrences));
                        it->occurrences_list->line = line;
                        it->occurrences_list->next = NULL;
                        it->num_ocorrences++;
                    }
                    else
                    {
                        printf("to na desgraca da chave: %s\n", sub);
                        Occurrences *it_o = it->occurrences_list;
                        while (it_o->next)
                        {
                            it_o = it_o->next;
                            printf("to em um loop infinito demoniaco\n");
                        }
                        Occurrences *c = (Occurrences *)malloc(sizeof(Occurrences));
                        c->line = line;
                        c->next = NULL;
                        it_o->next = c;
                        it->num_ocorrences++;
                    }
                }
            }
            if (token_space)
                token_space = strchr(token_space + 1, ' ');
            free(sub);
        }
        //if ((token[token_len - 1] >= 33 && token[token_len - 1] <= 47) || (token[token_len - 1] >= 58 && token[token_len - 1] <= 64))
        //  token[token_len - 1] = '\0';
        token = strtok(NULL, search);
        line++;
    }
    free(token);
    free(strstream);
    return true;
}
int index_createfrom(const char *key_file, const char *text_file, Index **idx)
{
    (*idx) = (Index *)malloc(sizeof(Index));
    (*idx)->text_file = (char *)malloc((strlen(text_file) + 1) * sizeof(char));
    strcpy((*idx)->text_file, text_file);
    (*idx)->array = malloc(M * sizeof(Index_node *)); //alocacao dinamica de um array de ponteiros de index_node
    for (unsigned int i = 0; i < M; i++)
    {
        (*idx)->array[i] = NULL;
    }
    if (index_addkeys(key_file, idx) == false)
        return false;
    if (index_addtext(text_file, idx, false) == false)
        return false;

    return true;
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
        //limpa as ocorrencias da tabela
        int index_key = index_hashing_funct(key);
        if (index_key >= 0)
        {
            if (!idx->array[index_key])
            {
                printf("criando no\n");
                idx->array[index_key] = (Index_node *)malloc(sizeof(Index_node));
                idx->array[index_key]->key = (char *)malloc(sizeof(char) * strlen(key));
                strcpy(idx->array[index_key]->key, key);
                idx->array[index_key]->occurrences_list = NULL;
                idx->array[index_key]->collisions = NULL;
                idx->array[index_key]->num_ocorrences = 0;
            }
            else
            {
                //cria colisao
                Index_node *it = idx->array[index_key];
                Index_node *ant = NULL;
                while (it && (strcmp(it->key, key) != 0))
                {
                    ant = it;
                    it = it->collisions;
                }
                if (!it)
                {
                    //elemento ainda nao existe na lista
                    ant->collisions = (Index_node *)malloc(sizeof(Index_node));
                    ant->collisions->key = (char *)malloc(sizeof(char) * strlen(key));
                    strcpy(ant->collisions->key, key);
                    ant->collisions->occurrences_list = NULL;
                    ant->collisions->collisions = NULL;
                    ant->collisions->num_ocorrences = 0;
                }
            }
        }
        if (index_addtext(idx->text_file, &idx, true) == false)
            return false;
        return true;
    }
    return false;
}
int index_print(const Index *idx)
{
    if (idx)
    {
        for (unsigned int i = 0; i < M; i++)
        {
            Index_node *it = idx->array[i];
            while (it)
            {
                printf("%s: ", it->key);
                Occurrences *it2 = it->occurrences_list;
                if (it2)
                {

                    while (it2->next)
                    {
                        printf("%d, ", it2->line);
                        it2 = it2->next;
                    }
                    if (it2)
                        printf("%d\n", it2->line);
                }
                printf("\n");
                it = it->collisions;
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