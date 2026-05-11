#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "my_string.h"

typedef struct my_string
{
	int size;
	int capacity;
	char* data;
} My_string;

///// HELPER FUNCTIONS /////
static Status my_string_resize(My_string* pString);

///// INTERFACE FUNCTIONS /////

MY_STRING my_string_init_default(void)
{
	My_string* pString = (My_string*)malloc(sizeof(My_string));

	if (pString == NULL)	return NULL;

	pString->size = 0;
	pString->capacity = 7;
	pString->data = malloc(sizeof(char) * pString->capacity);

	if (pString->data == NULL)
	{
		free(pString);
		return NULL;
	}

	return pString;
}

MY_STRING my_string_init_c_string(const char* c_string)
{
	My_string* pString = (My_string*)malloc(sizeof(My_string));
	int size = 0;
	int i;

	if (pString == NULL || c_string == NULL)	return NULL;

	while (c_string[size] != '\0')	size++;

	pString->size = size;
	pString->capacity = size + 1;
	pString->data = malloc(sizeof(char) * pString->capacity);

	if (pString->data == NULL)
	{
		free(pString);
		return NULL;
	}

	for (i = 0; i < pString->size; i++)	pString->data[i] = c_string[i];

	pString->data[pString->size] = '\0';

	return pString;
}

int my_string_get_capacity(MY_STRING hMy_string)
{
	const My_string* pString = (My_string*)hMy_string;

	return pString->capacity;
}

int my_string_get_size(MY_STRING hMy_string)
{
	const My_string* pString = (My_string*)hMy_string;

	return pString->size;
}

int my_string_compare(MY_STRING hLeft_string, MY_STRING hRight_string)
{
	My_string* left;
	My_string* right;
	int min_size;
	int i;

	if (hLeft_string == NULL || hRight_string == NULL)
	{
		printf("Error: Null parameter passed to my_string_compare.\n");
		exit(1);
	}

	left = (My_string*)hLeft_string;
	right = (My_string*)hRight_string;

	min_size = left->size;

	if (right->size < min_size)	min_size = right->size;

	for (i = 0; i < min_size; i++)
	{
		if (left->data[i] != right->data[i])	return left->data[i] - right->data[i];
	}

	return left->size - right->size;
}

Status my_string_extraction(MY_STRING hMy_string, FILE* fp)
{
	My_string* pString = (My_string*)hMy_string;
	int ch;

	if (pString == NULL || fp == NULL)	return FAILURE;

	pString->size = 0;
	ch = fgetc(fp);

	while (ch != EOF && isspace(ch))	ch = fgetc(fp);

	if (ch == EOF)	return FAILURE;

	while (ch != EOF && !isspace(ch))
	{
		if (pString->size >= pString->capacity - 1)
		{
			if (my_string_resize(pString) == FAILURE)	return FAILURE;
		}

		pString->data[pString->size] = ch;
		pString->size++;
		ch = fgetc(fp);
	}

	if (ch != EOF)	ungetc(ch, fp);

	pString->data[pString->size] = '\0';

	return SUCCESS;
}

Status my_string_insertion(MY_STRING hMy_string, FILE* fp)
{
	My_string* pString = (My_string*)hMy_string;
	int i;

	if (pString == NULL || fp == NULL)	return FAILURE;

	for (i = 0; i < pString->size; i++)
	{
		if (fputc(pString->data[i], fp) == EOF)	return FAILURE;
	}

	return SUCCESS;
}

Status my_string_push_back(MY_STRING hMy_string, char item)
{
	My_string* pString = (My_string*)hMy_string;

	if (pString == NULL)	return FAILURE;

	if (pString->size >= pString->capacity - 1)
	{
		if (my_string_resize(pString) == FAILURE)	return FAILURE;
	}

	pString->data[pString->size] = item;
	pString->size++;

	return SUCCESS;
}

Status my_string_pop_back(MY_STRING hMy_string)
{
	My_string* pString = (My_string*)hMy_string;

	if (pString == NULL || pString->size == 0)	return FAILURE;

	pString->size--;

	return SUCCESS;
}

char* my_string_at(MY_STRING hMy_string, int index)
{
	const My_string* pString = (My_string*)hMy_string;

	if (pString == NULL || index < 0 || index >= pString->size)	return NULL;

	return &(pString->data[index]);
}

char* my_string_c_str(MY_STRING hMy_string)
{
	My_string* pString = (My_string*)hMy_string;
	char* temp;
	int i;

	if (pString == NULL)	return NULL;

	if (pString->capacity <= pString->size)
	{
		temp = malloc(sizeof(char) * (pString->size + 1));

		if (temp == NULL)	return NULL;

		for (i = 0; i < pString->size; i++)	temp[i] = pString->data[i];

		free(pString->data);
		pString->data = temp;
		pString->capacity = pString->size + 1;
	}

	pString->data[pString->size] = '\0';

	return pString->data;
}

Status my_string_concat(MY_STRING hResult, MY_STRING hAppend)
{
	My_string* pResult = (My_string*)hResult;
	My_string* pAppend = (My_string*)hAppend;
	int new_size;
	int j;

	if (pResult == NULL || pAppend == NULL)	return FAILURE;

	new_size = pResult->size + pAppend->size;

	while (pResult->capacity <= new_size)
	{
		if (my_string_resize(pResult) == FAILURE)	return FAILURE;
	}

	for (j = 0; j < pAppend->size; j++)
	{
		pResult->data[pResult->size + j] = pAppend->data[j];
	}

	pResult->size = new_size;
	pResult->data[pResult->size] = '\0';

	return SUCCESS;
}

Boolean my_string_empty(MY_STRING hMy_string)
{
	const My_string* pString = (My_string*)hMy_string;

	if (pString == NULL)	return TRUE;

	return (pString->size == 0) ? TRUE : FALSE;
}

Status my_string_assignment(MY_STRING hOriginal, MY_STRING hOverwrite)
{
	My_string* pOriginal = (My_string*)hOriginal;
	My_string* pOverwrite = (My_string*)hOverwrite;
	char* temp;
	int i;

	if (pOriginal == NULL || pOverwrite == NULL)	return FAILURE;

	if (pOverwrite->capacity < pOriginal->size + 1)
	{
		temp = (char*)malloc(sizeof(char) * (pOriginal->size + 1));

		if (temp == NULL)	return FAILURE;

		free(pOverwrite->data);
		pOverwrite->data = temp;
		pOverwrite->capacity = pOriginal->size + 1;
	}

	for (i = 0; i < pOriginal->size; i++)	pOverwrite->data[i] = pOriginal->data[i];

	pOverwrite->data[pOriginal->size] = '\0';
	pOverwrite->size = pOriginal->size;

	return SUCCESS;
}

MY_STRING my_string_init_copy(MY_STRING hMy_string)
{
	My_string* pOriginal = (My_string*)hMy_string;
	My_string* pCopy;
	int i;

	if (pOriginal == NULL)	return NULL;

	pCopy = (My_string*)malloc(sizeof(My_string));

	if (pCopy == NULL)	return NULL;

	pCopy->size = pOriginal->size;
	pCopy->capacity = pOriginal->capacity;
	pCopy->data = (char*)malloc(sizeof(char) * pCopy->capacity);

	if (pCopy->data == NULL)
	{
		free(pCopy);
		return NULL;
	}

	for (i = 0; i < pOriginal->size; i++)	pCopy->data[i] = pOriginal->data[i];

	pCopy->data[pOriginal->size] = '\0';

	return pCopy;
}

void my_string_swap(MY_STRING hLeft, MY_STRING hRight)
{
	My_string* pLeft = (My_string*)hLeft;
	My_string* pRight = (My_string*)hRight;
	int temp_size;
	int temp_capacity;
	char* temp_data;

	if (pLeft == NULL || pRight == NULL)
	{
		printf("Error: Failed to swap strings\n");
		exit(1);
	}

	temp_size = pLeft->size;
	pLeft->size = pRight->size;
	pRight->size = temp_size;

	temp_capacity = pLeft->capacity;
	pLeft->capacity = pRight->capacity;
	pRight->capacity = temp_capacity;

	temp_data = pLeft->data;
	pLeft->data = pRight->data;
	pRight->data = temp_data;
}

void my_string_destroy(MY_STRING* phMy_string)
{
	My_string* pString;

	if (phMy_string == NULL || *phMy_string == NULL)	return;

	pString = (My_string*)*phMy_string;

	free(pString->data);
	free(pString);
	*phMy_string = NULL;
}
///// HELPER FUNCTIONS /////

static Status my_string_resize(My_string* pString)
{
	const int new_capacity = pString->capacity * 2;
	char* temp;
	int i;

	temp = malloc(sizeof(char) * new_capacity);

	if (temp == NULL)	return FAILURE;

	for (i = 0; i < pString->size; i++)	temp[i] = pString->data[i];

	free(pString->data);
	pString->data = temp;
	pString->capacity = new_capacity;

	return SUCCESS;
}