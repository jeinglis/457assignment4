#include "generic/AVLTree.h"

void testAVL() {

	// Generate Tree 1
	AVLTree<int> tree1 = AVLTree<int>();
	tree1.insert(5);
	tree1.insert(3);
	tree1.insert(6);
	tree1.insert(1);
	tree1.insert(4);
	tree1.insert(7);
	tree1.insert(-2);
	tree1.insert(2);
	tree1.insert(-3);

	// Test Tree 1

	for (int i = 0; i < 9; i++) {
		KOUT::outl;
	}
	KOUT::outl;
	KOUT::outl;

	// Generate Tree 2

	AVLTree<int> tree2 = AVLTree<int>();

	tree2.insert(20);
	tree2.insert(15);
	tree2.readMin();
	tree2.insert(10);
	tree2.insert(5);
	tree2.insert(-1);


	// Test Tree 2

	for (int i = 0; i < 5; i++) {
		KOUT::outl;
	}
	KOUT::outl;
	KOUT::outl;


	// Generate Tree 3

	AVLTree<int> tree3 = AVLTree<int>();
	tree3.insert(11);
	tree3.insert(6);
	tree3.insert(7);
	tree3.insert(25);
	tree3.popMin();
	tree3.insert(7);
	tree3.insert(99);
	tree3.insert(-1000);
	tree3.insert(10);

	// Test Tree 3

	tree3.deleteNode(7);
	tree3.deleteNode(10);
	for (int i = 0; i < 5; i++) {
		KOUT::outl;
	}
	KOUT::outl;
	KOUT::outl;

	return;
}
