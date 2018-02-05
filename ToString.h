//
//  MIT License
//
//  Copyright (c) 2018, Valentin Kuznetsov <valkuzn@gmail.com>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.
//

//
//  This file contains implementation of ToString function.
//  ToString extent STL string stream functions with support of STL containers
//  (e.g. it's possible to call ToString for maps, tuples, smart pointers, etc.)
//  and reflectable classes (see Reflection.h for details).

#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace vklib
{

template<class StreamT>
class ReflectionFieldStreamWriterVisitor
{
    StreamT& _stream;
    bool firstField = true;

public:
    ReflectionFieldStreamWriterVisitor(StreamT& stream) : _stream(stream) {}

    template<class T>
    bool VisitField(const char* fieldName, const T& value)
    {
        if(firstField)
            firstField = false;
        else
            _stream << ",";

        _stream << fieldName << "=" << value;
        return true;
    }
};

template<class StreamT>
class ObjectPrinter
{
protected:
    StreamT& _stream;

    template<class T>
    void _Print(const char* fieldName, const T& value, std::true_type)
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

    template<class T>
    void VisitList(const T& value)
    {
        _stream << "[";
        bool first = true;
        for(const auto& item : value)
        {
            if(first)
                first = false;
            else
                _stream << ',';

            Visit(item);
        }
        _stream << "]";
    }

    template<class T>
    void VisitPtr(const T& value)
    {
        if(value)
            Visit(*value);
        else
            _stream << "null";
    }

    template<int fieldId, typename T, typename S>
    struct TupleVisitor
    {
        static void Visit(S& stream, const T& value)
        {
            TupleVisitor<fieldId-1, T, S>::Visit(stream, value);
            stream << ",";
            stream << std::get<fieldId>(value);
        }
    };

    template<typename T, typename S>
    struct TupleVisitor<0, T, S>
    {
        static void Visit(S& stream, const T& value)
        {
            stream << std::get<0>(value);
        }
    };

public:
    ObjectPrinter(StreamT& stream) : _stream(stream) {}

    //
    //  Define specifications for strings, since we redefine Visit for generic "T*""
    //
    void Visit(const char* value) { _stream << value; }

    void Visit(const wchar_t* value) { _stream << value; }

    //
    //  Pointers (including smart)
    //
    template<typename T>
    void Visit(const T* value) { VisitPtr(value); }

    template<class T, class Deleter>
    void Visit(const std::unique_ptr<T, Deleter>& value) { VisitPtr(value); }

    template<class T>
    void Visit(const std::shared_ptr<T>& value) { Visit(value); }

    template<class T>
    void Visit(const std::weak_ptr<T>& value) { Visit(value.lock()); }

    //
    //  Pairs and tuples
    //
    template<typename T1, typename T2>
    void Visit(const std::pair<T1, T2>& value)
    {
        _stream << "{";
        Visit(value.first);
        _stream << ",";
        Visit(value.second);
        _stream << "}";
    }

    template<typename... TupleTypes>
    void Visit(const std::tuple<TupleTypes...>& value)
    {
        _stream << "{";
        TupleVisitor<sizeof...(TupleTypes)-1, std::tuple<TupleTypes...>, ObjectPrinter>::Visit(*this, value);
        _stream << "}";
    }

    //
    //  Sequence containers
    //
    template<class T, std::size_t N>
    void Visit(const std::array<T, N>& value) { VisitList(value); }

    template<class T, class Allocator>
    void Visit(const std::vector<T, Allocator>& value) { VisitList(value); }

    template<class T, class Allocator>
    void Visit(const std::deque<T, Allocator>& value) { VisitList(value); }

    template<class T, class Allocator>
    void Visit(const std::forward_list<T, Allocator>& value) { VisitList(value); }

    template<class T, class Allocator>
    void Visit(const std::list<T, Allocator>& value) { VisitList(value); }

    //
    //  Associative containers
    //
    template<class Key, class Compare, class Allocator>
    void Visit(const std::set<Key, Compare, Allocator>& value) { VisitList(value); }

    template<class Key, class T, class Compare, class Allocator>
    void Visit(const std::map<Key, T, Compare, Allocator>& value) { VisitList(value); }

    template<class Key, class Compare, class Allocator>
    void Visit(const std::multiset<Key, Compare, Allocator>& value) { VisitList(value); }

    template<class Key, class T, class Compare, class Allocator>
    void Visit(const std::multimap<Key, T, Compare, Allocator>& value) { VisitList(value); }

    //
    //  Unordered associative containers
    //
    template<class Key, class Hash, class KeyEqual, class Allocator>
    void Visit(const std::unordered_set<Key, Hash, KeyEqual, Allocator>& value) { VisitList(value); }

    template<class Key, class T, class Hash, class KeyEqual, class Allocator>
    void Visit(const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& value) { VisitList(value); }

    template<class Key, class Hash, class KeyEqual, class Allocator>
    void Visit(const std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& value) { VisitList(value); }

    template<class Key, class T, class Hash, class KeyEqual, class Allocator>
    void Visit(const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& value) { VisitList(value); }

    //
    //  Specification for reflectable class
    //
    template<class T>
    typename std::enable_if_t<Reflection::IsReflectable<T>(), void> Visit(const T& value)
    {
        ReflectionFieldStreamWriterVisitor<ObjectPrinter> visitor(*this);
        _stream << "{";
        Reflection::VisitFields(value, visitor);
        _stream << "}";
    }

    template<class T>
    bool VisitField(const char* fieldName, const T& value)
    {
        _stream << fieldName << "=";
        Visit(value, *this);
        _stream << " ";
        return true;
    }

    //
    //  General case - just leave for stream to resolve
    //
    template<class T>
    typename std::enable_if_t<!Reflection::IsReflectable<T>(), void> Visit(const T& value)
    {
        _stream << value;
    }

    template<class T>
    ObjectPrinter& operator<<(const T& value)
    {
        Visit(value);
        return *this;
    }
};

template<typename StreamT, typename ObjectT>
void ToString(StreamT& stream, ObjectT& obj)
{
    ObjectPrinter<std::stringstream> printer(stream);
    printer.Visit(obj);
}

template<typename ObjectT>
std::string ToString(ObjectT& obj)
{
    std::stringstream ss;
    ToString(ss, obj);
    return ss.str();
}

};  //  namespace vklib
