SinglyLinkedListNode* insertNodeAtPosition(SinglyLinkedListNode* llist, int data, int position) {
    SinglyLinkedListNode* new_node=new SinglyLinkedListNode(data);
    
    if (position==0) {
        new_node->next=llist;
        return new_node;
    }
    
    SinglyLinkedListNode* cur=llist;
    for (int i=0; i<position-1 && cur!=nullptr; ++i)
        cur=cur->next;
    
    if (cur!=nullptr) {
        new_node->next=cur->next;
        cur->next=new_node;
    }
    return llist;
}