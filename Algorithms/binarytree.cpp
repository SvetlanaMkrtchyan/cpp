#include <iostream>
#include <algorithm>
using namespace std;

struct Node {
    int data;
    Node *left, *right;

    Node(int val) {
        data = val;
        left = right = nullptr;
    }
};

class BinarySearchTree {
    private:
        Node* root;

        Node* insertRecursive(Node* node, int val) {
            if (node == nullptr) 
                return new Node(val);

            if (val < node->data)
                node->left = insertRecursive(node->left, val);
            else if (val > node->data)
                node->right = insertRecursive(node->right, val);

            return node;
        }

        bool searchRecursive(Node* node, int val) {
            if (node == nullptr) 
                return false;
            if (node->data == val) 
                return true;

            if (val < node->data)
                return searchRecursive(node->left, val);
            return searchRecursive(node->right, val);
        }

        Node* findMin(Node* node) {
            Node* current = node;
            while (current && current->left != nullptr)
                current = current->left;
            return current;
        }

        Node* deleteRecursive(Node* node, int val) {
            if (node == nullptr)    
                return node;

            if (val < node->data)
                node->left = deleteRecursive(node->left, val);
            else if (val > node->data)
                node->right = deleteRecursive(node->right, val);
            else {
                if (node->left == nullptr) {
                    Node* temp = node->right;
                    delete node;
                    return temp;
                } else if (node->right == nullptr) {
                    Node* temp = node->left;
                    delete node;
                    return temp;
                }

                Node* temp = findMin(node->right);
                node->data = temp->data;
                node->right = deleteRecursive(node->right, temp->data);
            }
            return node;
        }

        void inorderRecursive(Node* node) {
            if (node != nullptr) {
                inorderRecursive(node->left);
                cout << node->data << " ";
                inorderRecursive(node->right);
            }
        }

    public:
        BinarySearchTree() : root(nullptr) {}

        void insert(int val) { root = insertRecursive(root, val); }
        bool search(int val) { return searchRecursive(root, val); }
        void remove(int val) { root = deleteRecursive(root, val); }
        
        void displayInOrder() {
            inorderRecursive(root);
            cout << endl;
        }
};

int main() {
    BinarySearchTree bst;

    bst.insert(50);
    bst.insert(30);
    bst.insert(20);
    bst.insert(40);
    bst.insert(70);
    bst.insert(60);
    bst.insert(80);

    cout << "In-order շրջանցում (տեսակավորված): ";
    bst.displayInOrder(); 

    int key = 40;
    if (bst.search(key))
        cout << key << "-ը գտնվել է ծառում:" << endl;
    else
        cout << key << "-ը չկա ծառում:" << endl;

    cout << "Ջնջում ենք 20-ը (տերև):" << endl;
    bst.remove(20);
    bst.displayInOrder();

    cout << "Ջնջում ենք 30-ը (մեկ զավակով):" << endl;
    bst.remove(30);
    bst.displayInOrder();

    cout << "Ջնջում ենք 50-ը (արմատ, երկու զավակով):" << endl;
    bst.remove(50);
    bst.displayInOrder();

    return 0;
}