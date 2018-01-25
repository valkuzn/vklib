//
//  MIT License
//
//  Copyright (c) 2016, Valentin Kuznetsov <valkuzn@gmail.com>
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
//  This file contains Reflection class, which provides tools for enumerating
//  some defined set of class fields. Using this enumeration it's easy to
//  implement functionality operating over this set of fields (like fields
//  printing, comparation or assignment) without actually writing code for
//  particular field.
//  E.g. if we have the following class:
//  class ReflectableClass
//  {
//  public:
//       int IntField;
//       std::string StringField;
//       bool BoolField;
//       REFLECTABLE_FIELDS(IntField, StringField, BoolField);
//  };
//  And we have some visitor class which implements the following function:
// class FieldPrinter
// {
// public:
//     template<class T>
//     bool VisitField(const char* fieldName, const T& value)
//     {
//         return std::cout << fieldName << "=" << value << ";";
//     }
// };
//  We can print all fields of the classs, using the following code:
//  Reflection::VisitFields(ReflectableClassInstance, FieldPrinterInstance);
//  See Test/ReflectionTest.cpp for examples.
//


#pragma once

#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <type_traits>
#include <boost/functional/hash.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>

namespace vklib
{

class Reflection
{
protected:
    template<size_t offset>
    struct FieldsIterator
    {
        //
        //    Fields visiting
        //
        template<class ReflectableClass, class VisitorClass>
        typename std::enable_if_t<offset < ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, bool>
            static inline VisitFields(ReflectableClass& obj, VisitorClass& visitor)
        {
            if (!visitor.VisitField(GetFieldName<offset>(obj), GetFieldValue<offset>(obj)))
                return false;

            return FieldsIterator<offset + 1>::VisitFields(obj, visitor);
        }

        template<class ReflectableClass, class VisitorClass>
        typename std::enable_if_t<offset >= ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, bool>
            static inline VisitFields(ReflectableClass& obj, VisitorClass& visitor)
        {
            return true;
        }


        //
        //    Equal
        //
        template<class ReflectableClass>
        typename std::enable_if_t<offset < ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, bool>
            static inline Equal(const ReflectableClass& obj1, const ReflectableClass& obj2)
        {
            if (!(GetFieldValue<offset>(obj1) == GetFieldValue<offset>(obj2)))
                return false;

            return FieldsIterator<offset + 1>::Equal(obj1, obj2);
        }

        template<class ReflectableClass>
        typename std::enable_if_t<offset >= ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, bool>
            static inline Equal(const ReflectableClass& obj1, const ReflectableClass& obj2)
        {
            return true;
        }


        //
        //    Less
        //
        template<class ReflectableClass>
        typename std::enable_if_t<offset < ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, bool>
            static inline Less(const ReflectableClass& obj1, const ReflectableClass& obj2)
        {
            if (!(GetFieldValue<offset>(obj1) < GetFieldValue<offset>(obj2)))
                return false;

            return FieldsIterator<offset + 1>::Less(obj1, obj2);
        }

        template<class ReflectableClass>
        typename std::enable_if_t<offset >= ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, bool>
            static inline Less(const ReflectableClass& obj1, const ReflectableClass& obj2)
        {
            return true;
        }


        //
        //    Copy
        //
        template<class ReflectableClass>
        typename std::enable_if_t<offset < ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, void>
            static inline Copy(ReflectableClass& target, const ReflectableClass& source)
        {
            GetFieldValue<offset>(target) = GetFieldValue<offset>(source);
            FieldsIterator<offset + 1>::Copy(target, source);
        }

        template<class ReflectableClass>
        typename std::enable_if_t<offset >= ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, void>
            static inline Copy(ReflectableClass& target, const ReflectableClass& source)
        {
        }


        //
        //    Move
        //
        template<class ReflectableClass>
        typename std::enable_if_t<offset < ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, void>
            static inline Move(ReflectableClass& target, ReflectableClass& source)
        {
            GetFieldValue<offset>(target) = std::move(GetFieldValue<offset>(source));
            FieldsIterator<offset + 1>::Move(target, source);
        }

        template<class ReflectableClass>
        typename std::enable_if_t<offset >= ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, void>
            static inline Move(ReflectableClass& target, ReflectableClass& source)
        {
        }


        //
        //    Hash
        //
        template<class ReflectableClass>
        typename std::enable_if_t<offset < ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, void>
            static inline Hash(size_t& seed, const ReflectableClass& obj)
        {
            boost::hash_combine(seed, GetFieldValue<offset>(obj));
        }

        template<class ReflectableClass>
        typename std::enable_if_t<offset >= ReflectableClass::ReflectableFields::COUNT_OF_FIELDS, void>
            static inline Hash(size_t& seed, const ReflectableClass& obj)
        {
        }
    }; // class FieldsIterator

    template <typename... Ts> using void_t = void;

    template <typename T, typename = void>
    struct HasClassReflectableFields : std::false_type {};

    template <typename T>
    struct HasClassReflectableFields<T, void_t<typename T::ReflectableFields>> : std::true_type {};

public:

    template<class ReflectableClass>
    static constexpr bool IsReflectable()
    {
        return HasClassReflectableFields<ReflectableClass>::value;
    }

    template<class ReflectableClass>
    static constexpr bool IsReflectable(const ReflectableClass& obj)
    {
        return IsReflectable<ReflectableClass>();
    }

    template<const int fieldId, class T>
    static inline auto& GetFieldValue(T& obj)
    {
        return T::template ReflectableFieldDefinition<fieldId, T>::GetFieldValue(obj);
    }

    template<class T, const uint32_t fieldId>
    static inline constexpr const char* const GetFieldName()
    {
        return T::template ReflectableFieldDefinition<fieldId, const T>::GetFieldName();
    }

    template<const uint32_t fieldId, class T>
    static constexpr const char* const GetFieldName(T& obj)
    {
        return GetFieldName<T, fieldId>();
    }

    template<class T, const uint32_t fieldId>
    static constexpr const wchar_t* const GetFieldNameW()
    {
        return T::template ReflectableFieldDefinition<fieldId, T>::GetFieldNameW();
    }

    template<const uint32_t fieldId, class T>
    static constexpr const wchar_t* const GetFieldNameW(T& obj)
    {
        return GetFieldNameW<T, fieldId>();
    }

    template<class T>
    static inline constexpr uint32_t GetFieldCount()
    {
        return T::ReflectableFields::COUNT_OF_FIELDS;
    }

    template<class T>
    static __inline constexpr uint32_t GetFieldCount(const T& obj)
    {
        return GetFieldCount<T>();
    }

    //
    //  VisitFields iterate through all reflectable fields in the class and
    //  calls the function, with the following signature:
    //  bool VisitField(const char* fieldName, const T& value)
    //
    template<class ReflectableClass, class VisitorClass>
    static bool VisitFields(ReflectableClass& obj, VisitorClass& visitor)
    {
        return FieldsIterator<0>::VisitFields(obj, visitor);
    }

    template<class ReflectableClass>
    static bool Equal(const ReflectableClass * const obj1, const ReflectableClass * const obj2)
    {
        if (obj1 == obj2)
            return true;

        if (obj1 == nullptr || obj2 == nullptr)
            return false;

        return FieldsIterator<0>::Equal(*obj1, *obj2);
    }

    template<class ReflectableClass>
    static bool Equal(const ReflectableClass& obj1, const ReflectableClass& obj2)
    {
        if (&obj1 == &obj2)
            return true;

        return FieldsIterator<0>::Equal(obj1, obj2);
    }

    template<class ReflectableClass>
    static bool Less(const ReflectableClass& obj1, const ReflectableClass& obj2)
    {
        return FieldsIterator<0>::Less(obj1, obj2);
    }

    template<class ReflectableClass>
    static size_t Hash(const ReflectableClass& obj)
    {
        size_t seed = 0;
        FieldsIterator<0>::Hash(seed, obj);
        return seed;
    }

    template<class ReflectableClass>
    static void Copy(ReflectableClass& target, const ReflectableClass& source)
    {
        FieldsIterator<0>::Copy(target, source);
    }


    template<class ReflectableClass>
    static void Move(ReflectableClass& target, ReflectableClass&& source)
    {
        FieldsIterator<0>::Move(target, source);
    }
};  //  class Reflection


template<class ReflectableClass>
__inline typename std::enable_if_t<Reflection::IsReflectable<ReflectableClass>(), bool>
    operator==(const ReflectableClass& obj1, const ReflectableClass& obj2)
{
    return Reflection::Equal(obj1, obj2);
}

template<class ReflectableClass>
__inline typename std::enable_if_t<Reflection::IsReflectable<ReflectableClass>(), bool>
    operator!=(const ReflectableClass& obj1, const ReflectableClass& obj2)
{
    return !Reflection::Equal(obj1, obj2);
}

template<class ReflectableClass>
__inline typename std::enable_if_t<Reflection::IsReflectable<ReflectableClass>(), bool>
    operator<(const ReflectableClass& obj1, const ReflectableClass& obj2)
{
    return !Reflection::Less(obj1, obj2);
}

template<class ReflectableClass>
struct Hash : std::enable_if<Reflection::IsReflectable<ReflectableClass>()>
{
    typedef ReflectableClass argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& obj) const
    {
        return Reflection::Hash(obj);
    }
};


#define REFLECTABLE_FIELD_ID(field)    FIELD_ID_##field
#define REFLECTABLE_FULL_FIELD_ID(field)    ReflectableFields::FIELD_ID_##field

#define REFLECTABLE_GENERATE_FIELD_ID(elem)    REFLECTABLE_FIELD_ID(elem),
#define REFLECTABLE_RESTORE_GENERATE_FIELD_ID(r, data, elem)    REFLECTABLE_GENERATE_FIELD_ID(elem)


#define REFLECTABLE_GENERATE_FIELD_ACCESSOR(elem)                               \
template<typename T>                                                            \
class ReflectableFieldDefinition<REFLECTABLE_FULL_FIELD_ID(elem), T>            \
{                                                                               \
public:                                                                         \
    static inline auto& GetFieldValue(T& t)                                     \
    {                                                                           \
        return t.elem;                                                          \
    }                                                                           \
                                                                                \
    static constexpr const char* GetFieldName()                                 \
    {                                                                           \
        return #elem;                                                           \
    }                                                                           \
                                                                                \
    static constexpr const wchar_t* GetFieldNameW()                             \
    {                                                                           \
        return L###elem;                                                        \
    }                                                                           \
};                                                                              \


#define REFLECTABLE_RESTORE_REFLECTABLE_GENERATE_FIELD_ACCESSOR(r, data, elem)  REFLECTABLE_GENERATE_FIELD_ACCESSOR(elem)

#define REFLECTABLE_FIELDS_FROM_SEQ(SEQ)                                                        \
    private:                                                                                    \
    friend class vklib::Reflection;                                                             \
    class ReflectableFields                                                                     \
    {                                                                                           \
    public:                                                                                     \
        enum FieldIds : uint32_t                                                                \
        {                                                                                       \
            BOOST_PP_SEQ_FOR_EACH(REFLECTABLE_RESTORE_GENERATE_FIELD_ID, _, SEQ)                \
            COUNT_OF_FIELDS                                                                     \
        };                                                                                      \
    };                                                                                          \
    template<const uint32_t fieldId, class T>                                                   \
    class ReflectableFieldDefinition                                                            \
    {                                                                                           \
    public:                                                                                     \
        static inline auto& GetFieldValue(T& t)                                                 \
        {                                                                                       \
            static_assert(fieldId < ReflectableFields::COUNT_OF_FIELDS, "There is no field with such id");    \
            return t;                                                                           \
        }                                                                                       \
        static inline constexpr const char* GetFieldName()                                      \
        {                                                                                       \
            static_assert(fieldId < ReflectableFields::COUNT_OF_FIELDS, "There is no field with such id");    \
            return "";                                                                          \
        }                                                                                       \
        static inline constexpr const wchar_t* GetFieldNameW()                                  \
        {                                                                                       \
            static_assert(fieldId < ReflectableFields::COUNT_OF_FIELDS, "There is no field with such id");    \
            return L"";                                                                         \
        }                                                                                       \
    };                                                                                          \
                                                                                                \
    BOOST_PP_SEQ_FOR_EACH(REFLECTABLE_RESTORE_REFLECTABLE_GENERATE_FIELD_ACCESSOR, _, SEQ)      \


#define REFLECTABLE_FIELDS(...)        REFLECTABLE_FIELDS_FROM_SEQ(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define REFLECTABLE_SERIALIZABLE_FIELDS(...)        REFLECTABLE_FIELDS(__VA_ARGS__)  \
                                                    SERIALIZABLE_FIELDS(__VA_ARGS__)
}; //   namespace vklib
