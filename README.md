# QApi — Qt Shared Library for HTTP Requests

> A lightweight, reusable Qt library for handling HTTP requests (GET / POST) with native JSON support, custom headers, and asynchronous error handling.

---

## Table of Contents

- [Requirements](#requirements)
- [Installation](#installation)
  - [Option A — Git Submodule (recommended)](#option-a--git-submodule-recommended)
  - [Option B — Manual Copy](#option-b--manual-copy)
- [CMake Integration](#cmake-integration)
- [Usage](#usage)
  - [Simple GET Request](#simple-get-request)
  - [GET Request — Array Response](#get-request--array-response)
  - [GET Request with Options](#get-request-with-options)
  - [POST JSON Request](#post-json-request)
  - [POST Form URL Encoded Request](#post-form-url-encoded-request)
  - [Error Handling](#error-handling)
- [Signals Reference](#signals-reference)
- [Options Reference](#options-reference)
- [Tests](#tests)
- [Project Structure](#project-structure)
- [FAQ](#faq)

---

## Requirements

| Tool | Minimum Version |
|---|---|
| Qt | 5.15 or Qt 6.x |
| CMake | 3.16+ |
| Compiler | C++17 (MSVC, GCC, Clang) |

Required Qt modules: `Core`, `Network`

---

## Installation

### Option A — Git Submodule (recommended)

This option lets you keep QApi up to date independently inside your project.

**1. Add QApi as a submodule**

```bash
# From the root of your project
git submodule add https://github.com/mecanes/QApi.git libs/QApi
git commit -m "add: QApi submodule"
```

**2. When cloning your project on a new machine**

```bash
# Clone with submodules automatically
git clone --recurse-submodules https://github.com/your-org/your-project.git

# Or if you already cloned without the flag
git submodule update --init
```

**3. Update QApi to the latest version**

```bash
cd libs/QApi
git pull origin main
cd ../..
git add libs/QApi
git commit -m "update: QApi to latest version"
```

---

### Option B — Manual Copy

If you do not want to use Git Submodule, simply copy the `QApi` folder into your project.

```
MyProject/
  libs/
    QApi/           ← folder copied here
      CMakeLists.txt
      qapi.h
      qapi.cpp
      QApi_global.h
  src/
    main.cpp
  CMakeLists.txt
```

> ⚠️ With this method, QApi updates must be applied manually.

---

## CMake Integration

Once QApi is present in your project, update your `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

# ── 1. Declare QApi before your executable ──
add_subdirectory(libs/QApi)

# ── 2. Declare your executable ──
add_executable(MyProject
    src/main.cpp
)

# ── 3. Link QApi to your project ──
target_link_libraries(MyProject
    PRIVATE
        QApi
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
)
```

> ✅ QApi's `target_include_directories` is declared `PUBLIC`, so you do not need to add it manually.

---

## Usage

### Import

```cpp
#include "qapi.h"
```

### Instantiation

```cpp
// With QObject parent (recommended — automatic memory management)
QApi* api = new QApi(this);

// Without parent
QApi api;
```

---

### Simple GET Request

```cpp
#include "qapi.h"

QApi* api = new QApi(this);

// Listen for the response
connect(api, &QApi::QApiReadyObject, this, [](const QJsonObject& obj, int status) {
    qDebug() << "HTTP Status :" << status;
    qDebug() << "Username   :" << obj["pseudo"].toString();
    qDebug() << "Email      :" << obj["email"].toString();
});

// Listen for errors
connect(api, &QApi::QApiReadyErrorOccurred, this, [](const QString& err, int status) {
    qDebug() << "Error" << status << ":" << err;
});

// Fire the request
api->Get(QUrl("https://api.myserver.com/user"));
```

**Expected JSON response:**
```json
{ "pseudo": "johndoe", "email": "john.doe@example.com" }
```

---

### GET Request — Array Response

If your endpoint returns a JSON array, connect to `QApiReadyArray`:

```cpp
connect(api, &QApi::QApiReadyArray, this, [](const QJsonArray& arr, int status) {
    qDebug() << "Items count:" << arr.size();

    for (const QJsonValue& val : arr) {
        QJsonObject item = val.toObject();
        qDebug() << item["title"].toString();
    }
});

api->Get(QUrl("https://api.myserver.com/games"));
```

**Expected JSON response:**
```json
[
  { "id": 1, "title": "Game One" },
  { "id": 2, "title": "Game Two" }
]
```

---

### GET Request with Options

You can pass custom headers and query parameters via `QApi::Options`:

```cpp
QApi::Options opt;

// Custom headers
opt.headers.insert("Authorization", "Bearer my_jwt_token");
opt.headers.insert("X-App-Version", "1.0.0");

// Query params (?page=1&limit=20)
opt.query.addQueryItem("page", "1");
opt.query.addQueryItem("limit", "20");

api->Get(QUrl("https://api.myserver.com/games"), opt);

// Actual URL sent: https://api.myserver.com/games?page=1&limit=20
```

---

### POST JSON Request

By default, the body is encoded as `application/json`:

```cpp
QJsonObject body;
body["username"] = "johndoe";
body["password"] = "mypassword123";

connect(api, &QApi::QApiReadyObject, this, [](const QJsonObject& obj, int status) {
    qDebug() << "Token:" << obj["access_token"].toString();
});

api->Post(QUrl("https://api.myserver.com/auth/login"), body);
```

**Body sent:**
```json
{ "username": "johndoe", "password": "mypassword123" }
```

---

### POST Form URL Encoded Request

To send `application/x-www-form-urlencoded`, use `BodyType::FormUrlEncoded`:

```cpp
QJsonObject body;
body["grant_type"] = "password";
body["username"]   = "johndoe";
body["password"]   = "mypassword123";

QApi::Options opt;
opt.bodyType = QApi::BodyType::FormUrlEncoded;

api->Post(QUrl("https://api.myserver.com/oauth/token"), body, opt);

// Body sent: grant_type=password&username=johndoe&password=mypassword123
```

---

### Error Handling

All errors (invalid URL, network failure, JSON parse error) are reported through a single signal:

```cpp
connect(api, &QApi::QApiReadyErrorOccurred, this,
    [](const QString& message, int httpStatus) {

    if (httpStatus == -1) {
        // Error before the request (invalid URL, no network)
        qDebug() << "Local error:" << message;
    } else if (httpStatus == 401) {
        qDebug() << "Unauthorized — token expired?";
    } else if (httpStatus == 404) {
        qDebug() << "Resource not found";
    } else if (httpStatus >= 500) {
        qDebug() << "Server error:" << message;
    }
});
```

**Possible errors:**

| Case | httpStatus | Message |
|---|---|---|
| URL missing scheme or host | `-1` | `"Invalid API URL: missing scheme or host"` |
| Server unreachable | `-1` | Qt system error message |
| Non-JSON response | HTTP code | `"JSON parse error: ..."` |
| HTTP error (401, 404, 500…) | HTTP code | Network error message |

> ⚠️ A URL like `"localhost:3000"` is considered invalid because Qt interprets it as `scheme=localhost` with an empty host. Always use `"http://localhost:3000"`.

---

## Signals Reference

```cpp
// Full response (object or array)
void QApiReady(const QJsonDocument& json, int httpStatus);

// JSON object response  → { "key": "value" }
void QApiReadyObject(const QJsonObject& obj, int httpStatus);

// JSON array response   → [ {...}, {...} ]
void QApiReadyArray(const QJsonArray& arr, int httpStatus);

// Error (network, invalid URL, JSON parse failure)
void QApiReadyErrorOccurred(const QString& message, int httpStatus = -1);
```

> 💡 `QApiReady` is always emitted on success, followed by either `QApiReadyObject` **or** `QApiReadyArray` depending on the response type. Connect to the most specific signal for your use case.

---

## Options Reference

```cpp
QApi::Options opt;

// Custom HTTP headers (key / value as QByteArray)
opt.headers.insert("Authorization", "Bearer <token>");
opt.headers.insert("X-Custom-Header", "value");

// Body encoding for POST requests
opt.bodyType = QApi::BodyType::Json;            // application/json (default)
opt.bodyType = QApi::BodyType::FormUrlEncoded;  // application/x-www-form-urlencoded

// Query parameters appended to the URL
opt.query.addQueryItem("page", "1");
opt.query.addQueryItem("sort", "desc");

// Timeout in milliseconds (reserved for future use)
opt.timeoutMs = 30000; // 30s default
```

---

## Tests

QApi ships with a test suite based on **Qt Test**.

### Running the tests

```bash
cd build
cmake --build . --target testApi
ctest --output-on-failure
```

Or from **Qt Creator**: right-click on `testApi` → **Run**.

### Included tests

| Test | Description |
|---|---|
| `test_invalidUrl_emitsError` | URL without scheme → immediate synchronous error |
| `test_invalidUrl_async_emitsError` | Closed port → asynchronous network error |
| `test_get_realEndpoint` | Real GET on a public endpoint |
| `test_post_jsonBody` | POST with correctly encoded JSON body |
| `test_customHeaders` | Custom headers correctly forwarded to the server |

### Expected output

```
PASS   : testApi::initTestCase()
PASS   : testApi::test_invalidUrl_emitsError()
PASS   : testApi::test_invalidUrl_async_emitsError()
PASS   : testApi::test_get_realEndpoint()
PASS   : testApi::test_post_jsonBody()
PASS   : testApi::test_customHeaders()
PASS   : testApi::cleanupTestCase()
Totals: 7 passed, 0 failed
```

---

## Project Structure

```
QApi/
  CMakeLists.txt          ← shared library build
  QApi_global.h           ← QAPI_EXPORT macros (dllexport / dllimport)
  qapi.h                  ← public interface
  qapi.cpp                ← implementation
  QApiTest/
    CMakeLists.txt        ← test build
    tst_testapi.cpp       ← Qt Test suite
```

---

## FAQ

**Q: Do I need to manage `QApi` memory manually?**

No, as long as you pass a QObject `parent` to the constructor. Qt will automatically delete the instance when the parent is destroyed.

```cpp
QApi* api = new QApi(this); // deleted when "this" is destroyed
```

---

**Q: Can I fire multiple requests in parallel with the same instance?**

Yes. `QNetworkAccessManager` handles multiple concurrent requests. However, signals like `QApiReady`, `QApiReadyObject`, etc. will be emitted for **each response**. If you need to tell responses apart, create one instance per request or add an identifier in your callbacks.

```cpp
// Two parallel requests with separate instances
auto* apiGames = new QApi(this);
auto* apiUser  = new QApi(this);

connect(apiGames, &QApi::QApiReadyArray,  this, &MyClass::onGamesReady);
connect(apiUser,  &QApi::QApiReadyObject, this, &MyClass::onUserReady);

apiGames->Get(QUrl("https://api.myserver.com/games"));
apiUser->Get(QUrl("https://api.myserver.com/user"));
```

---

**Q: How do I attach a JWT token to all my requests?**

Create a small wrapper in your project that injects the header automatically:

```cpp
void MyApiService::authenticatedGet(const QUrl& url) {
    QApi::Options opt;
    opt.headers.insert("Authorization",
        QString("Bearer %1").arg(m_token).toUtf8());
    m_api->Get(url, opt);
}
```

---

**Q: Why does the URL `localhost:3000` not work?**

Qt parses `"localhost:3000"` as `scheme=localhost` with an empty host. Always provide the scheme explicitly:

```cpp
// ❌ Wrong
api->Get(QUrl("localhost:3000/api/user"));

// ✅ Correct
api->Get(QUrl("http://localhost:3000/api/user"));
```

---

**Q: How do I update QApi in my project?**

```bash
cd libs/QApi
git pull origin main        # fetch the latest version
cd ../..
git add libs/QApi
git commit -m "update: QApi"
```

---

## License

Proprietary — Mecanes © 2026. All rights reserved.
