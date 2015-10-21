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
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        else if (iterator.GetNext(interpreter, element))
        {
            frame->SetReturnValue(element);
        }
        else if (!interpreter->HasException())
        {
            if (args.GetSize() > 1)
            {
                frame->SetReturnValue(args[1]);
            } else {
                interpreter->Throw(interpreter->eStateError, "Iteration is empty");
            }
        }
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
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        else if (iterator.GetNext(interpreter, element))
        {
            while (iterator.GetNext(interpreter, element));
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(element);
            }
        }
        else if (!interpreter->HasException())
        {
            if (args.GetSize() > 1)
            {
                frame->SetReturnValue(args[1]);
            } else {
                interpreter->Throw(interpreter->eStateError, "Iteration is empty");
            }
        }
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
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        else if (iterator.GetNext(interpreter, element))
        {
            Value remaining;

            if (iterator.GetNext(interpreter, remaining))
            {
                interpreter->Throw(interpreter->eStateError, "Iteration contains more than one element");
                return;
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(element);
            }
        }
        else if (!interpreter->HasException())
        {
            if (args.GetSize() > 1)
            {
                frame->SetReturnValue(args[1]);
            } else {
                interpreter->Throw(interpreter->eStateError, "Iteration is empty");
            }
        }
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
        Value iterator;
        Value max;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        if (iterator.GetNext(interpreter, max))
        {
            Value element;
            Value result;
            Vector<Value> args2;

            if (args.GetSize() < 2)
            {
                bool b;

                args2.Reserve(1);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(max);
                    if (!element.CallMethod(interpreter, result, "__gt__", args2)
                        || !result.AsBool(interpreter, b))
                    {
                        return;
                    }
                    else if (b)
                    {
                        max = element;
                    }
                }
            } else {
                i64 i;

                args2.Reserve(2);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(max);
                    args2.PushBack(element);
                    if (!args[1].CallMethod(interpreter, result, "__call__", args2)
                        || !result.AsInt(interpreter, i))
                    {
                        return;
                    }
                    else if (i > 0)
                    {
                        max = element;
                    }
                }
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(max);
            }
        }
        else if (!interpreter->HasException())
        {
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }
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
        Value iterator;
        Value min;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        if (iterator.GetNext(interpreter, min))
        {
            Value element;
            Value result;
            Vector<Value> args2;

            if (args.GetSize() < 2)
            {
                bool b;

                args2.Reserve(1);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(min);
                    if (!element.CallMethod(interpreter, result, "__lt__", args2)
                        || !result.AsBool(interpreter, b))
                    {
                        return;
                    }
                    else if (b)
                    {
                        min = element;
                    }
                }
            } else {
                i64 i;

                args2.Reserve(2);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(min);
                    args2.PushBack(element);
                    if (!args[1].CallMethod(interpreter, result, "__call__", args2)
                        || !result.AsInt(interpreter, i))
                    {
                        return;
                    }
                    else if (i < 0)
                    {
                        min = element;
                    }
                }
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(min);
            }
        }
        else if (!interpreter->HasException())
        {
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }
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
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        if (iterator.GetNext(interpreter, element))
        {
            u64 count = 1;
            Value sum = element;
            Vector<Value> args2;

            if (args.GetSize() < 2)
            {
                args2.Reserve(1);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(element);
                    if (!sum.CallMethod(interpreter, sum, "__add__", args2))
                    {
                        return;
                    }
                    ++count;
                }
            } else {
                args2.Reserve(2);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(sum);
                    args2.PushBack(element);
                    if (!args[1].CallMethod(interpreter, sum, "__call__", args2))
                    {
                        return;
                    }
                    ++count;
                }
            }
            if (!interpreter->HasException())
            {
                Value result;

                args2.Clear();
                args2.PushBack(Value::NewInt(count));
                if (sum.CallMethod(interpreter, result, "__div__", args2))
                {
                    frame->SetReturnValue(result);
                }
            }
        }
        else if (!interpreter->HasException())
        {
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }
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
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        else if (iterator.GetNext(interpreter, element))
        {
            Value sum = element;
            Vector<Value> args2;

            if (args.GetSize() < 2)
            {
                args2.Reserve(1);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(element);
                    if (!sum.CallMethod(interpreter, sum, "__add__", args2))
                    {
                        return;
                    }
                }
            } else {
                args2.Reserve(2);
                while (iterator.GetNext(interpreter, element))
                {
                    args2.Clear();
                    args2.PushBack(sum);
                    args2.PushBack(element);
                    if (!args[1].CallMethod(interpreter, sum, "__call__", args2))
                    {
                        return;
                    }
                }
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(sum);
            }
        }
        else if (!interpreter->HasException())
        {
            interpreter->Throw(interpreter->eStateError, "Iteration is empty");
        }
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
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        else if (iterator.GetNext(interpreter, element))
        {
            do
            {
                Value result;
                bool b;

                if (!args[1].CallMethod(interpreter, result, "__call__", element)
                    || !result.AsBool(interpreter, b))
                {
                    return;
                }
                else if (!b)
                {
                    frame->SetReturnValue(Value::NewBool(false));
                    return;
                }
            }
            while (iterator.GetNext(interpreter, element));
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(Value::NewBool(true));
            }
        }
        else if (!interpreter->HasException())
        {
            frame->SetReturnValue(Value::NewBool(false));
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
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        else if (iterator.GetNext(interpreter, element))
        {
            do
            {
                Value result;
                bool b;

                if (!args[1].CallMethod(interpreter, result, "__call__", element)
                    || !result.AsBool(interpreter, b))
                {
                    return;
                }
                else if (b)
                {
                    frame->SetReturnValue(Value::NewBool(true));
                    return;
                }
            }
            while (iterator.GetNext(interpreter, element));
        }
        if (!interpreter->HasException())
        {
            frame->SetReturnValue(Value::NewBool(false));
        }
    }

    /**
     * Iterable#each(function(element))
     *
     * Iterates over each element in the iteration while passing them to the
     * given function.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_each)
    {
        Value iterator;
        Value element;

        if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        while (iterator.GetNext(interpreter, element))
        {
            if (!args[1].CallMethod(interpreter, "__call__", element))
            {
                return;
            }
        }
        if (!interpreter->HasException())
        {
            frame->SetReturnValue(args[0]);
        }
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
        Value iterator;

        if (args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            Handle<ListObject> list = new ListObject(interpreter->cList);
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                Value result;
                bool b;

                if (!args[1].CallMethod(interpreter, result, "__call__", element)
                    || !result.AsBool(interpreter, b))
                {
                    return;
                }
                else if (b)
                {
                    list->Append(element);
                }
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(Value(list));
            }
        }
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
        Value iterator;

        if (args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            Value element;
            Handle<ListObject> list = new ListObject(interpreter->cList);

            while (!iterator.GetNext(interpreter, element))
            {
                Value result;
                bool b;

                if (!args[1].CallMethod(interpreter, result, "__case__", element)
                    || !result.AsBool(interpreter, b))
                {
                    return;
                }
                else if (b)
                {
                    list->Append(element);
                }
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(Value(list));
            }
        }
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
        Value iterator;

        if (args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                bool b;

                if (args[1].Equals(interpreter, element, b))
                {
                    if (b)
                    {
                        frame->SetReturnValue(Value::NewBool(true));
                        return;
                    }
                } else {
                    return;
                }
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(Value::NewBool(false));
            }
        }
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

        if (args.GetSize() < 2)
        {
            separator = ", ";
        }
        else if (args.GetSize() == 2)
        {
            if (!args[1].AsString(interpreter, separator))
            {
                return;
            }
        } else {
            interpreter->Throw(interpreter->eValueError, "Too many arguments");
            return;
        }
        if (!args[0].HasFlag(CountedObject::FLAG_INSPECTING))
        {
            Value iterator;
            Value element;
            bool first = true;

            args[0].SetFlag(CountedObject::FLAG_INSPECTING);
            if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
            {
                args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);
                return;
            }
            while (iterator.GetNext(interpreter, element))
            {
                String repr;

                if (!element.ToString(interpreter, repr))
                {
                    args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);
                    return;
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
                return;
            }
        }
        frame->SetReturnValue(Value::NewString(buffer.ToString()));
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
        Value iterator;

        if (args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            Handle<ListObject> list = new ListObject(interpreter->cList);
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                Value result;

                if (!args[1].CallMethod(interpreter, result, "__call__", element))
                {
                    return;
                }
                list->Append(result);
            }
            if (!interpreter->HasException())
            {
                frame->SetReturnValue(Value(list));
            }
        }
    }

    static bool quicksort(const Handle<Interpreter>& interpreter,
                          Vector<Value>& vector,
                          const std::size_t offset,
                          const std::size_t slice_size)
    {
        std::size_t left;
        std::size_t right;
        std::size_t pivot;
        bool slot;

        if (slice_size < 2)
        {
            return true;
        }

        left = offset;
        right = slice_size - 1 + offset;
        pivot = slice_size / 2 + offset;

        if (pivot != right)
        {
            vector.Swap(pivot, right);
            pivot = right;
            --right;
        }

        while (left < right)
        {
            if (!vector[pivot].IsLessThan(interpreter, vector[left], slot))
            {
                return false;
            }
            else if (slot)
            {
                if (!vector[right].IsLessThan(interpreter, vector[pivot], slot))
                {
                    return false;
                }
                else if (slot)
                {
                    vector.Swap(left++, right--);
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
            if (!vector[pivot].IsLessThan(interpreter, vector[left], slot))
            {
                return false;
            }
            else if (slot)
            {
                vector.Swap(pivot, left);
                break;
            }
            ++left;
        }

        // Lower and upper slice
        return quicksort(interpreter, vector, offset, left++ - offset)
            && quicksort(interpreter, vector, left, slice_size - (left - offset));
    }

    static bool quicksort_callback(const Handle<Interpreter>& interpreter,
                                   Vector<Value>& vector,
                                   const std::size_t offset,
                                   const std::size_t slice_size,
                                   const Value& function)
    {
        std::size_t left;
        std::size_t right;
        std::size_t pivot;
        Vector<Value> args;
        Value result;
        bool slot;

        if (slice_size < 2)
        {
            return true;
        }

        args.Reserve(2);
        left = offset;
        right = slice_size - 1 + offset;
        pivot = slice_size / 2 + offset;

        if (pivot != right)
        {
            vector.Swap(pivot, right);
            pivot = right;
            --right;
        }

        while (left < right)
        {
            args.Clear();
            args.PushBack(vector[left]);
            args.PushBack(vector[pivot]);
            if (!function.CallMethod(interpreter, result, "__call__", args)
                || !result.AsBool(interpreter, slot))
            {
                return false;
            }
            else if (!slot)
            {
                args.Clear();
                args.PushBack(vector[right]);
                args.PushBack(vector[pivot]);
                if (!function.CallMethod(interpreter, result, "__call__", args)
                    || !result.AsBool(interpreter, slot))
                {
                    return false;
                }
                else if (slot)
                {
                    vector.Swap(left++, right--);
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
            args.Clear();
            args.PushBack(vector[pivot]);
            args.PushBack(vector[left]);
            if (!function.CallMethod(interpreter, result, "__call__", args)
                || !result.AsBool(interpreter, slot))
            {
                return false;
            }
            else if (slot)
            {
                vector.Swap(pivot, left);
                break;
            }
            ++left;
        }

        // Lower and upper slice
        return quicksort_callback(interpreter, vector, offset, left++ - offset, function)
            && quicksort_callback(interpreter, vector, left, slice_size - (left - offset), function);
    }

    /**
     * Iterable#sort([function(element1, element2)]) => List
     *
     * Creates a list which has all elements from the iteration in order sorted
     * with the "__lt__" method. An optional function can be given as argument
     * which is used for comparison instead of the "__lt__" method.
     *
     *     [3, 2, 1].sort();    #=> [1, 2, 3]
     */
    TEMPEARLY_NATIVE_METHOD(iterable_sort)
    {
        Value iterator;

        if (args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            Value element;
            Vector<Value> vector;

            while (iterator.GetNext(interpreter, element))
            {
                vector.PushBack(element);
            }
            if (!interpreter->HasException())
            {
                if (args.GetSize() < 2)
                {
                    if (quicksort(interpreter, vector, 0, vector.GetSize()))
                    {
                        Handle<ListObject> list = new ListObject(interpreter->cList);

                        list->Append(vector);
                        frame->SetReturnValue(Value(list));
                    }
                }
                else if (quicksort_callback(interpreter, vector, 0, vector.GetSize(), args[1]))
                {
                    Handle<ListObject> list = new ListObject(interpreter->cList);

                    list->Append(vector);
                    frame->SetReturnValue(Value(list));
                }
            }
        }
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
        Value iterator;

        if (args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            Handle<ListObject> result = new ListObject(interpreter->cList);
            Handle<ListObject> a = new ListObject(interpreter->cList);
            Handle<ListObject> b = new ListObject(interpreter->cList);
            Value element;

            while (iterator.GetNext(interpreter, element))
            {
                Value result;
                bool slot;

                if (!args[1].CallMethod(interpreter, result, "__call__", element)
                    || !result.AsBool(interpreter, slot))
                {
                    return;
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
                result->Append(Value(a));
                result->Append(Value(b));
                frame->SetReturnValue(Value(result));
            }
        }
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
            return;
        }
        else if (count < 0)
        {
            interpreter->Throw(interpreter->eValueError, "Negative count");
            return;
        }
        else if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
        {
            return;
        }
        while (count-- > 0)
        {
            if (!iterator.GetNext(interpreter, element))
            {
                if (!interpreter->HasException())
                {
                    frame->SetReturnValue(iterator);
                }
                return;
            }
        }
        frame->SetReturnValue(iterator);
    }

    /**
     * Iterable#as_json() => String
     *
     * Converts iterable object into JSON array literal and returns result.
     */
    TEMPEARLY_NATIVE_METHOD(iterable_as_json)
    {
        StringBuilder buffer;

        buffer << '[';
        if (!args[0].HasFlag(CountedObject::FLAG_INSPECTING))
        {
            Value iterator;
            Value element;
            bool first = true;

            args[0].SetFlag(CountedObject::FLAG_INSPECTING);
            if (!args[0].CallMethod(interpreter, iterator, "__iter__"))
            {
                args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);
                return;
            }
            while (iterator.GetNext(interpreter, element))
            {
                Value result;
                String json;

                if (!element.CallMethod(interpreter, result, "as_json")
                    || !result.AsString(interpreter, json))
                {
                    args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);
                    return;
                }
                if (first)
                {
                    first = false;
                } else {
                    buffer << ',';
                }
                buffer << json;
            }
            args[0].UnsetFlag(CountedObject::FLAG_INSPECTING);
            if (interpreter->HasException())
            {
                return;
            }
        }
        buffer << ']';
        frame->SetReturnValue(Value::NewString(buffer.ToString()));
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

        // Conversion methods
        i->cIterable->AddMethod(i, "as_json", 0, iterable_as_json);
    }
}
