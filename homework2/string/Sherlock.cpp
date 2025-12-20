string isValid(string s) {
    int freq[26]={0};

    for(char c : s) {
        freq[c-'a']++;
    }

    for(int i=0; i<26; ++i) {
        for(int j=0; j<=1; ++j) {
            if(j>freq[i]) 
                continue;
            
            freq[i]-=j;
            
            int t=0;
            bool found=true;
            for(int k=0; k<26; ++k) {
                if(freq[k]>0) {
                    if(t==0) 
                        t=freq[k];
                    else if(freq[k]!=t) 
                        found=false;
                }
            }
            
            if(found) 
                return "YES";
            
            freq[i]+=j; 
        }
    }
    return "NO";
}