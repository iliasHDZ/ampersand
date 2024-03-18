#pragma once

class DLChainItem {
protected:
    inline DLChainItem()
        : next(nullptr), prev(nullptr) {}

private:
    virtual void _() {}

public:
    DLChainItem* next;
    DLChainItem* prev;

};

template <typename T>
class DLChain {
public:
    DLChain() : head(nullptr) {}

    void append(T* item) {
        if (head == nullptr) {
            head = item;
            item->next = item;
            item->prev = item;
            return;
        }

        item->prev = head->prev;
        item->next = head;

        head->prev->next = item;
        head->prev       = item;
    }

    void prepend(T* item) {
        append(item);

        head = item;
    }

    T* first() {
        return head;
    }

    T* last() {
        if (head == nullptr) return nullptr;

        return head->prev;
    }

    bool has(T* item) {
        if (head == item)
            return true;

        T* it = (T*)(head->next);
        while (it != head) {
            if (it == item)
                return true;

            it = (T*)(it->next);
        }

        return false;
    }

    bool remove(T* item) {
        if (!has(item))
            return false;

        item->prev->next = item->next;
        item->next->prev = item->prev;

        if (head == item)
            head = (T*)(item->next);

        if (head == item)
            head = nullptr;

        return true;
    }

    T* after(T* item) {
        return (T*)(item->next);
    }

    T* before(T* item) {
        return (T*)(item->prev);
    }

    class iterator {
    private:
        iterator(bool first, T* item)
            : first(first), item(item) {}

    public:
        T& operator*() { return *item; }
        T* operator->() { return item; }

        iterator& operator++() {
            first = false;
            if (item)
                item = (T*)item->next;
            return *this;
        }

        iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const iterator& a, const iterator& b) {
            return a.first == b.first && a.item == b.item;
        }

        friend bool operator!=(const iterator& a, const iterator& b) {
            return !(a == b);
        }

    public:
        bool first;
        T* item;

        friend class DLChain;
    
    };

    iterator begin() {
        return iterator(true, head);
    }

    iterator end() {
        if (head != nullptr)
            return iterator(false, head);
        return iterator(true, nullptr);
    }

public:
    T* head;

};