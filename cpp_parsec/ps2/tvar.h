#ifndef PS_TVAR_H
#define PS_TVAR_H

#include <string>
#include <functional>
#include <any>

//#define PS_DEBUG
#define PS_CHURCH

#ifdef PS_DEBUG
#include <iostream>
#endif

namespace ps
{

using Id = std::uint32_t;
using TVarId = Id;

template <typename T>
struct TVar
{
    std::string name;
    TVarId id;

//    TVar()
//    {
//#ifdef PS_DEBUG
//        std::cout << "TVar: empty constructor " << std::endl;
//#endif
//    }

//    explicit TVar(const std::string& name,
//                  const TVarId& id)
//        : name(name)
//        , id(id)
//    {
//#ifdef PS_DEBUG
//        std::cout << "TVar: constructor " << std::endl;
//#endif
//    }

//    TVar(const TVar<T>& other)
//        : name(other.name)
//        , id(other.id)
//    {
//#ifdef PS_DEBUG
//        std::cout << "TVar: copy constructor, tvar id: "
//                  << other.id
//                  << ", tvar name: "
//                  << other.name
//                  << std::endl;
//#endif
//    }

//    TVar(const TVar<T>&& other)
//        : name(other.name)
//        , id(other.id)
//    {
//#ifdef PS_DEBUG
//        std::cout << "TVar: move constructor, tvar id: "
//                  << other.id
//                  << ", tvar name: "
//                  << other.name
//                  << std::endl;
//#endif
//    }

//    TVar<T>& operator=(TVar<T> other)
//    {
//#ifdef PS_DEBUG
//        std::cout << "TVar: copy assignment operator, tvar id: "
//                  << other.id
//                  << ", tvar name: "
//                  << other.name
//                  << std::endl;
//#endif
//        std::swap(name, other.name);
//        std::swap(id, other.id);
//        return *this;
//    }

//    TVar<T>& operator=(TVar<T>&& other)
//    {
//#ifdef PS_DEBUG
//        std::cout << "TVar: move assignment operator, tvar id: "
//                  << other.id
//                  << ", tvar name: "
//                  << other.name
//                  << std::endl;
//#endif
//        std::swap(name, other.name);
//        std::swap(id, other.id);
//        return *this;
//    }

//    friend void swap(TVar<T>& first, TVar<T>& second)
//    {
//#ifdef PS_DEBUG
//        std::cout << "TVar: swap, tvar id: "
//                  << second.id
//                  << ", tvar name: "
//                  << second.name
//                  << std::endl;
//#endif
//        std::swap(first.id, second.id);
//        std::swap(first.name, second.name);
//    }
};

} // namespace ps

#endif // PS_TVAR_H
