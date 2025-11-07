DoublyLinkedListNode* sortedInsert(DoublyLinkedListNode* llist, int data) {
    DoublyLinkedListNode* new_node=new DoublyLinkedListNode(data);

    if (llist==nullptr) 
        return new_node;

    if (data<=llist->data) {
        new_node->next=llist;
        llist->prev=new_node;
        return new_node;
    }

    DoublyLinkedListNode* cur=llist;
    while (cur->next!=nullptr && cur->next->data<data)
        cur=cur->next;
    
    new_node->next=cur->next;
    new_node->prev=cur;
    if (cur->next!=nullptr)
        cur->next->prev=new_node;
    cur->next=new_node;

    return llist;
}