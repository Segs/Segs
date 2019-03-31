## JCON-CPP

JCON-CPP is a portable C++ JSON RPC 2.0 library that depends on Qt.

## Introduction

If you're using **C++ 14** and **Qt**, and want to create a **JSON RPC 2.0**
client or server, using either **TCP** or **WebSockets** as underlying transport
layer, then **JCON-CPP** might prove useful.

In all of the following, replace "Tcp" with "WebSocket" to change the transport
method.

Platforms supported are: Linux, Windows, Mac OS, and Android.

## Building the Library

1. Get the source.
2. Create a `build` directory in the top directory.
3. Change to the `build` directory: `cd build`.
4. `cmake ..`
5. `make -j4`

The build depends on the build directive `CMAKE_PREFIX_PATH` to find the
required Qt dependencies, so if your CMake doesn't pick up on where to find Qt,
try adding `cmake -DCMAKE_PREFIX_PATH=<QTDIR> ..` in step 4 above.

## Include Files

Depending on if you're implementing a server or a client and whether you're
using TCP or WebSockets, you need to include some of these files:
```c++
#include <jcon/json_rpc_tcp_client.h>
#include <jcon/json_rpc_tcp_server.h>
#include <jcon/json_rpc_websocket_client.h>
#include <jcon/json_rpc_websocket_server.h>
```

## Example Code

There's example code of both a server and a client in the file `src/main.cpp`.

## Creating a Server

```c++
auto rpc_server = new jcon::JsonRpcTcpServer(parent);
```

Create a service (a collection of invokable methods):

1. Make your service class inherit `QObject`
2. Make sure your service method is accessible by the Qt meta object system
   (either by using the `Q_INVOKABLE` macro or by putting the method in a
   `public slots:` section). For instance:

```c++
class ExampleService : public QObject
{
    Q_OBJECT

public:
    ExampleService(QObject* parent = nullptr);
    virtual ~ExampleService();

    Q_INVOKABLE int getRandomInt(int limit);
};
```

Parameters and return types are automatically matched against the JSON RPC call,
using the Qt Meta object system, and you can use lists (`QVariantList`) and
dictionary type objects (`QVariantMap`) in addition to the standard primitive
types such as `QString`, `bool`, `int`, `float`, etc.

Register your service with:

```c++
rpc_server->registerServices({ new ExampleService() });
```

Note that (as of 2018-11-21) there is also a variant of `registerServices` that
takes a `QMap<QObject*, QString>`, where the keys are the services, and the
values are strings that will need to be used as prefixes when calling the
corresponding RPC methods. This can be used as a simple namespace mechanism.
Please refer to the example code in `src/main.cpp`.

The server will take over ownership of the service object, and the memory will
be freed at shutdown. Note that the `registerServices` method changed its
signature 2016-10-20, from being a variadic template expecting `unique_ptrs`, to
taking a `QObjectList`.

Finally, start listening for client connections by:

```c++
rpc_server->listen(6001);
```

Specify whatever port you want to use.


## Creating a Client

Simple:

```c++
auto rpc_client = new jcon::JsonRpcTcpClient(parent);
rpc_client->connectToServer("127.0.0.1", 6001);
```

(No need to use a smart pointer here, since the destructor will be called as
long as a non-null parent `QObject` is provided.)


### Invoking a Remote Method Asynchronously

```c++
auto req = rpc_client->callAsync("getRandomInt", 10);
```

The returned object (of type `std::shared_ptr<JsonRpcRequest>`) can be used to
set up a callback, that is invoked when the result of the JSON RPC call is
ready:

```c++
req->connect(req.get(), &jcon::JsonRpcRequest::result,
             [](const QVariant& result) {
                 qDebug() << "result of RPC call:" << result;
                 qApp->exit();
             });
```

To handle errors:

```c++
req->connect(req.get(), &jcon::JsonRpcRequest::error,
             [](int code, const QString& message, const QVariant& data) {
                 qDebug() << "RPC error: " << message << " (" << code << ")";
                 qApp->exit();
             });
```


### Invoking a Remote Method Synchronously

```c++
auto result = rpc_client->call("getRandomInt", 10);

if (result->isSuccess()) {
    QVariant res = result->result();
} else {
    QString err_str = result->toString();
}
```


### Expanding a List of Arguments

If you want to expand a list of arguments (instead of passing the list as a
single argument), use `callExpandArgs` and `callAsyncExpandArgs`.


## Known Issues

* Does not yet support batch requests/responses


## Contributing

Bug reports and pull requests are welcome on GitHub at
https://github.com/joncol/jcon-cpp.

Please follow these
[guidelines](https://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html)
for creating commit messages.

Also make sure to follow the existing code style. No lines with more than 80
characters, spaces instead of tabs for instance.


## License

The library is available as open source under the terms of the [MIT
License](http://opensource.org/licenses/MIT).
