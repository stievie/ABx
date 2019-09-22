#pragma once

namespace sa {

template <typename T>
class LinkedList
{
public:
    struct Node
    {
        T data;
        Node* next;
    };

    Node* head;
public:
    LinkedList() = default;
    void push(Node* newNode)
    {
      newNode->next = head;
      head = newNode;
    }
    Node* pop()
    {
      Node* top = head;
      head = head->next;
      return top;
    }
};

}
