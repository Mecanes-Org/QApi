#include <QTest>
#include <QSignalSpy>
#include <QCoreApplication>
#include "qapi.h"

class testApi : public QObject
{
    Q_OBJECT

public:
    testApi();
    ~testApi() override;

private slots:
    void initTestCase();
    void init();
    void cleanupTestCase();
    void cleanup();

    void test_invalidUrl_emitsError();
    void test_invalidUrl_async_emitsError();
    void test_get_realEndpoint();
    void test_post_jsonBody();
    void test_customHeaders();

private:
    QApi* m_api = nullptr;
};

testApi::testApi() {}
testApi::~testApi() = default;

void testApi::initTestCase() {}
void testApi::cleanupTestCase() {}

void testApi::init()
{
    m_api = new QApi(this);
}

void testApi::cleanup()
{
    delete m_api;
    m_api = nullptr;
}

// ── Test 1a : URL invalide syntaxiquement → émis immédiatement (synchrone) ──
void testApi::test_invalidUrl_emitsError()
{
    QSignalSpy spy(m_api, &QApi::QApiReadyErrorOccurred);

    // "not_a_url" → QUrl::isValid() == false → erreur immédiate dans ton code
    m_api->Get(QUrl("not_a_url"));

    // Synchrone : pas besoin de QTRY
    QCOMPARE(spy.count(), 1);
    QVERIFY(!spy.first().first().toString().isEmpty());
}

// ── Test 1b : URL valide mais serveur inexistant → erreur réseau async ──
void testApi::test_invalidUrl_async_emitsError()
{
    QSignalSpy spy(m_api, &QApi::QApiReadyErrorOccurred);

    m_api->Get(QUrl("http://localhost:19999")); // port fermé → erreur réseau

    // Async : doit attendre l'event loop
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 5000);
    QVERIFY(!spy.first().first().toString().isEmpty());
}

// ── Test 2 : GET réel ──
void testApi::test_get_realEndpoint()
{
    QSignalSpy spyOk(m_api, &QApi::QApiReadyArray);   // freetogame renvoie un array
    QSignalSpy spyErr(m_api, &QApi::QApiReadyErrorOccurred);

    m_api->Get(QUrl("https://www.freetogame.com/api/games"));

    QTRY_VERIFY_WITH_TIMEOUT(spyOk.count() > 0 || spyErr.count() > 0, 8000);

    QCOMPARE(spyErr.count(), 0);
    QCOMPARE(spyOk.count(), 1);
}

// ── Test 3 : POST JSON ──
void testApi::test_post_jsonBody()
{
    QSignalSpy spy(m_api, &QApi::QApiReady);

    QJsonObject body{{"username", "test"}, {"password", "1234"}};
    m_api->Post(QUrl("https://httpbin.org/post"), body);

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 8000);
}

// ── Test 4 : headers custom ──
void testApi::test_customHeaders()
{
    QSignalSpy spy(m_api, &QApi::QApiReady);

    QApi::Options opt;
    opt.headers.insert("X-Custom-Header", "mecanes");
    m_api->Get(QUrl("https://httpbin.org/get"), opt);

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 8000);

    const QJsonDocument doc = spy.first().first().value<QJsonDocument>();

    // Fix warning clazy : pas d'operator[] sur temporaire
    const QJsonObject headers = doc.object()["headers"].toObject();
    const QString val = headers["X-Custom-Header"].toString();
    QCOMPARE(val, QString("mecanes"));
}


// ── QTEST_MAIN au lieu de QTEST_APPLESS_MAIN → event loop active ──
QTEST_MAIN(testApi)
#include "tst_testapi.moc"