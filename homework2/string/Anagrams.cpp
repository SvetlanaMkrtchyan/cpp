int makeAnagram(string a, string b) {
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    int count=0, i=0, j=0;
    while(i<a.length() && j<b.length()){
        if(a[i]==b[j]){
            count++;
            i++;
            j++;
        }
        else if(a[i]<b[j])
            i++;
        else
            j++;
    }
    return a.length()+b.length()-2*count;
}