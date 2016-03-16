#ifndef MAP_HEADER
#define MAP_HEADER

#define NULL 0

template<class TKey, class TData>//, TData defaultval = 0>
struct TreeNode
{
	TreeNode* left;
	TreeNode* right;
	TreeNode* parent;

	TKey first;
	TData second;

	char color;

	TreeNode(TKey nkey, TreeNode* p)
	{
		color = 0;

		second = TData();

		first = nkey;

		left = NULL;
		right = NULL;

		parent = p;
	};

	TreeNode(TKey nkey, TData data)
	{
		color = 0;

		first = nkey;
		second = data;

		left = NULL;
		right = NULL;
		parent = NULL;
	};
};

template<class TKey, class TData, class TMap>
class map_iterator
{
public:
	typedef TreeNode<TKey, TData> node_t;
	typedef node_t* _Nodeptr;
	//typedef _Nodeptr& _Node;
	typedef map_iterator<TKey, TData, TMap> Iterator;

	node_t* ptr;
	TMap* _mymap;

	map_iterator()
	{
		ptr = 0;
	};

	map_iterator(node_t* pos, TMap* map)
	{
		ptr = pos;
	};

	inline void prot_inc()
	{
		if (ptr == 0)//nil pointer?
			;	// end() shouldn't be incremented, don't move
		else if (ptr->right != 0)
			ptr = TMap::_Min(ptr->right);	// ==> smallest of right subtree
		else
		{	// climb looking for right subtree
			node_t* _Pnode;
			while (((_Pnode = ptr->parent) != 0) && ptr == _Pnode->right)
				ptr = _Pnode;	// ==> parent while right subtree
			ptr = _Pnode;	// ==> parent (head if end())
		}
	};

	inline void prot_dec()
	{
		if (ptr == 0)
			ptr = ptr->right;	// end() ==> rightmost
		else if (ptr->left != 0)
			ptr = TMap::_Max(ptr->left);	// ==> largest of left subtree
		else
		{	// climb looking for left subtree
			node_t* _Pnode;
			while ((_Pnode = ptr->parent) != 0 && ptr == _Pnode->left)
				ptr = _Pnode;	// ==> parent while left subtree
			if (ptr == 0)
				;	// begin() shouldn't be decremented, don't move
			else
				ptr = _Pnode;	// ==> parent if not head
		}
	};

	Iterator& operator++()
	{
		prot_inc();
		return *this;
	};

	Iterator& operator++(int)
	{
		prot_inc();
		return *this;
	};

	Iterator& operator--()
	{
		prot_dec();
		return *this;
	};

	Iterator& operator--(int)
	{
		prot_dec();
		return *this;
	};

	bool operator==(const Iterator& r)
	{
		return this->ptr == r.ptr;
	}

	bool operator!=(const Iterator& r)
	{
		return this->ptr != r.ptr;
	}

	node_t* operator->() const
	{	// return pointer to class object
		return this->ptr;//this does pointer magic and gives a reference to the pair containing the key and value
	}

	TData operator*()
	{
		if (this->ptr)
			return this->ptr->second;//item;
		
		return 0;
	}
};

template<class TKey, class TData>
class MyMap
{
	TreeNode<TKey, TData>* root;

	int _size;

public:
	typedef map_iterator<TKey, TData, MyMap<TKey, TData> > iterator;
	typedef TreeNode<TKey, TData> node_t;

	MyMap()
	{
		_size = 0;

		root = NULL;
		_tParent = NULL;
	};

	~MyMap()
	{
		//delete all tree nodes
		this->clear();
	};

	TData& operator[](TKey id)
	{
		node_t* node = this->_treeFind(this->root, id);
		if (node)
			return node->second;
		else//no value, add it
		{
			this->_treeInsert(this->root, id);//inefficient to add then search again, but ok for now
			return this->_treeFind(this->root, id)->second;
		}
		//if no node, has wierd issues
	}

	bool _treeContains( node_t*& Root, TKey k ) {
		// Return true if item is one of the items in the binary
		// sort tree to which root points.   Return false if not.
		if ( root == NULL ) {
			// Tree is empty, so it certainly doesn't contain item.
			return false;
		}
		else if ( k == Root->key ) {
			// Yes, the item has been found in the root node.
			return true;
		}
		else if ( k < Root->key) {
			// If the item occurs, it must be in the left subtree.
			return _treeContains( Root->left, k );
		}
		else {
			// If the item occurs, it must be in the right subtree.
			return _treeContains( Root->right, k );
		}
	}

	node_t* _treeFindRecursive( node_t*& Root, TKey k ) {
		// Return true if item is one of the items in the binary
		// sort tree to which root points.   Return false if not.
		if ( Root == NULL ) {
			// Tree is empty, so it certainly doesn't contain item.
			return NULL;
		}
		else if ( k == Root->first ) {
			// Yes, the item has been found in the root node.
			return Root;
		}
		else if ( k < Root->first) {
			// If the item occurs, it must be in the left subtree.
			return _treeFind( Root->left, k );
		}
		else {
			// If the item occurs, it must be in the right subtree.
			return _treeFind( Root->right, k );
		}
	}

	node_t* _treeFind( node_t*& Root, TKey k)
	{
		node_t* temp = Root;
		while(true)
		{
			// Return true if item is one of the items in the binary
			// sort tree to which root points.   Return false if not.
			if ( temp == NULL ) {
				// Tree is empty, so it certainly doesn't contain item.
				return NULL;
			}
			else if ( k == temp->first ) {
				// Yes, the item has been found in the root node.
				return temp;
			}
			else if ( k < temp->first) {
				// If the item occurs, it must be in the left subtree.
				temp = temp->left;
			}
			else {
				// If the item occurs, it must be in the right subtree.
				temp = temp->right;
			}
		}
	}

	node_t* _tParent;
	void _treeInsert(node_t*& Root, TKey k) {
		// Add the item to the binary sort tree to which the parameter
		// "root" refers.  Note that root is passed by reference since
		// its value can change in the case where the tree is empty.
		if ( Root == NULL ) {
			// The tree is empty.  Set root to point to a new node containing
			// the new item.  This becomes the only node in the tree.
			Root = new node_t(k, _tParent);
			Root->color = 'R';//color the newly inserted node red

			//increase size
			_size += 1;

			//reset the parent temporary variable
			_tParent = NULL;

			_fixup(Root);

			return;
		}
		else if ( k < Root->first ) {
			_tParent = Root;
			_treeInsert( Root->left, k );
		}
		else {
			_tParent = Root;
			_treeInsert( Root->right, k );
		}
	}

	void erase(iterator i)
	{
		if (i.ptr == 0)//dont try to erase end()
			return;

		node_t* child;
		node_t* n = i.ptr;//lookup_node(t, key, compare);
		if (n == NULL) return;  /* Key not found, do nothing */
		if (n->left != NULL && n->right != NULL) {
			/* Copy key/value from predecessor and then delete it instead */
			node_t* pred = _Max(n->left);
			n->first  = pred->first;
			n->second = pred->second;
			n = pred;
		}

		//assert(n->left == NULL || n->right == NULL);
		child = n->right == NULL ? n->left  : n->right;
		if (node_color(n) == 'B') {
			n->color = node_color(child);
			delete_case1(n);
		}
		replace_node(n, child);
		if (n->parent == NULL && child != NULL)
			child->color = 'B';
		_size--;
		delete n;
	}

	char node_color(node_t* n) {
		return n == NULL ? 'B' : n->color;
	}

	node_t* sibling(node_t* n)
	{
		if (n == n->parent->left)
			return n->parent->right;
		else
			return n->parent->left;
	}

	void replace_node(node_t* oldn, node_t* newn)
	{
		if (oldn->parent == NULL) {
			root = newn;
		}
		else
		{
			if (oldn == oldn->parent->left)
				oldn->parent->left = newn;
			else
				oldn->parent->right = newn;
		}
		if (newn != NULL)
		{
			newn->parent = oldn->parent;
		}
	}

	void delete_one_child(node_t* n)
	{
		/*
		* Precondition: n has at most one non-null child.
		*/
		node_t* child = n->right ? n->left : n->right;

		if (child)
		{
			replace_node(n, child);
			if (n->color == 'B') {
				if (child->color == 'R')
					child->color = 'B';
				else
					delete_case1(child);
			}
		}
		delete n;//free(n);
	}

	void delete_case1(node_t* n)
	{
		if (n->parent != NULL)
			delete_case2(n);
	}

	void delete_case2(node_t* n)
	{
		node_t* s = sibling(n);

		if (s->color == 'R') {
			n->parent->color = 'R';
			s->color = 'B';
			if (n == n->parent->left)
				_leftRotate(n->parent);
			else
				_rightRotate(n->parent);
		}
		delete_case3(n);
	}

	void delete_case3(node_t* n)
	{
		node_t* s = sibling(n);

		if (node_color(n->parent) == 'B' &&
			node_color(s) == 'B' &&
			node_color(s->left) == 'B' &&
			node_color(s->right) == 'B')
		{
			s->color = 'R';
			delete_case1(n->parent);
		}
		else
			delete_case4(n);
	}

	void delete_case4(node_t* n)
	{
		node_t* s = sibling(n);

		if (node_color(n->parent) == 'R' &&
			node_color(s) == 'B' &&
			node_color(s->left) == 'B' &&
			node_color(s->right) == 'B')
		{
			s->color = 'R';
			n->parent->color = 'B';
		}
		else
			delete_case5(n);
	}

	void delete_case5(node_t* n)
	{
		node_t* s = sibling(n);

		if (n == n->parent->left &&
			node_color(s) == 'B' &&
			node_color(s->left) == 'R' &&
			node_color(s->right) == 'B')
		{
			s->color = 'R';
			s->left->color = 'B';
			_rightRotate(s);
		}
		else if (n == n->parent->right &&
			node_color(s) == 'B' &&
			node_color(s->right) == 'R' &&
			node_color(s->left) == 'B')
		{
			s->color = 'R';
			s->right->color = 'B';
			_leftRotate(s);
		}
		delete_case6(n);
	}

	void delete_case6(node_t* n)
	{
		node_t* s = sibling(n);

		s->color = node_color(n->parent);
		n->parent->color = 'B';
		if (n == n->parent->left) {
			//assert (node_color(sibling(n)->right) == 'R');
			s->right->color = 'B';
			_leftRotate(n->parent);
		}
		else
		{
			//assert (node_color(sibling(n)->left) == 'R');
			s->left->color = 'B';
			_rightRotate(n->parent);
		}
	}

	void clear()
	{
		if (this->root != 0)
			this->_Erase(this->root);
		this->root = 0;
		this->_size = 0;
	}

	int size()
	{
		return this->_size;
	};

	node_t* right(node_t* n)
	{
		//return the node to the right of the one given
		return 0;
	}

	TData search(TKey k)
	{
		//if (this->root == NULL)
		//return 0;
		node_t* node = this->_treeFind(this->root, k);
		if (node)
			return node->second;//found it
		else
			return 0;
	}

	iterator find(TKey k)
	{
		//find the node of the key
		if (this->root == 0)
			return end();

		node_t* node = this->_treeFind(this->root, k);
		if (node)
			return iterator(node, this);//found it
		else
			return end();//couldnt find it, return a null iterator
	}

	iterator begin()
	{
		//return an interator to the first node (leftmost node)
		if (this->root != 0)
			return iterator(_Min(this->root), this);
		else
			return end();
	}

	iterator end()
	{
		//return an iterator to the last node
		return iterator(0, this);
	}

	iterator begin() const
	{
		if (this->root != 0)
			return iterator(_Min(this->root), this);
		else
			return end();
	}

	iterator end() const
	{
		return iterator(0, this);
	}

	static node_t* _Max(node_t* _Node)
	{	// return rightmost node in subtree at _Pnode
		while (_Node->right != 0)
			_Node = _Node->right;
		return _Node;
	}

	/*int _Height(node_t* node)
	{
	int count = 0;
	while (node->right != 0)
	{
	node = node->right;
	count++;
	}

	return count;
	}*/

	static node_t* _Min(node_t* _Node)
	{	// return leftmost node in subtree at _Pnode
		while (_Node->left != 0)
			_Node = _Node->left;
		return _Node;
	}

	void _Erase(node_t* _Rootnode)
	{	// free entire subtree, recursively
		for (node_t* _Pnode = _Rootnode; _Pnode != 0; _Rootnode = _Pnode)
		{	// free subtrees, then node
			_Erase(_Pnode->right);
			_Pnode = _Pnode->left;

			delete _Rootnode;
		}
	}

	void _leftRotate( node_t* x )
	{
		node_t* y;
		y = x->right;
		x->right = y->left;
		if(y->left != NULL)
			y->left->parent = x;
		y->parent = x->parent;

		if ( x->parent == 0 )
			root = y;
		else if ( x == x->parent->left )
			x->parent->left = y;
		else
			x->parent->right = y;
		y->left = x;
		x->parent = y;
	}

	void _rightRotate( node_t* x )
	{
		node_t* y;

		y = x->left;
		x->left = y->right;
		if(y->right != NULL)
			y->right->parent = x;
		y->parent = x->parent;

		if ( x->parent == 0 )
			root = y;
		else if ( x == x->parent->right )
			x->parent->right = y;
		else
			x->parent->left = y;
		y->right = x;
		x->parent = y;
	}

	node_t* _successor(node_t *x)
	{
		node_t *temp, *temp2;
		temp = temp2 = x->right;
		while(temp != NULL)
		{
			temp2 = temp;
			temp = temp->left;
		}
		return temp2;
	}

	node_t* _grandparent(node_t* n)
	{
		if ((n != NULL) && (n->parent != NULL))
			return n->parent->parent;
		else
			return NULL;
	}

	node_t* _uncle(node_t* n)
	{
		node_t* g = _grandparent(n);
		if (g == NULL)
			return NULL; // No grandparent means no uncle
		if (n->parent == g->left)
			return g->right;
		else
			return g->left;
	}

	void insert_case1(node_t* n)
	{
		if (n->parent == NULL)
			n->color = 'B';
		else
			insert_case2(n);
	}

	void insert_case2(node_t* n)
	{
		if (n->parent->color == 'B')
			return; /* Tree is still valid */
		else
			insert_case3(n);
	}

	void insert_case3(node_t* n)
	{
		node_t* u = _uncle(n), *g;

		if ((u != NULL) && (u->color == 'R')) {
			n->parent->color = 'B';
			u->color = 'B';
			g = _grandparent(n);
			g->color = 'R';
			insert_case1(g);
		}
		else
		{
			insert_case4(n);
		}
	}

	void insert_case4(node_t* n)
	{
		node_t* g = _grandparent(n);

		if ((n == n->parent->right) && (n->parent == g->left)) {
			_leftRotate(n->parent);//rotate_left(n->parent);
			n = n->left;
		} else if ((n == n->parent->left) && (n->parent == g->right)) {
			_rightRotate(n->parent);//rotate_right(n->parent);
			n = n->right;
		}
		insert_case5(n);
	}

	void insert_case5(node_t* n)
	{
		node_t* g = _grandparent(n);

		n->parent->color = 'B';
		g->color = 'R';
		if (n == n->parent->left)
			_rightRotate(g);//rotate_right(g);
		else
			_leftRotate(g);//rotate_left(g);
	}

	void _fixup( node_t* n )
	{
		insert_case1(n);//start the fixup line
	}
};

#endif
