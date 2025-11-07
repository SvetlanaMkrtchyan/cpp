#include <iostream>

class Node{
    public:
        int data;
        Node* next;
        Node* prev;
        Node(int value): data(value), next(nullptr), prev(nullptr){} 
};

class List{
    private:
        Node* head;
        Node* tail;
        int _size;
    public:
        List(): head(nullptr), tail(nullptr), _size(0){}
        ~List(){
            while(head!=nullptr){
                Node* temp=head;
                head=head->next;
                delete temp;
                temp=nullptr;
            }
        }
        void push_front(int value) {
            Node* new_node=new Node(value);
            if (head==nullptr)
                head=tail=new_node;
            else {
                new_node->next=head;
                head->prev=new_node;
                head=new_node;
            }
            _size++;
        }

        void push_back(int value) {
            Node* new_node=new Node(value);
            if (tail==nullptr) 
                head=tail=new_node;
            else {
                new_node->prev=tail;
                tail->next=new_node;
                tail=new_node;
            }
            _size++;
        }

        void pop_front() {
            if (head==nullptr) 
                return;
            Node* temp=head;
            head=head->next;
            if (head==nullptr) 
                tail=nullptr;
            else 
                head->prev=nullptr;
            delete temp;
            temp=nullptr;
            _size--;
        }

        void pop_back() {
            if (tail==nullptr) 
                return;
            Node* temp=tail;
            tail=tail->prev;
            if (tail==nullptr) 
                head=nullptr;
            else
                tail->next=nullptr;
            delete temp;
            temp=nullptr;
            _size--;
        }

        int size() {
            return _size;
        }

        void print() {
            Node* current=head;
            while (current!=nullptr) {
                std::cout<<current->data<<" ";
                current=current->next;
            }
            std::cout<<std::endl;
        }
};

int main() {
    List ob;

    ob.push_back(10);
    ob.push_back(20);
    ob.push_front(5);
    std::cout <<"Size:"<<ob.size()<<std::endl;
    std::cout <<"List:";
    ob.print();

    ob.pop_front();
    ob.pop_back();
    std::cout<<"After pop_back() and pop_front()\n"<<"Size:"<<ob.size()<<std::endl;
    std::cout<<"List:";
    ob.print();

    return 0;
}