#include "index.h"
static const int false = 1;
static const int true = 0;
#define FACTOR 64
#define MAX_CHAR 16
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
/**
 * Function: strcicmpL
 * -----------------------------
 * Compares two strings in a case insensitivy way
 * Linux doesn't recognize the default stricmp like windows does
 * Source: https://stackoverflow.com/questions/5820810/case-insensitive-string-comp-in-c
 * 
 * a, b: sequence of characters
 * 
 * returns: an integer >1 if a>b and <1 if a<b or 0 if the sequences are the same
 */
static int strcicmpL(char const *a, char const *b)
{
    while (*a)
    {
        int d = tolower(*a) - tolower(*b);
        if (d)
        {
            return d;
        }
        a++;
        b++;
    }
    return 0;
}

/*
 * Function: limit_char
 * ----------------------------
 *   Removes all the characters that exceed the MAX_CHAR limit
 *
 *   str: sequence of characters
 *
 *   returns: nothing
 */
static void limit_char(char *str)
{
    if (str)
    {
        unsigned int s = strlen(str);
        if (s > MAX_CHAR)
        {
            memset(str + MAX_CHAR - 1, '\0', sizeof(char) * (s - MAX_CHAR));
        }
    }
}
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
    if (sequence && strcmp(sequence, "") != 0)
    {
        int buf_s = strlen((sequence)) - 1;
        for (int i = buf_s; i > 0; i--)
        //bigger than 0 because the user can search for "." or any other single special character
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
 *   file_name: string containing the file's name
 *
 *   returns: a string where the file's contents are stored
 */

static char *index_readfile(const char *file_name)
{
    FILE *input = NULL;
    char *strstream = NULL;
    char it = '\0';
    unsigned int ind = 0;
    input = fopen(file_name, "r");
    if (!input)
    {
        fprintf(stdout, "Failed to open the file!\n");
        fclose(input);
        return NULL;
    }
    else
    {
        rewind(input);
        unsigned int str_size = 256;
        strstream = malloc(str_size * sizeof(char));
        memset(strstream, '\0', sizeof(char) * str_size);
        if (!strstream)
            return NULL;
        while ((it = fgetc(input)) != EOF)
        {
            if (ind >= str_size)
            {
                str_size += 256;
                char *temp = realloc(strstream, str_size*sizeof(char));
                strstream = temp;
                memset(strstream + (str_size - 256), '\0', sizeof(char) * (256));
            }
            strstream[ind] = it;
            ind++;
        }
        fclose(input);
        return strstream;
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
    char *token = NULL;
    char *strstream = NULL;
    if (!idx)
        return false;
    strstream = index_readfile(key_file);
    if (!strstream)
        return false;
    token = strtok(strstream, search);
    if (!token) //there are no keys in the file
    {
        if (strstream)
            free(strstream);
        return true;
    }
    while (token)
    {
        //checks if there is more than 1 word per line
        //if there is it will only consider the first one
        limit_char(token);
        remove_specchar(token);
        char *space = strchr(token, 32);
        if (space)
            *space = '\0';
        int index = index_hashing_funct(token, (*idx)->table_size);
        if (index >= 0)
        {
            if (!(*idx)->array[index])
            {
                (*idx)->array[index] = (Index_node *)malloc(sizeof(Index_node));
                (*idx)->array[index]->key = malloc(sizeof(char) * (strlen(token) + 1));
                memset((*idx)->array[index]->key, '\0', strlen(token) + 1);
                strcpy((*idx)->array[index]->key, token);
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
    free(strstream);
    free(token);
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
static int index_addtext(const char *text_file, Index **idx)
{
    char *buffer = NULL;
    char *strstream = NULL;
    int line = 1;
    if (!idx)
        return false;
    unsigned size_buffer = MAX_CHAR + 1;
    buffer = (char *)malloc(sizeof(char) * size_buffer);
    if (!buffer)
        return false;
    memset(buffer, '\0', sizeof(char) * size_buffer);
    unsigned ind_buffer = 0;
    strstream = index_readfile(text_file);
    if (!strstream)
        return false;
    char *it = strstream;
    while ((it && *it != '\0') || (strcmp(buffer, "") != 0))
    //the text can end but the buffer still has content on it
    {
        if (*it == ' ' || *it == '\n' || *it == '\0')
        {
            if (strlen(buffer) > 0)
            {

                limit_char(buffer);
                remove_specchar(buffer);
                int index = index_hashing_funct(buffer, (*idx)->table_size);
                if (index >= 0 && (*idx)->array[index] != NULL)
                {
                    Index_node *it = (*idx)->array[index];
                    while (it && strcmp(it->key, buffer) != 0)
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
            }
            //got a word
            if (*it == '\n')
            {
                line++;
            }
            memset(buffer, '\0', sizeof(char) * size_buffer);
            ind_buffer = 0;
        }
        else
        {
            if (ind_buffer >= size_buffer)
            {
                size_buffer += 17;
                char *b = (char *)realloc(buffer, size_buffer * sizeof(char));
                buffer = b;
                memset(buffer + (size_buffer - 18), '\0', 17 * sizeof(char));
            }
            buffer[ind_buffer] = *it;
            ind_buffer++;
        }
        it++;
    }
    free(strstream);
    if (buffer && strlen(buffer) > 0)
            free(buffer);
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
    char *strstream = NULL;
    strstream = index_readfile(key_file);
    if (!strstream)
        return false;
    token = strtok(strstream, search);
    while (token)
    {
        size++;
        token = strtok(NULL, search);
    }
    free(strstream);
    return size + FACTOR;
}
int index_createfrom(const char *key_file, const char *text_file, Index **idx)
{
    (*idx) = (Index *)malloc(sizeof(Index));
    (*idx)->text_file = (char *)malloc((strlen(text_file) + 1) * sizeof(char));
    (*idx)->num_keys = 0;
    memset((*idx)->text_file, '\0', sizeof(char) * (strlen(text_file) + 1));
    strcpy((*idx)->text_file, text_file);
    (*idx)->table_size = index_getsize(key_file);
    (*idx)->array = malloc((*idx)->table_size * sizeof(Index_node *));
    for (int i = 0; i < (*idx)->table_size; i++)
        (*idx)->array[i] = NULL;
    if (index_addkeys(key_file, idx) == false)
        return false;
    if (index_addtext(text_file, idx) == false)
        return false;

    return true;
}
int index_get(const Index *idx, const char *key, int **occurrences, int *num_occurrences)
{
    if (idx)
    {
        char *n_key = malloc(sizeof(char) * (strlen(key) + 1));
        memset(n_key, '\0', sizeof(char) * (strlen(key) + 1));
        strcpy(n_key, key);
        char *space = strchr(n_key, 32);
        if (space)
        {
            *space = '\0';
        }
        limit_char(n_key);
        remove_specchar(n_key);
        int index = index_hashing_funct(n_key, idx->table_size);
        Index_node *it_col = idx->array[index];
        while (it_col && strcmp(it_col->key, n_key) != 0)
            it_col = it_col->collisions;
        if (n_key)
            free(n_key);
        if (it_col)
        {
            *num_occurrences = (it_col->num_occurrences);
            //maybe there will be a memory leak here
            if (*occurrences)
            {
                *occurrences = NULL;
            }
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
        Index_node *it_n = NULL;
        char *n_key = malloc(sizeof(char) * strlen(key) + 1);
        memset(n_key, '\0', (strlen(key) + 1) * sizeof(char));
        strcpy(n_key, key);
        char *buffer = NULL;
        char *strstream = NULL;
        int line = 1;
        unsigned size_buffer = MAX_CHAR + 1;
        buffer = (char *)malloc(sizeof(char) * size_buffer);
        if (!buffer)
            return false;
        memset(buffer, '\0', sizeof(char) * size_buffer);
        unsigned ind_buffer = 0;
        char *space = strchr(n_key, 32);
        if (space)
        {
            while (space)
            {
                *space = '\0';
                (*space)++;
            }
        }
        limit_char(n_key);
        remove_specchar(n_key);
        //remove all the occurrences of the table
        int index_key = index_hashing_funct(n_key, idx->table_size);
        if (index_key >= 0)
        {
            if (!idx->array[index_key])
            {
                it_n = (Index_node *)malloc(sizeof(Index_node));
                it_n->key = (char *)malloc(sizeof(char) * (strlen(n_key) + 1));
                memset(it_n->key, '\0', sizeof(char) * (strlen(n_key) + 1));
                strcpy(it_n->key, n_key);
                it_n->occurrences_list = NULL;
                it_n->collisions = NULL;
                it_n->num_occurrences = 0;
                idx->num_keys++;
                idx->array[index_key] = it_n;
                it_n = idx->array[index_key];
            }
            else
            {
                //creates a collision
                it_n = idx->array[index_key];
                Index_node *ant = NULL;
                while (it_n && (strcmp(it_n->key, n_key) != 0))
                {
                    ant = it_n;
                    it_n = it_n->collisions;
                }
                if (!it_n)
                {
                    //element doesn't exist yet
                    it_n = (Index_node *)malloc(sizeof(Index_node));
                    it_n->key = (char *)malloc(sizeof(char) * (strlen(n_key) + 1));
                    memset(it_n->key, '\0', sizeof(char) * (strlen(n_key) + 1));
                    strcpy(it_n->key, n_key);
                    it_n->occurrences_list = NULL;
                    it_n->collisions = NULL;
                    it_n->num_occurrences = 0;
                    idx->num_keys++;
                    if (ant)
                        ant->collisions = it_n;
                }
                else
                {
                    Occurrences *itc = it_n->occurrences_list;
                    Occurrences *ant_c = NULL;
                    it_n->num_occurrences = 0;
                    while (itc)
                    {
                        ant_c = itc;
                        itc = itc->next;
                        free(ant_c);
                        ant_c = NULL;
                    }
                    it_n->occurrences_list = NULL;
                }
            }
        }
        else
            return false;
        strstream = index_readfile(idx->text_file);
        if (!strstream)
            return false;
        char *it = strstream;
        while ((it && *it != '\0') || (strcmp(buffer, "") != 0))
        //the text can end but the buffer still has content on it
        {
            if (*it == ' ' || *it == '\n' || *it == '\0')
            {
                if (strlen(buffer) > 0)
                {

                    limit_char(buffer);
                    remove_specchar(buffer);
                    int index = index_hashing_funct(buffer, idx->table_size);
                    if (index == index_key && strcmp(it_n->key, buffer) == 0)
                    {
                        if (!it_n->occurrences_list)
                        {
                            it_n->occurrences_list = (Occurrences *)malloc(sizeof(Occurrences));
                            it_n->occurrences_list->line = line;
                            it_n->occurrences_list->next = NULL;
                        }
                        else
                        {
                            Occurrences *it_o = it_n->occurrences_list;
                            while (it_o->next)
                                it_o = it_o->next;
                            Occurrences *c = (Occurrences *)malloc(sizeof(Occurrences));
                            c->line = line;
                            c->next = NULL;
                            it_o->next = c;
                        }
                        it_n->num_occurrences++;
                    }
                }
                //got a word
                if (*it == '\n')
                    line++;
                memset(buffer, '\0', sizeof(char) * size_buffer);
                ind_buffer = 0;
            }
            else
            {
                if (ind_buffer >= size_buffer)
                {
                    size_buffer += 17;
                    char *b = realloc(buffer, size_buffer * sizeof(char));
                    buffer = b;
                    memset(buffer + (size_buffer - 17), '\0', 17 * sizeof(char));
                }
                buffer[ind_buffer] = *it;
                ind_buffer++;
            }
            it++;
        }
        free(strstream);
        if (buffer && strlen(buffer) > 0)
            free(buffer);
        free(n_key);
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
    {
        if (strcicmpL(array[k], c) < 0)
        {
            if (j != k)
            {

                t = array[j];
                array[j] = array[k];
                array[k] = t;
            }
            ++j;
        }
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
        int ind = 0;
        unsigned int j_ind = 0;
        while (ind < idx->num_keys)
        {
            Index_node *it = idx->array[j_ind];
            while (it)
            {
                array[ind] = NULL;
                array[ind] = (char *)malloc((strlen(it->key) + 1) * sizeof(char));
                memset(array[ind], '\0', (strlen(it->key) + 1) * sizeof(char));
                strcpy(array[ind], it->key);
                ind++;
                it = it->collisions;
            }
            j_ind++;
        }
        //sort
        quicksort(array, 0, idx->num_keys - 1);
        //print
        for (int i = 0; i < idx->num_keys; i++)
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
        for (int i = 0; i < idx->num_keys; i++)
        {
            if (array[i])
            {
                char *t = array[i];
                array[i] = NULL;
                free(t);
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
    if (!(*idx))
        return true;
    else
    {
        for (int i = 0; i < (*idx)->table_size; i++)
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
                    if (temp->key){
                        free(temp->key);
                    }
                    free(temp);
                }
            }
        }
        //destroy
        free((*idx)->array);
        if ((*idx)->text_file)
            free((*idx)->text_file);
        free(*idx);

        return true;
    }
}