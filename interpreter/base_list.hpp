
#ifndef LAMINALIST
#define LAMINALIST

#include <stdexcept>

namespace __LAMINA::ListType {

	template <class VT>//value type
	class base_list {
	public:

		class base_list_node {
		public:

			VT value;
			base_list_node* next;

			base_list_node() :value(), next(nullptr) {}
			base_list_node(base_list_node* n) : value(VT()), next(n) {}
			base_list_node(const VT& val, base_list_node* n) :value(val), next(n) {}

		};

		inline base_list_node* safe_new(base_list_node* i) {
			if (i == nullptr)throw std::runtime_error("Could not make node");
			return i;
		}

		base_list_node* root;
		base_list_node* end;

		base_list() { root = safe_new(new base_list_node()); end = root; }
		base_list(const VT& val) { root = safe_new(new base_list_node(val));  end = root; }
		base_list(const base_list& val) :base_list() {
			*this = val;
		}
		base_list(const std::initializer_list<VT>& initl) :base_list() {
			for (const VT& i : initl) {
				push_back(i);
			}
		}

		inline void push_back(const VT& val) {
			end->next = safe_new(new base_list_node(val,nullptr));
			end = end->next;
		}

		inline void push_front(const VT& val) {
			base_list_node* t = safe_new(new base_list_node(val,root->next));
			root->next = t;
		}

		inline void insert(size_t index,const VT& val) {
			base_list_node* t = root;
			while (index) {
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				index--;
			}
			push_ones_back(t, val);
		}

		inline void pop_back() {
			if (root == end)throw std::runtime_error("pop from empty list");
			base_list_node* t = root;
			while (t->next == end) {
				t = t->next;
			}
			delete t->next;
			t->next = nullptr;
		}

		inline void pop_front() {
			if (root == end)throw std::runtime_error("pop from empty list");
			base_list_node* del_node = root->next;
			root->next = root->next->next;
			delete del_node;
		}

		inline void pop(size_t index) {
			if (root == end)throw std::runtime_error("pop from empty list");
			base_list_node* t = root;
			while (index) {
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				index--;
			}
			base_list_node* del_node = t->next;
			t->next = t->next->next;
			delete del_node;
		}

		inline void eraser(size_t index) { pop(index); }

		inline void operator+=(const VT& val) {
			push_back(val);
		}

		inline void operator+=(const base_list<VT>& other) {
			base_list_node* t = other.root->next;
			while (t) {
				push_back(t->value);
				t = t->next;
			}
		}

		inline base_list<VT> operator+(const VT& val) const{
			base_list<VT> t = *this;
			t.push_back(val);
			return t;
		}

		inline base_list<VT> operator+(const base_list<VT>& other) const{
			base_list<VT> t = *this;
			t += other;
			return t;
		}

		inline VT& operator[](size_t index) {
			base_list_node* t = root->next;
			while (index) {
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				index--;
			}
			return t->value;
		}

		inline void operator=(const base_list<VT>& other) {
			base_list_node* t = other.root->next;
			while (t) {
				push_back(t->value);
				t = t->next;
			}
		}


	private:

		inline void push_ones_back(base_list_node* one, const VT& val) {
			base_list_node* t = safe_new(new base_list_node(val, one->next));
			one->next = t;
		}

	};

}



#endif // !LAMINALIST
