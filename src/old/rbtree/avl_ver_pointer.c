#include<stdio.h>
#include<stdlib.h>
#include<math.h>

struct node
{
	int key; // can be vid or lbn
	struct node *next; // freelist


	//meta data below
	int height;
	int state; // allocated or not
	struct node *left;
	struct node *right;
};

void deallocateNode(struct node *root);

int height(struct node *N)
{
	if (N == NULL)
		return 0;
	return N->height;
}

int Max(int a, int b) { return a > b ? a : b; }

struct node* newNode(int key)
{
	struct node* node = (struct node*)
		malloc(sizeof(struct node));
	node->key = key;
	node->left = NULL;
	node->right = NULL;
	node->height = 1; // new node is initially added at leaf
	return(node);
}

struct node *rightRotate(struct node *y)
{
	struct node *x = y->left;
	struct node *T2 = x->right;

	// Perform rotation
	x->right = y;
	y->left = T2;

	// Update heights
	y->height = Max(height(y->left), height(y->right)) + 1;
	x->height = Max(height(x->left), height(x->right)) + 1;

	// Return new root
	return x;
}

struct node *leftRotate(struct node *x)
{
	struct node *y = x->right;
	struct node *T2 = y->left;

	// Perform rotation
	y->left = x;
	x->right = T2;

	// Update heights
	x->height = Max(height(x->left), height(x->right)) + 1;
	y->height = Max(height(y->left), height(y->right)) + 1;

	// Return new root
	return y;
}

int getBalance(struct node *N)
{
	if (N == NULL)
		return 0;

	return height(N->left) - height(N->right);
}

struct node *find(struct node *root, int vid)
{
	struct node *localRoot = root;

	while (localRoot != NULL)
	{
		if (vid == localRoot->key)
			return localRoot;

		else if (vid < localRoot->key)
			localRoot = localRoot->left;

		else if (vid > localRoot->key)
			localRoot = localRoot->right;
	}

	return NULL;
}

struct node* insert(struct node *node, struct node *vte)
{
	if (node == NULL)
		return node = vte;

	if (vte->key < node->key)
		node->left = insert(node->left, vte);
	else
		node->right = insert(node->right, vte);

	//update height
	node->height = Max(height(node->left), height(node->right)) + 1;


	//rebalancing
	int balance = getBalance(node);
	
	// LL
	if (balance > 1 && vte->key < node->left->key)
		return rightRotate(node);
	
	// LR
	if (balance > 1 && vte->key > node->left->key)
	{
		node->left = leftRotate(node->left);
		return rightRotate(node);
	}	
	
	// RR
	if (balance < -1 && vte->key > node->right->key)
		return leftRotate(node);
	
	// RL
	if (balance < -1 && vte->key < node->right->key)
	{
		node->right = rightRotate(node->right);
		return leftRotate(node);
	}

	return node;
}

struct node* minValueNode(struct node* node)
{
	struct node* current = node;

	while (current->left != NULL)
		current = current->left;

	return current;
}

struct node* delete(struct node* root, struct node* vte)
{
	if (root == NULL)
		return root;

	if (vte->key < root->key)
		root->left = delete(root->left, vte);

	else if (vte->key > root->key)
		root->right = delete(root->right, vte);

	else
	{
		// 1 child || no child
		if ((root->left == NULL) || (root->right == NULL))
		{
			struct node *temp = root->left ? root->left : root->right;

			// no child 
			if (temp == NULL)
			{
				temp = root;
				root = NULL;
			}
			else // 1 child, copy the contents of the non-empty child
				*root = *temp;

			deallocateNode(temp);
		}

		else
		{
			// 2 children
			struct node *temp = minValueNode(root->right);

			// copy the inorder successor's data
			root->key = temp->key;

			// Delete the inorder successor
			root->right = delete(root->right, temp);
		}
	}

	if (root == NULL)
		return root;

	//update height
	root->height = Max(height(root->left), height(root->right)) + 1;

	//rebalancing
	int balance = getBalance(root);

	//LL
	if (balance > 1 && getBalance(root->left) >= 0)
		return rightRotate(root);

	//LR
	if (balance > 1 && getBalance(root->left) < 0)
	{
		root->left = leftRotate(root->left);
		return rightRotate(root);
	}

	//RR
	if (balance < -1 && getBalance(root->right) <= 0)
		return leftRotate(root);

	//RL
	if (balance < -1 && getBalance(root->right) > 0)
	{
		root->right = rightRotate(root->right);
		return leftRotate(root);
	}

	return root;
}

void PrintTree(struct node *root) {

	int numberOfNodes = 1 << (root->height);

	struct node **queue = malloc(sizeof(struct node *) * (numberOfNodes));
	
	queue[0] = NULL;
	queue[1] = root;

	for (int i = 2; i <= numberOfNodes; i++)
	{
		if (i % 2 == 0)
		{
			if (queue[i / 2] == NULL)
				queue[i] = NULL;

			else
				queue[i] = queue[i / 2]->left;
		}

		else
		{
			if (queue[i / 2] == NULL)
				queue[i] = NULL;

			else
				queue[i] = queue[i / 2]->right;
		}
	}

	for (int i = 1; i <= root->height; i++)
	{
		printf("%d Level : ", i);

		for (int j = pow((double)2, (double)(i - 1)); j< pow((double)2, (double)i); j++) 
		{
			if (queue[j] == NULL)
			{
				printf("%s", queue[j / 2] == NULL ? "  " : "N ");
			}

			else
			{
				printf("%d(%d) ", queue[j]->key, queue[j]->height);
			}
		}
		printf("\n");
	}
}

struct node *freeList;
struct node *freeList_head;

void initFreeList(void)
{
	int sizeOfFreeList = 0;

	printf("freelist를 몇개나 만들까요? ");
	scanf("%d", &sizeOfFreeList);

	freeList = malloc(sizeof(struct node) * sizeOfFreeList);
	
	//initialize
	for (int i = 0; i < sizeOfFreeList; i++)
	{
		freeList[i].key = 0;
		freeList[i].height = 1;
		freeList[i].state = 0;
		freeList[i].left = NULL;
		freeList[i].right = NULL;
	}

	for (int i = 0; i < sizeOfFreeList; i++)
	{
		if (i != sizeOfFreeList - 1)
			freeList[i].next = &freeList[i + 1];

		else
		{
			freeList[i].next = NULL;
		}		
	}
}

void printFreeList()
{
	struct node *printList = freeList;

	printf("freeList> \n");

	while (printList != NULL)
	{
		printf("%s", printList->state ? "■" : "□");
		printList = printList->next;
	}

	printf("\n");
}

struct node *allocateNode(int vid)
{
	struct node *allocate = freeList_head;

	allocate->key = vid;
	allocate->state = 1;
	
	printf("%노드가 할당되었습니다.\n");
	
	//꽉차면 무한루프
	while (freeList_head->state == 1)
	{
		if (freeList_head->next == NULL)
			freeList_head = freeList;
		
		else
			freeList_head = freeList_head->next;
	}
	
	return allocate;
}

void deallocateNode(struct node *root)
{
	//초기화
	root->key = 0;
	root->height = 1;
	root->state = 0;
	root->left = NULL;
	root->right = NULL;
		
	printf("노드가 반환되었습니다.\n");
}

int main()
{
	struct node *root = NULL;
	struct node *temp = NULL;
	int vid = 0;
	
	initFreeList();
	freeList_head = freeList;
	
	int state = 0;

#define INSERT 1
#define DELETE 2
#define PRINTFREELIST 3
#define PRINTTREE 4
#define QUIT 5
	
	while (state != QUIT)
	{
		printf("1.삽입\n2.삭제\n3.프리리스트 출력\n4.트리 출력\n5.나가기\n");
		printf("무슨 작업을 할까요? ");
		scanf("%d", &state);

		switch (state)
		{
		case INSERT:
			printf("삽입할 VTE의 vid를 입력하세요. ");
			scanf("%d", &vid);

			//찾는다
			temp = find(root, vid);

			//없으면
			if (!temp)
			{
				//프리리스트로부터 하나 받아와서
				temp = allocateNode(vid);
				
				//삽입
				root = insert(root, temp);
			}

			//있으면 아무 작업안함

			break;

		case DELETE:

			printf("삭제할 VTE의 vid를 입력하세요. ");
			scanf("%d", &vid);

			temp = find(root, vid);

			if (temp)
			{
				root = delete(root, temp);
			}

			break;

		case PRINTFREELIST:
			printFreeList(freeList);
			break;

		case PRINTTREE:
			if (root)
			{
				PrintTree(root);
			}
			break;
		case QUIT:
			state = QUIT;
			break;

		default:
			printf("올바르지 않은 입력입니다. 다시 입력해주세요.\n");
			break;
		}
	}

	return 0;
}
