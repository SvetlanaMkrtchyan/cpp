int findMergeNode(SinglyLinkedListNode* head1, SinglyLinkedListNode* head2) {
    SinglyLinkedListNode* p1=head1;
    SinglyLinkedListNode* p2=head2;

    while (p1!=p2) {
        if (p1==nullptr)
            p1=head2;
        else 
            p1=p1->next;
        if (p2==nullptr) 
            p2=head1;
        else 
            p2=p2->next;
    }
    return p1->data;
}