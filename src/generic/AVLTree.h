#ifndef AVL_TREE
#define AVL_TREE

#include "Node.h"
#include <assert.h>



using namespace std;
/*
	Implementation created with assistance from http://kukuruku.co/hub/cpp/avl-trees

REQUIRES:
	T must implement a copy constructor and overload < and > in order to call the functions
*/
template <class T> class AVLTree
{
private:
	// Fields
	Node<T>* root;

	// Operations
	Node<T>* nodeInsert(Node<T>*&, T);
	Node<T>* findMin(Node<T>*);
	Node<T>* removeMin(Node<T>*&);
	void destruct(Node<T>*&);
	Node<T>* deleteHelper(Node<T>*&, T);
	Node<T>* helper(Node<T>*);
public:
	//////// Operations ///////////
	bool empty();
	/*
	PROMISES:
		Creates an AVL tree with a null root
	*/
	AVLTree();
	/*
	PROMISES:
		will insert a Node into the AVL tree with a key of type T, and then rebalance the tree
	REQUIRES:
		
	*/
	Node<T>* insert(T);
	/*
	PROMISES:
		Will remove the left-most node from the tree, rebalance it, and return the value of the
		removed node to the user
	REQUIRES:
		The tree must contain nodes, will merely return NULL if called upon an empty tree
	*/
	T* popMin();
	void deleteNode(T);
	/*
	PROMISES:
		will return the value of the left-most node
	*/
	T* readMin();
	/*
	PROMISES:
		will delete all memory being used by the tree;
	*/
	~AVLTree();
};

template <class T>
AVLTree<T>::AVLTree()
	: root(nullptr)
{

}

template <class T>
Node<T>* AVLTree<T>::insert(T newItem) {
#ifdef DEBUG
	cout << "Starting insert operator...\n";
#endif
	return nodeInsert(root, newItem);
}


template <class T>
Node<T>* AVLTree<T>::nodeInsert(Node<T>* &head, T newItem) {
	if (!head) {
			head = new Node<T>(newItem);
	}
	else
	{
		if (newItem < head->itemM) {
			head->left = nodeInsert(head->left, newItem);
		}
		else {
			head->right = nodeInsert(head->right, newItem);
		}
	}
	head = head->balance();
	return head;
}

template <class T>
Node<T>* AVLTree<T>::findMin(Node<T>* node)
{
	if (!(node->left))
		return node;
	return node->left ? findMin(node->left) : node;
}

template <class T>
bool AVLTree<T>::empty()	{
	if (!root)
		return true;
	return false;
}

template <class T>
T* AVLTree<T>::readMin() {
	assert(root);
	return &((findMin(root))->itemM);
}


template<class T>
T* AVLTree<T>::popMin()
{
	if (!root)
		return NULL;
	Node<T>* destroy = removeMin(root);
	T target = destroy->itemM;
	delete destroy;
	destroy = NULL;
	if (root) {
		root = root->balance();
	}
	return &target;
}

template<class T>
void AVLTree<T>::deleteNode(T target)
{
	deleteHelper(root, target);
}

template<class T>
Node<T>* AVLTree<T>::deleteHelper(Node<T>*& head, T target)
{
	if (!head) {
		return NULL;
	}
	if (target < head->itemM)
		head->left = deleteHelper(head->left, target);
	else if (target > head->itemM)
		head->right = deleteHelper(head->right, target);
	else 
	{
		Node<T>* l = head->left;
		Node<T>* r = head->right;
		delete head;
		if (!r) {
			return l;
		}
		else
		{
			Node<T>* min = findMin(r);
			min->right = helper(r);
			min->left = l;
			head = min->balance();
		}
	}
	head = head->balance();
	return head;
}

template <class T>
Node<T>* AVLTree<T>::helper(Node<T>* p) {
	if (p->left == 0)
		return p->right;
	p->left = helper(p->left);
	return p->balance();
}


template <class T>
Node<T>* AVLTree<T>::removeMin(Node<T>* &head)
{
	Node<T>* target;
	if (!head) {	// Exceptional case (will only run on first run)
		return NULL;
	}
	if (!head->left) {	// Exceptional case (will only run on first run)
		target = head;
		if (!head->right) { // Head is only node
			head = NULL;
			return target;
		}
		// There are nodes to the right of head
		target = head;
		head = head->right; // Set the right of head as new head
		return target;
	}
	if (!(head->left->left)) { // Base case
		target = head->left;
		head->left = head->left->right;
		head = head->balance();
		return target;
	}
	return removeMin(head->left); // Move further down the left side of the tree
}



template<class T>
AVLTree<T>::~AVLTree()
{
	destruct(root);
	if (root)
		cout << "Tree was not properly destroyed\n";
}

template <class T>
void AVLTree<T>::destruct(Node<T>* &node) {
	if (!node) { // Exceptional case where the tree is empty to begin with
		return;
	}
	if (node->right)
		destruct(node->right);
	if (node->left)
		destruct(node->left);
	delete node;
	node = NULL;
}



#endif
