
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"

namespace sjtu {

template<
    class Key,
    class T,
    class Compare = std::less<Key>
> class map {
public:
    typedef pair<const Key, T> value_type;

private:
    enum Color { RED, BLACK };

    struct Node {
        value_type *value;
        Node *left, *right, *parent;
        Color color;

        Node(const value_type &v, Color c = RED)
            : value(new value_type(v)), left(nullptr), right(nullptr), parent(nullptr), color(c) {}
        Node(value_type &&v, Color c = RED)
            : value(new value_type(std::move(v))), left(nullptr), right(nullptr), parent(nullptr), color(c) {}
        
        // For header node
        Node() : value(nullptr), left(nullptr), right(nullptr), parent(nullptr), color(BLACK) {}

        ~Node() {
            if (value) delete value;
        }
    };

    Node *header;
    size_t _size;
    Compare compare;

    Node*& root() const { return header->parent; }
    Node*& leftmost() const { return header->left; }
    Node*& rightmost() const { return header->right; }

    Node* copyTree(Node *other_node, Node *p) {
        if (!other_node) return nullptr;
        Node *new_node = new Node(*(other_node->value), other_node->color);
        new_node->parent = p;
        try {
            new_node->left = copyTree(other_node->left, new_node);
            new_node->right = copyTree(other_node->right, new_node);
        } catch (...) {
            delete new_node;
            throw;
        }
        return new_node;
    }

    void deleteTree(Node *node) {
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

    void rotateLeft(Node *x) {
        Node *y = x->right;
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->parent = x->parent;
        if (x == root()) root() = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void rotateRight(Node *y) {
        Node *x = y->left;
        y->left = x->right;
        if (x->right) x->right->parent = y;
        x->parent = y->parent;
        if (y == root()) root() = x;
        else if (y == y->parent->right) y->parent->right = x;
        else y->parent->left = x;
        x->right = y;
        y->parent = x;
    }

    void insertFixup(Node *z) {
        while (z->parent && z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node *y = z->parent->parent->right;
                if (y && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotateLeft(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotateRight(z->parent->parent);
                }
            } else {
                Node *y = z->parent->parent->left;
                if (y && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotateRight(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotateLeft(z->parent->parent);
                }
            }
        }
        root()->color = BLACK;
    }

    void eraseFixup(Node *x, Node *x_parent) {
        while (x != root() && (!x || x->color == BLACK)) {
            if (x == x_parent->left) {
                Node *w = x_parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x_parent->color = RED;
                    rotateLeft(x_parent);
                    w = x_parent->right;
                }
                if ((!w->left || w->left->color == BLACK) && (!w->right || w->right->color == BLACK)) {
                    w->color = RED;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (!w->right || w->right->color == BLACK) {
                        if (w->left) w->left->color = BLACK;
                        w->color = RED;
                        rotateRight(w);
                        w = x_parent->right;
                    }
                    w->color = x_parent->color;
                    x_parent->color = BLACK;
                    if (w->right) w->right->color = BLACK;
                    rotateLeft(x_parent);
                    x = root();
                    break;
                }
            } else {
                Node *w = x_parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x_parent->color = RED;
                    rotateRight(x_parent);
                    w = x_parent->left;
                }
                if ((!w->right || w->right->color == BLACK) && (!w->left || w->left->color == BLACK)) {
                    w->color = RED;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (!w->left || w->left->color == BLACK) {
                        if (w->right) w->right->color = BLACK;
                        w->color = RED;
                        rotateLeft(w);
                        w = x_parent->left;
                    }
                    w->color = x_parent->color;
                    x_parent->color = BLACK;
                    if (w->left) w->left->color = BLACK;
                    rotateRight(x_parent);
                    x = root();
                    break;
                }
            }
        }
        if (x) x->color = BLACK;
    }

    Node* findNode(const Key &key) const {
        Node *curr = root();
        while (curr) {
            if (compare(key, curr->value->first)) curr = curr->left;
            else if (compare(curr->value->first, key)) curr = curr->right;
            else return curr;
        }
        return nullptr;
    }

public:
    class const_iterator;
    class iterator {
        friend class map;
    private:
        Node *node;
        const map *m;
    public:
        iterator() : node(nullptr), m(nullptr) {}
        iterator(Node *n, const map *mp) : node(n), m(mp) {}
        iterator(const iterator &other) : node(other.node), m(other.m) {}

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator & operator++() {
            if (node == m->header) return *this;
            if (node->right) {
                node = node->right;
                while (node->left) node = node->left;
            } else {
                Node *p = node->parent;
                while (p && node == p->right) {
                    node = p;
                    p = p->parent;
                }
                node = p ? p : m->header;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        iterator & operator--() {
            if (node == m->header) {
                node = m->rightmost();
            } else if (node->left) {
                node = node->left;
                while (node->right) node = node->right;
            } else {
                Node *p = node->parent;
                while (p && node == p->left) {
                    node = p;
                    p = p->parent;
                }
                node = p; // if p is null, it's undefined behavior or we handle it
            }
            return *this;
        }
        value_type & operator*() const { return *(node->value); }
        bool operator==(const iterator &rhs) const { return node == rhs.node && m == rhs.m; }
        bool operator==(const const_iterator &rhs) const { return node == rhs.node && m == rhs.m; }
        bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
        value_type * operator->() const { return node->value; }
    };

    class const_iterator {
        friend class map;
    private:
        const Node *node;
        const map *m;
    public:
        const_iterator() : node(nullptr), m(nullptr) {}
        const_iterator(const Node *n, const map *mp) : node(n), m(mp) {}
        const_iterator(const iterator &other) : node(other.node), m(other.m) {}

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        const_iterator & operator++() {
            if (node == m->header) return *this;
            if (node->right) {
                node = node->right;
                while (node->left) node = node->left;
            } else {
                const Node *p = node->parent;
                while (p && node == p->right) {
                    node = p;
                    p = p->parent;
                }
                node = p ? p : m->header;
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }
        const_iterator & operator--() {
            if (node == m->header) {
                node = m->rightmost();
            } else if (node->left) {
                node = node->left;
                while (node->right) node = node->right;
            } else {
                const Node *p = node->parent;
                while (p && node == p->left) {
                    node = p;
                    p = p->parent;
                }
                node = p;
            }
            return *this;
        }
        const value_type & operator*() const { return *(node->value); }
        bool operator==(const iterator &rhs) const { return node == rhs.node && m == rhs.m; }
        bool operator==(const const_iterator &rhs) const { return node == rhs.node && m == rhs.m; }
        bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
        const value_type * operator->() const { return node->value; }
    };

    map() : _size(0) {
        header = new Node();
        header->left = header->right = header;
        header->parent = nullptr;
    }

    map(const map &other) : _size(other._size), compare(other.compare) {
        header = new Node();
        header->parent = copyTree(other.root(), nullptr);
        if (root()) {
            Node *curr = root();
            while (curr->left) curr = curr->left;
            leftmost() = curr;
            curr = root();
            while (curr->right) curr = curr->right;
            rightmost() = curr;
        } else {
            leftmost() = rightmost() = header;
        }
    }

    map & operator=(const map &other) {
        if (this == &other) return *this;
        clear();
        _size = other._size;
        compare = other.compare;
        root() = copyTree(other.root(), nullptr);
        if (root()) {
            Node *curr = root();
            while (curr->left) curr = curr->left;
            leftmost() = curr;
            curr = root();
            while (curr->right) curr = curr->right;
            rightmost() = curr;
        } else {
            leftmost() = rightmost() = header;
        }
        return *this;
    }

    ~map() {
        clear();
        delete header;
    }

    T & at(const Key &key) {
        Node *n = findNode(key);
        if (!n) throw "index out of bound";
        return n->value->second;
    }
    const T & at(const Key &key) const {
        Node *n = findNode(key);
        if (!n) throw "index out of bound";
        return n->value->second;
    }

    T & operator[](const Key &key) {
        Node *curr = root(), *p = nullptr;
        while (curr) {
            p = curr;
            if (compare(key, curr->value->first)) curr = curr->left;
            else if (compare(curr->value->first, key)) curr = curr->right;
            else return curr->value->second;
        }
        // Not found, insert
        Node *z = new Node(value_type(key, T()));
        z->parent = p;
        if (!p) {
            root() = z;
            leftmost() = rightmost() = z;
        } else {
            if (compare(key, p->value->first)) {
                p->left = z;
                if (p == leftmost()) leftmost() = z;
            } else {
                p->right = z;
                if (p == rightmost()) rightmost() = z;
            }
        }
        _size++;
        insertFixup(z);
        return z->value->second;
    }

    const T & operator[](const Key &key) const {
        Node *n = findNode(key);
        if (!n) throw "index out of bound";
        return n->value->second;
    }

    iterator begin() { return iterator(leftmost(), this); }
    const_iterator cbegin() const { return const_iterator(leftmost(), this); }
    iterator end() { return iterator(header, this); }
    const_iterator cend() const { return const_iterator(header, this); }

    bool empty() const { return _size == 0; }
    size_t size() const { return _size; }

    void clear() {
        deleteTree(root());
        root() = nullptr;
        leftmost() = rightmost() = header;
        _size = 0;
    }

    pair<iterator, bool> insert(const value_type &value) {
        Node *curr = root(), *p = nullptr;
        while (curr) {
            p = curr;
            if (compare(value.first, curr->value->first)) curr = curr->left;
            else if (compare(curr->value->first, value.first)) curr = curr->right;
            else return {iterator(curr, this), false};
        }
        Node *z = new Node(value);
        z->parent = p;
        if (!p) {
            root() = z;
            leftmost() = rightmost() = z;
        } else {
            if (compare(value.first, p->value->first)) {
                p->left = z;
                if (p == leftmost()) leftmost() = z;
            } else {
                p->right = z;
                if (p == rightmost()) rightmost() = z;
            }
        }
        _size++;
        insertFixup(z);
        return {iterator(z, this), true};
    }

    void erase(iterator pos) {
        if (pos.m != this || pos.node == header || !pos.node) return;
        Node *z = pos.node;
        Node *y = z;
        Color y_original_color = y->color;
        Node *x, *x_parent;
        if (!z->left) {
            x = z->right;
            x_parent = z->parent;
            if (!z->parent) root() = x;
            else if (z == z->parent->left) z->parent->left = x;
            else z->parent->right = x;
            if (x) x->parent = z->parent;
        } else if (!z->right) {
            x = z->left;
            x_parent = z->parent;
            if (!z->parent) root() = x;
            else if (z == z->parent->left) z->parent->left = x;
            else z->parent->right = x;
            if (x) x->parent = z->parent;
        } else {
            y = z->right;
            while (y->left) y = y->left;
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) {
                x_parent = y;
            } else {
                x_parent = y->parent;
                y->parent->left = x;
                if (x) x->parent = y->parent;
                y->right = z->right;
                y->right->parent = y;
            }
            if (!z->parent) root() = y;
            else if (z == z->parent->left) z->parent->left = y;
            else z->parent->right = y;
            y->parent = z->parent;
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        
        if (z == leftmost()) {
            if (z->right && !z->left) {
                Node *tmp = z->right;
                while (tmp->left) tmp = tmp->left;
                leftmost() = tmp;
            } else if (z->left) {
                // This case is handled by successor y
                Node *tmp = root();
                if (tmp) {
                    while (tmp->left) tmp = tmp->left;
                    leftmost() = tmp;
                } else leftmost() = header;
            } else {
                Node *p = z->parent;
                if (!p) leftmost() = header;
                else {
                    Node *tmp = root();
                    while (tmp->left) tmp = tmp->left;
                    leftmost() = tmp;
                }
            }
        }
        if (z == rightmost()) {
            if (z->left && !z->right) {
                Node *tmp = z->left;
                while (tmp->right) tmp = tmp->right;
                rightmost() = tmp;
            } else if (z->right) {
                Node *tmp = root();
                if (tmp) {
                    while (tmp->right) tmp = tmp->right;
                    rightmost() = tmp;
                } else rightmost() = header;
            } else {
                Node *p = z->parent;
                if (!p) rightmost() = header;
                else {
                    Node *tmp = root();
                    while (tmp->right) tmp = tmp->right;
                    rightmost() = tmp;
                }
            }
        }

        delete z;
        _size--;
        if (y_original_color == BLACK) eraseFixup(x, x_parent);
        if (_size == 0) leftmost() = rightmost() = header;
    }

    size_t count(const Key &key) const {
        return findNode(key) ? 1 : 0;
    }

    iterator find(const Key &key) {
        Node *n = findNode(key);
        return n ? iterator(n, this) : end();
    }
    const_iterator find(const Key &key) const {
        Node *n = findNode(key);
        return n ? const_iterator(n, this) : cend();
    }
};

}

#endif
