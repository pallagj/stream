#include <iostream>
#include <vector>
#include <deque>
#include <vector>
#include <initializer_list>
#include <cmath>
#include <chrono>
#include <memory>
#include <functional>

using std::shared_ptr;
using std::make_shared;
using std::function;
using std::deque;

template<typename T>
struct StreamElem{
    T value;
    function<shared_ptr<StreamElem<T>>()> next;

    StreamElem(T value, function<shared_ptr<StreamElem<T>>()> next)
     : value{value}, next{next} {}
};

template<typename T>
shared_ptr<StreamElem<T>> stream_filter(shared_ptr<StreamElem<T>> stream, function<bool(T)> f){
    while(stream != nullptr && !f(stream->value))
        stream = stream->next();

    if(stream == nullptr)
        return nullptr;

    return make_shared<StreamElem<T>>(
        stream->value,
        [=]()->shared_ptr<StreamElem<T>>{ return stream_filter(stream->next(), f); }
    );
}

template<typename T>
shared_ptr<StreamElem<T>> stream_map(shared_ptr<StreamElem<T>> stream, function<T(T)> f){
    if(stream == nullptr)
        return nullptr;

    return make_shared<StreamElem<T>>(
        f(stream->value),
        [=]()->shared_ptr<StreamElem<T>>{ return stream_map(stream->next(), f); }
    );
}

template<typename T>
shared_ptr<StreamElem<T>> stream_join(shared_ptr<StreamElem<T>> stream_1, shared_ptr<StreamElem<T>> stream_2, function<shared_ptr<T>(shared_ptr<T>, shared_ptr<T>)> f){
    shared_ptr<T> a = stream_1 != nullptr ? make_shared(stream_1->value) : nullptr;
    shared_ptr<T> b = stream_2 != nullptr ? make_shared(stream_1->value) : nullptr;
    shared_ptr<T> y = f(a, b);

    if(y==nullptr)
        return nullptr;

    return make_shared<StreamElem<T>>(
        *y,
        [=]()->shared_ptr<StreamElem<T>>{ return stream_join(stream_1->next(), stream_2->next(), f); }
    );
}

template<typename T>
shared_ptr<StreamElem<T>> stream_join(shared_ptr<StreamElem<T>> stream_1, shared_ptr<StreamElem<T>> stream_2, function<T(T,T)> f){
    if(stream_1 == nullptr || stream_2 == nullptr)
        return nullptr;

    return make_shared<StreamElem<T>>(
        f(stream_1->value, stream_2->value),
        [=]()->shared_ptr<StreamElem<T>>{ return stream_join(stream_1->next(), stream_2->next(), f); }
    );
}

template<typename T>
shared_ptr<StreamElem<T>> stream_get(shared_ptr<StreamElem<T>> stream, size_t i){
    while(stream != nullptr && i-- != 0)
        stream = stream->next();

    if(stream == nullptr)
        return nullptr;

    return stream;
}

template<typename T>
void stream_for_each(shared_ptr<StreamElem<T>> stream, function<void(T)> f){
    while(stream != nullptr){
        f(stream->value);
        stream = stream->next();
    }
}

template<typename T>
void stream_for_each(shared_ptr<StreamElem<T>> stream, function<void(T,size_t)> f){
    size_t i=0;
    while(stream != nullptr){
        f(stream->value, i++);
        stream = stream->next();
    }
}

template<typename T>
shared_ptr<StreamElem<T>> stream_limit(shared_ptr<StreamElem<T>> stream, size_t n){
    if(stream == nullptr || n == 0)
        return nullptr;

    return make_shared<StreamElem<T>>(stream->value, [=](){return n == 0 ? nullptr : stream_limit(stream->next(), n-1);});
}

template<typename T, int ... inits>
class Iterate{
    deque<T> init_values;
    function<T(deque<T>, size_t index)> f;
    size_t index;

public:

    Iterate(deque<T> values, function<T(deque<T>, size_t index)> f, size_t index)
     : init_values{values}, f{f}, index{index} {}

    Iterate(function<T(size_t)> f) :
        init_values{inits...},
        f{[f](deque<T> deq, size_t index)->T{return f(index);}}, index{0} {}

    Iterate(function<T(deque<T>)> f) :
        init_values{inits...},
        f{[f](deque<T> deq, size_t index)->T{return f(deq);}}, index{0} {}

    Iterate(function<T(deque<T>, size_t)> f) :
        init_values{inits...},
        f{f}, index{0} {}

    shared_ptr<StreamElem<T>> operator() (){
        T y;
        deque<T> values{init_values};

        if(index<init_values.size()){
            y = init_values[index];
        }else{
            y = f(init_values, index);
            values.pop_front();
            values.push_back(y);
        }

        return make_shared<StreamElem<T>>(y, Iterate<T>{values, f, index+1});
    }
};

Iterate<int> naturalNumbers = Iterate<int>{[](size_t i){return i+2;}};

class PrimeIterate{
    size_t index;

public:
    PrimeIterate(size_t index=2) : index{index} {}

    shared_ptr<StreamElem<int>> operator() (){
        int i = index;
        return stream_filter<int>(make_shared<StreamElem<int>>(index, PrimeIterate{index+1}), [i](int x){return x==i || x%i != 0;});
    }
};

template<typename T, T ... ARGS>
class Stream{
    shared_ptr<StreamElem<T>> stream;
    Stream(shared_ptr<StreamElem<T>> stream): stream{stream} {}
public:
    //0->inf
    Stream() : stream{Iterate<int>{[](size_t i){return i;}}} {}

    //Inicializálás template-el
    Stream(function<T(deque<T>, size_t)> f)
     : stream{Iterate<T, ARGS...>{f}()}{}
    Stream(function<T(deque<T>)> f)
     : stream{Iterate<T, ARGS...>{f}()}{}
    Stream(function<T(size_t)> f)
     : stream{Iterate<T, ARGS...>{f}()}{}

    //Inicializálás listával
    Stream(std::initializer_list<T> init, function<T(deque<T>, size_t)> f)
     : stream{Iterate<T>{init, f, 0}()}{}
    Stream(std::initializer_list<T> init, function<T(deque<T>)> f)
     : stream{Iterate<T>{init, [f](deque<T> t, size_t i){return f(t);}, 0}()}{}
    Stream(std::initializer_list<T> init, function<T(size_t)> f)
     : stream{Iterate<T>{init, [f](deque<T> t, size_t i){return f(i);}, 0}()}{}

    Stream filter(function<bool(T)> f){
        return stream_filter<T>(stream, f);
    }

    shared_ptr<T> get(size_t index){
        auto v = stream_get(stream, index);
        if(v==nullptr)
            return nullptr;
        return make_shared<T>(v->value);
    }

    Stream skip(size_t n){
        return stream_get<T>(stream, n);
    }

    Stream map(function<T(T)> f){
        return stream_map<T>(stream, f);
    }

    Stream join(const Stream& stream, function<shared_ptr<T>(shared_ptr<T>,shared_ptr<T>)> f){
        return stream_join<T>(this->stream, stream.stream, f);;
    }

    Stream join(const Stream& stream, function<T(T,T)> f){
        return stream_join<T>(this->stream, stream.stream, f);
    }

    Stream limit(size_t n){
        return stream_limit(stream, n);
    }

    void for_each(function<void(T)> f){
        stream_for_each(stream, f);
    }

    void for_each(function<void(T, size_t)> f){
        stream_for_each(stream, f);
    }
};