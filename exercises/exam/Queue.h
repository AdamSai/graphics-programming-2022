#ifndef ITU_GRAPHICS_PROGRAMMING_QUEUE_H
#define ITU_GRAPHICS_PROGRAMMING_QUEUE_H

#include <vector>
#include <iostream>
#include "glm/vec2.hpp"

class Queue
{
    std::vector<glm::vec2> data;
    int front, back;
    int size;
    int capacity;

public:
    Queue( int capacity )
    {
        this->capacity = capacity;
        data = std::vector<glm::vec2>( capacity );
        front = 0;
        back = 0;
        size = 0;
    }

    void enqueue( glm::vec2 item )
    {
        if ( size == capacity )
        {
            std::cout << "Queue is full" << std::endl;
            return;
        }
        data[back] = item;
        back = ( back + 1 ) % capacity;
        size++;
    }

    glm::vec2 dequeue()
    {
        if ( size == 0 )
        {
            std::cout << "Queue is empty" << std::endl;
            return glm::vec2( 0, 0 );
        }
        glm::vec2 item = data[front];
        front = ( front + 1 ) % capacity;
        size--;
        return item;
    }

    glm::vec2 peek()
    {
        if ( size == 0 )
        {
            std::cout << "Queue is empty" << std::endl;
            return glm::vec2( 0, 0 );
        }
        return data[front];
    }

    bool isEmpty()
    {
        return size == 0;
    }

    bool isFull()
    {
        return size == capacity;
    }

    // clear queue
    void clear()
    {
        front = 0;
        back = 0;
        size = 0;
    }
};

#endif //ITU_GRAPHICS_PROGRAMMING_QUEUE_H
