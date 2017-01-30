#ifndef BSTREE_H_
#define BSTREE_H_

#include <utility>

namespace fsa
{
    template<class T>
    class bstree
    {
    protected:
        struct bst_node
        {
            T data;
            bst_node * father;
            bst_node *left, *right;

            bst_node() {}

            // Copy constructor. Copy the whole subtree, leave father with null.
            bst_node(const bst_node & other, bst_node * const pf = nullptr) : data(other.data), father(pf)
            {
                if(other.left) { left = new bst_node(*other.left, this); }
                if(other.right) { right = new bst_node(*other.right, this); }
            }

            // Move constructor. Move the whole subtree.
            bst_node(bst_node&& other, const bst_node * pf = nullptr)
                : data(std::move(other.data)), father(pf),
                  left(other.left), right(other.right)
            {
                left->father = right->father = this;
                // Clear moved value.
                other.left = other.right = nullptr;
            }

            bst_node(T _val, bst_node * _pf = nullptr) : data(_val), father(_pf) {}
            ~bst_node()
            {
                delete left;
                delete right;
            }
        };

        struct bst_iterator
        {
            bst_node * _ptr;

            bst_iterator(bst_node * _Ptr = nullptr) : _ptr(_Ptr) {}
            bst_iterator(const bst_iterator & _it) : _ptr(_it._ptr) {}

            T & operator*()
            {
                return *_ptr->data;
            }

            bst_node * operator->()
            {
                return _ptr;
            }

            bst_iterator & operator++()
            {
                // If `right` is not empty, find the smallest node in the subtree.
                if ((*this)->right != nullptr)
                {
                    this->ptr = (*this)->right;
                    while ((*this)->left != nullptr)
                    {
                        this->ptr = (*this)->left;
                    }
                }
                else
                {
                    // Find the first `ancestor` which is not smaller than `this`.
                    bst_node * prev = this->ptr;
                    this->ptr = (*this)->father;
                    while (prev == (*this)->right)
                    {
                        prev = this->ptr;
                        this->ptr = (*this)->father;
                    }
                }
                return *this;
            }

            bst_iterator operator++(int)
            {
                bst_iterator tmp_iter = *this;
                *(this)++;
                return tmp_iter;
            }

            bst_iterator & operator--()
            {
                // If `left` is not empty, find the largest node in the subtree.
                if ((*this)->left != nullptr)
                {
                    this->ptr = (*this)->left;
                    while ((*this)->right != nullptr)
                    {
                        this->ptr = (*this)->right;
                    }
                }
                else
                {
                    // Find the first `ancestor` which is not greater than `this`.
                    bst_node * prev = this->ptr;
                    this->ptr = (*this)->father;
                    while (prev == (*this)->left)
                    {
                        prev = this->ptr;
                        this->ptr = (*this)->father;
                    }
                }
                return *this;
            }

            bst_iterator operator--(int)
            {
                bst_iterator tmp_iter = *this;
                *(this)--;
                return tmp_iter;
            }

            bool operator==(bst_iterator & _it)
            {
                return _ptr == _it._ptr;
            }

            bool operator!=(bst_iterator & _it)
            {
                return _ptr != _it._ptr;
            }
        };

    public:
        typedef T             value_type;
        typedef T &&          reference;
        typedef bst_node      node_type;
        typedef bst_node *    pointer;
        typedef bst_iterator  iterator;
        typedef unsigned int  size_type;

    private:
        pointer _header_ptr;
        size_type _size;

    protected:
        pointer & _root() const { return _header_ptr->father; }
        pointer & _left_most() { return _header_ptr->right; }
        pointer &  _right_most() { return _header_ptr->left; }

    public:
        // Con-/De-structors and `operator=`.
        bstree() : _header_ptr(new node_type()), _size(0)
        { _header_ptr->father = _header_ptr->left = _header_ptr->right = _header_ptr; }

        explicit bstree(value_type & _val) : _header_ptr(nullptr), _size(1)
        {
            _root() = new node_type(_val, _header_ptr->_ptr);
            _left_most() = _header_ptr->right = _root();
        }

        virtual ~bstree();
        bstree & operator=(const bstree & _opr);
        bstree & operator=(bstree&& _opr);

        // Capasity and element access.
        size_type size() const { return _size; }
        bool empty() const { return (_root() == _header_ptr); }
        iterator begin() { return iterator(_left_most()); }
        iterator end() { return iterator(_header_ptr); }
        reference front() const { return _left_most()->data; }
        reference back() const { return _right_most()->data; }
        iterator find(reference _val);

        // Modifiers.
        void clear();
        void insert(reference _val);
        void erase(iterator _pos);
        void remove(reference _val);
        void swap(bstree & _opr);
        void splice(bstree & _opr);
    };

    template<class T>
    inline void bstree<T>::clear()
    {
        if (!empty())
        {
            delete _root();
            _root() = _header_ptr;
        }
    }

    template<class T>
    inline bstree<T>::~bstree()
    {
        clear();
        // Reset hanging pointer.
        _header_ptr->left = _header_ptr->right = nullptr;
        delete _header_ptr;
    }

    template<class T>
    bstree<T> & bstree<T>::operator=(const bstree<T> & _opr)
    {
        // We do this only if they are not the same.
        if(this != &_opr)
        {
            clear();
            if(!_opr.empty())
            {
                _root() = new node_type(*_opr._root(), _header_ptr);
            }
        }
        return *this;
    }

    template<class T>
    typename bstree<T>::iterator bstree<T>::find(bstree<T>::reference _val)
    {
        pointer _ptr = _root();

        while (_ptr != nullptr)
        {
            if (_val == _ptr->data) // Value _val found, break.
            {
                break;
            }
            else
            {
                _ptr = (_val < _ptr->data)
                    ? _ptr->left
                    : _ptr->right;
            }
        }

        if (_ptr == nullptr) // If value _val is not found, return end().
        {
            return end();
        }
        else
        {
            return iterator(_ptr);
        }
    }

    template<class T>
    void bstree<T>::insert(bstree<T>::reference _val)
    {
        if (empty())
        {
            // Create the first node, i.e. root.
            _header_ptr->father = new node_type(_val, _header_ptr);
            _header_ptr->left = _header_ptr->right = _header_ptr->father;
        }
        else
        {
            pointer _ptr = _header_ptr->father;
            pointer _prev = _ptr;

            while (_ptr != nullptr) // Search for value _val.
            {
                if (_val == _ptr->data) // If the value already exists, do nothing.
                {
                    return;
                }
                else
                {
                    _prev = _ptr;
                    _ptr = (_val < _ptr->data)
                        ? _ptr->left
                        : _ptr->right;
                }
            }

            // Create new node.
            if (_val < _prev->data)
            {
                _prev->left = new node_type(_val, _prev);
            }
            else
            {
                _prev->right = new node_type(_val, _prev);
            }

            // Adjust `left_most` and `right_most`.
            if (_header_ptr->left->left != nullptr)
            {
                _header_ptr->left = _header_ptr->left->left;
            }
            if (_header_ptr->right->right != nullptr)
            {
                _header_ptr->right = _header_ptr->right->right;
            }
        }
    }

}

#endif // BSTREE_H_
