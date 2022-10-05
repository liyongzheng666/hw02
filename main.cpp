/* 基于智能指针实现双向链表 */
#include <cstddef>
#include <cstdio>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T>
struct Node {
  // 这两个指针会造成什么问题？
  // bug: 相邻的两个节点next和prev会出现小彭老师讲过的循环引用的情况
  //     互相引用导致引用计数无法清零
  //     从而造成内存泄漏的情况
  // TODO: 这里需要更换为 Unique_ptr 和 普通指针
  std::unique_ptr<Node> next;
  Node *prev;
  // 如果能改成 unique_ptr 就更好了!

  // TODO: 改为模板的形式
  T value;
  //构造的时候直接把值传入，避免赋值
  Node(T const &val) : value(val), prev(nullptr) {}
  //拷贝构造函数以及移动赋值函数
  Node(const Node &) = default;
  Node &operator=(Node &&) = default;

  void erase() {
    //如果当下节点的下一个节点存在，我们应当让下一个节点的上一个节点变成
    //当下节点的上一个节点
    if (next) next->prev = prev;
    //当下节点的上一个节点的next节点改为当下节点的next节点，更改所有权move!
    if (prev) prev->next = std::move(next);
  }

  ~Node() {
    printf("~Node()\n");  // 应输出多少次？为什么少了？
  }
};

template <typename T>
struct List {
  //下面是自己定义的迭代器！
  class iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using value_type = Node<T>;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(pointer ptr) : m_ptr(ptr) {}
    reference operator*() { return *m_ptr; }
    pointer operator->() { return m_ptr; }

    //前缀增量
    iterator &operator++() {
      m_ptr = m_ptr->next.get();
      return *this;
    }

    //后缀增量
    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    friend bool operator==(const iterator &a, const iterator &b) {
      return a.m_ptr == b.m_ptr;
    };
    friend bool operator!=(const iterator &a, const iterator &b) {
      return a.m_ptr != b.m_ptr;
    };

   private:
    Node<T> *m_ptr;
  };
  std::unique_ptr<Node<T>> head;
  Node<T> *back = nullptr;
  List() = default;

  List(List const &other_) {
    printf("List 被拷贝！\n");
    List &other = const_cast<List &>(other_);
    printf("正在拷贝！");
    // todo: 这里进行的是浅拷贝向深拷贝的转换！
    for (auto it = other.begin(); it != other.end(); it++) {
      push_back(it->value);
    }
    // 请实现拷贝构造函数为 **深拷贝**
  }

  // FIXME: 因为我们可以用移动赋值来进行代替
  List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

  List(List &&) = default;
  List &operator=(List &&) = default;

  Node<T> *front() const { return head.get(); }

  T pop_front() {
    if (begin() == end()) throw std::out_of_range("pop_front()!");
    T ret = head->value;
    if (head.get() == back) back = nullptr;
    head = std::move(head->next);
    return ret;
  }

  void push_front(const T &value) {
    auto node = std::make_unique<Node<T>>(value);
    if (head)
      head->prev = node.get();
    else
      back = node.get();
    node->next = std::move(head);
    head = std::move(node);
  }
  void push_back(const T &value) {
    auto node = std::make_unique<Node<T>>(value);
    if (back) {
      node->prev = back;
      back->next = std::move(node);
      back = back->next.get();
    } else {
      head = std::move(node);
      back = head.get();
    }
  }

  Node<T> *at(size_t index) const {
    auto curr = front();
    for (size_t i = 0; i < index; i++) {
      curr = curr->next.get();
    }
    return curr;
  }

  iterator begin() { return iterator(head.get()); }
  iterator end() { return iterator(nullptr); }

  iterator begin() const { return iterator(head.get()); }
  iterator end() const { return iterator(nullptr); }
};

void print(List<int> const &lst) {  // 有什么值得改进的？
  printf("[");
  for (auto &v : lst) {
    printf(" %d", v.value);
  }
  printf(" ]\n");
}

int main() {
  List<int> a;

  a.push_front(7);
  a.push_front(5);
  a.push_front(8);
  a.push_front(2);
  a.push_front(9);
  a.push_front(4);
  a.push_front(1);

  print(a);  // [ 1 4 9 2 8 5 7 ]

  a.at(2)->erase();

  print(a);  // [ 1 4 2 8 5 7 ]

  List<int> b = a;

  a.at(3)->erase();

  print(a);  // [ 1 4 2 5 7 ]
  print(b);  // [ 1 4 2 8 5 7 ]

  b = {};
  a = {};

  return 0;
}
