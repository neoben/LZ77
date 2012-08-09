#ifndef TREEMANAGER_H
#define TREEMANAGER_H

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define UNUSED -1
#define FREE -2

/* Node structure */
struct node
{
        unsigned int pos;
        int father;
        int right;              
        int left;  
};

/* Token structure */
struct token
{
        int match_len;         
        int match_pos; 
};

/* Paramters Structure */
struct param
{
	int start;
	int end;
	uint32_t n;
	uint16_t l;
	int offset;
};

/* The init_tree function:
*  initializes the tree setting all the starting node parameters.
*  @param n: window size (and so number of nodes).
*/
void init_tree(struct node *tree, int n);

/* The token_match function:
*  looks for a match of the data in the tree;
*  returns a token with the match length and the match position (if any).
*  @param root: tree root;
*  @param start: pointer to the beginning of the window;
*  @param end: pointer to the end of the window;
*  @param n: window size;
*  @param l: lookahead buffer size;
*  @param offset: file offset.
*/
struct token match(char* window, struct node *tree, int root, struct param p);

/* The appen_node function:
*  appends a new node to the tree.
*  @param root: tree root;
*  @param p: window parameters.
*/
void append_node(char* window, struct node *tree, int *root, struct param p);

/* The update_tree function:
*  updates the tree structure reordering the node order.
*  @param root: tree root;
*  @param w_offset: index of the tree node to delete from the tree structure.
*/
void update_tree(struct node *tree, int *root, int w_offset);

/* The print_tree function:
*  prints the tree structure.
*  DEBUG FUNCTION.
*/
void print_tree(struct node *tree, int root, int n);

#endif

