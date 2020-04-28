#include "index.h"
static const int false = 1;
static const int true = 0;
#define FACTOR 64
typedef struct occurrences
{
    int line;
    struct occurrences *next;
} Occurrences;
typedef struct index_node
{
    char *key;                     //keyword
    int num_occurrences;           //number of occurrences
    Occurrences *occurrences_list; //pointer for a list of occurrences
    struct index_node *collisions; //pointer for a list of collisions
} Index_node;
struct index
{
    char *text_file; //name of the text file
    Index_node **array;
    int num_keys;
    int table_size;
};
/*
 * Function: remove_specchar
 * ----------------------------
 *   Removes all the special characters of a string
 *
 *   sequence: sequence of characters
 *
 *   returns: nothing
 */
static void remove_specchar(char *sequence)
{
    if (sequence)
    {
        int it = strlen((sequence)) - 1;
        for (unsigned int i = it; i >= 0; i--)
        {
            if ((int)sequence[i] < 48)
                sequence[i] = '\0';
            else if ((int)sequence[i] > 57)
            {
                if ((int)sequence[i] < 65)
                    sequence[i] = '\0';
                else if ((int)sequence[i] > 90)
                {
                    if ((int)sequence[i] < 97)
                        sequence[i] = '\0';
                    else if ((int)sequence[i] > 122)
                        sequence[i] = '\0';
                    else
                        break;
                }
                else
                    break;
            }
            else
                break;
        }
    }
}
/*
 * Function: substring
 * ----------------------------
 *   Returns the substring of a sequence of characters
 *
 *   source: sequence of characters
 *   beg: index where the substring begins
 *   end: index where the substring ends
 *
 *   returns: the substring of the sequence of characters
 */
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
/*
 * Function: index_hashing_funct
 * ----------------------------
 *   Computes the index of the hash table using a key
 *
 *   key: sequence of characters
 *   size: size of the table
 *
 *   returns: the index of the key in the table
 */
static int index_hashing_funct(const char *key, int size)
{
    if (key)
    {
        int result = 0;
        int off = 0;
        int i = 0;
        while (key[i] != '\0')
        {
            result += (i + 1) * key[i];
            i++;
        }
        return (result % size);
    }
    else
        return -1;
}
/*
 * Function: index_readfile
 * ----------------------------
 *   Opens a file and saves its contents in a string
 *
 *   strstream: string where the contents will be saved
 *   file_name: string containing the file's name
 *
 *   returns: an integer signaling if the operation was successfull or not
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
        return true;
    }
}
/*
 * Function: index_addkeys
 * ----------------------------
 *   Adds the keyswords from a file to the table
 *
 *   idx: pointer of the Hash table structure
 *   key_file: string containing the file's name
 *
 *   returns: an integer signaling if the operation was successfull or not
 */
static int index_addkeys(const char *key_file, Index **idx)
{
    char *search = "\n";
    char *token;
    char *strstream;
    if (index_readfile(&strstream, key_file) == false)
        return false;
    token = strtok(strstream, search);
    remove_specchar(token);
    if (!token || !idx)
        return false;
    while (token)
    {
        int index = index_hashing_funct(token, (*idx)->table_size);
        if (index >= 0)
        {
            if (!(*idx)->array[index])
            {
                (*idx)->array[index] = (Index_node *)malloc(sizeof(Index_node));
                (*idx)->array[index]->key = token;
                (*idx)->array[index]->occurrences_list = NULL;
                (*idx)->array[index]->collisions = NULL;
                (*idx)->array[index]->num_occurrences = 0;
                (*idx)->num_keys++;
            }
            else
            {
                //creates a collision
                Index_node *it = (*idx)->array[index];
                Index_node *ant = NULL;
                while (it && (strcmp(it->key, token) != 0))
                {
                    ant = it;
                    it = it->collisions;
                }
                if (!it)
                {
                    ant->collisions = (Index_node *)malloc(sizeof(Index_node));
                    ant->collisions->key = token;
                    ant->collisions->occurrences_list = NULL;
                    ant->collisions->collisions = NULL;
                    ant->collisions->num_occurrences = 0;
                    (*idx)->num_keys++;
                }
            }
        }
        token = strtok(NULL, search);
    }
    return true;
}
/*
 * Function: index_addkeys
 * ----------------------------
 *   Adds the occurrences of the keywords from a file to the table
 *
 *   idx: pointer of the Hash table structure
 *   text_file: string containing the file's name
 *
 *   returns: an integer signaling if the operation was successfull or not
 */
static int index_addtext(const char *text_file, Index **idx, int clean)
{
    if (!idx)
        return false;
    if (clean == true)
    {
        for (unsigned int i = 0; i < (*idx)->table_size; i++)
        {
            Index_node *it = (*idx)->array[i];
            while (it)
            {
                Occurrences *ant = NULL, *it_o = it->occurrences_list;
                it->occurrences_list = NULL;
                while (it_o)
                {
                    ant = it_o;
                    it_o = it_o->next;
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
    token = strtok(strstream, search); //split everything in lines
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
            remove_specchar(sub);
            int index = index_hashing_funct(sub, (*idx)->table_size);
            if (index >= 0 && (*idx)->array[index] != NULL)
            {
                Index_node *it = (*idx)->array[index];
                while (it && strcmp(it->key, sub) != 0)
                    it = it->collisions;
                if (it)
                {
                    if (!it->occurrences_list)
                    {
                        it->occurrences_list = (Occurrences *)malloc(sizeof(Occurrences));
                        it->occurrences_list->line = line;
                        it->occurrences_list->next = NULL;
                    }
                    else
                    {
                        Occurrences *it_o = it->occurrences_list;
                        while (it_o->next)
                            it_o = it_o->next;
                        Occurrences *c = (Occurrences *)malloc(sizeof(Occurrences));
                        c->line = line;
                        c->next = NULL;
                        it_o->next = c;
                    }
                    it->num_occurrences++;
                }
            }
            if (token_space)
                token_space = strchr(token_space + 1, ' ');
            free(sub);
        }
        token = strtok(NULL, search);
        line++;
    }
    free(strstream);
    return true;
}
/*
 * Function: index_getsize
 * ----------------------------
 *   Gets the number of keys
 *
 *   key_file: string containing the key file's name
 *
 *   returns: the number of keys plus the factor
 */
static int index_getsize(const char *key_file)
{
    int size = 0;
    char *search = "\n";
    char *token;
    char *strstream;
    if (index_readfile(&strstream, key_file) == false)
        return false;
    token = strtok(strstream, search);
    while (token)
    {
        size++;
        token = strtok(NULL, search);
    }
    return size + FACTOR;
}
int index_createfrom(const char *key_file, const char *text_file, Index **idx)
{
    (*idx) = (Index *)malloc(sizeof(Index));
    (*idx)->text_file = (char *)malloc((strlen(text_file) + 1) * sizeof(char));
    (*idx)->num_keys = 0;
    strcpy((*idx)->text_file, text_file);
    (*idx)->table_size = index_getsize(key_file);
    (*idx)->array = malloc((*idx)->table_size * sizeof(Index_node *));
    for (unsigned int i = 0; i < (*idx)->table_size; i++)
        (*idx)->array[i] = NULL;
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
        int index = index_hashing_funct(key, idx->table_size);
        Index_node *it_col = idx->array[index];
        while (it_col && strcmp(it_col->key, key) != 0)
            it_col = it_col->collisions;
        if (it_col)
        {
            *num_occurrences = (it_col->num_occurrences);
            if (occurrences)
                free(occurrences);
            (*occurrences) = malloc(sizeof(int) * it_col->num_occurrences);
            Occurrences *it = it_col->occurrences_list;
            unsigned int i = 0;
            while (it)
            {
                (*occurrences)[i] = it->line;
                it = it->next;
                i++;
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
        char *n_key = malloc(sizeof(char) * strlen(key) + 1);
        strcpy(n_key, key);
        char *space = strchr(n_key, 32);
        if (space)
        {
            *space = '\0';
        }
        remove_specchar(n_key);
        //remove all the occurrences of the table
        int index_key = index_hashing_funct(n_key, idx->table_size);
        if (index_key >= 0)
        {
            if (!idx->array[index_key])
            {
                idx->array[index_key] = (Index_node *)malloc(sizeof(Index_node));
                idx->array[index_key]->key = (char *)malloc(sizeof(char) * strlen(n_key));
                strcpy(idx->array[index_key]->key, n_key);
                idx->array[index_key]->occurrences_list = NULL;
                idx->array[index_key]->collisions = NULL;
                idx->array[index_key]->num_occurrences = 0;
                idx->num_keys++;
            }
            else
            {
                //creates a collision
                Index_node *it = idx->array[index_key];
                Index_node *ant = NULL;
                while (it && (strcmp(it->key, n_key) != 0))
                {
                    ant = it;
                    it = it->collisions;
                }
                if (!it)
                {
                    //element doesn't exist yet
                    ant->collisions = (Index_node *)malloc(sizeof(Index_node));
                    ant->collisions->key = (char *)malloc(sizeof(char) * strlen(n_key));
                    strcpy(ant->collisions->key, n_key);
                    ant->collisions->occurrences_list = NULL;
                    ant->collisions->collisions = NULL;
                    ant->collisions->num_occurrences = 0;
                    idx->num_keys++;
                }
            }
        }
        free(n_key);
        if (index_addtext(idx->text_file, &idx, true) == false)
            return false;
        return true;
    }
    return false;
}
/*
 * Function: split
 * ----------------------------
 *   Function that helps the quicksort function finding the pivot and ordering the array
 *
 *   array: pointer of char pointers
 *
 *   returns: the pivot index
 */
static int split(char **array, int beg, int end)
{
    char *c = array[end]; // pivot
    int j = beg;
    char *t;
    for (int k = beg; k < end; ++k)
        if (strcmp(array[k], c) < 0)
        {
            if (j != k)
            {

                t = array[j];
                array[j] = array[k];
                array[k] = t;
            }
            ++j;
        }
    t = array[j];
    array[j] = array[end];
    array[end] = t;
    return j;
}

/*
 * Function: quicksort
 * ----------------------------
 *   Function that orders an array of characters
 *
 *   array: pointer of char pointers
 *   beg: index of the beginning of the array
 *   end: index of the end of the array
 *   
 */
static void quicksort(char **array, int beg, int end)
{
    if (beg < end)
    {
        int med = split(array, beg, end);
        quicksort(array, beg, med - 1);
        quicksort(array, med + 1, end);
    }
}
int index_print(const Index *idx)
{
    if (idx)
    {
        //copy
        char **array;
        array = (char **)malloc(idx->num_keys * sizeof(char *));
        unsigned int ind = 0;
        unsigned int j_ind = 0;
        while (ind < idx->num_keys)
        {
            Index_node *it = idx->array[j_ind];
            while (it)
            {
                array[ind] = NULL;
                array[ind] = (char *)malloc(strlen(it->key) * sizeof(char));
                strcpy(array[ind], it->key);
                ind++;
                it = it->collisions;
            }
            j_ind++;
        }
        //sort using bubblesort
        /*
        char *temp = NULL;
        for (unsigned int i = 0; i < idx->num_keys - 1; i++)
        {
            for (unsigned int j = i + 1; j < idx->num_keys; j++)
            {
                if (strcmp(array[i], array[j]) > 0)
                {
                    if (temp)
                        free(temp);
                    temp = (char *)malloc(sizeof(char) * strlen(array[i]));
                    strcpy(temp, array[i]);
                    strcpy(array[i], array[j]);
                    strcpy(array[j], temp);
                }
            }
        }*/
        //sort
        quicksort(array, 0, idx->num_keys - 1);
        //print
        for (unsigned int i = 0; i < idx->num_keys; i++)
        {

            Index_node *it = idx->array[index_hashing_funct(array[i], idx->table_size)];
            while (strcmp(it->key, array[i]) != 0)
            {
                it = it->collisions;
            }
            if (it)
            {
                fprintf(stdout, "%s: ", it->key);
                Occurrences *it2 = it->occurrences_list;
                if (it2)
                {

                    while (it2->next)
                    {
                        fprintf(stdout, "%d, ", it2->line);
                        it2 = it2->next;
                    }
                    if (it2)
                        fprintf(stdout, "%d", it2->line);
                }
                fprintf(stdout, "\n");
                it = it->collisions;
            }
        }
        /* invalid pointer error*/
        for (unsigned int i = 0; i < idx->num_keys; i++)
        {
            if (array[i])
            {
                free(array[i]);
                array[i] = NULL;
            }
        }
        free(array);
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
        for (unsigned int i = 0; i < (*idx)->table_size; i++)
        {
            if ((*idx)->array[i])
            {
                Index_node *it = (*idx)->array[i];
                Index_node *temp = NULL;
                while (it)
                {
                    if (it->occurrences_list)
                    {
                        Occurrences *it_o = it->occurrences_list;
                        Occurrences *temp_o = NULL;
                        while (it_o)
                        {
                            temp_o = it_o;
                            it_o = it_o->next;
                            free(temp_o);
                        }
                    }
                    temp = it;
                    it = it->collisions;
                    free(temp);
                }
            }
        }
        //destroy
        return true;
    }
}