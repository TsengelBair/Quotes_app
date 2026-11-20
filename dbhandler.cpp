#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileInfo>

#include "dbhandler.h"

DbHandler::DbHandler(const QString &dbPath)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        /// добавить эмит сигнала, чтобы можно было отобразить сообщение в MainWindow
        qDebug() << "Database connection error:" << m_db.lastError().text();
        return;
    }

    QSqlQuery pragma(m_db);
    if (!pragma.exec("PRAGMA foreign_keys = ON"))
        qDebug() << "PRAGMA foreign_keys error:" << pragma.lastError().text();

    if (!initSchema()) {
        /// добавить эмит сигнала, чтобы можно было отобразить сообщение в MainWindow
        qDebug() << "initSchema: не удалось создать/проверить схему БД";
    }
}

DbHandler::~DbHandler()
{
    if (m_db.isOpen())
        m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

std::optional<QVector<Country>> DbHandler::loadData()
{
    QVector<Country> countries;
    if (!m_db.isOpen()) return std::nullopt;

    QSqlQuery query(m_db);

    if (!query.exec("SELECT id, name FROM countries ORDER BY name")) {
        qDebug() << "loadData (countries) error:" << query.lastError().text();
        return std::nullopt;
    }

    QHash<int, int> countriesData; // country.id -> индекс в countries
    while (query.next()) {
        Country c;
        c.id = query.value("id").toInt();
        c.name = query.value("name").toString();
        countriesData.insert(c.id, countries.size());
        countries.append(c);
    }

    if (!query.exec("SELECT id, country_id, name FROM authors ORDER BY name")) {
        qDebug() << "loadData (authors) error:" << query.lastError().text();
        return std::nullopt;
    }

    QHash<int, QPair<int, int>> authorsData; // author.id -> (countryIdx, authorIdx)
    while (query.next()) {
        const int authorId  = query.value("id").toInt();
        const int countryId = query.value("country_id").toInt();

        auto it = countriesData.find(countryId);
        if (it == countriesData.end()) {
            qDebug() << "loadData: автор" << authorId
                     << "ссылается на несуществующую страну" << countryId;
            continue;
        }

        const int countryIdx = it.value();
        Author a;
        a.id   = authorId;
        a.name = query.value("name").toString();

        const int authorIdx = countries[countryIdx].authors.size();
        countries[countryIdx].authors.append(a);
        authorsData.insert(authorId, {countryIdx, authorIdx});
    }

    if (!query.exec("SELECT id, author_id, name FROM books ORDER BY name")) {
        qDebug() << "loadData (books) error:" << query.lastError().text();
        return std::nullopt;
    }

    QHash<int, std::tuple<int, int, int>> booksData; // book.id -> (countryIdx, authorIdx, bookIdx)
    while (query.next()) {
        const int bookId   = query.value("id").toInt();
        const int authorId = query.value("author_id").toInt();

        auto it = authorsData.find(authorId);
        if (it == authorsData.end()) {
            qDebug() << "loadData: книга" << bookId
                     << "ссылается на несуществующего автора" << authorId;
            continue;
        }

        const int countryIdx = it.value().first;
        const int authorIdx  = it.value().second;

        Book b;
        b.id   = bookId;
        b.name = query.value("name").toString();

        const int bookIdx = countries[countryIdx].authors[authorIdx].books.size();
        countries[countryIdx].authors[authorIdx].books.append(b);
        booksData.insert(bookId, {countryIdx, authorIdx, bookIdx});
    }

    if (!query.exec("SELECT id, book_id, text, author FROM quotes ORDER BY id")) {
        qDebug() << "loadData (quotes) error:" << query.lastError().text();
        return std::nullopt;
    }

    while (query.next()) {
        const int quoteId = query.value("id").toInt();
        const int bookId  = query.value("book_id").toInt();

        auto it = booksData.find(bookId);
        if (it == booksData.end()) {
            qDebug() << "loadData: цитата" << quoteId
                     << "ссылается на несуществующую книгу" << bookId;
            continue;
        }

        const int countryIdx = std::get<0>(it.value());
        const int authorIdx  = std::get<1>(it.value());
        const int bookIdx    = std::get<2>(it.value());

        Quote q;
        q.id     = quoteId;
        q.text   = query.value("text").toString();
        q.author = query.value("author").toString();
        countries[countryIdx].authors[authorIdx].books[bookIdx].quotes.append(q);
    }

    return countries;
}

int DbHandler::addCountry(const QString &name)
{
    if (!m_db.isOpen()) {
        qDebug() << "addCountry: база не открыта";
        return -1;
    }
    if (name.isEmpty()) {
        qDebug() << "addCountry: пустое имя";
        return -1;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO countries (name) VALUES (:name)");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "addCountry error:" << query.lastError().text();
        return -1;
    }

    const QVariant id = query.lastInsertId();
    if (!id.isValid()) {
        qDebug() << "addCountry: не удалось получить id";
        return -1;
    }
    return id.toInt();
}

int DbHandler::addAuthor(int countryId, const QString &name)
{
    if (!m_db.isOpen()) {
        qDebug() << "addAuthor: база не открыта";
        return -1;
    }
    if (name.isEmpty() || countryId <= 0) {
        qDebug() << "addAuthor: пустое имя или неверный countryId";
        return -1;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO authors (country_id, name) VALUES (:country_id, :name)");
    query.bindValue(":country_id", countryId);
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "addAuthor error:" << query.lastError().text();
        return -1;
    }

    const QVariant id = query.lastInsertId();
    if (!id.isValid()) {
        qDebug() << "addAuthor: не удалось получить id";
        return -1;
    }
    return id.toInt();
}

int DbHandler::addBook(int authorId, const QString &name)
{
    if (!m_db.isOpen()) {
        qDebug() << "addBook: база не открыта";
        return -1;
    }
    if (name.isEmpty() || authorId <= 0) {
        qDebug() << "addBook: пустое имя или неверный authorId";
        return -1;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO books (author_id, name) VALUES (:author_id, :name)");
    query.bindValue(":author_id", authorId);
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "addBook error:" << query.lastError().text();
        return -1;
    }

    const QVariant id = query.lastInsertId();
    if (!id.isValid()) {
        qDebug() << "addBook: не удалось получить id";
        return -1;
    }
    return id.toInt();
}

int DbHandler::addQuote(int bookId, const QString &text, const QString &author)
{
    if (!m_db.isOpen()) {
        qDebug() << "addQuote: база не открыта";
        return -1;
    }
    if (text.isEmpty() || bookId <= 0) {
        qDebug() << "addQuote: пустой текст или неверный bookId";
        return -1;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO quotes (book_id, text, author) VALUES (:book_id, :text, :author)");
    query.bindValue(":book_id", bookId);
    query.bindValue(":text", text);
    query.bindValue(":author", author); // пустая строка допустима

    if (!query.exec()) {
        qDebug() << "addQuote error:" << query.lastError().text();
        return -1;
    }

    const QVariant id = query.lastInsertId();
    if (!id.isValid()) {
        qDebug() << "addQuote: не удалось получить id";
        return -1;
    }
    return id.toInt();
}

bool DbHandler::editQuote(int id, const QString &newText, const QString &newAuthor)
{
    if (!m_db.isOpen()) {
        qDebug() << "editQuote: база не открыта";
        return false;
    }
    if (id <= 0) {
        qDebug() << "editQuote: неверный id" << id;
        return false;
    }
    if (newText.isEmpty()) {
        qDebug() << "editQuote: пустой текст";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE quotes SET text = :text, author = :author WHERE id = :id");
    query.bindValue(":text", newText);
    query.bindValue(":author", newAuthor); // пустая строка допустима, как в addQuote
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "editQuote error:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() <= 0) {
        QSqlQuery check(m_db);
        check.prepare("SELECT 1 FROM quotes WHERE id = :id");
        check.bindValue(":id", id);
        if (!check.exec() || !check.next()) {
            qDebug() << "editQuote: цитата с id" << id << "не найдена";
            return false;
        }
        // запись есть, значения не изменились — no-op, успех
    }
    return true;
}

bool DbHandler::removeCountry(int id)
{
    if (!m_db.isOpen()) {
        qDebug() << "removeCountry: база не открыта";
        return false;
    }
    if (id <= 0) {
        qDebug() << "removeCountry: неверный id" << id;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM countries WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "removeCountry error:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "removeCountry: страна с id" << id << "не найдена";
        return false;
    }
    return true;
}

bool DbHandler::removeAuthor(int id)
{
    if (!m_db.isOpen()) {
        qDebug() << "removeAuthor: база не открыта";
        return false;
    }
    if (id <= 0) {
        qDebug() << "removeAuthor: неверный id" << id;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM authors WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "removeAuthor error:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "removeAuthor: автор с id" << id << "не найден";
        return false;
    }
    return true;
}

bool DbHandler::removeBook(int id)
{
    if (!m_db.isOpen()) {
        qDebug() << "removeBook: база не открыта";
        return false;
    }
    if (id <= 0) {
        qDebug() << "removeBook: неверный id" << id;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM books WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "removeBook error:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "removeBook: книга с id" << id << "не найдена";
        return false;
    }
    return true;
}

bool DbHandler::removeQuote(int id)
{
    if (!m_db.isOpen()) {
        qDebug() << "removeQuote: база не открыта";
        return false;
    }
    if (id <= 0) {
        qDebug() << "removeQuote: неверный id" << id;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM quotes WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "removeQuote error:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "removeQuote: цитата с id" << id << "не найдена";
        return false;
    }
    return true;
}

bool DbHandler::exportTo(const QString &path) const
{
    if (!m_db.isOpen()) {
        qDebug() << "exportTo: база не открыта";
        return false;
    }
    if (path.trimmed().isEmpty()) {
        qDebug() << "exportTo: пустой путь";
        return false;
    }

    const QString absPath = QFileInfo(path).absoluteFilePath();

    // VACUUM INTO не перезаписывает существующий файл — удаляем заранее.
    if (QFileInfo::exists(absPath)) {
        if (!QFile::remove(absPath)) {
            qDebug() << "exportTo: не удалось удалить существующий файл" << absPath;
            return false;
        }
    }

    // Экранируем одинарные кавычки в пути (SQLite-строка в одинарных кавычках).
    QString escaped = absPath;
    escaped.replace('\'', "''");

    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("VACUUM INTO '%1'").arg(escaped))) {
        qDebug() << "exportTo error:" << query.lastError().text()
                 << "\ntarget:" << absPath;
        return false;
    }

    return true;
}

bool DbHandler::initSchema()
{
    if (!m_db.isOpen()) {
        qDebug() << "initSchema: база не открыта";
        return false;
    }

    const QStringList statements = {
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS countries ("
            "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS authors ("
            "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  country_id INTEGER NOT NULL REFERENCES countries(id) ON DELETE CASCADE,"
            "  name       TEXT NOT NULL"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS books ("
            "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  author_id INTEGER NOT NULL REFERENCES authors(id) ON DELETE CASCADE,"
            "  name      TEXT NOT NULL"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS quotes ("
            "  id      INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  book_id INTEGER NOT NULL REFERENCES books(id) ON DELETE CASCADE,"
            "  text    TEXT NOT NULL,"
            "  author  TEXT NOT NULL DEFAULT ''"
            ")"),
        /// индексы по FK — ускоряют каскадное удаление и выборки по родителю.
        QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_authors_country_id ON authors(country_id)"),
        QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_books_author_id ON books(author_id)"),
        QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_quotes_book_id ON quotes(book_id)"),
    };

    if (!m_db.transaction()) {
        qDebug() << "initSchema: не удалось начать транзакцию:"
                 << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    for (const QString &sql : statements) {
        if (!query.exec(sql)) {
            qDebug() << "initSchema error:" << query.lastError().text()
                     << "\nSQL:" << sql;
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        qDebug() << "initSchema: commit error:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return true;
}
