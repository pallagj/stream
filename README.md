# stream
You can create infinity streams with this `C++ lib`. It is very easy to insert it to your project.

## Available functions
| Name         | Parameters                           | Description                                    |
|:------------ | ------------------------------------ | ---------------------------------------------- |
| **filter**   | `function<bool(T)>` f                | Filter the values, where the `f` is true       |
| **get**      | `size_t` index                       | Get the `index`th value                        |
| **skip**     | `size_t` n                           | Skip first `n` value                           |    
| **map**      | `function<T(T)>`  f                  | Change every value to other with `f` function  |
| **join**     | `Stream& stream, function<T(T,T)>` f | Join two `stream` together                     |
| **limit**    | `size_t` n                           | Get the first n value                          |
| **for_each** | `function<void(T)>` f                | Do for each value                              |  
| **for_each** | `function<void(T, size_t)>` f        | Do for each value with index                   |

## Examples

### Create a fibonacci sequence
You can create an infinity sequence with any start value, and you can give the connection to next value with a function.
```c++
Stream<int, 1, 1> fib_stream_1{
    [](deque<int> t){ 
        return t[0] + t[1]; 
    }
};
```

### Print first ten even fibonacci numbers devided by two
You can use `filter`, `map`, `limit`, or `for_each`.
```c++
fib_stream_1
    .filter([](int a){ 
        return a % 2 == 0; //filter even numbers
    })  
    .map([](int a){ 
        return a / 2;      //deviding all values by two
    }) 
    .limit(10)               //get the first 10 values
    .for_each([](int a){
        std::cout << a << std::endl; //print
    }
);
```

### Create n! stream
You can either not only use the previous list to determine the new value, but you can also use the index.
```c++
Stream<int> factorial_stream{
    {1},
    [](deque<int> t, size_t i){
        return t[0]*(i+1);
    }
};
```

### Let's print the first five value with index
You can use the index in the `for_each` too.
```c++
factorial_stream
    .limit(5)
    .for_each([](int x, size_t i){
        std::cout << i+1 << "!: " << x << std::endl;
    }
);
```

### Create !n stream

```c++
Stream<int> subfactorial_stream{
    {0, 1}, 
    [](deque<int> t, size_t i){
        return (t[0]+t[1])*i;
    }
};
```
### Find the euler number
We know that the lim n->inf n!/!n = e, so we can find this with our previous streams.
```c++
subfactorial_stream
    .skip(1)
    .join(
        factorial_stream
            .skip(1), 
        [](int a, int b) {
            return 10000 * b / a;
        })
    .limit(7)
    .for_each(
        [](int x){
        std::cout << x/10000.0  << std::endl;
    }
);
```
