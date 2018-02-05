
#include "../Reflection.h"
#include "../ToString.h"
#include <iostream>
#include <cassert>

using namespace vklib;

class ReflectableClass1
{
    int intField = 3;
    std::string stringField = "StringFieldTest";
    bool boolField = true;

    REFLECTABLE_FIELDS(intField, stringField, boolField);
};

class ReflectableClass2
{
    std::unique_ptr<ReflectableClass1> classField { new ReflectableClass1() };
    double doubleField = 3423.532;
    std::tuple<char, int> tupleField {'c', 6786 };
    std::unordered_map<int, double> unorderedMapField {{1, 1.1}, {2, 2.2}};

    REFLECTABLE_FIELDS(classField, doubleField, tupleField, unorderedMapField);
};

void GeneralTest()
{
    //  TODO: write some reasonable tests ;)
    ReflectableClass2 rc;
    std::string expected = "{classField={intField=3,stringField=StringFieldTest,boolField=1},doubleField=3423.53,tupleField={c,6786},unorderedMapField=[{2,2.2},{1,1.1}]}";
    std::string actual = ToString(rc);
    std::cout << actual;
    assert(actual == expected);
}

int main(int argc, char** argv)
{
    GeneralTest();
    return 0;
}
