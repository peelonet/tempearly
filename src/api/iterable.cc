#include "interpreter.h"
#include "api/list.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    /**
     * Iterable#first([default_value]) => Object
     *
     * Returns first element from iteration.
     *
     * Throws: StateError - If iteration is empty and no default value is
     * given.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_first)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Value element;

            if (iterator.GetNext(interpreter, element))
            {
                return element;
            }
            else if (!interpreter->HasException())
            {
                if (args.size() > 1)
                {
                    return args[1];
                }
                interpreter->Throw(interpreter->eStateError, "Iteration is empty");
            }
        }

        return Value();
    }

    /**
     * Iterable#last([default_value]) => Object
     *
     * Returns last element from iteration.
     *
     * Throws: StateError - If iteration is empty and no default value is
     * given.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_last)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");
        Value element;

        if (!iterator)
        {
            return Value();
        }
        else if (iterator.GetNext(interpreter, element))
        {
            while (iterator.GetNext(interpreter, element));
            if (!interpreter->HasException())
            {
                return element;
            }
        }
        else if (!interpreter->HasException())
        {
            if (args.size() > 1)
            {
                return args[1];
            }
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }

        return Value();
    }

    /**
     * Iterable#single([default_value]) => Object
     *
     * Returns single element from iteration.
     *
     * Throws: StateError - If iteration is empty and no default value is given
     * or if the iteration contains more than one element.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_single)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");
        Value element;

        if (!iterator)
        {
            return Value();
        }
        if (iterator.GetNext(interpreter, element))
        {
            Value remaining;

            if (iterator.GetNext(interpreter, remaining))
            {
                interpreter->Throw(interpreter->eStateError, "Iteration contains more than one element");

                return Value();
            }
            if (!interpreter->HasException())
            {
                return element;
            }
        }
        else if (!interpreter->HasException())
        {
            if (args.size() > 1)
            {
                return args[1];
            }
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }

        return Value();
    }

    /**
     * Iterable#max()                       => Object
     * Iterable#max(function(max, element)) => Object
     *
     * Determines the maximum value from elements in the iteration by using
     * "__gt__" method. If the iteration is empty, StateError is thrown.
     *
     * If optional function is given, it is used for comparison instead of the
     * "__gt__" method.
     *
     *     [3, 9, 6].max()  #=> 9
     *
     * Throws: StateError - If the iteration is empty.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_max)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Value max;

            if (iterator.GetNext(interpreter, max))
            {
                Value element;
                Value result;
                std::vector<Value> args2;

                if (args.size() < 2)
                {
                    bool b;

                    args2.reserve(1);
                    while (iterator.GetNext(interpreter, element))
                    {
                        args2.assign(1, max);
                        if (!(result = element.Call(interpreter, "__gt__", args2))
                            || !result.AsBool(interpreter, b))
                        {
                            return Value();
                        }
                        else if (b)
                        {
                            max = element;
                        }
                    }
                } else {
                    i64 i;

                    args2.reserve(2);
                    while (iterator.GetNext(interpreter, element))
                    {
                        args2.clear();
                        args2.push_back(max);
                        args2.push_back(element);
                        if (!(result = args[1].Call(interpreter, "__call__", args2))
                            || !result.AsInt(interpreter, i))
                        {
                            return Value();
                        }
                        else if (i > 0)
                        {
                            max = element;
                        }
                    }
                }
                if (!interpreter->HasException())
                {
                    return max;
                }
            }
            else if (!interpreter->HasException())
            {
                interpreter->Throw(interpreter->eStateError, "Iteration is empty");
            }
        }

        return Value();
    }

    /**
     * Iterable#min()                       => Object
     * Iterable#min(function(min, element)) => Object
     *
     * Determines minimum value from elements in the iteration by using the
     * "__lt__" method. If iteration is empty, StateError is thrown.
     *
     * If optional function is given, it is used for comparison instead of the
     * "__lt__" method.
     *
     *     [6, 3, 9].min()  #=> 3
     *
     * Throws: StateError - If the iteration is empty.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_min)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Value min;

            if (iterator.GetNext(interpreter, min))
            {
                Value element;
                Value result;
                std::vector<Value> args2;

                if (args.size() < 2)
                {
                    bool b;

                    args2.reserve(1);
                    while (iterator.GetNext(interpreter, element))
                    {
                        args2.assign(1, min);
                        if (!(result = element.Call(interpreter, "__lt__", args2))
                            || !result.AsBool(interpreter, b))
                        {
                            return Value();
                        }
                        else if (b)
                        {
                            min = element;
                        }
                    }
                } else {
                    i64 i;

                    args2.reserve(2);
                    while (iterator.GetNext(interpreter, element))
                    {
                        args2.clear();
                        args2.push_back(min);
                        args2.push_back(element);
                        if (!(result = args[1].Call(interpreter, "__call__", args2))
                            || !result.AsInt(interpreter, i))
                        {
                            return Value();
                        }
                        else if (i < 0)
                        {
                            min = element;
                        }
                    }
                }
                if (!interpreter->HasException())
                {
                    return min;
                }
            }
            else if (!interpreter->HasException())
            {
                interpreter->Throw(interpreter->eStateError, "Iteration is empty");
            }
        }

        return Value();
    }

    /**
     * Iterable#avg()                           #=> Object
     * Iterable#avg(function(sum, element))     #=> Object
     *
     * Calculates average value from the elements contained in the collection.
     * This is done by calculating sum of the elements with "__add__" method
     * and then dividing that sum with the number of elements contained in the
     * collection with "__div__" method. If the collection is empty, StateError
     * is thrown instead.
     *
     * If optional function is given, value returned from that function is used
     * instead of "__add__" method for calculating sum of the elements.
     *
     * Throws: StateError - If the collection is empty.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_avg)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");
        Value element;

        if (!iterator)
        {
            return Value();
        }
        if (iterator.GetNext(interpreter, element))
        {
            u64 count = 1;
            Value sum = element;
            std::vector<Value> args2;

            if (args.size() < 2)
            {
                args2.reserve(1);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.assign(1, element);
                    if (!(sum = sum.Call(interpreter, "__add__", args2)))
                    {
                        return Value();
                    }
                    ++count;
                }
            } else {
                args2.reserve(2);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.clear();
                    args2.push_back(sum);
                    args2.push_back(element);
                    if (!(sum = args[1].Call(interpreter, "__call__", args2)))
                    {
                        return Value();
                    }
                    ++count;
                }
            }
            if (!interpreter->HasException())
            {
                args2.clear();
                args2.push_back(Value::NewInt(count));

                return sum.Call(interpreter, "__div__", args2);
            }
        }
        else if (!interpreter->HasException())
        {
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }

        return Value();
    }

    /**
     * Iterable#sum()                       => Object
     * Iterable#sum(function(sum, element)) => Object
     *
     * Calculates sum of the elements by concatenating them with the "_add__"
     * method. If the iteration is empty, StateError is thrown.
     *
     * If optional function is given, sum is calculated from values returned by
     * that function instead of the "__add__" method.
     *
     *     [3, 6, 9].sum()                        #=> 18
     *     [3, 6, 9].sum(function(a, b) => a * b) #=> 162
     *
     * Throws: StateError - If the iteration is empty.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_sum)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");
        Value element;

        if (!iterator)
        {
            return Value();
        }
        if (iterator.GetNext(interpreter, element))
        {
            Value sum = element;
            std::vector<Value> args2;

            if (args.size() < 2)
            {
                args2.reserve(1);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.assign(1, element);
                    if (!(sum = sum.Call(interpreter, "__add__", args2)))
                    {
                        return Value();
                    }
                }
            } else {
                args2.reserve(2);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.clear();
                    args2.push_back(sum);
                    args2.push_back(element);
                    if (!(sum = args[1].Call(interpreter, "__call__", args2)))
                    {
                        return Value();
                    }
                }
            }
            if (!interpreter->HasException())
            {
                return sum;
            }
        }
        else if (!interpreter->HasException())
        {
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }

        return Value();
    }

    /**
     * Iterable#all(function(element)) => Bool
     *
     * Returns true if the given function returns true for every element in the
     * iteration, false otherwise.
     *
     *     ["ant", "bear", "cat"].all(function(w) => w.length() >= 3) #=> true
     *     ["ant", "bear", "cat"].all(function(w) => w.length() >= 4) #=> false
     */
    TEMPEARLY_NATIVE_METHOD(iterable_all)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");
        Value element;

        if (!iterator)
        {
            return Value();
        }
        if (iterator.GetNext(interpreter, element))
        {
            do
            {
                Value result = args[1].Call(interpreter, "__call__", element);
                bool b;

                if (!result || !result.AsBool(interpreter, b))
                {
                    return Value();
                }
                else if (!b)
                {
                    return Value::NewBool(false);
                }
            }
            while (iterator.GetNext(interpreter, element));
            if (interpreter->HasException())
            {
                return Value();
            }

            return Value::NewBool(true);
        }
        else if (interpreter->HasException())
        {
            return Value();
        } else {
            return Value::NewBool(false);
        }
    }

    /**
     * Iterable#any(function(element)) => Bool
     *
     * Returns true if the given function returns true for at least one element
     * in the iteration.
     *
     *     ["ant", "bear", "cat"].any(function(w) => w.length() >= 3) #=> true
     *     ["ant", "bear", "cat"].any(function(w) => w.length() >= 4) #=> true
     */
    TEMPEARLY_NATIVE_METHOD(iterable_any)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");
        Value element;

        if (!iterator)
        {
            return Value();
        }
        if (iterator.GetNext(interpreter, element))
        {
            do
            {
                Value result = args[1].Call(interpreter, "__call__", element);
                bool b;

                if (!result || !result.AsBool(interpreter, b))
                {
                    return Value();
                }
                else if (b)
                {
                    return Value::NewBool(true);
                }
            }
            while (iterator.GetNext(interpreter, element));
        }
        if (interpreter->HasException())
        {
            return Value();
        }

        return Value::NewBool(false);
    }

    /**
     * Iterable#each(function(element))
     *
     * Iterates over each element in the iteration while passing them to the
     * given function.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_each)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                if (!args[1].Call(interpreter, "__call__", element))
                {
                    return Value();
                }
            }
            if (!interpreter->HasException())
            {
                return args[0];
            }
        }

        return Value();
    }

    /**
     * Iterable#filter(function(element)) => List
     *
     * Returns an list which contains elements from iteration for which the given
     * function returned true.
     *
     *     (1..4).filter(function(i) => i % 2 == 0) #=> [2, 4]
     */
    TEMPEARLY_NATIVE_METHOD(iterable_filter)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Handle<ListObject> list = new ListObject(interpreter->cList);
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                Value result = args[1].Call(interpreter, "__call__", element);
                bool b;

                if (!result || !result.AsBool(interpreter, b))
                {
                    return Value();
                }
                else if (b)
                {
                    list->Append(element);
                }
            }
            if (!interpreter->HasException())
            {
                return Value::NewObject(list);
            }
        }

        return Value();
    }

    /**
     * Iterable#grep(pattern) => List
     *
     * Returns a list containing elements from iteration for which the given
     * pattern returns true when invoked with "__case__" method.
     *
     *     (1..100).grep(38..44) #=> [38, 39, 40, 41, 42, 43, 44]
     */
    TEMPEARLY_NATIVE_METHOD(iterable_grep)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Value element;
            Handle<ListObject> list = new ListObject(interpreter->cList);

            while (!iterator.GetNext(interpreter, element))
            {
                Value result = args[1].Call(interpreter, "__case__", element);
                bool b;

                if (!result || !result.AsBool(interpreter, b))
                {
                    return Value();
                }
                else if (b)
                {
                    list->Append(element);
                }
            }
            if (!interpreter->HasException())
            {
                return Value::NewObject(list);
            }
        }

        return Value();
    }

    /**
     * Iterable#has(object) => Bool
     *
     * Tests whether the iteration contains object given as argument. Equality
     * is tested with "__eq__" method.
     *
     *     [1, 2, 3, 4].has(3)  #=> true
     *     [1, 2, 3, 4].has(5)  #=> false
     */
    TEMPEARLY_NATIVE_METHOD(iterable_has)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                bool b;

                if (args[1].Equals(interpreter, element, b))
                {
                    if (b)
                    {
                        return Value::NewBool(true);
                    }
                } else {
                    return Value();
                }
            }
            if (!interpreter->HasException())
            {
                return Value::NewBool(false);
            }
        }

        return Value();
    }

    /**
     * Iterable#join(separator = ", ") => String
     *
     * Returns a string created by converting each element of the iteration into a
     * string separated by the given separator.
     *
     *     ["a", "b", "c"].join()       #=> "a, b, c"
     *     ["a", "b", "c"].join("-")    #=> "a-b-c"
     */
    TEMPEARLY_NATIVE_METHOD(iterable_join)
    {
        StringBuilder buffer;
        String separator;

        if (args.size() < 2)
        {
            separator = ", ";
        }
        else if (args.size() == 2)
        {
            if (!args[1].AsString(interpreter, separator))
            {
                return Value();
            }
        } else {
            interpreter->Throw(interpreter->eValueError, "Too many arguments");

            return Value();
        }
        if (!args[0].HasFlag(CountedObject::FLAG_INSPECTING))
        {
            Value iterator;
            Value element;
            bool first = true;

            args[0].SetFlag(CountedObject::FLAG_INSPECTING);
            if (!(iterator = args[0].Call(interpreter, "__iter__")))
            {
                args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);

                return Value();
            }
            while (iterator.GetNext(interpreter, element))
            {
                String repr;

                if (!element.ToString(interpreter, repr))
                {
                    args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);

                    return Value();
                }
                if (first)
                {
                    first = false;
                } else {
                    buffer.Append(separator);
                }
                buffer.Append(repr);
            }
            args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);
            if (interpreter->HasException())
            {
                return Value();
            }
        }

        return Value::NewString(buffer.ToString());
    }

    /**
     * Iterable#map(function(element)) => List
     *
     * Returns a list where each element of iteration is replaced by result of the
     * given function.
     *
     *     [1, 2, 3].map(function(i) => i * i) #=> [1, 4, 9]
     */
    TEMPEARLY_NATIVE_METHOD(iterable_map)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Handle<ListObject> list = new ListObject(interpreter->cList);
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                Value result = args[1].Call(interpreter, "__call__", element);

                if (!result)
                {
                    return Value();
                }
                list->Append(result);
            }
            if (!interpreter->HasException())
            {
                return Value::NewObject(list);
            }
        }

        return Value();
    }

    static bool quicksort(const Handle<Interpreter>& interpreter,
                          std::vector<Value>& vector,
                          const std::size_t offset,
                          const std::size_t slice_size)
    {
        std::size_t left;
        std::size_t right;
        std::size_t pivot;
        int cmp;

        if (slice_size < 2)
        {
            return true;
        }

        left = offset;
        right = slice_size - 1 + offset;
        pivot = slice_size / 2 + offset;

        if (pivot != right)
        {
            std::swap(vector[pivot], vector[right]);
            pivot = right;
            --right;
        }

        while (left < right)
        {
            if (!vector[left].Compare(interpreter, vector[pivot], cmp))
            {
                return false;
            }
            else if (cmp > 0)
            {
                if (!vector[right].Compare(interpreter, vector[pivot], cmp))
                {
                    return false;
                }
                else if (cmp < 0)
                {
                    std::swap(vector[left++], vector[right--]);
                } else {
                    --right;
                }
            } else {
                ++left;
            }
        }

        left = offset;

        while (left - offset < slice_size - 1)
        {
            if (!vector[pivot].Compare(interpreter, vector[left], cmp))
            {
                return false;
            }
            else if (cmp < 0)
            {
                std::swap(vector[pivot], vector[left]);
                break;
            }
            ++left;
        }

        // Lower and upper slice
        return quicksort(interpreter, vector, offset, left++ - offset)
            && quicksort(interpreter, vector, left, slice_size - left - offset);
    }

    static bool quicksort_callback(const Handle<Interpreter>& interpreter,
                                   std::vector<Value>& vector,
                                   const std::size_t offset,
                                   const std::size_t slice_size,
                                   const Value& function)
    {
        std::size_t left;
        std::size_t right;
        std::size_t pivot;
        std::vector<Value> args;
        Value result;
        i64 cmp;

        if (slice_size < 2)
        {
            return true;
        }

        args.reserve(2);
        left = offset;
        right = slice_size - 1 + offset;
        pivot = slice_size / 2 + offset;

        if (pivot != right)
        {
            std::swap(vector[pivot], vector[right]);
            pivot = right;
            --right;
        }

        while (left < right)
        {
            args.clear();
            args.push_back(vector[left]);
            args.push_back(vector[pivot]);
            if (!(result = function.Call(interpreter, "__call__", args)) || !result.AsInt(interpreter, cmp))
            {
                return false;
            }
            else if (cmp > 0)
            {
                args.clear();
                args.push_back(vector[right]);
                args.push_back(vector[pivot]);
                if (!(result = function.Call(interpreter, "__call__", args)) || !result.AsInt(interpreter, cmp))
                {
                    return false;
                }
                else if (cmp < 0)
                {
                    std::swap(vector[left++], vector[right--]);
                } else {
                    --right;
                }
            } else {
                ++left;
            }
        }

        left = offset;

        while (left - offset < slice_size - 1)
        {
            args.clear();
            args.push_back(vector[pivot]);
            args.push_back(vector[left]);
            if (!(result = function.Call(interpreter, "__call__", args)) || !result.AsInt(interpreter, cmp))
            {
                return false;
            }
            else if (cmp < 0)
            {
                std::swap(vector[pivot], vector[left]);
                break;
            }
            ++left;
        }

        // Lower and upper slice
        return quicksort_callback(interpreter, vector, offset, left++ - offset, function)
            && quicksort_callback(interpreter, vector, left, slice_size - left - offset, function);
    }

    /**
     * Iterable#sort([function(element1, element2)]) => List
     *
     * Creates a list which has all elements from the iteration in order sorted
     * by the "__cmp__" method. An optional function can be given as argument
     * which is used for comparison instead of the "__cmp__" method.
     *
     *     [3, 2, 1].sort();    #=> [1, 2, 3]
     */
    TEMPEARLY_NATIVE_METHOD(iterable_sort)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Value element;
            std::vector<Value> vector;

            while (iterator.GetNext(interpreter, element))
            {
                vector.push_back(element);
            }
            if (!interpreter->HasException())
            {
                if (args.size() < 2)
                {
                    if (quicksort(interpreter, vector, 0, vector.size()))
                    {
                        return Value::NewObject(new ListObject(interpreter->cList, vector));
                    }
                }
                else if (quicksort_callback(interpreter, vector, 0, vector.size(), args[1]))
                {
                    return Value::NewObject(new ListObject(interpreter->cList, vector));
                }
            }
        }

        return Value();
    }

    /**
     * Iterable#split(function(element)) => List
     *
     * Returns list of two lists, the first containing the elements from
     * iteration which the given function evaluates as true and the second
     * containing the rest.
     *
     *     (1..6).split(function(i) => i & 1 == 0)
     *
     * Produces:
     *     [[2, 4, 6], [1, 3, 5]]
     */
    TEMPEARLY_NATIVE_METHOD(iterable_split)
    {
        Value iterator = args[0].Call(interpreter, "__iter__");

        if (iterator)
        {
            Handle<ListObject> result = new ListObject(interpreter->cList);
            Handle<ListObject> a = new ListObject(interpreter->cList);
            Handle<ListObject> b = new ListObject(interpreter->cList);
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                Value result = args[1].Call(interpreter, "__call__", element);
                bool slot;

                if (!result || !result.AsBool(interpreter, slot))
                {
                    return Value();
                }
                else if (slot)
                {
                    a->Append(element);
                } else {
                    b->Append(element);
                }
            }
            if (!interpreter->HasException())
            {
                result->Append(Value::NewObject(a));
                result->Append(Value::NewObject(b));

                return Value::NewObject(result);
            }
        }

        return Value();
    }

    /**
     * Iterable#take(count) => Iterator
     *
     * Returns an iterator which contains at most +count+ elements from the
     * iteration. Returned iterator might contain fewer elements if the
     * iteration contains fewer than +count+ elements.
     *
     * Throws: ValueError - If +count+ is negative.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_take)
    {
        i64 count;
        Value iterator;
        Value element;
        
        if (!args[1].AsInt(interpreter, count))
        {
            return Value();
        }
        else if (count < 0)
        {
            interpreter->Throw(interpreter->eValueError, "Negative count");

            return Value();
        }
        else if (!(iterator = args[0].Call(interpreter, "__iter__")))
        {
            return Value();
        }
        while (count-- > 0)
        {
            if (!iterator.GetNext(interpreter, element))
            {
                if (interpreter->HasException())
                {
                    return Value();
                } else {
                    return iterator;
                }
            }
        }

        return iterator;
    }

    void init_iterable(Interpreter* i)
    {
        i->cIterable = i->AddClass("Iterable", i->cObject);

        i->cIterable->AddMethod(i, "first", -1, iterable_first);
        i->cIterable->AddMethod(i, "last", -1, iterable_last);
        i->cIterable->AddMethod(i, "single", -1, iterable_single);

        i->cIterable->AddMethod(i, "max", -1, iterable_max);
        i->cIterable->AddMethod(i, "min", -1, iterable_min);
        i->cIterable->AddMethod(i, "avg", -1, iterable_avg);
        i->cIterable->AddMethod(i, "sum", -1, iterable_sum);

        i->cIterable->AddMethod(i, "all", 1, iterable_all);
        i->cIterable->AddMethod(i, "any", 1, iterable_any);
        i->cIterable->AddMethod(i, "each", 1, iterable_each);
        i->cIterable->AddMethod(i, "filter", 1, iterable_filter);
        i->cIterable->AddMethod(i, "grep", 1, iterable_grep);
        i->cIterable->AddMethod(i, "has", 1, iterable_has);
        i->cIterable->AddMethod(i, "join", -1, iterable_join);
        i->cIterable->AddMethod(i, "map", 1, iterable_map);
        i->cIterable->AddMethod(i, "sort", -1, iterable_sort);

        i->cIterable->AddMethod(i, "split", 1, iterable_split);
        i->cIterable->AddMethod(i, "take", 1, iterable_take);
    }
}
