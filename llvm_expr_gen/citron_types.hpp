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

// enum class Type {
//     DummyTy,
//     NumberTy,
//     StringTy,
//     BoolTy
// };

class TypeC {
    //typedef TypeC* resolve_t (TypeC, std::vector<TypeC>);
    int index;
    size_t data_;
    static TypeC no_op_resolve(std::vector<TypeC> args) {
        return args[0]; //self
    }
public:
    using resolve_t = decltype(no_op_resolve)*;

    resolve_t res_fn;
    static constexpr int length() { return 5; }
    TypeC(size_t data = 0) : index(0), data_(data), res_fn((&no_op_resolve)) {}
    constexpr explicit TypeC(int index, size_t data = 0, resolve_t resolve = nullptr) : index(index), data_(data), res_fn(resolve?:(&no_op_resolve)) {}
    constexpr operator int() const { return index; }

    bool operator == (const TypeC& other) { return index == other.index || other.index == length() || index == length(); }
    bool operator != (const TypeC& other) { return index != other.index && other.index != length() && index != length(); }
    
    const TypeC transform(size_t data) const { return TypeC{index, data, res_fn}; }
    const TypeC transform(resolve_t resolver) const { return TypeC{index, data_, resolver}; }
    inline TypeC resolve(std::vector<TypeC> args) const { return res_fn(args); }
    size_t data() { return data_; }

    TypeC operator+ (size_t i) const { return transform(data_+i); }
};

namespace Type {
constexpr TypeC TyInfer(TypeC::length()); // TyVar : out of range of length()
constexpr TypeC DummyTy(0);
constexpr TypeC NumberTy(1);
constexpr TypeC StringTy(2); //no-op transform string type
constexpr TypeC BoolTy(3);
constexpr TypeC FunTy(4); //indexes into Type::funtypes

static std::map<int, std::vector<TypeC>> funtypes = {
    {0, {DummyTy}}
};
static std::map<int, std::vector<TypeC>> incomplete_types = {};

static size_t tyvar(std::vector<TypeC> args) {
    auto tyv = TyInfer.transform(args.size()); 
    args.insert(args.begin(), tyv);
    incomplete_types[incomplete_types.size()] = args;
    return tyv;
}

static bool unify(size_t tys, TypeC tyt) {
    return true; //TODO finish
}
}

static std::map<TyVar, TypeC> TypeVarMap;

static const std::string ty_as_string(Citron::TypeC ty) {
    switch (ty) {
    case Citron::Type::DummyTy:
        return "Dummy";
    case Citron::Type::NumberTy:
        return "Number";
    case Citron::Type::BoolTy:
        return "Boolean";
    case Citron::Type::StringTy:
        return ("String[" + std::to_string(ty.data()) + "]");
    case Citron::Type::TyInfer:
        return ("UnknownType[" + std::to_string(ty.data()) + "]");
    }
    return "Unknown";
}

bool citron_type_equals (const TypeC& t0, const TypeC& t1) {
    int i0 = t0, i1 = t1;
    int l = TypeC::length();
    // std::cout << i0 << "," << i1 << '\n';
    return i0 == i1 || i0 == l || i1 == l;
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