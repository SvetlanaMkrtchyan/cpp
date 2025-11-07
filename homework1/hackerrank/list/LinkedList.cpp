bool has_cycle(Node* head) {
    if (head==nullptr) 
        return false;

    Node* s=head;
    Node* f=head;

    while (f!=nullptr && f->next!=nullptr) {
        s=s->next;
        f=f->next->next;

        if (s==f)
            return true;
    }
    return false;
}