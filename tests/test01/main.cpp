#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QList>
#include <QVariant>

#include <QDeferred>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

// NOTE : * cannot use catch to test threaded functionality.
//        * needed to provide a custom main() function, to
//        be able to initialize the QCoreApplication.

int main(int argc, char* argv[])
{
	// global setup...
	QCoreApplication a(argc, argv);

	int result = Catch::Session().run(argc, argv);

	// global clean-up...

	return (result < 0xff ? result : 0xff);
}

TEST_CASE("Should call done callback after resolve called", "[done][resolve]")
{
	// init
	QDefer defer;
	// subscribe done callback
	defer.done([]() {
		// test that gets called
		REQUIRE(true);
	}).fail([]() {
		REQUIRE(false);
	});
	// resolve
	defer.resolve();
}

TEST_CASE("Should call fail callback after reject called", "[fail][reject]")
{
	//init
	QDefer defer;
	// subscribe fail callback
	defer.fail([]() {
		// test that gets called
		REQUIRE(true);
	}).done([]() {
		REQUIRE(false);
	});
	// reject
	defer.reject();
}

TEST_CASE("Should change state from pending to resolved after resolve called", "[done][resolve][state]")
{
	// init
	QDefer defer;
	// test unresolved state
	REQUIRE(defer.state() == QDeferredState::PENDING);
	// subscribe done callback
	defer.done([defer]() {
		// test resolved state
		REQUIRE(defer.state() == QDeferredState::RESOLVED);
	});
	// resolve
	defer.resolve();
}

TEST_CASE("Should change state from pending to rejected after reject called", "[fail][reject][state]")
{
	// init
	QDefer defer;
	// test unresolved state
	REQUIRE(defer.state() == QDeferredState::PENDING);
	// subscribe fail callback
	defer.fail([defer]() {
		// test resolved state
		REQUIRE(defer.state() == QDeferredState::REJECTED);
	});
	// reject
	defer.reject();
}

TEST_CASE("Should call done callback with simple argument after resolve called", "[done][resolve][args]")
{
	// init
	QDeferred<int> defer;
	int i = 123;
	// subscribe done callback
	defer.done([i](int val) {
		// test that gets called with argument
		REQUIRE(i == val);
	});
	// resolve
	defer.resolve(i);
}

TEST_CASE("Should call fail callback with simple argument after reject called", "[fail][reject][args]")
{
	//init
	QDeferred<int> defer;
	int i = 123;
	// subscribe fail callback
	defer.fail([i](int val) {
		// test that gets called with argument
		REQUIRE(i == val);
	});
	// reject
	defer.reject(i);
}

TEST_CASE("Should call done callback with complex argument after resolve called", "[done][resolve][args]")
{
	// init
	QDeferred<QList<QVariant>> defer;
	QList<QVariant> listArgs;
	listArgs.append("Hello");
	listArgs.append("World");
	listArgs.append(12345);
	// subscribe done callback
	defer.done([listArgs](QList<QVariant> val) {
		// test that gets called with argument
		REQUIRE(listArgs == val);
	});
	// resolve
	defer.resolve(listArgs);
}

TEST_CASE("Should call fail callback with complex argument after reject called", "[fail][reject][args]")
{
	//init
	QDeferred<QList<QVariant>> defer;
	QList<QVariant> listArgs;
	listArgs.append("Hello");
	listArgs.append("World");
	listArgs.append(12345);
	// subscribe fail callback
	defer.fail([listArgs](QList<QVariant> val) {
		// test that gets called with argument
		REQUIRE(listArgs == val);
	});
	// reject
	defer.reject(listArgs);
}

TEST_CASE("Should call done callbacks in the order they were registered", "[done][resolve][multi]")
{
	// init
	QDefer defer;
	int i = 0;
	// subscribe done callbacks
	defer.done([&i]() {
		// test that gets called in order
		i++;
		REQUIRE(i == 1);
	}).done([&i]() {
		// test that gets called in order
		i++;
		REQUIRE(i == 2);
	}).done([&i]() {
		// test that gets called in order
		i++;
		REQUIRE(i == 3);
	});
	// resolve
	defer.resolve();
}

TEST_CASE("Should call fail callbacks in the order they were registered", "[fail][reject][multi]")
{
	// init
	QDefer defer;
	int i = 0;
	// subscribe fail callbacks
	defer.fail([&i]() {
		// test that gets called in order
		i++;
		REQUIRE(i == 1);
	}).fail([&i]() {
		// test that gets called in order
		i++;
		REQUIRE(i == 2);
	}).fail([&i]() {
		// test that gets called in order
		i++;
		REQUIRE(i == 3);
	});
	// reject
	defer.reject();
}

TEST_CASE("Should call done callback when argument of deferred is another deferred", "[done][resolve][args]")
{
	// init
	QDeferred<QDefer> deferOfDefer;
	QDefer defer;
	// subscribe done callback
	defer.done([]() {
		// test that gets called
		REQUIRE(true);
	});
	// subscribe done callback
	deferOfDefer.done([](QDefer defer) {
		// resolve
		defer.resolve();
	});
	// resolve
	deferOfDefer.resolve(defer);
}

TEST_CASE("Should call done callback with argument when argument of deferred is another deferred", "[done][resolve][args]")
{
	// init
	QDeferred<QDeferred<int>> deferOfDefer;
	QDeferred<int> defer;
	// subscribe done callback
	defer.done([](int val) {
		// test that gets called
		REQUIRE(val == 123);
	});
	// subscribe done callback
	deferOfDefer.done([](QDeferred<int> defer) {
		// resolve with argument
		int i = 123;
		defer.resolve(i);
	});
	// resolve
	deferOfDefer.resolve(defer);
}

TEST_CASE("Should call progress callback after notify called", "[progress][notify]")
{
	// init
	QDeferred<int> defer;
	int i = 123;
	// subscribe progress callback
	defer.progress([&i](int val) {
		// test that gets called
		REQUIRE(i == val);
	});
	// notify
	defer.notify(i);
}

TEST_CASE("Should call progress callback as many times as notify called", "[progress][notify]")
{
	// init
	QDeferred<int> defer;
	// subscribe progress callback
	defer.progress([](int val) {
		static int counter = 0;
		// test that gets called
		counter++;
		REQUIRE(counter == val);
	});
	// notify multiple times
	for (int i = 1; i <= 3; i++)
	{
		defer.notify(i);
	}
}

TEST_CASE("Should call multiple progress callback as many times as notify called", "[progress][notify]")
{
	// init
	QDeferred<int> defer;
	// subscribe progress callback
	defer.progress([](int val) {
		static int counter = 0;
		// test that gets called
		counter++;
		REQUIRE(counter == val);
	}).progress([](int val) {
		static int counter = 0;
		// test that gets called
		counter++;
		REQUIRE(counter == val);
	});
	// notify multiple times
	for (int i = 1; i <= 3; i++)
	{
		defer.notify(i);
	}
}

TEST_CASE("Should call done callback of when deferred when all deferreds resolve", "[when][done][resolve]")
{
	// init
	QDefer defer1;
	QDefer defer2;
	QDefer defer3;
	int i = 0;
	// subscribe other deferreds to increment and prove order
	defer1.done([&i]() {
		i++;
	});
	defer2.done([&i]() {
		i++;
	});
	defer3.done([&i]() {
		i++;
	});
	// subscribe done callback of when
	QDefer::when(defer1, defer2, defer3).done([&i]() {
		// test that gets called
		REQUIRE(i == 3);
	});
	// resolve
	defer1.resolve();
	defer2.resolve();
	defer3.resolve();
}

TEST_CASE("Should call done callback of when deferred when all deferreds of different types resolve", "[when][done][resolve]")
{
	// init
	QDeferred<int>            defer1;
	QDeferred<double>         defer2;
	QDeferred<QList<QString>> defer3;
	int            i = 0;
	double         d = 3.1416;
	QList<QString> list = QList<QString>() << "one" << "two" << "three";
	// subscribe other deferreds to increment and prove order
	defer1.done([&i](int iVal) {
		Q_UNUSED(iVal);
		i++;
	});
	defer2.done([&i](double dblVal) {
		Q_UNUSED(dblVal);
		i++;
	});
	defer3.done([&i](QList<QString> strList) {
		Q_UNUSED(strList);
		i++;
	});
	// subscribe done callback of when
	QDefer::when(defer1, defer2, defer3).done([&i]() {
		// test that gets called
		REQUIRE(i == 3);
	});
	// resolve
	defer1.resolve(i);
	defer2.resolve(d);
	defer3.resolve(list);
}

TEST_CASE("Should call then resolved callback after resolve called", "[then][resolve]")
{
	// init
	QDefer defer1;
	// subscribe then callback
	defer1.then<int>([]() {
		QDeferred<int> defer2;
		// test that gets called
		REQUIRE(true);
		// return deferred
		return defer2;
	});
	// resolve
	defer1.resolve();
}

TEST_CASE("Should call chained then resolved callback after resolve called", "[then][resolve][chain]")
{
	// init
	QDefer defer1;
	// subscribe then callback
	defer1.then<int>([]() {
		QDeferred<int> defer2;
		// test that gets called
		REQUIRE(true);
		// resolve and return
		int i = 123;
		defer2.resolve(i);
		return defer2;
	}).then<double, QString>([](int val) {
		QDeferred<double, QString> defer3;
		// test that gets called
		REQUIRE(val == 123);
		// resolve and return
		double  d   = 3.141592;
		QString str = "hello";
		defer3.resolve(d, str);
		return defer3;
	}).done([](double val, QString str) { // NOTE : call 'done' at the end of the chain
		// test that gets called
		REQUIRE(val == 3.141592);
		REQUIRE(str == QString("hello"));
	});
	// resolve
	defer1.resolve();
}

TEST_CASE("Should call then rejected callback after rejected called", "[then][reject]")
{
	// init
	QDefer defer1;
	// subscribe then callback
	defer1.then<int>([]() {
		QDeferred<int> defer2;
		REQUIRE(false);
		// return deferred
		return defer2;
	}, []() { // fail over 'defer1' with zero arguments
		// test that gets called
		REQUIRE(true);
	}).fail([](int val) {
		// fail over 'defer2', which is not resolved nor rejected, therefore not called
		Q_UNUSED(val);
		REQUIRE(false);
	});
	// reject after
	defer1.reject(); // reject here
}

TEST_CASE("Should call then rejected callback after rejected called. Reject called first.", "[then][reject]")
{
	// init
	QDefer defer1;
	// reject before
	defer1.reject(); // reject here
	// subscribe then callback
	defer1.then<int>([]() {
		QDeferred<int> defer2;
		REQUIRE(false);
		// return deferred
		return defer2;
	}, []() { // fail over 'defer1' with zero arguments
		// test that gets called
		REQUIRE(true);
	}).fail([](int val) {
		// fail over 'defer2', which is not resolved nor rejected, therefore not called
		Q_UNUSED(val);
		REQUIRE(false);
	});
}

TEST_CASE("Should call second chained then rejected callback after second rejected called", "[then][reject][chain]")
{
	// init
	QDefer defer1;
	// subscribe then callback
	defer1.then<int>([]() {
		QDeferred<int> defer2;
		// test that gets called
		REQUIRE(true);
		// reject second and return
		defer2.reject(123); // reject here
		return defer2;
	}).fail([](int val) { // fail over 'defer2' with arguments
		Q_UNUSED(val);
		// test that gets called
		REQUIRE(true);
	}).then<int>([](int val) {
		QDeferred<int> defer3;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer3;
	}, []() { // fail over 'defer2' with zero arguments
		// test that gets called
		REQUIRE(true);
	});
	// resolve
	defer1.resolve();
}

TEST_CASE("Should call last chained then rejected callback after first rejected called", "[then][reject][chain]")
{
	// init
	QDeferred<bool> defer1;
	// subscribe then callback
	defer1.then<int>([](bool val) {
		Q_UNUSED(val);
		QDeferred<int> defer2;
		REQUIRE(false);
		return defer2;
	}).then<int>([](int val) {
		QDeferred<int> defer3;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer3;
	}).then<int>([](int val) {
		QDeferred<int> defer4;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer4;
	}).then<int>([](int val) {
		QDeferred<int> defer5;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer5;
	}, []() { // fail over 'defer1' with zero args
		// test that gets called
		REQUIRE(true);
	});
	// reject
	defer1.reject(false);
}

TEST_CASE("Should call last chained then rejected callback after second rejected called", "[then][reject][chain]")
{
	// init
	QDefer defer1;
	// subscribe then callback
	defer1.then<int>([]() {
		QDeferred<int> defer2;
		// test that gets called
		REQUIRE(true);
		// reject second and return
		defer2.reject(123); // reject here
		return defer2;
	}).then<int>([](int val) {
		QDeferred<int> defer3;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer3;
	}).then<int>([](int val) {
		QDeferred<int> defer4;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer4;
	}).then<int>([](int val) {
		QDeferred<int> defer5;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer5;
	}, []() { // fail over 'defer2' with zero args
		// test that gets called
		REQUIRE(true);
	});
	// resolve
	defer1.resolve();
}

TEST_CASE("Should call last chained then rejected callback after third rejected called", "[then][reject][chain]")
{
	// init
	QDefer defer1;
	// subscribe then callback
	defer1.then<int>([]() {
		QDeferred<int> defer2;
		// test that gets called
		REQUIRE(true);
		// resolve second and return
		defer2.resolve(123);
		return defer2;
	}).then<int>([](int val) {
		QDeferred<int> defer3;		
		// test that gets called
		REQUIRE(val == 123);
		// reject third and return
		defer3.reject(456); // reject here
		return defer3;
	}).then<int>([](int val) {
		QDeferred<int> defer4;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer4;
	}).then<int>([](int val) {
		QDeferred<int> defer5;
		Q_UNUSED(val);
		REQUIRE(false);
		return defer5;
	}, []() { // fail over 'defer3' with zero args
		// test that gets called
		REQUIRE(true);
	});
	// resolve
	defer1.resolve();
}