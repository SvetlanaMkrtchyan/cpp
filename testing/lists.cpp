#include <iostream>

/* //SINGLE LINKED LIST//

class Node {
    public:
        int data;
        Node* next;
        Node(int value): data(value), next(nullptr){}
};

class SingleLinkedList{
    private:
        Node* head;
    public:
        SingleLinkedList(): head(nullptr){}
        ~SingleLinkedList(){
            while(head!=nullptr){
                Node* temp=head;
                head=head->next;
                delete temp;
            }
        }

        void add(int value){
            Node* NewNode=new Node(value);
            NewNode->next=head;
            head=NewNode;
        }

        void print(){
            Node* current=head;
            while(current!=nullptr){
                std::cout<<current->data<<std::endl;
                current=current->next;
            }
            std::cout<<"nullptr";
        }
};

int main(){
    SingleLinkedList obj;
    obj.add(5);
    obj.add(7);
    obj.add(3);
    obj.print();
    return 0;
}*/



/* //DOUBLY LINKED LIST

class Node{
    public:
        int data;
        Node* next;
        Node* prev;
        Node(int value): data(value), next(nullptr), prev(nullptr){} 
};

class DoublyLinkedList{
    private:
        Node* head;
        Node* tail;
    public:
        DoublyLinkedList(): head(nullptr), tail(nullptr){}
        ~DoublyLinkedList(){
            while(head!=nullptr){
                Node* temp=head;
                head=head->next;
                delete temp;
            }
        }
        void push_front(int value){
            Node* newNode=new Node(value);
            if(head==nullptr)
                head=tail=newNode;
            else {
                newNode->next=head;
                head->prev=newNode;
                head=newNode;
            }
        }
        void push_back(int value){
            Node* newNode=new Node(value);
            if(tail==nullptr)
                head=tail=newNode;
            else{
                newNode->prev=tail;
                tail->next=newNode;
                tail=newNode;
            }
        }
        void print(){
            Node* current=head;
            while(current!=nullptr){
                std::cout<<current->data<<std::endl;
                current=current->next;
            }
            std::cout<<"nullptr";
        }
};
int main(){
    DoublyLinkedList obj;
    obj.push_front(5);
    obj.push_front(2);
    obj.push_front(6);
    obj.push_back(3);
    obj.print();
    return 0;
}*/



/* //CIRCULAR LINKED LIST

class Node{
    public:
        int data;
        Node* next;
        Node(int value): data(value), next(nullptr){}
};

class CircularLinkedList{
    private:
        Node* head;
    public:
        CircularLinkedList(): head(nullptr){}
        ~CircularLinkedList(){
            if(head==nullptr)
                return;
            Node* current=head->next;
            while(current!=head){
                Node* temp=current;
                current=current->next;
                delete temp;
            }
            delete head;
        }
        void push_front(int value){
            Node* newNode=new Node(value);
            if(head==nullptr){
                head=newNode;
                head->next=head;
            } else{
            Node* last=head;
            while (last->next!=head) {
                last=last->next;
            }
            newNode->next=head;
            last->next=newNode;
            head=newNode;
            }
        }
        void print() {
        if (head==nullptr) {
            std::cout<<"List is empty"<<std::endl;
            return;
        }
        Node* current=head;
        do{
            std::cout<<current->data<<"->";
            current=current->next;
        } while(current!=head);
        std::cout<<head->data<<std::endl;
    }
};

int main(){
    CircularLinkedList obj;
    obj.push_front(5);
    obj.push_front(3);
    obj.push_front(2);
    obj.print();
    return 0;
} */