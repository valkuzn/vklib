
#include "../Reflection.h"
#include <iostream>
#include <cassert>

using namespace vklib;

template<class StreamT>
class ReflectionFieldWriterVisitor
{
    StreamT& _stream;

    template<class T>
	bool VisitField(const char* fieldName, const T& value, std::true_type)
	{
		_stream << fieldName << "={";
		Reflection::VisitFields(value, *this);
		_stream << "} ";
		return true;
	}

	template<class T>
	bool VisitField(const char* fieldName, const T& value, std::false_type)
	{
		_stream << fieldName << "=" << value << " ";
		return true;
	}

public:
    ReflectionFieldWriterVisitor(StreamT& stream) : _stream(stream) {}

	template<class T>
	bool VisitField(const char* fieldName, const T& value)
	{
		return VisitField(fieldName, value, typename std::integral_constant<bool, Reflection::IsReflectable<T>()>());
	}
};

class ReflectableClass1
{
public:
	int intField = 3;
	std::string stringField = "StringFieldTest";
	bool boolField = true;

	REFLECTABLE_FIELDS(intField, stringField, boolField);
};

class ReflectableClass2
{
public:
	const ReflectableClass1 classField;
    double doubleField = 3423.532;

	REFLECTABLE_FIELDS(classField, doubleField);
};

void GeneralTest()
{
    //  TODO: write some reasonable tests ;)
    ReflectableClass2 rc;
    std::stringstream ss;

    ReflectionFieldWriterVisitor<std::stringstream> writer(ss);
    Reflection::VisitFields(rc, writer);

    std::string expected = "classField={intField=3 stringField=StringFieldTest boolField=1 } doubleField=3423.53 ";
    std::string actual = ss.str();
    assert(actual == expected);

    std::cout << ss.str();
}

int main(int argc, char** argv)
{
    GeneralTest();
    return 0;
}
