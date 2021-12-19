#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_SENTENCE_LENGTH 1000
#define BASE 10
#define NEKUDA '.'
#define EMPTY " \n"
#define READALL -1
#define MINARGS 4
#define MAXARGS 5
#define STARTS_WITH_ZERO 0
#define STARTS_WITH_ONE 1
#define FILE_OPEN_ERR "Error: Unable to open file"
#define TWEET_INIT "Tweet %d: "
#define ERRMSG "Allocation failure: Unable to allocate memory."


typedef struct WordStruct {
  char *word;
  struct WordProbability *prob_list;
  int occurrences;
  int probs;
} WordStruct;

typedef struct WordProbability {
  struct WordStruct *word_struct_ptr;
  int times;
} WordProbability;

typedef struct Node {
    WordStruct *data;
    struct Node *next;
} Node;

typedef struct LinkList {
    Node *first;
    Node *last;
    int size;
} LinkList;

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add(LinkList *link_list, WordStruct *data)
{
  Node *new_node = malloc(sizeof(Node));
  if (new_node == NULL)
  {
	return 1;
  }
  *new_node = (Node){data, NULL};

  if (link_list->first == NULL)
  {
	link_list->first = new_node;
	link_list->last = new_node;
  }
  else
  {
	link_list->last->next = new_node;
	link_list->last = new_node;
  }

  link_list->size++;
  return 0;
}
/*************************************/

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number)
{
  int random = rand();
  if (max_number > RAND_MAX)
  {
    return max_number % random;
  }
  return random % max_number;
}

/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct *get_first_random_word(LinkList *dictionary)
{
  Node *current = dictionary->first;
  int randomized = get_random_number(dictionary->size), i = 0;
  for (; i < randomized; i++)
  {
    current = current->next;
  }
  if (current->data->prob_list == NULL)
  {
    return get_first_random_word(dictionary);
  }
  else
  {
    return current->data;
  }
}

/**
 * Choose randomly the next word. Depend on it's occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct *get_next_random_word(WordStruct *word_struct_ptr)
{
  int random = get_random_number(word_struct_ptr->occurrences);
  int curr_val = STARTS_WITH_ZERO, i = 1;
  WordProbability* curr = word_struct_ptr->prob_list;
  for (; i <= word_struct_ptr->probs; i++)
  {
    curr_val += curr->times;
    if (curr_val > random)
    {
      return curr->word_struct_ptr;
    }
    curr++;
  }
  curr--;
  return curr->word_struct_ptr; //For returning in any case
}

/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence(LinkList *dictionary)
{
  int amount = STARTS_WITH_ONE;
  WordStruct* curr = get_first_random_word(dictionary);
  while (curr->prob_list != NULL)
  {
    printf("%s ", curr->word);
    amount++;
    curr = get_next_random_word(curr);
    if (amount == MAX_WORDS_IN_SENTENCE_GENERATION)
    {
      break;
    }
  }
  printf("%s\n", curr->word);
  return amount;
}

/**
 * Receives a word pointer, pointer to a prob_list and number of probs,
 * and returns the location of the WordProbability pointer if existed.
 * @param char* word to check if already existed.
 * @param WordProbability* prob_list to look in.
 * @param int probs shows how many probs existed in prob_list.
 * @return If existed returns the pointer to this prob, if not returns a
 * pointer to the beginning of the prob_list.
 */
WordProbability* prob_existance(char* word, WordProbability* prob_list,
                              int probs)
{
  WordProbability* curr = prob_list;
  int i = 0;
  for (; i < probs; i++)
  {
    if (strcmp(word, curr[i].word_struct_ptr->word) == 0)
    {
      return &curr[i];
    }
  }
  return NULL;
}

/**
 * Checks whether the word ends with '.' or not.
 * @param char* word the word being checked.
 * @return 0 if ends with dot, 1 if not.
 */
int check_end(char* word)
{
  return (word[strlen(word) - 1] == NEKUDA) ? 0 : 1;
}

/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list(WordStruct *first_word,
                                 WordStruct *second_word)
{
  if (first_word == NULL || check_end(first_word->word) == 0)
  {
    return 1;
  }
  WordProbability* curr = prob_existance(second_word->word,
                                        first_word->prob_list,
                                        first_word->probs);
  if(curr != NULL)
  {
    curr->times += 1;
    return 0;
  }
  else
  {
    WordProbability* temp = realloc (first_word->prob_list,
                    ((first_word->probs)+1)*(sizeof(WordProbability)));
    if (temp == NULL)
    {
      printf(ERRMSG);
      exit(1);
    }
    first_word->prob_list = temp;
    WordProbability new_prob = {second_word, 1};
    WordProbability* ptr_to_new = first_word->prob_list;
    for (int i = 0; i < first_word->probs; i++)
    {
      ptr_to_new++;
    }
    *ptr_to_new = new_prob;
    first_word->probs += 1;
    return 1;
  }
}

/**
 * Receives a word pointer, and a dictionary and returns checks whether
 * the word is already in the dictionary.
 * @param char* word to check if already existed.
 * @param LinkList *dictionary dictionary of all words.
 * @return If existed returns the pointer to this prob, if not returns NULL.
 */
WordStruct* llist_existance(char* word, LinkList *dictionary)
{
  Node* current = dictionary->first;
  for (int i = 0; i <= dictionary->size; i++)
  {
    if (current == NULL) return NULL;
    if (strcmp(word, current->data->word) == 0)
    {
      return current->data;
    }
    current = current->next;
  }
  return NULL;
}

/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary(FILE *fp, int words_to_read, LinkList *dictionary)
{
  WordStruct* prev = NULL;
  WordStruct* curr = NULL;
  char str[MAX_SENTENCE_LENGTH];
  char* word;
  int readed = STARTS_WITH_ZERO;
  while (fgets(str, MAX_SENTENCE_LENGTH, fp) != NULL)
  {
    word = strtok(str, EMPTY);
    while (word != NULL)
    {
      curr = llist_existance(word, dictionary);
      if (curr != NULL)
      {
        curr->occurrences += 1;
      }
      else
      {
        char* word_val = (char*)malloc(strlen(word)+1);
        if (word_val == NULL)
        {
          printf(ERRMSG);
          fclose(fp);
          exit(1);
        }
        strcpy(word_val, word);
        WordStruct* new = (WordStruct*)malloc(sizeof(WordStruct));
        WordStruct new_val = {word_val, NULL, 1, 0};
        *new = new_val;
        curr = new;
        int tester = add(dictionary, curr);
        if (tester == 1)
        {
          printf(ERRMSG);
          fclose(fp);
          exit(1);
        }
      }
      add_word_to_probability_list(prev, curr);
      prev = curr;
      readed++;
      if (readed == words_to_read)
      {
        return;
      }
      word = strtok(NULL, EMPTY);
    }
  }
}

/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList *dictionary)
{
  Node* current = dictionary->first;
  while (current != NULL)
  {
    Node* node_mem = current;
    WordStruct* ws_mem = current->data;
    WordProbability* wp_mem = current->data->prob_list;
    char* word_mem = current->data->word;
    current = current->next;
    free(node_mem);
    free(ws_mem);
    free(wp_mem);
    free(word_mem);
  }
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int main(int argc, char *argv[])
{
  if (argc < MINARGS || argc > MAXARGS)
  {
    printf("Usage: Should be in format <seed> <number of tweets> "
           "<text path> <number of words to read>\n");
    return EXIT_FAILURE;
  }
  int seed = (int)strtol(argv[1], NULL, BASE);
  srand(seed);
  int num_of_tweets = (int)strtol(argv[2], NULL, BASE);
  int words_to_read;
  if (argc == MAXARGS)
  {
    words_to_read = (int)strtol(argv[4], NULL, BASE);
  }
  else
  {
    words_to_read = READALL;
  }
  if (words_to_read == 0)
  {
    return EXIT_SUCCESS;
  }
  FILE *fp = fopen(argv[3], "r");
  if (fp == NULL)
  {
    printf(FILE_OPEN_ERR);
    return EXIT_FAILURE;
  }
  LinkList dictionary = {NULL, NULL, 0};
  fill_dictionary(fp, words_to_read, &dictionary);
  for (int i = 1; i <= num_of_tweets; i++)
  {
    printf(TWEET_INIT, i);
    generate_sentence(&dictionary);
  }
  free_dictionary(&dictionary);
  fclose(fp);
  return EXIT_SUCCESS;
}