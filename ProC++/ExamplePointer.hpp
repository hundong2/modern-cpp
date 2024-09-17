#ifndef EXAMPLEPOINTER_HPP
#define EXAMPLEPOINTER_HPP

#include <iostream>
#include <format>

using namespace std;
class Document
{
    public:
        Document() { cout << "Document constructor called!" << endl; }
        Document(const std::string& name) : documentName { name } { cout << "Document constructor called!" << endl; }
        ~Document() { cout << "Document destructor called!" << endl; }
        static Document* getDocument() 
        {
            return new Document{ "Document" };
        }
        static std::string getDocumentName() { return "Document"; }
    private: 
        std::string documentName { "Document" };
};

// Include any necessary libraries or headers here
void ExamplePage347()
{
    cout << format("using {}", __func__) << endl;
    //C style casting 
    Document* documentPtr { Document::getDocument()};
    char* myCharPtr { (char*)documentPtr }; //not occured error. 
    char* myCharPtr2 { static_cast<char*>(documentPtr) }; // occured error, error: invalid ‘static_cast’ from type ‘Document*’ to type ‘char*’
}
// Declare your class or function prototypes here

#endif // EXAMPLEPOINTER_HPP