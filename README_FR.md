# QApi — Shared Library Qt pour requêtes HTTP

> Bibliothèque Qt légère et réutilisable pour gérer vos requêtes HTTP (GET / POST) avec support JSON natif, headers custom et gestion d'erreurs asynchrone.

---

## Table des matières

- [Prérequis](#prérequis)
- [Installation](#installation)
  - [Option A — Git Submodule (recommandé)](#option-a--git-submodule-recommandé)
  - [Option B — Copie manuelle](#option-b--copie-manuelle)
- [Intégration CMake](#intégration-cmake)
- [Utilisation](#utilisation)
  - [Requête GET simple](#requête-get-simple)
  - [Requête GET avec options](#requête-get-avec-options)
  - [Requête POST JSON](#requête-post-json)
  - [Requête POST Form URL Encoded](#requête-post-form-url-encoded)
  - [Gestion des erreurs](#gestion-des-erreurs)
- [Référence des signaux](#référence-des-signaux)
- [Référence des options](#référence-des-options)
- [Tests](#tests)
- [Structure du projet](#structure-du-projet)
- [FAQ](#faq)

---

## Prérequis

| Outil | Version minimale |
|---|---|
| Qt | 5.15 ou Qt 6.x |
| CMake | 3.16+ |
| Compilateur | C++17 (MSVC, GCC, Clang) |

Modules Qt requis : `Core`, `Network`

---

## Installation

### Option A — Git Submodule (recommandé)

Cette option permet de garder QApi à jour indépendamment dans votre projet.

**1. Ajouter QApi comme submodule dans votre projet**

```bash
# Depuis la racine de votre projet
git submodule add https://github.com/mecanes/QApi.git libs/QApi
git commit -m "add: QApi submodule"
```

**2. Lors du clone de votre projet sur une nouvelle machine**

```bash
# Clone avec les submodules automatiquement
git clone --recurse-submodules https://github.com/votre-org/votre-projet.git

# Ou si vous avez déjà cloné sans le flag
git submodule update --init
```

**3. Mettre à jour QApi vers la dernière version**

```bash
cd libs/QApi
git pull origin main
cd ../..
git add libs/QApi
git commit -m "update: QApi to latest version"
```

---

### Option B — Copie manuelle

Si vous ne souhaitez pas utiliser Git Submodule, copiez simplement le dossier `QApi` dans votre projet.

```
MonProjet/
  libs/
    QApi/           ← dossier copié ici
      CMakeLists.txt
      qapi.h
      qapi.cpp
      QApi_global.h
  src/
    main.cpp
  CMakeLists.txt
```

> ⚠️ Avec cette méthode, les mises à jour de QApi devront être faites manuellement.

---

## Intégration CMake

Une fois QApi présent dans votre projet, modifiez votre `CMakeLists.txt` :

```cmake
cmake_minimum_required(VERSION 3.16)
project(MonProjet LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

# ── 1. Déclarer QApi avant votre exécutable ──
add_subdirectory(libs/QApi)

# ── 2. Déclarer votre exécutable ──
add_executable(MonProjet
    src/main.cpp
)

# ── 3. Lier QApi à votre projet ──
target_link_libraries(MonProjet
    PRIVATE
        QApi
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
)
```

> ✅ Le `target_include_directories` de QApi est déclaré `PUBLIC`, donc vous n'avez pas besoin de l'ajouter manuellement.

---

## Utilisation

### Import

```cpp
#include "qapi.h"
```

### Instanciation

```cpp
// Avec parent QObject (recommandé — gestion mémoire automatique)
QApi* api = new QApi(this);

// Sans parent
QApi api;
```

---

### Requête GET simple

```cpp
#include "qapi.h"

// Instancier
QApi* api = new QApi(this);

// Écouter la réponse
connect(api, &QApi::QApiReadyObject, this, [](const QJsonObject& obj, int status) {
    qDebug() << "Status HTTP :" << status;
    qDebug() << "Pseudo :" << obj["pseudo"].toString();
    qDebug() << "Email  :" << obj["email"].toString();
});

// Écouter les erreurs
connect(api, &QApi::QApiReadyErrorOccurred, this, [](const QString& err, int status) {
    qDebug() << "Erreur" << status << ":" << err;
});

// Lancer la requête
api->Get(QUrl("https://api.monserveur.com/user"));
```

**Réponse JSON attendue :**
```json
{ "pseudo": "johndoe", "email": "john.doe@example.com" }
```

---

### Requête GET — réponse Array

Si votre endpoint retourne un tableau JSON, connectez-vous à `QApiReadyArray` :

```cpp
connect(api, &QApi::QApiReadyArray, this, [](const QJsonArray& arr, int status) {
    qDebug() << "Nombre d'items :" << arr.size();

    for (const QJsonValue& val : arr) {
        QJsonObject item = val.toObject();
        qDebug() << item["title"].toString();
    }
});

api->Get(QUrl("https://api.monserveur.com/games"));
```

**Réponse JSON attendue :**
```json
[
  { "id": 1, "title": "Game One" },
  { "id": 2, "title": "Game Two" }
]
```

---

### Requête GET avec options

Vous pouvez passer des headers custom et des query params via `QApi::Options` :

```cpp
QApi::Options opt;

// Headers custom
opt.headers.insert("Authorization", "Bearer mon_token_jwt");
opt.headers.insert("X-App-Version", "1.0.0");

// Query params (?page=1&limit=20)
opt.query.addQueryItem("page", "1");
opt.query.addQueryItem("limit", "20");

api->Get(QUrl("https://api.monserveur.com/games"), opt);

// URL réelle envoyée : https://api.monserveur.com/games?page=1&limit=20
```

---

### Requête POST JSON

Par défaut, le body est encodé en `application/json` :

```cpp
QJsonObject body;
body["username"] = "johndoe";
body["password"] = "motdepasse123";

connect(api, &QApi::QApiReadyObject, this, [](const QJsonObject& obj, int status) {
    qDebug() << "Token :" << obj["access_token"].toString();
});

api->Post(QUrl("https://api.monserveur.com/auth/login"), body);
```

**Body envoyé :**
```json
{ "username": "johndoe", "password": "motdepasse123" }
```

---

### Requête POST Form URL Encoded

Pour envoyer en `application/x-www-form-urlencoded`, utilisez `BodyType::FormUrlEncoded` :

```cpp
QJsonObject body;
body["grant_type"] = "password";
body["username"]   = "johndoe";
body["password"]   = "motdepasse123";

QApi::Options opt;
opt.bodyType = QApi::BodyType::FormUrlEncoded;

api->Post(QUrl("https://api.monserveur.com/oauth/token"), body, opt);

// Body envoyé : grant_type=password&username=johndoe&password=motdepasse123
```

---

### Gestion des erreurs

Toutes les erreurs (URL invalide, réseau, parse JSON) sont remontées via un seul signal :

```cpp
connect(api, &QApi::QApiReadyErrorOccurred, this,
    [](const QString& message, int httpStatus) {

    if (httpStatus == -1) {
        // Erreur avant la requête (URL invalide, pas de réseau)
        qDebug() << "Erreur locale :" << message;
    } else if (httpStatus == 401) {
        qDebug() << "Non autorisé — token expiré ?";
    } else if (httpStatus == 404) {
        qDebug() << "Ressource introuvable";
    } else if (httpStatus >= 500) {
        qDebug() << "Erreur serveur :" << message;
    }
});
```

**Erreurs possibles :**

| Cas | httpStatus | Message |
|---|---|---|
| URL sans scheme ou host | `-1` | `"Invalid API URL: missing scheme or host"` |
| Serveur injoignable | `-1` | Message système Qt |
| Réponse non JSON | code HTTP | `"JSON parse error: ..."` |
| Erreur HTTP (401, 404, 500...) | code HTTP | Message de l'erreur réseau |

> ⚠️ Une URL comme `"localhost:3000"` est considérée invalide car Qt l'interprète comme `scheme=localhost` sans host. Utilisez toujours `"http://localhost:3000"`.

---

## Référence des signaux

```cpp
// Réponse complète (objet ou array)
void QApiReady(const QJsonDocument& json, int httpStatus);

// Réponse de type JSON object  → { "key": "value" }
void QApiReadyObject(const QJsonObject& obj, int httpStatus);

// Réponse de type JSON array   → [ {...}, {...} ]
void QApiReadyArray(const QJsonArray& arr, int httpStatus);

// Erreur (réseau, URL invalide, parse JSON)
void QApiReadyErrorOccurred(const QString& message, int httpStatus = -1);
```

> 💡 `QApiReady` est toujours émis en cas de succès, puis `QApiReadyObject` **ou** `QApiReadyArray` selon le type de réponse. Connectez-vous au signal le plus précis pour votre cas.

---

## Référence des options

```cpp
QApi::Options opt;

// Headers HTTP custom (clé / valeur en QByteArray)
opt.headers.insert("Authorization", "Bearer <token>");
opt.headers.insert("X-Custom-Header", "valeur");

// Encodage du body pour les requêtes POST
opt.bodyType = QApi::BodyType::Json;           // application/json (défaut)
opt.bodyType = QApi::BodyType::FormUrlEncoded; // application/x-www-form-urlencoded

// Query params ajoutés à l'URL
opt.query.addQueryItem("page", "1");
opt.query.addQueryItem("sort", "desc");

// Timeout en millisecondes (réservé pour usage futur)
opt.timeoutMs = 30000; // 30s par défaut
```

---

## Tests

QApi est livré avec une suite de tests basée sur **Qt Test**.

### Lancer les tests

```bash
cd build
cmake --build . --target testApi
ctest --output-on-failure
```

Ou depuis **Qt Creator** : clic droit sur `testApi` → **Run**.

### Tests inclus

| Test | Description |
|---|---|
| `test_invalidUrl_emitsError` | URL sans scheme → erreur immédiate synchrone |
| `test_invalidUrl_async_emitsError` | Port fermé → erreur réseau asynchrone |
| `test_get_realEndpoint` | GET réel sur endpoint public |
| `test_post_jsonBody` | POST JSON encodé correctement |
| `test_customHeaders` | Headers custom transmis au serveur |

### Résultat attendu

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

## Structure du projet

```
QApi/
  CMakeLists.txt          ← build de la shared library
  QApi_global.h           ← macros QAPI_EXPORT (dllexport / dllimport)
  qapi.h                  ← interface publique
  qapi.cpp                ← implémentation
  QApiTest/
    CMakeLists.txt        ← build des tests
    tst_testapi.cpp       ← suite de tests Qt Test
```

---

## FAQ

**Q : Dois-je gérer la mémoire des `QApi` manuellement ?**

Non, si vous passez un `parent` QObject au constructeur. Qt supprimera l'instance automatiquement quand le parent est détruit.

```cpp
QApi* api = new QApi(this); // supprimé quand "this" est détruit
```

---

**Q : Puis-je faire plusieurs requêtes en parallèle avec la même instance ?**

Oui. `QNetworkAccessManager` gère plusieurs requêtes simultanées. Cependant, les signaux `QApiReady`, `QApiReadyObject` etc. seront émis pour **chaque réponse** — si vous avez besoin de distinguer les réponses, créez une instance par requête ou ajoutez un identifiant dans vos callbacks.

```cpp
// Exemple avec deux requêtes parallèles distinguées
auto* apiGames = new QApi(this);
auto* apiUser  = new QApi(this);

connect(apiGames, &QApi::QApiReadyArray,  this, &MyClass::onGamesReady);
connect(apiUser,  &QApi::QApiReadyObject, this, &MyClass::onUserReady);

apiGames->Get(QUrl("https://api.monserveur.com/games"));
apiUser->Get(QUrl("https://api.monserveur.com/user"));
```

---

**Q : Comment ajouter un token JWT à toutes mes requêtes ?**

Créez un wrapper dans votre projet qui injecte le header automatiquement :

```cpp
void MyApiService::authenticatedGet(const QUrl& url) {
    QApi::Options opt;
    opt.headers.insert("Authorization",
        QString("Bearer %1").arg(m_token).toUtf8());
    m_api->Get(url, opt);
}
```

---

**Q : L'URL `localhost:3000` ne fonctionne pas, pourquoi ?**

Qt parse `"localhost:3000"` comme `scheme=localhost` avec un host vide. Utilisez toujours le scheme explicitement :

```cpp
// ❌ Incorrect
api->Get(QUrl("localhost:3000/api/user"));

// ✅ Correct
api->Get(QUrl("http://localhost:3000/api/user"));
```

---

**Q : Comment mettre à jour QApi dans mon projet ?**

```bash
cd libs/QApi
git pull origin main        # récupère la dernière version
cd ../..
git add libs/QApi
git commit -m "update: QApi"
```

---

## Licence

Propriétaire — Mecanes © 2026. Tous droits réservés.
