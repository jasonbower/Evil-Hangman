#include <stdio.h>
#include <stdlib.h>
#include "associative_array.h"

typedef struct node Node;
struct node
{
	MY_STRING key;
	GENERIC_VECTOR words;
	int height;
	Node* left;
	Node* right;
};

typedef struct associative_array
{
	Node* root;
} Associative_array;

///// HELPER FUNCTIONS /////
static ITEM my_string_init_copy_item(ITEM item);
static void my_string_destroy_item(ITEM* pItem);
static Node* avl_create_node(MY_STRING key, MY_STRING word);
static Node* avl_insert(Node* root, MY_STRING key, MY_STRING word, Status* pStatus);
static Node* avl_search(Node* root, MY_STRING key);
static int avl_get_height(Node* root);
static int avl_get_max_height(Node* left, Node* right);
static int avl_get_balance(Node* root);
static Node* avl_rotate_right(Node* root);							// Performs a right rotation and updates node heights.
static Node* avl_rotate_left(Node* root);							// Performs a left rotation and updates node heights.
static void avl_find_largest_family(Node* root, Node** ppBestNode);	// Traverses the tree and stores the node with the largest word vector in ppBestNode.
static void avl_destroy_node(Node* pNode);
static void avl_destroy(Node* root);

///// INTERFACE FUNCTIONS /////

ASSOCIATIVE_ARRAY associative_array_init_default(void)
{
	Associative_array* pArray = (Associative_array*)malloc(sizeof(Associative_array));

	if (pArray == NULL)
	{
		fprintf(stderr, "Error 3: failed to allocate memory for Associative_array object in associative_array_init_default\n");
		exit(1);
	}

	pArray->root = NULL;

	return pArray;
}

GENERIC_VECTOR associative_array_find(ASSOCIATIVE_ARRAY hArray, MY_STRING key)
{
	Associative_array* pArray;
	Node* pNode;

	if (hArray == NULL || key == NULL)
	{
		fprintf(stderr, "Error 4: null parameter passed to associative_array_find\n");
		exit(1);
	}

	pArray = (Associative_array*)hArray;
	pNode = avl_search(pArray->root, key);

	if (pNode == NULL)	return NULL;

	return pNode->words;
}

Status associative_array_insert(ASSOCIATIVE_ARRAY hArray, MY_STRING key, MY_STRING word)
{
	Associative_array* pArray;
	Status status = SUCCESS;

	if (hArray == NULL || key == NULL || word == NULL)	return FAILURE;

	pArray = (Associative_array*)hArray;
	pArray->root = avl_insert(pArray->root, key, word, &status);

	return status;
}

GENERIC_VECTOR associative_array_get_largest_family(ASSOCIATIVE_ARRAY hArray, MY_STRING winning_key)
{
	Node* bestNode = NULL;
	Associative_array* pArray;

	if (hArray == NULL || winning_key == NULL)
	{
		fprintf(stderr, "Error 5: null parameter passed to associative_array_get_largest_family\n");
		exit(1);
	}

	pArray = (Associative_array*)hArray;
	avl_find_largest_family(pArray->root, &bestNode);

	if (bestNode == NULL || my_string_assignment(bestNode->key, winning_key) == FAILURE)	return NULL;	// Copies the largest family's key into winning_key.

	return bestNode->words;
}

void associative_array_destroy(ASSOCIATIVE_ARRAY* phArray)
{
	Associative_array* pArray;

	if (phArray == NULL || *phArray == NULL)	return;

	pArray = (Associative_array*)(*phArray);
	avl_destroy(pArray->root);
	free(pArray);
	*phArray = NULL;
}

///// HELPER FUNCTIONS /////

static ITEM my_string_init_copy_item(ITEM item)
{
	return (ITEM)my_string_init_copy((MY_STRING)item);
}

static void my_string_destroy_item(ITEM* pItem)
{
	my_string_destroy((MY_STRING*)pItem);
}

static Node* avl_create_node(MY_STRING key, MY_STRING word)
{
	Node* new_node = (Node*)malloc(sizeof(Node));

	if (new_node == NULL)
	{
		fprintf(stderr, "Error 6: failed to allocate memory for Node object in avl_create_node\n");
		avl_destroy_node(new_node);
		exit(1);
	}

	new_node->key = my_string_init_copy(key);
	new_node->words = generic_vector_init_default(my_string_init_copy_item, my_string_destroy_item);
	new_node->height = 1;
	new_node->left = NULL;
	new_node->right = NULL;

	if (new_node->key == NULL || new_node->words == NULL || generic_vector_push_back(new_node->words, word) == FAILURE)
	{
		avl_destroy_node(new_node);
		exit(1);
	}

	return new_node;
}

static Node* avl_insert(Node* root, MY_STRING key, MY_STRING word, Status* pStatus)
{
	int comparison;
	int balance_factor;

	if (root == NULL)	return avl_create_node(key, word);

	comparison = my_string_compare(key, root->key);

	if		(comparison < 0)	root->left  = avl_insert(root->left,  key, word, pStatus);
	else if (comparison > 0)	root->right = avl_insert(root->right, key, word, pStatus);
	else	// Matching keys belong to the same word family.
	{
		if (generic_vector_push_back(root->words, word) == FAILURE)	*pStatus = FAILURE;

		return root;
	}

	root->height = 1 + avl_get_max_height(root->left, root->right);
	balance_factor = avl_get_balance(root);

	// Rebalance the AVL tree if inserting this key made it unbalanced.
	if (balance_factor >  1 && my_string_compare(key, root->left->key)  < 0)	return avl_rotate_right(root);

	if (balance_factor < -1 && my_string_compare(key, root->right->key) > 0)	return avl_rotate_left(root);

	if (balance_factor >  1 && my_string_compare(key, root->left->key)  > 0)
	{
		root->left = avl_rotate_left(root->left);

		return avl_rotate_right(root);
	}

	if (balance_factor < -1 && my_string_compare(key, root->right->key) < 0)
	{
		root->right = avl_rotate_right(root->right);

		return avl_rotate_left(root);
	}

	return root;
}

static Node* avl_search(Node* root, MY_STRING key)
{
	int comparison;

	if (root == NULL)	return NULL;

	comparison = my_string_compare(key, root->key);

	if (comparison == 0)	return root;

	if (comparison < 0)		return avl_search(root->left, key);

	return avl_search(root->right, key);
}

static int avl_get_height(Node* root)
{
	if (root == NULL)	return 0;

	return root->height;
}

static int avl_get_max_height(Node* left, Node* right)
{
	int left_height  = avl_get_height(left);
	int right_height = avl_get_height(right);

	if (left_height > right_height)	return left_height;

	return right_height;
}

static int avl_get_balance(Node* root)
{
	if (root == NULL)	return 0;

	return avl_get_height(root->left) - avl_get_height(root->right);
}

static Node* avl_rotate_right(Node* root)
{
	Node* new_root = root->left;
	Node* middle_subtree = new_root->right;

	new_root->right = root;
	root->left = middle_subtree;
	root->height = 1 + avl_get_max_height(root->left, root->right);
	new_root->height = 1 + avl_get_max_height(new_root->left, new_root->right);

	return new_root;
}

static Node* avl_rotate_left(Node* root)
{
	Node* new_root = root->right;
	Node* middle_subtree = new_root->left;

	new_root->left = root;
	root->right = middle_subtree;
	root->height = 1 + avl_get_max_height(root->left, root->right);
	new_root->height = 1 + avl_get_max_height(new_root->left, new_root->right);

	return new_root;
}

static void avl_find_largest_family(Node* root, Node** ppBestNode)
{
	if (root == NULL)	return;

	if (*ppBestNode == NULL || generic_vector_get_size(root->words) > generic_vector_get_size((*ppBestNode)->words))	*ppBestNode = root;

	avl_find_largest_family(root->left, ppBestNode);
	avl_find_largest_family(root->right, ppBestNode);
}

static void avl_destroy_node(Node* pNode)
{
	if (pNode != NULL)
	{
		my_string_destroy(&pNode->key);
		generic_vector_destroy(&pNode->words);
		free(pNode);
	}
}

static void avl_destroy(Node* root)
{
	if (root == NULL)	return;

	avl_destroy(root->left);
	avl_destroy(root->right);
	avl_destroy_node(root);
}