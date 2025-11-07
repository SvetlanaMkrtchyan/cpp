DoublyLinkedListNode* reverse(DoublyLinkedListNode* llist) {
    if (llist==nullptr || llist->next==nullptr) 
        return llist;

    DoublyLinkedListNode* cur=llist;
    DoublyLinkedListNode* temp=nullptr;

    while (cur!=nullptr) {
        temp=cur->next;
        cur->next=cur->prev;
        cur->prev=temp;
        cur=temp;
    }

    temp=llist;
    llist=llist->prev;
    temp->prev=nullptr;

    return llist;
}