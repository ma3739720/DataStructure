#pragma once
//	This is an updated version of code originally
//  created by Frank M. Carrano and Timothy M. Henry.
//  Copyright (c) 2017 Pearson Education, Hoboken, New Jersey.

/** ADT stack: Array-based implementation.
 @file ArrayStack.h */

#ifndef ARRAY_STACK_
#define ARRAY_STACK_

#include <iostream>
#include "Stack.h"

 //Unless spesificed by the stack user, the default size is 100
template<typename T>
class ArrayStack : public StackADT<T>
{
	enum { MAX_SIZE = 5000 };
protected:  //in case you need to inherit for the project
	T items[MAX_SIZE];		// Array of stack items
	int top;                   // Index to top of stack

public:

	ArrayStack()
	{
		top = -1;
	}  // end default constructor

	bool isEmpty() const
	{
		return top == -1;
	}  // end isEmpty

	bool push(const T& newEntry)
	{
		if (top == MAX_SIZE - 1) return false;	//Stack is FULL

		top++;
		items[top] = newEntry;
		return true;
	}  // end push

	bool pop(T& TopEntry)
	{
		if (isEmpty()) return false;

		TopEntry = items[top];
		top--;
		return true;
	}  // end pop

	bool peek(T& TopEntry) const
	{
		if (isEmpty()) return false;

		TopEntry = items[top];
		return true;
	}  // end peek

	// Number of items. O(1)   (allowed project addition)
	int GetCount() const
	{
		return top + 1;
	}

	// Prints from the TOP (most recent) to the bottom, separated by sep. Stack is not changed. O(n)
	// (allowed project addition)
	void print(std::ostream& out, const char* sep = " ") const
	{
		for (int i = top; i >= 0; i--)
		{
			out << items[i];
			if (i > 0) out << sep;
		}
	}

}; // end ArrayStack

#endif
