using namespace std;

class semantics;
class methods {
    public:
        std::string class_name;
        std::string class_method;

        class_name_and_method(std::string class_name, std::string class_method){
            this->class_name = class_name;
            this->class_method = class_method;
        }
    void print_method(){
        std::cout << "\t class_name: " << this->class_name << std::endl;
        std::cout << "\t class_method: " << this->class_method << std::endl;
    }
}