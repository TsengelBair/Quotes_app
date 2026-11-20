#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <optional>
#include <QSqlDatabase>
#include <QString>

#include "structs.h"

class DbHandler
{
public:
    explicit DbHandler(const QString &dbPath = QStringLiteral("quotes_app.db"));
    ~DbHandler();

    bool isOpen() const { return m_db.isOpen(); };

    std::optional<QVector<Country>> loadData();

    int addCountry(const QString &name);
    int addAuthor(int countryId, const QString &name);
    int addBook(int authorId, const QString &name);
    int addQuote(int bookId, const QString &text, const QString &author);
    bool editQuote(int id, const QString &newText, const QString &newAuthor);

    bool removeCountry(int id);
    bool removeAuthor(int id);
    bool removeBook(int id);
    bool removeQuote(int id);

private:
    bool initSchema();

private:
    QSqlDatabase m_db;
};

#endif // DBHANDLER_H
