#pragma once
#include <array>

template<class T, size_t MAX_ELEMENTS>
class queue {
protected:
    size_t front;// always point to the next empty space.
    size_t rear; // always point to the last element that remove.
    size_t size;
    std::array<T*, MAX_ELEMENTS> buffer;

public:
    queue() : front(1), rear(0), size(0) {};

    bool Add(T* obj) {
        if (!IsFull()) {
            buffer[front++] = obj;
            front = (front % MAX_ELEMENTS);
            size++;
            return true;
        }
        return false;
    }

    bool Get(T& obj) {
        if (!IsEmpty()) {
            rear = (++rear % MAX_ELEMENTS);
            obj = buffer[rear];
            size--;
            return true;
        }
        return false;
    }

    bool IsEmpty() const { return size == 0; }
    bool IsFull()  const { return size == MAX_ELEMENTS; }
};

template<class T, size_t MAX_ELEMENTS>
class queue_with_iterator :public queue<T, MAX_ELEMENTS> {
    enum direction { FOWARD, BACKWARD };
private:
    size_t  m_iterator;// always point to the last element that visit.
    size_t  m_item_left;
    direction m_direction;

public:
    void Set_Iterator_To_First() {
        m_direction = direction::FOWARD;
        m_iterator = this->rear;
        m_item_left = this->size;
    }

    void Set_Iterator_To_Last() {
        m_direction = direction::BACKWARD;
        m_iterator = this->front;
        m_item_left = this->size;
    }

    bool Next(T& obj) {
        if (m_item_left) {
            switch (m_direction) {
            case direction::FOWARD:
                m_iterator = (++m_iterator == MAX_ELEMENTS) ? 0 : m_iterator;
                obj = this->buffer[m_iterator];
                m_item_left--;
                return true;
            case direction::BACKWARD:
                m_iterator = (m_iterator == 0) ? MAX_ELEMENTS : m_iterator;
                obj = this->buffer[--m_iterator];
                m_item_left--;
                return true;
            }
        }
        return false;
    }

};