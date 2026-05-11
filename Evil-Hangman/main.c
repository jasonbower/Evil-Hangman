#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "my_string.h"
#include "generic_vector.h"
#include "generic.h"
#include "status.h"
#include "associative_array.h"

#define MAX_WORD_LENGTH 29
#define WORD_BUCKET_COUNT 30
#define WORD_FAMILY_CHAR '-'

///// HELPER FUNCTIONS /////
ITEM my_string_init_copy_item(ITEM item);
void my_string_destroy_item(ITEM* pItem);
void load_dictionary(GENERIC_VECTOR word_buckets[]);
void play_one_game(GENERIC_VECTOR word_buckets[]);
void destroy_game_state(GENERIC_VECTOR* pCurrent_words, MY_STRING* pCurrent_word_family, MY_STRING* pGuessed_letters, MY_STRING* pNew_key);
Status play_one_turn(GENERIC_VECTOR* pCurrent_words, MY_STRING current_word_family, MY_STRING guessed_letters, MY_STRING new_key, int* pRemaining_guesses);
void display_game_state(int remaining_guesses, MY_STRING guessed_letters, MY_STRING current_word_family, GENERIC_VECTOR current_words, char running_total_choice);
void destroy_dictionary(GENERIC_VECTOR word_buckets[]);
void clear_keyboard_buffer(void);
int get_word_length(GENERIC_VECTOR word_buckets[]);
int get_number_of_guesses(void);
char get_running_total_option(void);
MY_STRING build_initial_word_family(int word_length);
char get_guess(MY_STRING guessed_letters);
int already_guessed(MY_STRING guessed_letters, char guess);
int word_family_changed(MY_STRING old_family, MY_STRING new_family);
Status get_word_key_value(MY_STRING current_word_family, MY_STRING new_key, MY_STRING word, char guess);
int word_is_solved(MY_STRING current_word_family);
char ask_to_play_again(void);
MY_STRING get_losing_word(GENERIC_VECTOR current_words);
GENERIC_VECTOR copy_vector_of_words(GENERIC_VECTOR hVector);

int main(void)
{
	GENERIC_VECTOR word_buckets[WORD_BUCKET_COUNT];
	char play_again = 'y';

	load_dictionary(word_buckets);

	while (play_again == 'y' || play_again == 'Y')
	{
		play_one_game(word_buckets);
		play_again = ask_to_play_again();
	}

	destroy_dictionary(word_buckets);

	return 0;
}

///// HELPER FUNCTIONS /////

void load_dictionary(GENERIC_VECTOR word_buckets[])
{
	FILE* fp;
	MY_STRING hWord;
	int i;
	int length;

	fp = fopen("dictionary.txt", "r");

	if (fp == NULL)
	{
		printf("Error: could not open dictionary.txt\n");
		exit(1);
	}

	for (i = 0; i < WORD_BUCKET_COUNT; i++)
	{
		word_buckets[i] = generic_vector_init_default(my_string_init_copy_item, my_string_destroy_item);

		if (word_buckets[i] == NULL)
		{
			printf("Error: failed to initialize vector %d\n", i);
			fclose(fp);
			exit(1);
		}
	}

	hWord = my_string_init_default();

	if (hWord == NULL)
	{
		printf("Error: failed to initialize MY_STRING\n");
		fclose(fp);
		exit(1);
	}

	while (my_string_extraction(hWord, fp) == SUCCESS)
	{
		length = my_string_get_size(hWord);

		if (length >= 1 && length <= MAX_WORD_LENGTH)
		{
			if (generic_vector_push_back(word_buckets[length], hWord) == FAILURE)
			{
				printf("Error: failed to push word into bucket\n");
				my_string_destroy(&hWord);
				fclose(fp);
				destroy_dictionary(word_buckets);
				exit(1);
			}
		}
	}

	my_string_destroy(&hWord);
	fclose(fp);
}

void play_one_game(GENERIC_VECTOR word_buckets[])
{
	GENERIC_VECTOR current_words;
	GENERIC_VECTOR largest_family;
	int word_length;
	int number_of_guesses;
	int remaining_guesses;
	int i;
	char running_total_choice;
	char guess;
	MY_STRING current_word_family;
	MY_STRING guessed_letters;
	MY_STRING new_key;
	MY_STRING old_word_family;
	MY_STRING losing_word;
	ASSOCIATIVE_ARRAY hArray;

	word_length = get_word_length(word_buckets);
	number_of_guesses = get_number_of_guesses();
	running_total_choice = get_running_total_option();
	remaining_guesses = number_of_guesses;
	current_words = copy_vector_of_words(word_buckets[word_length]);

	if (current_words == NULL)
	{
		printf("Error: failed to copy current word list\n");
		destroy_dictionary(word_buckets);
		exit(1);
	}

	current_word_family = build_initial_word_family(word_length);
	guessed_letters = my_string_init_default();
	new_key = my_string_init_default();

	if (current_word_family == NULL || guessed_letters == NULL || new_key == NULL)
	{
		printf("Error: failed to initialize game state\n");
		destroy_game_state(&current_words, &current_word_family, &guessed_letters, &new_key);
		destroy_dictionary(word_buckets);
		exit(1);
	}

	while (remaining_guesses > 0 && word_is_solved(current_word_family) == 0)
	{
		display_game_state(remaining_guesses, guessed_letters, current_word_family, current_words, running_total_choice);

		if (play_one_turn(&current_words, current_word_family, guessed_letters, new_key, &remaining_guesses) == FAILURE)
		{
			destroy_game_state(&current_words, &current_word_family, &guessed_letters, &new_key);
			destroy_dictionary(word_buckets);
			exit(1);
		}
	}

	if (word_is_solved(current_word_family))	printf("\nYou win! The word was %s\n", my_string_c_str(current_word_family));
	else
	{
		losing_word = get_losing_word(current_words);
		printf("\nYou lose! The word was %s\n", my_string_c_str(losing_word));
	}

	destroy_game_state(&current_words, &current_word_family, &guessed_letters, &new_key);
}

void destroy_game_state(GENERIC_VECTOR* pCurrent_words, MY_STRING* pCurrent_word_family, MY_STRING* pGuessed_letters, MY_STRING* pNew_key)
{
	generic_vector_destroy(pCurrent_words);
	my_string_destroy(pCurrent_word_family);
	my_string_destroy(pGuessed_letters);
	my_string_destroy(pNew_key);
}

Status play_one_turn(GENERIC_VECTOR* pCurrent_words, MY_STRING current_word_family, MY_STRING guessed_letters, MY_STRING new_key, int* pRemaining_guesses)
{
	GENERIC_VECTOR largest_family;
	MY_STRING old_word_family;
	ASSOCIATIVE_ARRAY hArray;
	MY_STRING word;
	char guess;
	int i;

	guess = get_guess(guessed_letters);
	old_word_family = my_string_init_copy(current_word_family);

	if (old_word_family == NULL)
	{
		printf("Error: failed to copy current word family\n");

		return FAILURE;
	}

	hArray = associative_array_init_default();

	if (hArray == NULL)
	{
		printf("Error: failed to initialize associative array\n");
		my_string_destroy(&old_word_family);

		return FAILURE;
	}

	for (i = 0; i < generic_vector_get_size(*pCurrent_words); i++)
	{
		word = *(MY_STRING*)generic_vector_at(*pCurrent_words, i);

		if (get_word_key_value(current_word_family, new_key, word, guess) == FAILURE)
		{
			printf("Error: failed to generate word key\n");
			associative_array_destroy(&hArray);
			my_string_destroy(&old_word_family);

			return FAILURE;
		}

		if (associative_array_insert(hArray, new_key, word) == FAILURE)
		{
			printf("Error: failed to insert into associative array\n");
			associative_array_destroy(&hArray);
			my_string_destroy(&old_word_family);

			return FAILURE;
		}
	}

	largest_family = associative_array_get_largest_family(hArray, current_word_family);

	if (word_family_changed(old_word_family, current_word_family) == 0)
	{
		(*pRemaining_guesses)--;
		printf("Sorry, there are no %c's\n", guess);
	}
	else	printf("Yes, the letter %c is in the word.\n", guess);

	generic_vector_destroy(pCurrent_words);
	*pCurrent_words = copy_vector_of_words(largest_family);

	if (*pCurrent_words == NULL)
	{
		printf("Error: failed to copy largest family\n");
		associative_array_destroy(&hArray);
		my_string_destroy(&old_word_family);

		return FAILURE;
	}

	associative_array_destroy(&hArray);
	my_string_destroy(&old_word_family);

	return SUCCESS;
}

void display_game_state(int remaining_guesses, MY_STRING guessed_letters, MY_STRING current_word_family, GENERIC_VECTOR current_words, char running_total_choice)
{
	printf("\nRemaining guesses: %d\n", remaining_guesses);
	printf("Guessed letters: %s\n", my_string_c_str(guessed_letters));
	printf("Current word family: %s\n", my_string_c_str(current_word_family));

	if (running_total_choice == 'y' || running_total_choice == 'Y')
		printf("Words remaining: %d\n", generic_vector_get_size(current_words));
}

void destroy_dictionary(GENERIC_VECTOR word_buckets[])
{
	int i;

	for (i = 0; i < WORD_BUCKET_COUNT; i++)
	{
		if (word_buckets[i] != NULL)	generic_vector_destroy(&word_buckets[i]);
	}
}

ITEM my_string_init_copy_item(ITEM item)
{
	return (ITEM)my_string_init_copy((MY_STRING)item);
}

void my_string_destroy_item(ITEM* pItem)
{
	my_string_destroy((MY_STRING*)pItem);
}

void clear_keyboard_buffer(void)
{
	char c;

	scanf("%c", &c);

	while (c != '\n')	scanf("%c", &c);
}

int get_word_length(GENERIC_VECTOR word_buckets[])
{
	int word_length;
	int scanf_result;
	int valid_input = 0;

	while (valid_input == 0)
	{
		printf("Enter the desired word length: ");
		scanf_result = scanf("%d", &word_length);

		if (scanf_result != 1)
		{
			printf("Invalid input. Please enter an integer.\n");
			clear_keyboard_buffer();
		}

		else
		{
			clear_keyboard_buffer();

			if (word_length < 1 || word_length > MAX_WORD_LENGTH)
				printf("Invalid word length. Please enter a number from 1 to 29.\n");
			else if (generic_vector_get_size(word_buckets[word_length]) == 0)
				printf("No words of that length exist in the dictionary.\n");
			else	valid_input = 1;
		}
	}

	return word_length;
}

int get_number_of_guesses(void)
{
	int guesses;
	int scanf_result;
	int valid_input = 0;

	while (valid_input == 0)
	{
		printf("Enter the number of guesses: ");
		scanf_result = scanf("%d", &guesses);

		if (scanf_result != 1)
		{
			printf("Invalid input. Please enter an integer.\n");
			clear_keyboard_buffer();
		}

		else
		{
			clear_keyboard_buffer();
			if (guesses <= 0)	printf("Number of guesses must be greater than 0.\n");
			else				valid_input = 1;
		}
	}

	return guesses;
}

char get_running_total_option(void)
{
	char answer;
	int valid_input = 0;
	int scanf_result;

	while (valid_input == 0)
	{
		printf("Would you like to see the number of words remaining? (y/n): ");
		scanf_result = scanf("%c", &answer);

		if (scanf_result != 1)
		{
			printf("Invalid input.\n");
			clear_keyboard_buffer();
		}

		else
		{
			clear_keyboard_buffer();

			if		(answer == 'y' || answer == 'Y')	valid_input = 1;
			else if (answer == 'n' || answer == 'N')	valid_input = 1;
			else	printf("Invalid input. Please enter y or n.\n");
		}
	}

	return answer;
}

MY_STRING build_initial_word_family(int word_length)
{
	MY_STRING hWord = my_string_init_default();
	int i;

	if (hWord == NULL)	return NULL;

	for (i = 0; i < word_length; i++)
	{
		if (my_string_push_back(hWord, WORD_FAMILY_CHAR) == FAILURE)
		{
			my_string_destroy(&hWord);

			return NULL;
		}
	}

	return hWord;
}

char get_guess(MY_STRING guessed_letters)
{
	char guess;
	char extra_character;
	int scanf_result;
	int valid_input = 0;

	while (valid_input == 0)
	{
		printf("Enter guess: ");
		scanf_result = scanf("%c%c", &guess, &extra_character);

		if (scanf_result != 2)
		{
			printf("Invalid input. Please enter one letter.\n");
			clear_keyboard_buffer();
		}

		else
		{
			if (extra_character != '\n')
			{
				printf("Invalid input. Please enter exactly one letter.\n");
				clear_keyboard_buffer();
			}

			else
			{
				guess = tolower(guess);

				if		(guess < 'a' || guess > 'z')				printf("Invalid input. Please enter a letter.\n");
				else if (already_guessed(guessed_letters, guess))	printf("You already guessed that letter.\n");
				else
				{
					if (my_string_push_back(guessed_letters, guess) == FAILURE)
					{
						printf("Error: failed to store guessed letter\n");
						exit(1);
					}

					valid_input = 1;
				}
			}
		}
	}

	return guess;
}

int already_guessed(MY_STRING guessed_letters, char guess)
{
	int i;
	int size = my_string_get_size(guessed_letters);
	char* pChar;

	for (i = 0; i < size; i++)
	{
		pChar = my_string_at(guessed_letters, i);

		if (*pChar == guess)	return 1;
	}

	return 0;
}

int word_family_changed(MY_STRING old_family, MY_STRING new_family)
{
	int i;
	int size = my_string_get_size(old_family);
	char* old_char;
	char* new_char;

	for (i = 0; i < size; i++)
	{
		old_char = my_string_at(old_family, i);
		new_char = my_string_at(new_family, i);

		if (*old_char != *new_char)	return 1;
	}

	return 0;
}

Status get_word_key_value(MY_STRING current_word_family, MY_STRING new_key, MY_STRING word, char guess)
{
	int i;
	int size;
	char* word_char;
	char* key_char;

	if (current_word_family == NULL || new_key == NULL || word == NULL)	return FAILURE;

	if (my_string_assignment(current_word_family, new_key) == FAILURE)	return FAILURE;

	size = my_string_get_size(word);

	for (i = 0; i < size; i++)
	{
		word_char = my_string_at(word, i);
		key_char = my_string_at(new_key, i);

		if (*word_char == guess)	*key_char = guess;
	}

	return SUCCESS;
}

int word_is_solved(MY_STRING current_word_family)
{
	int i;
	int size = my_string_get_size(current_word_family);
	char* pChar;

	for (i = 0; i < size; i++)
	{
		pChar = my_string_at(current_word_family, i);

		if (*pChar == WORD_FAMILY_CHAR)	return 0;
	}

	return 1;
}

char ask_to_play_again(void)
{
	char answer;
	int scanf_result;
	int valid_input = 0;

	while (valid_input == 0)
	{
		printf("Would you like to play again? (y/n): ");
		scanf_result = scanf("%c", &answer);

		if (scanf_result != 1)
		{
			printf("Invalid input.\n");
			clear_keyboard_buffer();
		}

		else
		{
			clear_keyboard_buffer();

			if (answer == 'y' || answer == 'Y' || answer == 'n' || answer == 'N')	valid_input = 1;
			else	printf("Invalid input. Please enter y or n.\n");
		}
	}

	return answer;
}

MY_STRING get_losing_word(GENERIC_VECTOR current_words)
{
	return *(MY_STRING*)generic_vector_at(current_words, 0);
}

GENERIC_VECTOR copy_vector_of_words(GENERIC_VECTOR hVector)
{
	GENERIC_VECTOR copy = generic_vector_init_default(my_string_init_copy_item, my_string_destroy_item);
	int i;
	MY_STRING word;

	if (copy == NULL)	return NULL;

	for (i = 0; i < generic_vector_get_size(hVector); i++)
	{
		word = *(MY_STRING*)generic_vector_at(hVector, i);

		if (generic_vector_push_back(copy, word) == FAILURE)
		{
			generic_vector_destroy(&copy);

			return NULL;
		}
	}

	return copy;
}
