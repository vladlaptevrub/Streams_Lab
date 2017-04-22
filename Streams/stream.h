#ifndef STREAM_H_
#define STREAM_H_

#include <vector>
#include <iostream>
#include <ostream>
#include <functional>

template<typename T> class Stream;

template<typename Iterator>
auto MakeStream(Iterator begin, Iterator end){
    std::vector<typename std::iterator_traits<Iterator>::value_type> cont;
    for (; begin < end; ++begin){
        cont.emplace_back(std::move(*begin));
    }
    std::cout << "MakeStream(Iterator begin, Iterator end) is called" << std::endl;
    return Stream<typename std::iterator_traits<Iterator>::value_type>(cont);
}

template<typename Container>
auto MakeStream(const Container& cont){
    std::cout << "MakeStream(const Container& cont) is called" << std::endl;
    return Stream<typename Container::value_type>(cont);
}

template<typename Container>
auto MakeStream(Container&& cont){
    std::cout << "MakeStream(Container&& cont) is called" << std::endl;
    return Stream<typename Container::value_type>(cont);
}

template<typename T>
auto MakeStream(std::initializer_list<T> init){
    std::cout << "MakeStream(std::initializer_list<T> init) is called" << std::endl;
    return Stream<T>(std::move(init));
}

template<typename T>
class operation{
public:
    operation(){};
    
    virtual std::vector<T> apply_to(std::vector<T>){}
};

template<typename T>
class Stream{
public:
    std::vector<T> stream;
    std::vector<operation<T>*> operations;

    Stream(std::vector<T> cont): stream(std::move(cont)){};

    Stream(std::vector<T> cont, std::vector<operation<T>*> op): stream(std::move(cont)), operations(op){};

    std::ostream& print_stream(std::ostream& os, const char* delimiter = " ") {
        std::vector<T> vec = std::move(stream);

        for (auto i = operations.begin(); i < operations.end(); ++i){
            vec = std::move((*i)->apply_to(std::move(vec)));
        }

        for (auto i = vec.begin(); i < vec.end(); ++i) {
            os << *i << delimiter;
        }
        return os;
	}

    template<typename Operation>
    auto operator| (Operation&& operation) -> decltype(operation.activate(this)){
        return operation.activate(this);
    }
};

template<typename Function, typename T>
class filter_op : public operation<T>{
private:
    Function func;

public:
    filter_op(Function&& f): func(std::forward<Function>(f)){};

    std::vector<T> apply_to(std::vector<T> stream){
        std::vector<T> init;
        
        for (auto i = stream.begin(); i < stream.end(); ++i){
            if (func(std::move(*i))){
                init.emplace_back(std::move(*i));
            }
        }

        return std::move(init);
    }
};

template<typename Function>
class filter_activator{
private:
Function func;

public:
    filter_activator(Function&& f): func(std::forward<Function>(f)){};

    template<typename T>
    Stream<T> activate(Stream<T>* stream){

        stream->operations.push_back(new filter_op<Function, T>(std::move(func)));

        return Stream<T>(std::move(stream->stream), std::move(stream->operations));
    }
};

template<typename Function>
auto filter(Function&& func){

    return filter_activator<Function>(std::move(func));
}

template<typename Function, typename T>
class map_op : public operation<T>{
private:
    Function func;

public:
    map_op(Function&& f): func(std::forward<Function>(f)){};

    std::vector<T> apply_to(std::vector<T> stream){
        std::vector<T> init;

        for(auto i = stream.begin(); i < stream.end(); ++i){
            init.emplace_back(func(std::move(*i)));
        }

        return move(init);
    }
};

template<typename Function>
class map_activator{
private:
Function func;

public:
    map_activator(Function&& f): func(std::forward<Function>(f)){};

    template<typename T>
    Stream<T> activate(Stream<T>* stream){

        stream->operations.push_back(new map_op<Function, T>(std::move(func)));

        return Stream<T>(std::move(stream->stream), std::move(stream->operations));
    }
};

template<typename Function>
auto map(Function&& func){
    return map_activator<Function>(std::move(func));
}

class sum{
public:
    sum(){};

    template<typename T>
    auto activate(Stream<T>* stream){
        std::vector<T> vec = std::move(stream->stream);

        for (auto i = stream->operations.begin(); i < stream->operations.end(); ++i){
            vec = std::move((*i)->apply_to(std::move(vec)));
        }

        auto i = vec.begin();
        T value = std::move(*i);
        i++;

        for (; i < vec.end(); ++i){
            value += *i;
        }

        return std::move(value);
    }
};

class nth{
private:
    int index;
    int step = 1;

public:
    nth(int i): index(i){};

    template<typename T>
    auto activate(Stream<T>* stream){
        std::vector<T> vec = std::move(stream->stream);

        for (auto i = stream->operations.begin(); i < stream->operations.end(); ++i){
            vec = std::move((*i)->apply_to(std::move(vec)));
        }

        if(vec.size() < index || index < 0){
            return 0;
        } else{
            auto value = vec.begin();
            while(step < index){
                value++;
                step++;
            }

            return std::move(*value);
        }
    }
};

template<typename T>
class skip_op : public operation<T>{
private:
    int amount;
    int step = 1;

public:
    skip_op(int i): amount(i){};

    std::vector<T> apply_to(std::vector<T> stream){
        std::vector<T> init;
        
        for(auto i = stream.begin(); i < stream.end(); ++i){
            if (amount < step){
                init.emplace_back(std::move(*i));
            }
            step++;
        }

        return std::move(init);
    }
};

class skip_activator{
private:
    int amount;
public:
    skip_activator(int a): amount(a){};

    template<typename T>
    Stream<T> activate(Stream<T>* stream){

        stream->operations.push_back(new skip_op<T>(amount));

        return Stream<T>(std::move(stream->stream), std::move(stream->operations));
    }
};

auto skip(int amount){
    return skip_activator(amount);
}

template<typename Function>
class reduce_op{
private:
    Function func;

public:
    reduce_op(Function&& f): func(std::forward<Function>(func)){};

    template<typename T>
    auto activate(Stream<T>* stream){
        std::vector<T> vec = std::move(stream->stream);

        for (auto i = stream->operations.begin(); i < stream->operations.end(); ++i){
            vec = std::move((*i)->apply_to(std::move(vec)));
        }

        std::vector<T> init;

        auto i = vec.begin();
        T value = std::move(*i);
        i++;

        for(; i < vec.end(); ++i){
            value = func(std::move(value), std::move(*i));
        }

        return std::move(value);
    }
};

template<typename Function>
auto reduce(Function&& func){
    return reduce_op<Function>(std::move(func));
}

class group{
private:
    int step;
public:
    group(int s): step(s){};

    template<typename T>
    Stream<std::vector<T>> activate(Stream<T>* stream){
        std::vector<T> vec = std::move(stream->stream);

        for (auto i = stream->operations.begin(); i < stream->operations.end(); ++i){
            vec = std::move((*i)->apply_to(std::move(vec)));
        }

        int vec_size = vec.size();

		std::vector<std::vector<T>> new_vec;

		auto i = vec.begin();

        while (i < vec.end()) {
            if (vec_size > step) {
                std::vector<T> part_vec;
                for (int a = 0; a < step; a++) {
                    part_vec.emplace_back(std::move(*i));
                    i++;
                }
                vec_size -= step;
                new_vec.emplace_back(std::move(part_vec));

            } else{
                step = vec_size;
                std::vector<T> part_vec;
                for (int a = 0; a < step; a++) {
                    part_vec.emplace_back(std::move(*i));
                    i++;
                }
                new_vec.emplace_back(std::move(part_vec));
            }
        }

        for (auto b = new_vec.begin(); b < new_vec.end(); ++b) {
            std::cout << "{";
            for (auto c = (*b).begin(); c < (*b).end(); ++c) {
                std::cout << *c << " ";
            }
            std::cout << "}" << " ";
        }

        return Stream<std::vector<T>>(new_vec);
    }
};

class to_vector{
public:
    to_vector(){};

    template<typename T>
    std::vector<T> activate(Stream<T>* stream){
        std::vector<T> vec = std::move(stream->stream);

        for (auto i = stream->operations.begin(); i < stream->operations.end(); ++i){
            vec = std::move((*i)->apply_to(std::move(vec)));
        }

        return std::move(vec);
    }
};

class print_to {
private:
	std::ostream& str;
	const char* del;
public:
	print_to(std::ostream& os, const char* delimiter = " ") : str(os), del(delimiter) {};

	template<typename T>
	std::ostream& activate(Stream<T>* stream) {
		return stream->print_stream(str, del);
	}
};

#endif