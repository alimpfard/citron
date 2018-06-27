#pragma once

template<typename E, E first, E head>
void advanceEnum(E& v)
{
    if(v == head)
        v = first;
}

template<typename E, E first, E head, E next, E... tail>
void advanceEnum(E& v)
{
    if(v == head)
        v = next;
    else
        advanceEnum<E,first,next,tail...>(v);
}

template<typename E, E first, E... values>
struct EnumValues
{
    static void advance(E& v)
    {
        advanceEnum<E, first, first, values...>(v);
    }
};

namespace Citron {

static int tyvar_id_ = 0;

class TyVar {
public:
    const int id;
    TyVar() : id(tyvar_id_++) {}
    TyVar(const TyVar& t) = default;
    TyVar(TyVar&& t) = default;

    bool operator== (const TyVar& t) { return id == t.id; }
};

enum class Type {
    DummyTy,
    NumberTy,
    StringTy,
    BoolTy
};

static std::map<TyVar, Type> TypeVarMap;

static const std::string ty_as_string(Citron::Type ty) {
    switch (ty) {
    case Citron::Type::DummyTy:
        return "Dummy";
    case Citron::Type::NumberTy:
        return "Number";
    case Citron::Type::BoolTy:
        return "Boolean";
    case Citron::Type::StringTy:
        return "String";
    }
    return "Unknown";
}

typedef EnumValues<Type,
    Type::DummyTy,
    Type::NumberTy,
    Type::StringTy,
    Type::BoolTy
> TypesEnum;

Type& operator++(Type& t) {
    TypesEnum::advance(t);
    return t;
}

bool citron_type_equals (const Type& t0, const Type& t1) {
    return static_cast<int>(t0) == static_cast<int>(t1);
}
// bool citron_type_equals (const Type& t0, const TyVar& tv) {
//     auto mt = TypeVarMap.find(tv);
//     if(mt == TypeVarMap.end()) {
//         TypeVarMap[tv] = t0;
//         return true;
//     }
//      return mt->second == t0;
// }
// bool citron_type_equals (const TyVar& tv, const Type& t0) {
//     auto mt = TypeVarMap.find(tv);
//     if(mt == TypeVarMap.end()) {
//         TypeVarMap[tv] = t0;
//         return true;
//     }
//      return mt->second == t0;
// }
}