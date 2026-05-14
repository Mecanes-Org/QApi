#include "qapi.h"

#include <QJsonParseError>

QApi::QApi(QObject* parent)
    : QObject(parent)
{}

void QApi::applyHeaders(QNetworkRequest& req, const Options& opt)
{
    req.setHeader(QNetworkRequest::UserAgentHeader, "QApi/1.0");
    for (auto it = opt.headers.cbegin(); it != opt.headers.cend(); ++it)
        req.setRawHeader(it.key(), it.value());
}

QByteArray QApi::encodeBody(const QJsonValue& body, const Options& opt, QString* outContentType) const
{
    if (opt.bodyType == BodyType::Json) {
        if (outContentType) *outContentType = "application/json";
        QJsonDocument doc;
        if (body.isArray())        doc = QJsonDocument(body.toArray());
        else if (body.isObject())  doc = QJsonDocument(body.toObject());
        else                       doc = QJsonDocument(QJsonObject{{"value", body.toVariant().toString()}});
        return doc.toJson(QJsonDocument::Compact);
    }

    if (outContentType) *outContentType = "application/x-www-form-urlencoded";
    if (!body.isObject()) return {};

    QUrlQuery q;
    const QJsonObject obj = body.toObject();
    for (auto it = obj.begin(); it != obj.end(); ++it)
        q.addQueryItem(it.key(), it.value().toVariant().toString());
    return q.query(QUrl::FullyEncoded).toUtf8();
}

void QApi::Get(const QUrl& api)                              { Get(api, Options{}); }
void QApi::Post(const QUrl& api, const QJsonValue& body)     { Post(api, body, Options{}); }

void QApi::Get(const QUrl& api, const Options& opt)
{
    // isValid() ne suffit pas — une URL sans scheme passe quand même
    if (!api.isValid() || api.scheme().isEmpty() || api.host().isEmpty()) {
        emit QApiReadyErrorOccurred("Invalid API URL: missing scheme or host");
        return;
    }

    QUrl url = api;
    if (!opt.query.isEmpty()) {
        QUrlQuery q(url);
        for (const auto& it : opt.query.queryItems(QUrl::FullyDecoded))
            q.addQueryItem(it.first, it.second);
        url.setQuery(q);
    }

    QNetworkRequest req(url);
    applyHeaders(req, opt);
    handleReply(m_nam.get(req));
}

void QApi::Post(const QUrl& api, const QJsonValue& body, const Options& opt)
{
    if (!api.isValid() || api.scheme().isEmpty() || api.host().isEmpty()) {
        emit QApiReadyErrorOccurred("Invalid API URL: missing scheme or host");
        return;
    }

    QUrl url = api;
    if (!opt.query.isEmpty()) {
        QUrlQuery q(url);
        for (const auto& it : opt.query.queryItems(QUrl::FullyDecoded))
            q.addQueryItem(it.first, it.second);
        url.setQuery(q);
    }

    QNetworkRequest req(url);
    applyHeaders(req, opt);

    QString contentType;
    const QByteArray payload = encodeBody(body, opt, &contentType);

    if (!opt.headers.contains("Content-Type"))
        req.setHeader(QNetworkRequest::ContentTypeHeader, contentType);

    handleReply(m_nam.post(req, payload));
}

void QApi::handleReply(QNetworkReply* reply)
{
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (reply->error() != QNetworkReply::NoError) {
            emit QApiReadyErrorOccurred(reply->errorString(), status);
            reply->deleteLater();
            return;
        }

        const QByteArray body = reply->readAll();
        QJsonParseError pe;
        QJsonDocument doc = QJsonDocument::fromJson(body, &pe);

        if (pe.error != QJsonParseError::NoError) {
            emit QApiReadyErrorOccurred(QString("JSON parse error: %1").arg(pe.errorString()), status);
            reply->deleteLater();
            return;
        }

        emit QApiReady(doc, status);
        if (doc.isArray())  emit QApiReadyArray(doc.array(), status);
        if (doc.isObject()) emit QApiReadyObject(doc.object(), status);

        reply->deleteLater();
    });
}