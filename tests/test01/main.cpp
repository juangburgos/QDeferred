#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QList>
#include <QVariant>

#include <QDeferred>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

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
		// test called
		REQUIRE(true);
	});
	// resolve
	defer.resolve();

	QCoreApplication::processEvents();
}

TEST_CASE("Should call fail callback after reject called", "[fail][reject]")
{
	//init
	QDefer defer;
	// subscribe fail callback
	defer.fail([]() {
		// test called
		REQUIRE(true);
	});
	// reject
	defer.reject();

	QCoreApplication::processEvents();
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

	QCoreApplication::processEvents();
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

	QCoreApplication::processEvents();
}

TEST_CASE("Should call done callback with simple argument after resolve called", "[done][resolve][args]")
{
	// init
	QDeferred<int> defer;
	int i = 123;
	// subscribe done callback
	defer.done([i](int val) {
		// test called with argument
		REQUIRE(i == val);
	});
	// resolve
	defer.resolve(i);

	QCoreApplication::processEvents();
}

TEST_CASE("Should call fail callback with simple argument after reject called", "[fail][reject][args]")
{
	//init
	QDeferred<int> defer;
	int i = 123;
	// subscribe fail callback
	defer.fail([i](int val) {
		// test called with argument
		REQUIRE(i == val);
	});
	// reject
	defer.reject(i);

	QCoreApplication::processEvents();
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
		// test called with argument
		REQUIRE(listArgs == val);
	});
	// resolve
	defer.resolve(listArgs);

	QCoreApplication::processEvents();
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
		// test called with argument
		REQUIRE(listArgs == val);
	});
	// reject
	defer.reject(listArgs);

	QCoreApplication::processEvents();
}

TEST_CASE("Should call done callbacks in the order they were registered", "[done][resolve][multi]")
{
	// init
	QDefer defer;
	int i = 0;
	// subscribe done callbacks
	defer.done([&i]() {
		// test called in order
		i++;
		REQUIRE(i == 1);
	}).done([&i]() {
		// test called in order
		i++;
		REQUIRE(i == 2);
	}).done([&i]() {
		// test called in order
		i++;
		REQUIRE(i == 3);
	});
	// resolve
	defer.resolve();

	QCoreApplication::processEvents();
}

TEST_CASE("Should call fail callbacks in the order they were registered", "[fail][reject][multi]")
{
	// init
	QDefer defer;
	int i = 0;
	// subscribe fail callbacks
	defer.fail([&i]() {
		// test called in order
		i++;
		REQUIRE(i == 1);
	}).fail([&i]() {
		// test called in order
		i++;
		REQUIRE(i == 2);
	}).fail([&i]() {
		// test called in order
		i++;
		REQUIRE(i == 3);
	});
	// reject
	defer.reject();

	QCoreApplication::processEvents();
}

TEST_CASE("Should call done callback when argument of deferred is another deferred", "[done][resolve][args]")
{
	// init
	QDeferred<QDefer> deferOfDefer;
	QDefer defer;
	// subscribe done callback
	defer.done([]() {
		// test called
		REQUIRE(true);
	});
	// subscribe done callback
	deferOfDefer.done([](QDefer defer) {
		// resolve
		defer.resolve();
	});
	// resolve
	deferOfDefer.resolve(defer);

	QCoreApplication::processEvents();
}

TEST_CASE("Should call done callback with argument when argument of deferred is another deferred", "[done][resolve][args]")
{
	// init
	QDeferred<QDeferred<int>> deferOfDefer;
	QDeferred<int> defer;
	// subscribe done callback
	defer.done([](int val) {
		// test called
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

	QCoreApplication::processEvents();
}

TEST_CASE("Should call then resolved callback after resolve called", "[then][resolve]")
{
	// init
	QDefer defer;
	// subscribe then callback
	defer.then([]() {
		// test called
		REQUIRE(true);
	});
	// resolve
	defer.resolve();

	QCoreApplication::processEvents();
}

TEST_CASE("Should call then rejected callback after rejected called", "[then][reject]")
{
	// init
	QDefer defer;
	// subscribe then callback
	defer.then([]() {
		Q_ASSERT_X(false, 
			"Should call then resolved callback after resolve called", 
			"Resolve callback must be unreachable");
	}, []() {
		// test called
		REQUIRE(true);
	});
	// reject
	defer.reject();

	QCoreApplication::processEvents();
}

TEST_CASE("Should call done callback of when deferred when all other deferreds resolve", "[when][done][resolve]")
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
		// test called
		REQUIRE(i == 3);
	});
	// resolve
	defer1.resolve();
	defer2.resolve();
	defer3.resolve();

	// NOTE : need to call twice ??
	// TODO : find a way to empty event queue!
	QCoreApplication::processEvents();
	QCoreApplication::processEvents();
}
