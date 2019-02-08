
# QDeferred

A Qt C++ alternative for handling async code execution, specially useful for threaded applications.

This library consists of two main helper classes:

* `QLambdaThreadWorker` : to execute code in a thread.

* `QDeferred` : to passing data between threads.

## Why?

Qt already provides a work flow for safely execute code in threads which can be found in [their documentation](https://doc.qt.io/qt-5/qthread.html#details), but it basically consists in:

* Create a "worker" object which implements the code to be executed in a thread in *Qt slots*. 

* Use *Qt signals* within the "worker" object to notify when the work is done.

* Create a "controller" object that owns an instance of the "worker" object and an instance of a `QThread`.

* Upon "controller" object instantiation **move** the "worker" object to the `QThread` instance and start the thread.

* Use *Qt signals* within the "controller" object to invoke the *Qt slots* in the "worker" object (which actually do the threaded work).

* Use *Qt slots* within the "controller" object to handle the results of the threaded work.

* Do to forget to quit the thread and wait for it in the "controller" object destructor.

I think this already explains the **why**, not only do we have to create two classes, but we have to remeber to implement all this signals and slots, connect them, move objects to threads, etc. And I almost forgot; is not like you can send any type using signals and slots, if you want so send a custom type you have to [do a lot of stuff to make your type Qt-compatible](https://doc.qt.io/qt-5/custom-types.html).

Man, I just want to execute some code in a thread!

## QLambdaThreadWorker

This class is the first of two parts of the proposed solution. To execute code in a thread we use `QLambdaThreadWorker` as follows:

```c++
#include <QCoreApplication>
#include <QDebug>
#include <QLambdaThreadWorker>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QLambdaThreadWorker worker;

	worker.execInThread([]() {
		qDebug() << "Hello world from worker thread" << QThread::currentThread();
	});

	qDebug() << "Hello world from main thread" << QThread::currentThread();

	return a.exec();
}
```

Possibe output:

```
Hello world from main thread QThread(0x1a1398cae10)
Hello world from worker thread QThread(0x1a1398cdc50)
```

There is no more to it, just give the `execInThread` method a lambda, and it will execute it in a thread.

The `QLambdaThreadWorker` handles just one thread which is **reusable**, let's call it thread *A*. This means that every time we call the `execInThread` method on the same `QLambdaThreadWorker` instance, we are exectuing code in the same thread *A*.

If we make a second `QLambdaThreadWorker` instance, all the calls to the `execInThread` method, will be executed in thread *B*. You get the idea.

Internally, `QLambdaThreadWorker` uses [Qt's event loop](https://wiki.qt.io/Threads_Events_QObjects), so all calls to the `execInThread` method are **serialized** in the order they were called. So all is safe and good Qt-wise.

Now you might be wondering : "Ok cool, but how to I pass data and retrieve data from the thread?". I am glad you asked!

To pass data data to the thread is easy, just pass it through the *lambda capture*:

```c++
QLambdaThreadWorker worker;

QString strName = "Laura";
// pass 'strName' through lambda capture by copy
worker.execInThread([strName]() {
	qDebug() << "Hello" << strName;
});
```

In the previous snippet we passed the string *by-copy*. We could also pass variables *by-reference*, but we would have to be careful that the variables still exist by the time the lambda is executed.

Note that we can pass **any type** though the lambda capture. There is no need to register our custom types in the Qt's system.

Now, to retrieve data from the thread we use the second part of the solution, the `QDeferred` class.

## QDeferred

The `QDeferred` represents the result of some async code execution, it also represents whether the execution of the code **suceeded** or **failed**.

The `QDeferred` API is divided in two:

* The **privider** API : the methods used by the *provider* (or producer) of the result.

* The **consumer** API : the methods used by the *consumer* of the result.

The way it works is that the *provider*, which might be just a simple function, returns a templated `QDeferred` instance to the *consumer*.

For example, if the result is an integer, the function signature would be as follows:

```c++
QDeferred<int> multiplyPositiveNumbers(int x, int y);
```

Note that we define the actual return type in the template argument `<int>`. We could put there any other type, without the need to register it in the Qt's system.

The actual implementation of that function, using the **provider** API would like something like this:

```c++
QDeferred<int> multiplyPositiveNumbers(int x, int y)
{
	QDeferred<int> retDefered;

	if (x < 0 || y < 0)
	{
		retDefered.reject(0);
	}
	else
	{
		retDefered.resolve(x * y);
	}

	return retDefered;
}
```

We use the `reject` method to indicate that the execution has **failed**, and we use the `resolve` method to indicate that the execution has **succeded**. As arguments to these methods we pass in the results.

Bear in mind that the arguments to the `reject` and `resolve` methods must match the template argument provided to the `QDeferred` object, else the compiler will complain.

Now we can use the **consumer** API to handle the result of calling the `multiplyPositiveNumbers` method. It would be as follows:

```c++
multiplyPositiveNumbers(3, 4)
.fail([](int res) {
	qDebug() << "multiplyPositiveNumbers failed!";
})
.done([](int res) {
	qDebug() << "multiplyPositiveNumbers succeded!";
});
```

The `multiplyPositiveNumbers` return the `QDeferred<int>` instance, so we can inmediatly call the `fail` method on it. We pass a lambda to the `fail` method which acts as a callback in case the execution failed.

The `fail` method itself returns the same `QDeferred<int>` instance which allows to call the `done` method inmediatly. The lambda passed to the `done` method acts as a callback in case the execution succeded.

The fact that both the `fail` and `done` methods return the same `QDeferred<T>` instance, allow us to **chain** the methods and call them in a row as many times as we want, which is some nice syntactic sugar to have.

In fact we can call the `fail` and `done` methods as many times as we want in a row:

```c++
multiplyPositiveNumbers(3, 4)
.done([](int res) {
	qDebug() << "multiplyPositiveNumbers succeded!";
})
.done([](int res) {
	qDebug() << 3 << "*" << 4 << "=" << res;
})
.done([](int res) {
	qDebug() << "Hell yeaaaahh!";
});
```

The output would be:

```c++
multiplyPositiveNumbers succeded!
3 * 4 = 12
Hell yeaaaahh!
```

If we change the first argument of `multiplyPositiveNumbers` to be a negative number:

```c++
multiplyPositiveNumbers(-3, 4)
.fail([](int res) {
	qDebug() << "multiplyPositiveNumbers failed!";
})
.done([](int res) {
	qDebug() << "multiplyPositiveNumbers succeded!";
});
```

The output will be:

```
multiplyPositiveNumbers failed!
```

Note that the callbacks passed in to the `fail` and `done` methods must be lambdas whose arguments match the types passed in the template argument of the `QDeferred`. For example if we create a new deferred object like:

```c++
QDeferred<int, QString> retDefered;
```

Then the `reject` and `resolve` methods would need to be called with the correct types `<int, QString>` like:

```c++ 
retDefered.resolve(3, "Hello");
```

And the `fail` and `done` lambdas should also have `<int, QString>` as arguments:

```c++ 
retDefered.done([](int num, QString string){
	// do something with the <int, QString> results
});
```

We can pass as many template arguments as we want to `QDeferred<...>`, potentially having any number of return values.

So far we have seen no advantage in using `QDeferred<T>` as a return argument of a function, it is actually much more cumbersome than just returning the resulting integer directly. The real value comes when using `QDeferred<T>` as return values of **threaded code execution**.

Let's assume I want to execute the `multiplyPositiveNumbers` in a thread. Using our `QLambdaThreadWorker` helper, we could implement it as follows:

```c++
QLambdaThreadWorker worker;

QDeferred<int> multiplyPositiveNumbers(int x, int y)
{
	QDeferred<int> retDefered;
	
	worker.execInThread([retDefered, x, y]() mutable {
		if (x < 0 || y < 0)
		{
			retDefered.reject(0);
		}
		else
		{
			retDefered.resolve(x * y);
		}
	});

	return retDefered;
}
```

There are two important things to realize from the previous code:

* The first one is that the `QLambdaThreadWorker` is outside the function, not in the stack. The reason is because if we put it on the stack, the `QLambdaThreadWorker` would disappear as soon as the function returns, without the chance to execute the threaded code.

* The second is the use of the `mutable` keyword in the lambda definition. This is because the lambda actually changes the `QDeferred<int>` state when calling the `reject` or `resolved` methods.

The *consumer* of this function would use it as follows:

```c++
int x = 3;
int y = 4;
multiplyPositiveNumbers(x, y)
.fail([](int res) {
	Q_UNUSED(res);
	qDebug() << "multiplyPositiveNumbers failed!";
})
.done([x, y](int res) {
	qDebug() << x << "*" << y << "=" << res;
});
```

The `fail` and `done` callbacks are actually executed in the same thread where the `multiplyPositiveNumbers` was called, while the actual multiplication code is executed in the thread handled by `QLambdaThreadWorker`. 

The `QDeferred<T>` instance makes sure that the callbacks are executed in the thread where they were defined, regardless of the thread where the `reject` or `resolve` methods are called. This is all automagically handled behind the scenes using the Qt's event loop.

So that's it! You managed to execute code in a thread, pass in data and returning the result safely. 

All of this without creating any class definitions, or connecting signals, or registering types. Actually all the code fits nicely in the `main` method. Less than 50 lines of code in total, including error handling.

```c++
#include <QCoreApplication>
#include <QDebug>
#include <QLambdaThreadWorker>
#include <QDeferred>

QLambdaThreadWorker worker;

QDeferred<int> multiplyPositiveNumbers(int x, int y)
{
	QDeferred<int> retDefered;
	worker.execInThread([retDefered, x, y]() mutable {
		qDebug() << "Execution in thread :" << QThread::currentThread();
		if (x < 0 || y < 0)
		{
			retDefered.reject(0);
		}
		else
		{
			retDefered.resolve(x * y);
		}
	});
	return retDefered;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	int x = 3;
	int y = 4;
	qDebug() << "Call in thread :" << QThread::currentThread();
	multiplyPositiveNumbers(x, y)
	.fail([](int res) {
		Q_UNUSED(res);
		qDebug() << "multiplyPositiveNumbers failed!";
	})
	.done([x, y](int res) {
		qDebug() << "Result in thread :" << QThread::currentThread();
		qDebug() << x << "*" << y << "=" << res;
	});

	return a.exec();
}
```

The output is:

```
Call in thread : QThread(0x178e39ba640)
Execution in thread : QThread(0x178e39baaf0)
Result in thread : QThread(0x178e39ba640)
3 * 4 = 12
```

The only thing missing is how to include this library in your project so you can start using it right away!

## Include

To include this library in your project, just include [qlambdathreadworker.pri](./src/qlambdathreadworker.pri) and [qdeferred.pri](./src/qdeferred.pri) in your QMake Project file (`*.pro` file) as follows:

```cmake
include($$PATH_TO_THIS_REPO/src/qdeferred.pri)
include($$PATH_TO_THIS_REPO/src/qlambdathreadworker.pri)
```

Just replace `$$PATH_TO_THIS_REPO` with the relative or absolute path to this repo. Then in your C++ code include the headers:

```c++
#include <QLambdaThreadWorker>
#include <QDeferred>
```

Take a look at the [tests folder](./tests/) to see how projects are creacted. 

## Advanced QLambdaThreadWorker

You thought that was it? That was just a simple use case. `QLambdaThreadWorker` and `QDeferred` still have some very useful functionalities that will make your life easier when coding threaded applications.

For example, another interesting thing to do in a thread is to execute code in a cycle. With `QLambdaThreadWorker` you can do exactly this, by calling the `startLoopInThread` method:

```c++
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	int counter = 0;
	QLambdaThreadWorker worker;

	worker.startLoopInThread([&counter]() {
		qDebug() << "Counting" << counter++;
	}, 1000);

	return a.exec();
}
```

The previous example will count in a thread every second indefinetively.

We pass as a first argument to the `startLoopInThread` method a lambda to get executed cyclically in the thread. As a second argument we pass a cycle time in miliseconds.

Note that in the previous example the counting works because the `counter` variable is passed in *by-reference*, so the `counter++` actually acts on the same variable. We managed ot get away with it because we know that `a.exec()` is blocking, thus the `counter` variable exists as long as the `QCoreApplication` exists.

The `startLoopInThread` method returns an `int` that works as a cycle handler. We can use the handler to stop the cycle later using the `stopLoopInThread` method:

```c++
int counter = 0;
`QLambdaThreadWorker` state. worker;

int handle = worker.startLoopInThread([&counter]() {
	qDebug() << "Counting" << counter++;
}, 1000);

QTimer::singleShot(5000, [worker, handle]() mutable {
	worker.stopLoopInThread(handle);
});
```

The previous example will count in a thread from 0 to 4. Note the need of the `mutable` keyword in the `QTimer::singleShot` callback, because the `stopLoopInThread` modifies the `QLambdaThreadWorker` state.

One important fact to underscore, is that even after calling the `startLoopInThread` method, the `QLambdaThreadWorker` is still reusable. Meaning we can still call the `execInThread` while a cyle is executing, and it will work:

```c++
int counter = 0;
QLambdaThreadWorker worker;

worker.startLoopInThread([&counter]() {
	qDebug() << "Counting" << counter++;
}, 1000);

QTimer::singleShot(5000, [worker]() mutable {
	worker.execInThread([]() {
		qDebug() << "It has beed 5 seconds.";
	});
});
```

The possible output:

```Counting 0
Counting 1
Counting 2
Counting 3
Counting 4
It has beed 5 seconds.
Counting 5
Counting 6
```

Where both callbacks are being executed in the same thread. This is possible because a cycle in the thread leaves so much within executions, that we can use the same thread to do many other things.

The callbacks will never overlap because `QLambdaThreadWorker` internally queues the callback executions in the Qt event loop, therefore they are all serialized.

Finally, we can create as many loops as we want, just bear in mind we might need to keep track of the handles of we want to stop the cycles sometime in the future.

`QLambdaThreadWorker` was made to be reusable, this means you can still **move** an object to the thread handled by it using the `moveQObjectToThread` method, or just get a reference to the thread itself using the `getThread` method.

## Advanced QDeferred

TODO .

---

## License

LICENSE is MIT.

Copyright (c) 2017-2019 Juan Gonzalez Burgos