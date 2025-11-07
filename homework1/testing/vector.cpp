#include <iostream>
#include <vector>
#include <algorithm>

/*int main(){
    std::vector<int> arr={1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int sum=0;
    for(size_t i=0; i<arr.size(); ++i)
        sum+=arr.at(i);
    std::cout<<sum;
    return 0;
}*/

/*int main(){
    std::vector<int> arr={1, 5, -4, 0, 23};
    int max=-1;
    for(size_t i=0; i<arr.size(); ++i)
        if(max<arr.at(i))
            max=arr.at(i);
    std::cout<<max;
    return 0;
}*/

/*int main(){
    std::vector<int> arr={1, 5, -4, 0, 23};
    std::vector<int> temp;
    for(size_t i=0; i<arr.size(); ++i)
        temp.push_back(arr.at(i)*2);
    for(size_t i=0; i<temp.size(); ++i)
        std::cout<<temp.at(i)<<" ";
    return 0;
}*/

/*int main(){
    std::vector<int> arr={10,20,30,40};
    arr.pop_back();
    arr.push_back(50);
    for(size_t i=0; i<arr.size(); ++i)
        std::cout<<arr.at(i)<<" ";
    return 0;
}*/

/*int main(){
    std::vector<int> arr={0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> odd;
    std::vector<int> even;

    for(size_t i=0; i<arr.size(); ++i){
        if(arr.at(i)%2==0 && arr.at(i)>0)
            even.push_back(arr.at(i));
        else if(arr.at(i)%2==1)
            odd.push_back(arr.at(i));
    }

    for(size_t i=0; i<even.size(); ++i)
        std::cout<<even.at(i)<<" ";
    std::cout<<"\n";
    for(size_t i=0; i<odd.size(); ++i)
        std::cout<<odd.at(i)<<" ";
    return 0;
}*/

/*int main(){
    std::vector<int> arr={1, 5, -4, 0, 23};
    int max=arr.at(0), min=arr.at(0);
    for(size_t i=0; i<arr.size(); ++i){
        if(arr.at(i)>max)
            max=arr.at(i);
        if(arr.at(i)<min)
            min=arr.at(i);
    }
    int sub=max-min;
    std::cout<<sub;
    return 0;
}*/

/*int main(){
    std::vector<int> arr={1, 5, -4, 0, 23};
    int sum=0, count=0;
    for(size_t i=0; i<arr.size(); ++i){
        sum+=arr.at(i);
        count++;
    }
    int average=sum/count;
    for(size_t i=0; i<arr.size(); ++i)
        if(average<arr.at(i))
            std::cout<<arr.at(i)<<" ";
    return 0;
}*/

/*int main(){
    std::vector<int> arr={1, 5, 5, -4, 0, 0, 23};
    for(size_t i=0; i<arr.size(); ++i){
        int count=0;
        for(size_t j=0; j<arr.size(); ++j)
            if(arr.at(i)==arr.at(j))
                count++;
        if(count==1)
            std::cout<<arr.at(i)<<" ";
    }
    return 0;
}*/

/*int main(){
    std::vector<int> arr={1, 5, -4, 0, 23, 54, -2};
    std::reverse(arr.begin(), arr.end());
    for(int x:arr)
        std::cout<<x<<" ";
    return 0;
}*/