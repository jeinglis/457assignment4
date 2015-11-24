#ifndef NODE_H
#define NODE_H
#include <iostream>

using namespace std;

template <class S> class Node	{
	template <class T> friend class AVLTree;
public:
	Node();
	Node(const Node&);
	Node(S key);
	S getItem();
	~Node();
private:
	// Fields 
	S itemM;
	int height;
	Node<S>* left;
	Node<S>* right;

	// Operations
	int calcBalance();
	void repair();
	Node<S>* balance();
	Node<S>* rotateRight();
	Node<S>* rotateLeft();

};

template <class S>
Node<S>::Node()
	: itemM(0), left(nullptr), right(nullptr), height(-1)
{

}

template<class S>
Node<S>::Node(const Node<S> &src)
	: left(src.left), right(src.right), itemM(src.itemM), height(src.height)
{
}



template<class S>
Node<S>::Node(S key)
	: itemM(key), left(nullptr), right(nullptr), height(-1)
{
}

template <class S>
S Node<S>::getItem() {
	return itemM;
}

template <class S>
Node<S>* Node<S>::balance() {
	repair();
	if (calcBalance() == 2) {
		if (right->calcBalance() < 0)
			right = right->rotateRight();
		return rotateLeft();
	}
	if (calcBalance() == -2) {
		if (left->calcBalance() > 0)
			left = left->rotateLeft();
		return rotateRight();
	}
	return this;
}

template<class S>
Node<S>* Node<S>::rotateRight()
{
	Node<S>* temp = left;
	left = temp->right;
	temp->right = this;
	repair();
	temp->repair();
	return temp;
}

template<class S>
Node<S>* Node<S>::rotateLeft()
{
	Node<S>* temp = right;
	right = temp->left;
	temp->left = this;
	repair();
	temp->repair();
	return temp;
}

template<class S>
int Node<S>::calcBalance()
{
	if (!this) {
		return 0;
	}
	else if (!left || !right) {
		if (!left && !right)
			return 0;
		else if (!left)
			return right->height;
		else
			return -left->height;
	}
	else
		return right->height - left->height;
}

template<class S>
void Node<S>::repair()
{
	// Check for null values:
	if (!left || !right) {
		if (!left && !right)
			height = 1;
		else if (!left)
			height = right->height + 1;
		else
			height = left->height + 1;
	}
	else
	{
		// Take the difference
		int h1 = left->height;
		int h2 = right->height;
		height = (h1 > h2 ? h1 : h2) + 1;
	}
}

template <class S>
Node<S>::~Node()
{

}
#endif
