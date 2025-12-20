#include <iostream>
#include <string>
#include <vector>
#include <memory>

class Shape {
    protected:
        std::string color;
    public:
        Shape(const std::string& c) : color(c){}
        
        virtual ~Shape() {
            std::cout<<"Shape destructor called\n";
        }
        
        virtual double area() const=0;
        
        virtual void draw() const {
            std::cout<<color<<"\n";
        }
        
        void setColor(const std::string& c) {
            color=c;
        }
        
        std::string getColor() const {
            return color;
        }
    };

class Circle : public Shape {
    private:
        double radius;
    public:
        Circle(const std::string& c, double r) : Shape(c), radius(r) {}
        
        ~Circle() override {
            std::cout<<"Circle destructor called\n";
        }

        double area() const override {
            return 3.14*radius*radius;
        }
        
        void draw() const override {
            std::cout<<"Radius "<<radius<<" and color "<<color<<"\n";
        }
};

class Rectangle : public Shape {
    private:
        double width, height;
    public:
        Rectangle(const std::string& c, double w, double h) : Shape(c), width(w), height(h) {}
        
        ~Rectangle() override {
            std::cout<<"Rectangle destructor called\n";
        }
        
        double area() const override {
            return width*height;
        }
        
        void draw() const override {
            std::cout<< "Rectangle "<<width<<"x"<<height<<" and color "<<color<<"\n";
        }
};

void displayShapeInfo(const Shape& shape) {
    shape.draw();
    std::cout<<"Area: "<<shape.area()<<"\n\n";
}

int main() {
    Shape* shape1=new Circle("Red", 5.0);
    Shape* shape2=new Rectangle("Blue", 4.0, 6.0);
    
    shape1->draw();
    std::cout<<"Area: "<<shape1->area()<<"\n\n";
    
    shape2->draw();
    std::cout<<"Area: "<<shape2->area()<<"\n\n";
    
    delete shape1;
    delete shape2;

    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>("Yellow", 3.0));
    shapes.push_back(std::make_unique<Rectangle>("Purple", 5.0, 2.0));
    
    for (const auto& shape : shapes) {
        displayShapeInfo(*shape);
    }
    
    Circle circle("Cyan", 4.0);
    Rectangle rect("Magenta", 3.0, 7.0);
    
    displayShapeInfo(circle);
    displayShapeInfo(rect);
    
    return 0;
}