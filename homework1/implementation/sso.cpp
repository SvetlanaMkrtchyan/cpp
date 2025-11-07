#include <iostream>
#include <cstring>

class String {
    private:
        union {
            struct {
                char* data;
                size_t size;
                size_t capacity;
            };
            char buffer[24];
        };

        bool foo() const{
            return (buffer[23]&1)==0;
        }

        void set(size_t len) {
            buffer[23]=(len<<1)&0b11111110;
        }

        size_t get() const{
            return buffer[23]>>1;
        }

    public:
        String(const char* str) {
            size_t len=strlen(str);
            if (len<=22) {
                std::memcpy(buffer, str, len);
                buffer[len]='\0';
                set(len);
            } 
            else {
                data=new char[len+1];
                std::memcpy(data, str, len+1);
                size=len;
                capacity=len;
                buffer[23]=1;
            }
        }

        ~String() {
            if (!foo()) {
                delete[] data;
                data=nullptr;
            }
        }

        const char* sentence() const{
            return foo() ? buffer : data;
        }

        size_t length() const{
            return foo() ? get() : size;
        }

        bool f() const{
            return foo();
        }
};

int main() {
    String ob1("Svetlana");
    String ob2("Svetlana and Venera got a chance to travel");

    std::cout<<"s1:"<<ob1.sentence()<<" ("<<ob1.length()<<" chars, "<<(ob1.f() ? "SSO" : "heap")<<")\n";
    std::cout<<"s2:"<<ob2.sentence()<<" ("<<ob2.length()<<" chars, "<<(ob2.f() ? "SSO" : "heap")<<")\n";
    return 0;
}