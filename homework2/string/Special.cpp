long substrCount(int n, string s) {
    long count = 0;
    
    for(int i=0; i<n; ++i) {
        int j=i;
        while(j<n && s[j]==s[i]) {
            j++;
        }
        int len=j-i;
        count+=(long)len*(len+1)/2;
        i=j-1;
    }

    for(int i=1; i<n-1; ++i) {
        if(s[i-1]==s[i+1] && s[i]!=s[i-1]) {
            int left=i-1;
            int right=i+1;
            while(left>=0 && right<n && s[left]==s[right] && s[left]==s[i-1]) {
                count++;
                left--;
                right++;
            }
        }
    }
    
    return count;
}