#ifndef QAPI_H
#define QAPI_H

#include "QApi_global.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QUrl>

class QAPI_EXPORT QApi : public QObject
{
    Q_OBJECT

public:
    enum class BodyType {
        Json,           // application/json
        FormUrlEncoded  // application/x-www-form-urlencoded
    };
    Q_ENUM(BodyType)

    struct QAPI_EXPORT Options {
        QMap<QByteArray, QByteArray> headers;
        BodyType bodyType = BodyType::Json;
        QUrlQuery query;
        int timeoutMs = 30000;
    };

    explicit QApi(QObject* parent = nullptr);

    Q_INVOKABLE void Get(const QUrl& api);
    Q_INVOKABLE void Get(const QUrl& api, const Options& options);
    Q_INVOKABLE void Post(const QUrl& api, const QJsonValue& body);
    Q_INVOKABLE void Post(const QUrl& api, const QJsonValue& body, const Options& options);

signals:
    void QApiReady(const QJsonDocument& json, int httpStatus);
    void QApiReadyArray(const QJsonArray& arr, int httpStatus);
    void QApiReadyObject(const QJsonObject& obj, int httpStatus);
    void QApiReadyErrorOccurred(const QString& message, int httpStatus = -1);

private:
    void applyHeaders(QNetworkRequest& req, const Options& opt);
    QByteArray encodeBody(const QJsonValue& body, const Options& opt, QString* outContentType) const;
    void handleReply(QNetworkReply* reply);

private:
    QNetworkAccessManager m_nam;
};

#endif // QAPI_H
