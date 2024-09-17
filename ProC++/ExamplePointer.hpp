#ifndef EXAMPLEPOINTER_HPP
#define EXAMPLEPOINTER_HPP

#include <iostream>
#include <format>
#include <array>

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
    //char* myCharPtr2 { static_cast<char*>(documentPtr) }; // occured error, error: invalid ‘static_cast’ from type ‘Document*’ to type ‘char*’
}
// Declare your class or function prototypes here

void doubleInts(int* theArray, size_t size)
{
    for( size_t i { 0 }; i < size; i++ )
    {
        theArray[i] *= 2;
    }
}
void doublePrint(int theArray[], size_t size)
{
    std::cout << "doublePrint(int theArray[], size_t size)" << std::endl;
    for( int element {0}; element < size; element++ )
    {
        std::cout << std::format("{} ", theArray[element]);
    }
    std::cout << std::endl << "-------" << std::endl;
}
void doublePrint2(int *theArray, size_t size)
{
    std::cout << "doublePrint2(int *theArray, size_t size)" << std::endl;
    for( int element {0}; element < size; element++ )
    {
        std::cout << std::format("{} ", theArray[element]);
    }
    std::cout << std::endl << "-------" << std::endl;
}
void doublePrint3(int theArray[2], size_t size)
{
    std::cout << "doublePrint3(int theArray[2], size_t size)" << std::endl;
    for( int element {0}; element < size; element++ )
    {
        std::cout << std::format("{} ", theArray[element]);
    }
    std::cout << std::endl << "-------" << std::endl;
}

void doubleIntsStack(int (&theArray)[4])
{
    std::cout << "doubleIntsStack(int (&theArray)[4])" << std::endl;
    for( int element {0}; element < 4; element++ )
    {
        std::cout << std::format("{} ", theArray[element]);
    }
    std::cout << std::endl << "-------" << std::endl;
}
void ExamplePage349()
{
    std::cout << format("using {}", __func__) << std::endl;
    size_t arraySize { 4 };
    int* freeStoreArray { new int[arraySize] { 1, 5, 3, 4} };
    doubleInts(freeStoreArray, arraySize );
    delete[] freeStoreArray;
    freeStoreArray = nullptr;

    int stackArray[] { 5, 7, 9, 11};
    size_t arrSize = std::size(stackArray); //after C++17
    //arrSize = sizeof(stackArray) / sizeof(stackArray[0]); //before C++17
    doubleInts(stackArray, arrSize);
    doubleInts(&stackArray[0], arraySize);
    doublePrint(stackArray, arrSize);
    doublePrint(&stackArray[0], arrSize);
    doublePrint(&stackArray[2], arrSize);

    doublePrint(stackArray, arrSize);
    doublePrint2(stackArray, arrSize);
    doublePrint3(stackArray, arrSize);
    doubleIntsStack(stackArray);

    //int* freeStoreArray { new int[] { 5, 7, 9, 11, 13, 15, 17, 19}};
    int stackArray2[] { 5, 7, 9, 11, 13, 15, 17, 19};
    //doubleIntsStack(stackArray2); // error: invalid initialization of reference of type ‘int (&)[4]’ from expression of type ‘int [8]’

}
#endif // EXAMPLEPOINTER_HPP