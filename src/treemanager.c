#include "../include/treemanager.h"

/***********************************************************************	
	Tree Management Functions Reference:
	"The Data Compression Book" 2nd Edition - Mark Nelson
***********************************************************************/

void init_tree(struct node *tree, int n) {
	
	/* All the node are empty */
	int i;
	for(i = 0; i < n; i++) {
		tree[i].pos = 0;
		tree[i].father = FREE;
		tree[i].right = UNUSED;
		tree[i].left = UNUSED;
	}
}

struct token match(char* window, struct node *tree, int root, struct param p)
{
	int start = p.start;
        int end = p.end;
        uint32_t n = p.n;
        uint16_t l = p.l;
        int offset = p.offset;	

	struct token t;
	t.match_len = 0;
	t.match_pos = 0;

	int node;
	int diff_offset;
	int w_offset;
	int tmp_match;
	int len;
	int cmp;
	int match_len = 0;

	/* The tree is empty, no possible match */
	if(root == UNUSED) {
		t.match_len = 0;
		return t;
	}

	/* Start from the ROOT */
	node = root;

	/* Look for a match */
	while(1) {
		
		tmp_match = 0;
		len = 1;	

		diff_offset = offset - tree[node].pos;

		/* Properly set the window offset */
		if(offset <= n) {				
			w_offset = tree[node].pos;
		}
		else {
			w_offset = n - diff_offset;
		}

		/* Look for a 'len' match */
		cmp = memcmp(window + start + w_offset, window + end, len);

		/* Find more match in the same node */
		while(cmp == 0 && match_len != l) {

			tmp_match++;
			
			/* Store the longest match length */
                        if(tmp_match > match_len) {
                                match_len = tmp_match;
                                t.match_len = match_len;
                                t.match_pos = w_offset;
                        }

			len++;

			cmp = memcmp(window + start + w_offset, window + end, len);
		}
		
		/* The tree node is greater than the data, see the LEFT side of the tree */
                if(cmp > 0) {
                        node = tree[node].left;
                }
                /* The tree node is smaller than the data, see the RIGHT side of the tree */
                else {
                        node = tree[node].right;
                }

                /* End of the tree or longest match found */
                if(node == UNUSED || match_len == l) {
                        break;
		}
	}	

	return t;
}

/* When I wrote this, only God and I understood that I was doing.
*  Now, God only knows. (quote)
*/

void update_tree(struct node *tree, int *root, int w_offset)
{
	int app;
	int tmp_index;
	int tmp_father;
	int tmp_node = UNUSED;

	/* OVERVIEW:
	*  'w_offset' node: node to be deleted;
	*  'tmp_node' node: node to properly be positioned in the place of the 'w_offset' node.
	*/

	/* The left side of the tree is not empty..."Enjoy the LEFT side, Luke!" */	
	if(tree[w_offset].left != UNUSED) {

		/* The left son of the 'w_offset' node becomes the father of the partial tree*/
		tmp_node = tree[w_offset].left;
		tmp_index = tmp_node;	

		/* The last right son of the partial tree father is linked to the right son of the 'w_offset' node */
		while(tree[tmp_index].right != UNUSED) {
			tmp_index = tree[tmp_index].right;
		}

		tree[tmp_index].right = tree[w_offset].right;

		if(tree[w_offset].right != UNUSED) {
			app = tree[w_offset].right;
			tree[app].father = tmp_index;
		}
	
		/* If the 'w_offset' node was the root node, replace the root */	
		if(*root == w_offset) {
                	*root = tmp_node;
        	}	

	}
	/* The right side of the tree is not empty..."Enjoy the RIGHT side, Luke" */
	else {
		/* The right son of the 'w_offset' node becomes the father of the partial tree */
		if(tree[w_offset].right != UNUSED) {
			tmp_node = tree[w_offset].right;
		}
		/* The 'w_offset' node has no sons, update the 'w_offset' father parameters */
		else {
			tmp_father = tree[w_offset].father;
			if(tree[tmp_father].left == w_offset) {
				tree[tmp_father].left = UNUSED;
			}
			else {
				tree[tmp_father].right = UNUSED;
			}
		}
		
		/* If the 'w_offset' node was the root node, replace the root */
		if(*root == w_offset) {
                        *root = tmp_node;
                }
	
	}
	
	/* Replace the 'w_offset' position with the 'tmp_node' position */
	if(tmp_node != UNUSED) {
		tree[tmp_node].father = tree[w_offset].father;
		if(tree[w_offset].father != UNUSED) {
			app = tree[w_offset].father;
			if(tree[app].left == w_offset) {
				tree[app].left = tmp_node;
			}
			else if(tree[app].right == w_offset) {
				tree[app].right = tmp_node;
			}
		}
	}

}

void append_node(char* window, struct node *tree, int *root, struct param p)
{
	int start = p.start;
        int end = p.end;
        uint32_t n = p.n;
        uint16_t l = p.l;
        int offset = p.offset;

	int index;
	int w_pos;
	int w_offset;
	int diff_offset;
	int tmp_node;
	int tmp_father;
	int cmp = 0;

	w_offset = (offset % n);

	/* The tree is empty, append the ROOT node */
	if(*root == UNUSED) {
		*root = w_offset;
		tree[w_offset].pos = offset;
		tree[w_offset].father = UNUSED;
		tree[w_offset].left = UNUSED;
		tree[w_offset].right = UNUSED;	
		return;
	}

	/* The tree vector is full, delete the node with the 'w_offset' index and reorder the tree */
	if(tree[w_offset].father != FREE) {
		update_tree(tree, root, w_offset);
	}

	/* Start from the ROOT */
	index = *root;

	while(1) {
		
		diff_offset = offset - tree[index].pos;

		/* Properly set the window offset */
                if(offset <= n) {
			w_pos = tree[index].pos;
		}
		else {
			w_pos = n - diff_offset;
		}
	
		cmp = memcmp(window + start + w_pos, window + end, l);
		
		/* Found the same node, so replace the node */
		if(cmp == 0) {
		
			/* The found node is the ROOT node, so set the ROOT parameters */	
			if(index == *root) {	
				*root = w_offset;
				tree[w_offset].father = UNUSED;	 
			}
			/* Set the node (that is not the ROOT node) parameters */
			else {
				tmp_father = tree[index].father;

				if(tree[tmp_father].left == index) {
					tree[tmp_father].left = w_offset;	// left son
				}
				else {
					tree[tmp_father].right = w_offset;	// right son
				}
			}

			/**********************************************
			 Replace the index node with the w_offset node
			**********************************************/
						
			/* Update the hierarchy: father -> son */
			if(tree[index].left != UNUSED) {
				tmp_node = tree[index].left;		// left son
				tree[tmp_node].father = w_offset;
			}
			if(tree[index].right != UNUSED) {
				tmp_node = tree[index].right;		// right son	
				tree[tmp_node].father = w_offset;
			}

			/* Overwrite the replaced node */
			tree[index].father = FREE;
			tree[w_offset] = tree[index];
			tree[w_offset].pos = offset;
			break;
		}

		/* The new one is smaller than the current node..."enjoy the LEFT side of the tree, Luke!" */
		if(cmp > 0) {
			tmp_father = index;
			index = tree[tmp_father].left;
		}
		/* The new one is greater than the current one..."enjoy the RIGHT side of the tree, Luke!" */
		else {
			tmp_father = index;
			index = tree[tmp_father].right;
		}

		/* End of the tree, append the new node and update the parameters */
		if(index == UNUSED) {
			tree[w_offset].pos = offset;
			tree[w_offset].father = tmp_father;
			tree[w_offset].left = UNUSED;
			tree[w_offset].right = UNUSED;
			if(cmp > 0) {  

				tree[tmp_father].left = w_offset;	// left son
			}
			else {
				tree[tmp_father].right = w_offset;	// right son
			}
			break;
		}
	}
}

void print_tree(struct node *tree, int root, int n)
{
        int i;

        printf("%c[1m",27);
        printf("Testing the Tree Structure...\n");
        printf("%c[0m",27);

	printf("ROOT INDEX: %d\n", root);

        for(i = 0; i < n; i++) {
            
                printf("INDEX: %d - ", i);
                printf("FATHER: %d - ", tree[i].father);
                printf("LEFT: %d - ", tree[i].left);
                printf("RIGHT: %d - ", tree[i].right);
                printf("POSITION: %d\n", tree[i].pos);
        }

        printf("%c[1m",27);
        printf("........................................\n");
        printf("%c[0m",27);
}

