#include <memory>
#include <cassert>
#include <iostream>

#ifndef DLR_HPP_
#define DLR_HPP_


enum _WHERE {
    FRONT,
    BACK
};

template<typename Key>
class Ring {
private:
    struct node {
        Key _key;

        std::shared_ptr<node> next;
        //weak_ptr used to break reference cycles formed by shared_ptr
        std::weak_ptr<node> prev;

        node() = default;

        explicit node(const Key &k = 0, std::shared_ptr<node> p = nullptr) : _key(k), next(p), prev(p) {}
    };

    int _size = 0;

    std::shared_ptr<node> any = nullptr;

    std::shared_ptr<node> append(std::shared_ptr<node> nodeptr, const Key &, _WHERE);

public:

    Ring() = default;

    Ring(const Ring &);

    Ring &operator=(const Ring &);

    ~Ring() {
        if (any) any->next.reset();
        any.reset();
    }

    void clear() {
        if (any) any.reset();
        _size = 0;
    }

    void append(const Key &, _WHERE);

    bool insertAfter(const Key &, const Key &where, uint which = 1);

    bool insertAt(const Key &, const Key &where, uint which = 1);

    bool erase(const Key &);

    void print();

    void reversePrint();

    bool isEmpty() const { return (any == nullptr); }

    int size() const { return _size; }

    struct BiDirectionalIterator;

    BiDirectionalIterator wrappedAny() const { return BiDirectionalIterator(any); }

    bool find(const Key &k) {
        if (any) {
            auto temp = wrappedAny();
            do {
                ++temp;
            } while (temp != wrappedAny() && *temp != k);
            if (temp != wrappedAny()) return true;
        }
        return false;
    }

    struct BiDirectionalIterator {
        BiDirectionalIterator() = default;

        BiDirectionalIterator(std::shared_ptr<node> x) : it{x} {}

        BiDirectionalIterator &operator=(std::shared_ptr<node> pNode) {
            this->it = pNode;
            return *this;
        }

        Key &operator*() const { return it ? it->_key : 0; };

        BiDirectionalIterator &operator--() {
            it = it->prev;
            return *this;
        }


        BiDirectionalIterator operator--(int) {
            BiDirectionalIterator tmp = *this;
            auto shWeak = it->prev.lock();
            it = shWeak;
            return tmp;
        }

        BiDirectionalIterator &operator++() {
            it = it->next;
            return *this;
        }

        BiDirectionalIterator operator++(int) {
            BiDirectionalIterator tmp = *this;
            it = it->next;
            return tmp;
        }

        BiDirectionalIterator &operator+=(int dist) {
            for (uint i = 0; i < dist; i++) {
                it = it->next;
            }
            return *this;
        }

        BiDirectionalIterator &operator-=(int dist) {
            for (uint i = 0; i < dist; i++) {
                auto shWeak = it->prev.lock();
                it = shWeak;
            }
            return *this;
        }

        bool operator==(const BiDirectionalIterator &rhs) const { return (it == rhs.it); }

        bool operator!=(const BiDirectionalIterator &rhs) const { return !(it == rhs.it); }

    private:
        std::shared_ptr<node> it;
    };

};

template<typename Key>
Ring<Key>::Ring(const Ring &rhs) {
    auto temp = any;
    do {
        append(temp->_key);
        temp = temp->next;
    } while (temp != rhs.any);
}

template<typename Key>
Ring<Key> &Ring<Key>::operator=(const Ring &rhs) {
    if (this == &rhs) {
        return *this;
    }
    clear();
    auto temp = rhs.any;
    do {
        append(temp->_key);
        temp = temp->next;
    } while (temp != rhs.any);
    return *this;
}

template<typename Key>
std::shared_ptr<typename Ring<Key>::node> Ring<Key>::append(std::shared_ptr<Ring::node> nodeptr,
                                                            const Key &k, _WHERE whr) {
    if (nodeptr) {
        auto newNode = std::make_shared<node>(k);
        switch (whr) {
            case FRONT:
                newNode->next = nodeptr->next;
                newNode->prev = nodeptr;
                nodeptr->next->prev = nodeptr;
                nodeptr->next = newNode;
                _size++;
                return newNode;
                break;
            case BACK:
                newNode->next = nodeptr;
                newNode->prev = nodeptr->prev;
                auto shWeak = nodeptr->prev.lock();
                shWeak->next = newNode;
                nodeptr->prev = newNode;
                _size++;
                return newNode;
                break;
        }
    }
    return nullptr;
}

template<typename Key>
void Ring<Key>::append(const Key &k, _WHERE whr) {
    auto newNode{std::make_shared<node>(k)};
    if (!any) {
        newNode->next = newNode;
        newNode->prev = newNode;
        any = newNode;
        _size++;
    } else {
        newNode->prev = any->prev;
        newNode->next = any;
        auto lockWeak = any->prev.lock();
        lockWeak->next = newNode;
        any->prev = newNode;
        _size++;
        switch (whr) {
            case FRONT:
                any = newNode;
                break;
            case BACK:
                break;
        }

    }
}

template<typename Key>
bool Ring<Key>::insertAfter(const Key &k, const Key &where, uint which) {
    if (any && which > 0) {
        auto temp = any;
        uint i = 0;
        do {
            if (temp->_key == where) {
                ++i;
                if (i == which) break;
            }
            temp = temp->next;
        } while (temp != any);
        append(temp, k, FRONT);
        return true;
    }
    return false;
}


template<typename Key>
bool Ring<Key>::insertAt(const Key &k, const Key &where, uint which) {

    if (any && which > 0) {
        auto temp = any;
        uint i = 0;
        do {
            if (temp->_key == where) {
                ++i;
                if (i == which) break;
            }
            temp = temp->next;
        } while (temp != any);

        auto newNode = append(temp, k, BACK);

        if (where == 1) any = newNode;
        return true;
    }
    return false;
}


template<typename Key>
void Ring<Key>::print() {
    if (any) {
        auto temp = any;
        do {
            std::cout << temp->_key << std::endl;
            temp = temp->next;
        } while (temp != any);
        std::cout << "->number of nodes in ring: " << _size << std::endl;
        return;
    }
    std::cout << "@empty ring\n";
}

template<typename Key>
void Ring<Key>::reversePrint() {
    if (any) {
        auto temp = any;
        do {
            auto tmpPrev = temp->prev.lock();
            std::cout << "/*/" << temp->_key << std::endl;
            temp = tmpPrev;
        } while (temp != any);
        std::cout << "->number of nodes in ring: " << _size << std::endl;
        return;
    }
    std::cout << "@empty ring\n";
}


template<typename Key>
bool Ring<Key>::erase(const Key &k) {
    if (any) {
        //single object situation
        if (any->next == any && any->_key == k) {
            any->next = nullptr;
            any = nullptr;
            _size--;
            return true;
        }
        //more than one object, key stored in any
        if (any->_key == k) {
            any->next->prev = any->prev;
            auto shWeak = any->prev.lock();
            shWeak->next = any->next;
            any = any->next;
            _size--;
            return true;
        }

        auto temp = any;
        do {
            if (temp->_key == k) {
                temp->next->prev = temp->prev;
                auto shWeak = temp->prev.lock();
                shWeak->next = temp->next;
                _size--;
                return true;
            }
            temp = temp->next;
        } while (temp != any);

    }
    return false;
}


#endif