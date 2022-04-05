#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>

// Effective std::set analogue based on AVL tree.
template<class T>
class Set {
  private:
    class Node;

  public:
    class iterator {
      public:
        iterator() : node_(nullptr) {}

        explicit iterator(const Node* node) : node_(node) {}

        iterator& operator++() {
            node_ = node_->next;
            return *this;
        }

        iterator& operator--() {
            node_ = node_->prev;
            return *this;
        }

        iterator operator++(int) {
            iterator copied(node_);
            node_ = node_->next;
            return copied;
        }

        iterator operator--(int) {
            iterator copied(node_);
            node_ = node_->prev;
            return copied;
        }

        const T& operator*() const { return node_->value; }

        const T* operator->() const { return &node_->value; }

        bool operator!=(const iterator it) const { return node_ != it.node_; }

        bool operator==(const iterator it) const { return node_ == it.node_; }

      private:
        const Node* node_;
    };

  public:
    Set() {
        size_ = 0;
        root_ = min_ = max_ = nullptr;
        imaginary_ = new Node();  // An auxiliary node to maintain valid end().
    }

    Set(const Set& other) {
        root_ = deep_copy(other.root_);
        size_ = other.size_;
        std::vector<Node*> temp;
        fix_prev_next(root_, temp);
        for (size_t i = 1; i < size_; ++i) {
            temp[i - 1]->next = temp[i];
            temp[i]->prev = temp[i - 1];
        }
        temp.clear();
        min_ = get_min(root_);
        max_ = get_max(root_);
        imaginary_ = new Node();
        imaginary_->prev = max_;
        if (max_ != nullptr) {
            max_->next = imaginary_;
        }
    }

    Set& operator=(const Set& other) {
        if (&other == this) {
            return *this;
        }
        del(root_);
        delete imaginary_;
        root_ = deep_copy(other.root_);
        size_ = other.size_;
        std::vector<Node*> temp;
        fix_prev_next(root_, temp);
        for (size_t i = 1; i < size_; ++i) {
            temp[i - 1]->next = temp[i];
            temp[i]->prev = temp[i - 1];
        }
        temp.clear();
        min_ = get_min(root_);
        max_ = get_max(root_);
        imaginary_ = new Node();
        imaginary_->prev = max_;
        if (max_ != nullptr) {
            max_->next = imaginary_;
        }
        return *this;
    }

    template<typename Iterator>
    Set(Iterator first, Iterator last) {
        size_ = 0;
        imaginary_ = new Node();
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    Set(std::initializer_list<T>& elems) {
        size_ = 0;
        imaginary_ = new Node();
        for (auto elem : elems) {
            insert(elem);
        }
    }

    Set(std::initializer_list<T> elems) {
        size_ = 0;
        imaginary_ = new Node();
        for (auto elem : elems) {
            insert(elem);
        }
    }

    // Returns the size of the set.
    size_t size() const { return size_; }

    // Checks whether the set is empty.
    bool empty() const { return size_ == 0; }

    // Inserts an element if it not already in the set.
    void insert(const T& elem) {
        bool ok = false;
        root_ = insert(root_, elem, ok);
    }

    // Erases an element from the set.
    void erase(const T& elem) { root_ = erase(root_, elem); }

    // Returns an iterator for the first element of the set.
    iterator begin() const {
        if (min_ == nullptr) {
            return end();
        }
        return iterator(min_);
    }

    // Returns an iterator that points after the last element of the set.
    iterator end() const { return iterator(imaginary_); }

    // Returns an iterator that points at the elem or end() if there is no such.
    iterator find(const T& elem) const { return find(root_, elem); }

    // Returns an iterator that points at the first element >= elem or end() if there is no such.
    iterator lower_bound(const T& elem) const { return lower_bound(root_, elem); }

    // Returns the depth of the tree.
    size_t get_depth() const { return get_h(root_); }

    ~Set() {
        del(root_);
        delete imaginary_;
    }

  private:
    class Node {
      public:
        Node() : value() {
            left = right = nullptr;
            next = prev = nullptr;
            height = 0;
        }

        explicit Node(const T& elem) : value(elem) {
            left = right = nullptr;
            next = prev = nullptr;
            height = 1;
        }

        T value;
        Node *left = nullptr, *right = nullptr;
        Node *next = nullptr, *prev = nullptr;
        size_t height = 0;
    };

  private:
    // Deletes a node v from the set.
    void del(Node* v) {
        if (v == nullptr) {
            return;
        }
        del(v->left);
        del(v->right);
        delete v;
    }

    // Deep copies the tree with root a and returns the new root.
    Node* deep_copy(Node* a) {
        if (a == nullptr) {
            return nullptr;
        }
        Node* v = new Node(a->value);
        v->left = deep_copy(a->left);
        v->right = deep_copy(a->right);
        return v;
    }

    // Fills the vector temp with valid prev and next values.
    void fix_prev_next(Node* v, std::vector<Node*>& temp) {
        if (v == nullptr) {
            return;
        }
        fix_prev_next(v->left, temp);
        temp.emplace_back(v);
        fix_prev_next(v->right, temp);
    }

    // Inserts a node with value elem to the tree with root v.
    Node* insert(Node* v, const T& elem, bool& ok) {
        if (v == nullptr) {
            v = new Node(elem);
            v->next = imaginary_;
            if (min_ == nullptr || elem < min_->value) {
                min_ = v;
            }
            if (max_ == nullptr || max_->value < elem) {
                max_ = v;
                imaginary_->prev = max_;
            }
            ++size_;
            ok = true;
            return v;
        }
        if (elem < v->value) {
            v->left = insert(v->left, elem, ok);
            if (ok) {
                v->left->next = v;
                if (v->prev != nullptr) {
                    v->left->prev = v->prev;
                    v->prev->next = v->left;
                }
                v->prev = v->left;
                ok = false;
            }
        }
        if (v->value < elem) {
            v->right = insert(v->right, elem, ok);
            if (ok) {
                v->right->prev = v;
                if (v->next != nullptr) {
                    v->right->next = v->next;
                    v->next->prev = v->right;
                }
                v->next = v->right;
                ok = false;
            }
        }
        return balance(v);
    }

    // Erases a node with value elem from the tree with root v.
    Node* erase(Node* v, const T& elem) {
        if (v == nullptr) {
            return nullptr;
        }
        if (elem < v->value) {
            v->left = erase(v->left, elem);
        } else if (v->value < elem) {
            v->right = erase(v->right, elem);
        } else {
            if (v == min_) {
                min_ = v->next;
            }
            if (v == max_) {
                max_ = v->prev;
                imaginary_->prev = max_;
            }

            Node* A = v->prev;
            Node* B = v->next;
            if (A != nullptr) {
                A->next = B;
            }
            if (B != nullptr) {
                B->prev = A;
            }
            A = v->left;
            B = v->right;
            delete v;
            --size_;
            if (B == nullptr) {
                return A;
            }
            Node* leftest = get_min(B);
            leftest->right = no_min(B);
            leftest->left = A;
            return balance(leftest);
        }
        return balance(v);
    }

    // Finds the node with minimal value in the tree.
    Node* get_min(Node* v) {
        if (v == nullptr || v->left == nullptr) {
            return v;
        } else {
            return get_min(v->left);
        }
    }

    // Find the node with maximal value in the tree.
    Node* get_max(Node* v) {
        if (v == nullptr || v->right == nullptr) {
            return v;
        } else {
            return get_max(v->right);
        }
    }

    // Returns a tree with erased min node (needed for erase method).
    Node* no_min(Node* v) {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->left == nullptr) {
            return v->right;
        }
        v->left = no_min(v->left);
        return balance(v);
    }

    // Find method implementation.
    iterator find(Node* v, const T& elem) const {
        if (v == nullptr) {
            return end();
        }
        if (elem < v->value) {
            return find(v->left, elem);
        }
        if (v->value < elem) {
            return find(v->right, elem);
        }
        return iterator(v);
    }

    // Lower bound method implementation.
    iterator lower_bound(Node* v, const T& elem) const {
        if (v == nullptr) {
            return end();
        }
        if (v->value < elem) {
            return lower_bound(v->right, elem);
        }
        auto it = lower_bound(v->left, elem);
        if (it != end()) {
            return it;
        }
        if (!(v->value < elem)) {
            return iterator(v);
        }
        return end();
    }

    // Auxiliary method for balancing the AVL tree.
    Node* small_left_rotate(Node* a) {
        Node* b = a->right;
        a->right = b->left;
        b->left = a;
        update(a);
        update(b);
        return b;
    }

    // Auxiliary method for balancing the AVL tree.
    Node* small_right_rotate(Node* a) {
        Node* b = a->left;
        a->left = b->right;
        b->right = a;
        update(a);
        update(b);
        return b;
    }

    // Auxiliary method for balancing the AVL tree.
    Node* big_left_rotate(Node* a) {
        a->right = small_right_rotate(a->right);
        return small_left_rotate(a);
    }

    // Auxiliary method for balancing the AVL tree.
    Node* big_right_rotate(Node* a) {
        a->left = small_left_rotate(a->left);
        return small_right_rotate(a);
    }

    // Balancing the AVL tree.
    Node* balance(Node* v) {
        update(v);
        if (diff(v) == -2) {
            if (diff(v->right) > 0) {
                return big_left_rotate(v);
            } else {
                return small_left_rotate(v);
            }
        }
        if (diff(v) == 2) {
            if (diff(v->left) < 0) {
                return big_right_rotate(v);
            } else {
                return small_right_rotate(v);
            }
        }
        return v;
    }

    // Returns the height of node v.
    size_t get_h(Node* v) const {
        if (v == nullptr) {
            return 0;
        } else {
            return v->height;
        }
    }

    // Returns the height difference for children of node v that is needed for balancing the tree.
    int diff(Node* v) const {
        if (v == nullptr) {
            return 0;
        }
        return get_h(v->left) - get_h(v->right);
    }

    // Updates the height value for node v.
    void update(Node* v) {
        if (v == nullptr) {
            return;
        }
        int left_h = get_h(v->left);
        int right_h = get_h(v->right);
        v->height = std::max(left_h, right_h) + 1;
    }

    size_t size_ = 0;
    Node* root_ = nullptr;
    Node* min_ = nullptr;
    Node* max_ = nullptr;
    Node* imaginary_ = nullptr;
};
