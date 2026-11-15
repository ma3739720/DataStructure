#pragma once
#include <iostream>
#include "priNode.h"


//This class impelements the priority queue as a sorted list (Linked List)
//The item with highest priority is at the front of the queue
template <typename T>
class priQueue
{
protected:  //in case you need to inherit for the project
    priNode<T>* head;
    int count;      // number of items (for GetCount)
public:
    priQueue() : head(nullptr), count(0) {}

    ~priQueue() {
        T tmp;
        int p;
        while (dequeue(tmp, p));
    }

    //insert the new node in its correct position according to its priority
    void enqueue(const T& data, int priority) {
        priNode<T>* newNode = new priNode<T>(data, priority);
        count++;

        if (head == nullptr || priority > head->getPri()) {

            newNode->setNext(head);
            head = newNode;
            return;
        }

        priNode<T>* current = head;
        while (current->getNext() && priority <= current->getNext()->getPri()) {
            current = current->getNext();
        }
        newNode->setNext(current->getNext());
        current->setNext(newNode);
    }

    bool dequeue(T& topEntry, int& pri) {
        if (isEmpty())
            return false;

        topEntry = head->getItem(pri);
        priNode<T>* temp = head;
        head = head->getNext();
        delete temp;
        count--;
        return true;
    }

    bool peek(T& topEntry, int& pri) {
        if (isEmpty())
            return false;

        topEntry = head->getItem(pri);
        return true;
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    // Number of items. O(1)   (allowed project addition)
    int GetCount() const {
        return count;
    }

    // Prints every item from the front, separated by sep. The queue is not changed. O(n)
    // (allowed project addition)
    void print(std::ostream& out, const char* sep = " ") const {
        bool first = true;
        int pri;
        for (priNode<T>* p = head; p; p = p->getNext()) {
            if (!first) out << sep;
            out << p->getItem(pri);
            first = false;
        }
    }
};
