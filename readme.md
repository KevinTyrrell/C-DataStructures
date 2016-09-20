## Data Structures in C

The following are static libraries for use in my C coding projects. They were written by me and I didn't look online for any of the proper algorithms. O complexity might be improved by researching the most optimal ways of writing these structures.


| Data Structure | Header File    | C File         | Constructor Function | Functions needed for the constructor |
| -------------- | -------------- | -------------- | -------------------- | ----- |
| LinkedList     | LinkedList.h   | LinkedList.c   | LinkedList_new()	  | equals(void*, void*), toString(void*)
| HashMap        | HashMap.h	  | HashMap.c      | HashMap_new()		  | unsigned int hash(void*), equals(void*, void*), toString(void*)
| RedBlackTree   | RedBlackTree.h | RedBlackTree.c | RedBlackTree_new()   | int compare(void*, void*)