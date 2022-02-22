#include <cstddef>
#include <initializer_list>
#include <algorithm>
#include <iostream>

template<class T>
class Set {

public:
    class Node;

    class iterator;

    class Node {
    public:
        Node() : val() {
            L = R = nullptr;
            next = prev = nullptr;
            h = 0;
        }

        explicit Node(const T &elem) : val(elem) {
            L = R = nullptr;
            next = prev = nullptr;
            h = 1;
        }

        T val;
        Node *L = nullptr, *R = nullptr;
        Node *next = nullptr, *prev = nullptr;
        int h = 0;
    };

    class iterator {
    public:
        iterator() : node_(nullptr) {
        }

        explicit iterator(const Node *node) : node_(node) {
        }

        iterator &operator++() {
            node_ = node_->next;
            return *this;
        }

        iterator &operator--() {
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

        const T &operator*() const {
            return node_->val;
        }

        const T *operator->() const {
            return &node_->val;
        }

        bool operator!=(const iterator it) const {
            return node_ != it.node_;
        }

        bool operator==(const iterator it) const {
            return node_ == it.node_;
        }

    private:
        const Node *node_;
    };


public:
    Set() {
        size_ = 0;
        root_ = min_ = max_ = nullptr;
        im_ = new Node();
    }

    Set(const Set &other) {
        root_ = deep_copy(other.root_);
        size_ = other.size_;
        std::vector<Node *> temp;
        fix_prev_next(root_, temp);
        for (size_t i = 1; i < size_; ++i) {
            temp[i - 1]->next = temp[i];
            temp[i]->prev = temp[i - 1];
        }
        temp.clear();
        min_ = get_min(root_);
        max_ = get_max(root_);
        im_ = new Node();
        im_->prev = max_;
        if (max_ != nullptr) {
            max_->next = im_;
        }
    }

    Set &operator=(const Set &other) {
        if (&other == this) {
            return *this;
        }
        del(root_);
        delete im_;
        root_ = deep_copy(other.root_);
        size_ = other.size_;
        std::vector<Node *> temp;
        fix_prev_next(root_, temp);
        for (size_t i = 1; i < size_; ++i) {
            temp[i - 1]->next = temp[i];
            temp[i]->prev = temp[i - 1];
        }
        temp.clear();
        min_ = get_min(root_);
        max_ = get_max(root_);
        im_ = new Node();
        im_->prev = max_;
        if (max_ != nullptr) {
            max_->next = im_;
        }
        return *this;
    }

    template<typename Iterator>
    Set(Iterator first, Iterator last) {
        size_ = 0;
        im_ = new Node();
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    Set(std::initializer_list<T> &elems) {
        size_ = 0;
        im_ = new Node();
        for (auto elem: elems) {
            insert(elem);
        }
    }

    Set(std::initializer_list<T> elems) {
        size_ = 0;
        im_ = new Node();
        for (auto elem: elems) {
            insert(elem);
        }
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void insert(const T &elem) {
        bool ok = false;
        root_ = insert(root_, elem, ok);
    }

    void erase(const T &elem) {
        root_ = erase(root_, elem);
    }

    iterator begin() const {
        if (min_ == nullptr) {
            return end();
        }
        return iterator(min_);
    }

    iterator end() const {
        return iterator(im_);
    }

    iterator find(const T &elem) const {
        return find(root_, elem);
    }

    iterator lower_bound(const T &elem) const {
        return lower_bound(root_, elem);
    }

    void print() const {
        print(root_);
        std::cout << '\n';
    }

    int get_depth() const {
        return geth(root_);
    }

    ~Set() {
        del(root_);
        delete im_;
    }

private:
    void print(Node *v) const {
        if (v == nullptr) return;
        print(v->L);
        std::cout << v->val << ' ';
        print(v->R);
    }

    void del(Node *v) {
        if (v == nullptr) {
            return;
        }
        del(v->L);
        del(v->R);
        delete v;
    }

    Node *deep_copy(Node *a) {
        if (a == nullptr) {
            return nullptr;
        }
        Node *v = new Node(a->val);
        v->L = deep_copy(a->L);
        v->R = deep_copy(a->R);
        return v;
    }

    void fix_prev_next(Node *v, std::vector<Node *> &temp) {
        if (v == nullptr) {
            return;
        }
        fix_prev_next(v->L, temp);
        temp.emplace_back(v);
        fix_prev_next(v->R, temp);
    }

    Node *insert(Node *v, const T &elem, bool &ok) {
        if (v == nullptr) {
            v = new Node(elem);
            v->next = im_;
            if (min_ == nullptr || elem < min_->val) {
                min_ = v;
            }
            if (max_ == nullptr || max_->val < elem) {
                max_ = v;
                im_->prev = max_;
            }
            ++size_;
            ok = true;
            return v;
        }
        if (elem < v->val) {
            v->L = insert(v->L, elem, ok);
            if (ok) {
                v->L->next = v;
                if (v->prev != nullptr) {
                    v->L->prev = v->prev;
                    v->prev->next = v->L;
                }
                v->prev = v->L;
                ok = false;
            }
        }
        if (v->val < elem) {
            v->R = insert(v->R, elem, ok);
            if (ok) {
                v->R->prev = v;
                if (v->next != nullptr) {
                    v->R->next = v->next;
                    v->next->prev = v->R;
                }
                v->next = v->R;
                ok = false;
            }
        }
        return balance(v);
    }

    Node *erase(Node *v, const T &elem) {
        if (v == nullptr) {
            return nullptr;
        }
        if (elem < v->val) {
            v->L = erase(v->L, elem);
        } else if (v->val < elem) {
            v->R = erase(v->R, elem);
        } else {
            if (v == min_) {
                min_ = v->next;
            }
            if (v == max_) {
                max_ = v->prev;
                im_->prev = max_;
            }
            {
                Node *A = v->prev;
                Node *B = v->next;
                if (A != nullptr) {
                    A->next = B;
                }
                if (B != nullptr) {
                    B->prev = A;
                }
            }
            Node *A = v->L;
            Node *B = v->R;
            delete v;
            --size_;
            if (B == nullptr) {
                return A;
            }
            Node *leftest = get_min(B);
            leftest->R = no_min(B);
            leftest->L = A;
            return balance(leftest);
        }
        return balance(v);
    }

    Node *get_min(Node *v) {
        if (v == nullptr || v->L == nullptr) {
            return v;
        } else {
            return get_min(v->L);
        }
    }

    Node *get_max(Node *v) {
        if (v == nullptr || v->R == nullptr) {
            return v;
        } else {
            return get_max(v->R);
        }
    }

    Node *no_min(Node *v) {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->L == nullptr) {
            return v->R;
        }
        v->L = no_min(v->L);
        return balance(v);
    }

    iterator find(Node *v, const T &elem) const {
        if (v == nullptr) {
            return end();
        }
        if (elem < v->val) {
            return find(v->L, elem);
        }
        if (v->val < elem) {
            return find(v->R, elem);
        }
        return iterator(v);
    }

    iterator lower_bound(Node *v, const T &elem) const {
        if (v == nullptr) {
            return end();
        }
        if (v->val < elem) {
            return lower_bound(v->R, elem);
        }
        auto it = lower_bound(v->L, elem);
        if (it != end()) {
            return it;
        }
        if (!(v->val < elem)) {
            return iterator(v);
        }
        return end();
    }

    Node *small_left_rotate(Node *a) {
        Node *b = a->R;
        a->R = b->L;
        b->L = a;
        update(a);
        update(b);
        return b;
    }

    Node *small_right_rotate(Node *a) {
        Node *b = a->L;
        a->L = b->R;
        b->R = a;
        update(a);
        update(b);
        return b;
    }

    Node *big_left_rotate(Node *a) {
        a->R = small_right_rotate(a->R);
        return small_left_rotate(a);

    }

    Node *big_right_rotate(Node *a) {
        a->L = small_left_rotate(a->L);
        return small_right_rotate(a);
    }

    Node *balance(Node *v) {
        update(v);
        if (diff(v) == -2) {
            if (diff(v->R) > 0) {
                return big_left_rotate(v);
            } else {
                return small_left_rotate(v);
            }
        }
        if (diff(v) == 2) {
            if (diff(v->L) < 0) {
                return big_right_rotate(v);
            } else {
                return small_right_rotate(v);
            }
        }
        return v;
    }

    int geth(Node *v) const {
        if (v == nullptr) {
            return 0;
        } else {
            return v->h;
        }
    }

    int diff(Node *v) const {
        if (v == nullptr) {
            return 0;
        }
        return geth(v->L) - geth(v->R);
    }

    void update(Node *v) {
        if (v == nullptr) {
            return;
        }
        int Lh = geth(v->L);
        int Rh = geth(v->R);
        v->h = std::max(Lh, Rh) + 1;
    }

    size_t size_ = 0;
    Node *root_ = nullptr;
    Node *min_ = nullptr, *max_ = nullptr;
    Node *im_ = nullptr;
};
