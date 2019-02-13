
# QDeferred

A Qt C++ alternative for handling async code execution, specially useful for threaded applications.

This library consists of two main helper classes:

* `QLambdaThreadWorker` : execute code in a thread.

* `QDeferred` : pass data between threads.

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

Qt provides other classes that facilitate threaded and retrieval of results of async code execution, namely `QtConcurrent`, `QFuture` and `QFutureWatcher`. A comparison between `QtConcurrent` and `QDeferred` async code and their differences is shown in the last section of this document.

`QDeferred` is not *the one and only solution* for your threaded code execution problems, but just another tool that can help create nice async APIs, easy to read and write code. See the *Conclusions and Recommendations* section at the end of this document for recommendations on when to use `QDeferred`.

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

Take a look at the [tests folder](./tests/) to see how projects are created.

This library requires **C++11**.

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

**Note** : similar results can be achieved with `QtConcurrent` :

```c++
QFuture<void> future = QtConcurrent::run([=]() {
    // Code in this block will run in another thread
});
```

See the end of this document to see what advantages `QDeferred` provides.

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

We pass as a first argument to the `startLoopInThread` method, a lambda to get executed cyclically in the thread. As a second argument we pass the cycle time in miliseconds.

Note that in the previous example, the counting works because the `counter` variable is passed in *by-reference*, so the `counter++` actually acts on the same variable. We managed to get away with it because we know that `a.exec()` is blocking, thus the `counter` variable exists as long as the `QCoreApplication` exists.

The `startLoopInThread` method returns an `int` that works as a cycle **handler**. We can use this handler to stop the cycle later using the `stopLoopInThread` method:

```c++
int counter = 0;
QLambdaThreadWorker worker;

int handle = worker.startLoopInThread([&counter]() {
	qDebug() << "Counting" << counter++;
}, 1000);

QTimer::singleShot(5000, [worker, handle]() mutable {
	worker.stopLoopInThread(handle);
});
```

The previous example will count in a thread from 0 to 4 and then stop. Note the need of the `mutable` keyword in the `QTimer::singleShot` callback, because the `stopLoopInThread` modifies the `QLambdaThreadWorker` state.

One important fact to underscore, is that even after calling the `startLoopInThread` method, the `QLambdaThreadWorker` is still reusable. Meaning we can still call the `execInThread` while a cyle is running, and it will work:

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

Both callbacks are being executed in the same thread. This is possible because a cycle in the thread leaves free time within executions, so we can use the same thread to do many other things on that free time.

The executions will never overlap because `QLambdaThreadWorker` internally queues the callback executions in the Qt event loop, therefore they are all serialized by Qt.

Finally, we can create **as many loops as we want**, just bear in mind we might need to keep track of the handles if we want to stop the cycles sometime in the future.

`QLambdaThreadWorker` was made to be reusable, this means you can still **move** an object to the thread handled by it using, using the `moveQObjectToThread` method, or just get a reference to the thread itself using the `getThread` method.

## Advanced QDeferred

### Handling State

Consider the case where we are tasked to design an async API for a network client, using `QDeferred` the API would look as follows:

```c++
#include <QObject>
#include <QDeferred>
#include <QLambdaThreadWorker>

class MyClient : public QObject
{
    Q_OBJECT
public:
    explicit MyClient(QObject *parent = 0);
    ~MyClient();

    QDeferred<QString> connect(const QString &strAddress);

    QDeferred<QString> request(const QString &strRequest);

signals:
	void disconnected();

private:
	QLambdaThreadWorker m_worker;
};
```

Note that our API also uses Qt's signals, they are not incompatible and they actually serve a different purpose than `QDeferred`.

Qt's signals and slots are very good for handling **unexpected events**, for example that our client unexpectedly disconnects. While `QDeferred` is better suited for **expected events**, for example, after our client makes a *request* to the server, we expect a *response* soon after.

Using a well designed `QDeferred` based API, we could use our client as follows:

```c++
MyClient client;

client.request("GET")
.done([](QString strGetReponse){
	// this callback for the GET request
});

client.request("POST")
.done([](QString strPostReponse){
	// this callback for the POST request
});
```

Note, the two previous calls to the `request` method are async, and we don't know for sure which reponse will arrive first. But that doesn't matter because `QDeferred` calls the correct callback for each request.

If we used Qt's signals and slots for this task, we would force the user of our client API to keep track of the state for each request, and figure out which reponse corresponds to which request.

Talking about state, in our client example we forgot to make sure the client is connected before we can make a request. We could fix it by **nesting** our async calls as follows:

```c++
MyClient client;

client.connect()
.done([&client](QString strAddress) {
	// nested
	client.request("GET")
	.done([](QString strGetReponse) {
		// do something with 'strGetReponse'
	});
});
```

But this inmediatly looks wrong, what if the server we are connecting to, has a state and it requires for example, a series of login requests called in order? Then we would be forced to do something like this:

```c++
client.request("POST:login/user=bill,pass=123")
.done([&client](QString strResponse) {
	client.request("POST:select/account_1")
	.done([&client](QString strResponse) {
		client.request("POST:select/january")
		.done([&client](QString strResponse) {
			client.request("GET:balance")
			.done([&client](QString strResponse) {
				// and so on ..............
			});
		});
	});
});
```

This is what is called in other programming languages as the [pyramid of doom](https://en.wikipedia.org/wiki/Pyramid_of_doom_(programming)), which makes the code unreadible and difficult to debug.

To solve this, `QDeferred` has a very handy method called `then`, which allows us to **chain** the async operations of our client:

```c++
MyClient client;

client.connect()
.then<QString>([&client](QString strAddress) {
	// do something reponse
	return client.request("POST:login/user=bill,pass=123"); // NOTE : callback returns a QDeferred<QString>
})
.then<QString>([&client](QString strResponse) {
	// do something reponse
	return client.request("POST:select/account_1");
})
.then<QString>([&client](QString strResponse) {
	// do something reponse
	return client.request("POST:select/january");
})
.then<QString>([&client](QString strResponse) {
	// do something reponse
	return client.request("GET:balance");
})
.done([](QString strResponse) {
	// finish
});
```

The callback passed to the `then` method will only be executed if the deferred object is *resolved* (success), same as in the `done` method. The main difference is that the `then` method accepts a template argument, which is the template argument of the expected `QDeferred` object to be returned by the `then` callback.

The code becomes more readible, because now the order of execution of the async operations is much more clear. It reads as:

```
connect, then login, then select account1, then select january, then get balance
```

Remember that all those callbacks are actually executed in order, and in the thread in which they are defined. If any of those steps fails, then the rest of the callbacks are not executed. We have not yet handled errors, but for each step we could define a `fail` callback:

```c++
MyClient client;

client.connect()
.fail([](QString strError){
	qDebug() << "Failed connecting";
})
.then<QString>([&client](QString strAddress) {
	// do something reponse
	return client.request("POST:login/user=bill,pass=123"); // NOTE : callback returns a QDeferred<QString>
})
.fail([](QString strError){
	qDebug() << "Failed logging in";
})
.then<QString>([&client](QString strResponse) {
	// do something reponse
	return client.request("GET:balance");
})
.fail([](QString strError){
	qDebug() << "Failed getting balance";
})
.done([](QString strResponse) {
	// finish
});
```

This way we can know exactly in which step something went wrong.

### Handling Progress

There is another very handy feature of the `QDeferred` that allows notifying partial progress of an async operation.

Say we want to count from 1 to 10 in a thread and *notify* every time the count changes:

```c++
QLambdaThreadWorker worker;

QDeferred<int> countToTen()
{
	QDeferred<int> retDefered;
	QSharedPointer<int> handle(new int);
	QSharedPointer<int> counter(new int);
	// init counter
	*counter = 0;
	*handle  = worker.startLoopInThread([retDefered, handle, counter]() mutable {
		// notify progress
		retDefered.notify(++*counter);
		// end counting
		if (*counter == 10)
		{
			// stop thread
			worker.stopLoopInThread(*handle);
			retDefered.resolve(*counter);
		}
	}, 1000);

	return retDefered;
}
```

We use the `notify` method to notify *progress* of our async code execution. We pass to the method the current value of the progress.

Note we needed to create `handle` and `counter` in the heap, because those variables need to exist throughout the whole counting life-cycle.

To susbcribe to the progress we use the `progress` method and pass a callback to it. The coutning function is called as follows:

```c++
countToTen()
.progress([](int counter) {
	qDebug() << "Progress" << counter;
})
.done([](int counter) {
	qDebug() << "Finished" << counter;
});
```

The output would be:

```
Progress 1
Progress 2
Progress 3
Progress 4
Progress 5
Progress 6
Progress 7
Progress 8
Progress 9
Progress 10
Finished 10
```

### Handling Multiple QDeferred

Consider a simplified version of our network client example:

```c++
struct MyClient
{
	QDefer connect()
	{
		QDefer retDefer;
		m_worker.execInThread([retDefer]() mutable {
			// sleep random 0 to 5 secs before resolving
			static int seed = 123;
			qsrand(++seed);
			QThread::sleep(qrand() % 5);
			// resolve
			retDefer.resolve();
		});
		return retDefer;
	};

	QLambdaThreadWorker m_worker;
};
```

The client has a `connect` method that gets executed in a thread, and resolves after a random delay between 0 and 5 seconds.

We create three instances of the client, and would like to execute some code once **all** of them have been connected. This could complicate our code, but luckly `QDeferred` has a handly **static** method called `QDefer::when` that allows us to sync the states of multiple `QDeferred` instances:

```c++
MyClient client1, client2, client3;
// connect client 1
auto defer1 = client1.connect()
.done([]() {
	qDebug() << "Client 1 connected";
});
// connect client 2
auto defer2 = client2.connect()
.done([]() {
	qDebug() << "Client 2 connected";
});
// connect client 3
auto defer3 = client3.connect()
.done([]() {
	qDebug() << "Client 3 connected";
});

// execute callback when all connected
QDefer::when(defer1, defer2, defer3)
.done([]() {
	qDebug() << "All clients connected";
});
```

One execution of the previous code could output:

```
Client 3 connected
Client 2 connected
Client 1 connected
All clients connected
```

The next execution could output:

```
Client 3 connected
Client 1 connected
Client 2 connected
All clients connected
```

It doesn't matter which `QDeferred` instances are resolved first or when they are resolved, the `QDefer::when` method waits (not-blocking) until all of them are resolved and then executes its own `done` callback. Of course if **at least one** `QDeferred` is rejected, then the `fail` callback of the `QDefer::when` method is called.

It is not necesary that all the `QDeferred` instances passed to the `QDefer::when` method are of the same template type. The following code is also valid:

```c++
QDeferred<int>            defer1;
QDeferred<double>         defer2;
QDeferred<QList<QString>> defer3;

// ...

QDefer::when(defer1, defer2, defer3)
.done([]() {
	qDebug() << "All done";
})
.fail([](){
	qDebug() << "At least one failed";
});
```

### QDeferred State

One last thing to mention is that once a `QDeferred` instances has been resolved or rejected, it is not possible to resolve or reject it again.

Therefore sometimes it might be handy to test the **state** of a deferred instance before trying to resolve/reject it:

```c++
QDeferred<int> defer;

if (defer.state() == QDeferredState::PENDING)
{
	defer.resolve(123);
}
```

The `QDeferredState` enum has only three possible values:

* `PENDING` : not *resolved* nor *rejected*.

* `RESOLVED` : already *resolved*.

* `REJECTED` : already *rejected*.

Using the `state` method we can now if an async operation has been already processed.

### QDeferred in the same Thread

We don't need a thread to use `QDeferred`. Very much as Qt's signals and slots, `QDeferred` accepts a [Qt::ConnectionType](https://doc.qt.io/qt-5/qt.html#ConnectionType-enum) as an argument to each of its *consumer* API callbacks (`done`, `fail` and `progress`). This gives control over which thread is used to execute the callback, by default all callbacks have a `Qt::AutoConnection` connection type.

```c++
// Qt::DirectConnection explicitly forces to execute the 'progress' in the same thread in which 'notify' is called
// but this would happen by default anyway because Qt::AutoConnection figures aout both methods exist in the same thread.
defer.progress([](int val) {
	qDebug() << "Counting in the same thread :" << val;
}, Qt::DirectConnection);

for (int i = 0; i < 100; i++)
{
	// Will print de debug message inmediatly
	defer.notify(i + 1);
}
```

If we would like to wait until the next execution of the Qt event loop we could force it as follows:

```c++
// The debug messages will be executed 'later' on the next Qt event loop iteration
defer.progress([](int val) {
	qDebug() << "Counting in the same thread :" << val;
}, Qt::QueuedConnection);

for (int i = 0; i < 100; i++)
{
	defer.notify(i + 1);
}
```

### Multiple Subscribers

A `QDeferred` instance is an [explicitly shared object](https://doc.qt.io/qt-5/qexplicitlyshareddatapointer.html), this means it can be passed around by copy, and all these copies reference to the same internal instance. This means we can reuse a `QDeferred` instance, pass it around and subscribe to `done`, `fail` or `progress` callbacks elsewhere in the code:

```c++
QDeferred<int> defer = myMethod()
.done([](int val) {
	qDebug() << val;
})
.fail([](int val) {
	qDebug() << val;
});

// we can pass "defer" around since is a explicitly shared object
// ...

// subscribe elsewhere
defer
.done([](int val) {
	qDebug() << val;
})
.fail([](int val) {
	qDebug() << val;
});
```

If the `QDeferred` was already resolved/rejected when a new subscription is done, then the callback is called inmediatly (depending on the connection-type).

## QDeferred vs QtConcurrent

Qt provides its own classes to facilitate the execution of threaded async code. In many ways, very similar results can be achieved using both approaches. For example, to multiply two numbers in a thread:

```c++
// Using QtConcurrent

QFuture<int> future;
QFutureWatcher<int> watcher;
QObject::connect(&watcher, &QFutureWatcher<int>::finished, [&future]() {
	qDebug() << "Result QFuture" << future.result();
});
future = QtConcurrent::run([]() {
	return 4 * 3;
});
watcher.setFuture(future);

// Using QDeferred

QLambdaThreadWorker worker;
QDeferred<int> def;
worker.execInThread([def]() mutable {
	def.resolve(4 * 3);
});
def.done([](int result) {
	qDebug() << "Result QDeferred" << result;
});
```

So both APIs allow similar results with similar ease. But `QDeferred` provides a couple of advantages with respect to the `QtConcurrent` approach:

* It allows for multiple template arguments.

```c++
QLambdaThreadWorker worker;
QDeferred<int, QString, QList<double>> def;
worker.execInThread([def]() mutable {
	def.resolve(123, "hello", QList<double>() << 1.1 << 2.2 << 3.3);
});
def.done([](int result, QString string, QList<double> list) {
	qDebug() << "Result QDeferred" << result;
	qDebug() << "Result QDeferred" << string;
	qDebug() << "Result QDeferred" << list;
});
```

En the example above we define 3 template arguments for the `QDeferred`, which allows us to pass back 3 variables as a result.

* It allows chaining of async operations.

```c++
MyClient client;

client.connect()
.then<QString>([&client](QString strAddress) {
	// do something reponse
	return client.request("POST:login/user=bill,pass=123"); // chaining : returns a QDeferred
})
.then<QString>([&client](QString strResponse) {
	// do something reponse
	return client.request("GET:balance"); // chaining
})
.done([](QString strResponse) {
	// finish
});
```

See the *Handling State* section for more info.

* Allows *non-blocking* sync of `QDeferred` instances.

```c++
QDeferred<double> defer1, defer2, defer3;

// ...

// non-blocking : execute callback when all resolved
QDefer::when(defer1, defer2, defer3)
.done([]() {
	qDebug() << "All resolved";
});
```

Similar to what can be done with `QFutureSynchronizer`, but non-blocking. See the *Handling Multiple QDeferred* section.

# Conclusion and Recommendations

As we saw on the examples throughout this document, `QDeferred` is just another tool that can be used alongside other Qt APIs for threaded and async code execution. In special, `QDeferred` was designed for solving the issue of calling async code in another thread, and retrieving the result to the calling thread. Below is a list of recommended API for some specific use cases:

* `Unexpected Events` : For unexpected events within threads, [Qt's signals and slots](https://doc.qt.io/qt-5/signalsandslots.html) is still the best solution. For example and unexpected network client disconnection.

* `Expected Events` : For calling code in a thread and retrieving results or progress to the calling thread, then `QDeferred` is recommended as an alternative of `QtConcurrent` and `QFuture`. Of course if the `QFuture` API fullfills your certain needs better, then by all means stick with it. You can still use `QDeferred` elsewhere in your code wherever it fits better. Examples of this use case are a network client request/response cycle, or large file operations.

* `Batch Processing` : For processing data in parallel, [QThreadPool](https://doc.qt.io/qt-5/qthreadpool.html) is the best solution. Examples can be a network server or image processing.

In regards to `QLambdaThreadWorker`, it was designed as a companion to `QDeferred` in order to easier manage a long term living thread. Mainly to hide thread management to the user of an API. It is better used as a member of a class:

```c++
class MyClient
{
	QDefer connect()
	{
		QDefer retDefer;
		m_worker.execInThread([retDefer]() mutable {
			// here create a socket, subscribe to connect event, and when connected then...
			// ...
			// resolve
			retDefer.resolve();
			// update internal variables
			m_strPeerName = socket->peerName();
		});
		return retDefer;
	};

	QDeferred<QString> getPeerName()
	{
		QDeferred<QString> retDefer;
		m_worker.execInThread([retDefer]() mutable {
			// serialize all access to data used in another thread
			retDefer.resolve(m_strPeerName);
		});
		return retDefer;
	};

	QString m_strPeerName;
	QLambdaThreadWorker m_worker;
};
```

It is adviced to serialize read and write access to data used in a thread to avoid the common pitfalls of threading (race-conditions, etc.). By serializing access to the `m_strPeerName` variable in the example above we avoid to use mutexes and other constructs that can increase the complexity of our application. Access data directly (read/write) only within one thread, use mechanisms such as `QDeferred` to access it indirectly between different threads and all should be fine.

Remember to make good use of the `then` method:

```c++
MyClient client;

client.connect()
.then<QString>([&client](){
	return client.getPeerName();
})
.done([](QString strPeerName){
	qDebug() << "Connected to" << strPeerName;
});
```

And finally remember to make sure that any lamba captures made by reference live long enough for the execution of your callbacks. E.g. in the example above make sure `client` lives until after the `then` and `done` callbacks are executed.

---

## License

LICENSE is MIT.

Copyright (c) 2017-2019 Juan Gonzalez Burgos