#include <QDebug>
#include "customtree.h"

/// компараторы, которые используем 4 параметром в std::lower_bound (чтобы не дублировать лямбды)
namespace
{
    bool countryComparator(const Country &a, const Country &b) { return QString::localeAwareCompare(a.name, b.name) < 0; }
    bool countryComparatorName(const Country &a, const QString &b) { return QString::localeAwareCompare(a.name, b) < 0; }
    bool authorComparator(const Author &a, const Author &b) { return QString::localeAwareCompare(a.name, b.name) < 0; }
    bool authorComparatorName(const Author &a, const QString &b) { return QString::localeAwareCompare(a.name, b) < 0; }
    bool bookComparator(const Book &a, const Book &b) { return QString::localeAwareCompare(a.name, b.name) < 0; }
    bool bookComparatorName(const Book &a, const QString &b) { return QString::localeAwareCompare(a.name, b) < 0; }
}

/// не исп-ем findCountry, чтобы не было двойного бинарного поиска через std::lower_bound
AddResult CustomTree::addCountry(const QString &name)
{
    AddResult res;

    if (name.isEmpty()) {
        res.status = AddStatus::EmptyName;
        return res;
    }

    Country c;
    c.name = name;

    auto countryIt = std::lower_bound(m_countries.begin(), m_countries.end(), c, countryComparator);
    if (countryIt != m_countries.end() && QString::localeAwareCompare(countryIt->name, name) == 0) {
        res.status = AddStatus::AlreadyExists;
        return res;
    }

    /// позиция вставки — расстояние до итератора ДО insert
    res.row = static_cast<int>(countryIt - m_countries.begin());
    m_countries.insert(countryIt, c);

    res.status = AddStatus::Added;
    return res;
}

AddResult CustomTree::addAuthor(const QString &countryName, const QString &name)
{
    AddResult res;

    if (countryName.isEmpty() || name.isEmpty()) {
        res.status = AddStatus::EmptyName;
        return res;
    }

    Country* country = findCountry(countryName);
    if (!country) {
        res.status = AddStatus::NotFound;
        return res;
    }

    Author a;
    a.name = name;

    auto authorIt = std::lower_bound(country->authors.begin(), country->authors.end(), a, authorComparator);
    if (authorIt != country->authors.end() && QString::localeAwareCompare(authorIt->name, name) == 0) {
        res.status = AddStatus::AlreadyExists;
        return res;
    }

    res.row = static_cast<int>(authorIt - country->authors.begin());
    country->authors.insert(authorIt, a);

    res.status = AddStatus::Added;
    return res;
}

AddResult CustomTree::addBook(const QString &countryName, const QString &authorName, const QString &name)
{
    AddResult res;

    if (countryName.isEmpty() || authorName.isEmpty() || name.isEmpty()) {
        res.status = AddStatus::EmptyName;
        return res;
    }

    Author* author = findAuthor(countryName, authorName);
    if (!author) {
        res.status = AddStatus::NotFound;
        return res;
    }

    Book b;
    b.name = name;

    auto bookIt = std::lower_bound(author->books.begin(), author->books.end(), b, bookComparator);
    if (bookIt != author->books.end() && QString::localeAwareCompare(bookIt->name, name) == 0) {
        res.status = AddStatus::AlreadyExists;
        return res;
    }

    res.row = static_cast<int>(bookIt - author->books.begin());

    author->books.insert(bookIt, b);

    res.status = AddStatus::Added;
    return res;
}

AddResult CustomTree::addQuote(const QString &countryName, const QString &authorName, const QString &bookName,
                               const QString &text, const QString &quoteAuthor)
{
    AddResult res;

    if (countryName.isEmpty() || authorName.isEmpty() || bookName.isEmpty() || text.isEmpty()) {
        res.status = AddStatus::EmptyName;
        return res;
    }


    Book *book = findBook(countryName, authorName, bookName);
    if (!book) {
        res.status = AddStatus::NotFound;
        return res;
    }

    /// цитаты не сортируем, просто добавляем в конец
    Quote q;
    q.text = text;
    q.author = quoteAuthor;

    res.row = book->quotes.size();
    book->quotes.append(q);

    res.status = AddStatus::Added;
    return res;
}

Country *CustomTree::findCountry(const QString &name)
{
    if (name.isEmpty()) return nullptr;

    auto it = std::lower_bound(m_countries.begin(), m_countries.end(), name, countryComparatorName);
    if (it == m_countries.end() || QString::localeAwareCompare(it->name, name) != 0)
        return nullptr;

    return &(*it);
}

Author *CustomTree::findAuthor(const QString &countryName, const QString &name)
{
    if (countryName.isEmpty() || name.isEmpty()) return nullptr;

    Country* country = findCountry(countryName);
    if (!country) return nullptr;

    auto it = std::lower_bound(country->authors.begin(), country->authors.end(), name, authorComparatorName);
    if (it == country->authors.end() || QString::localeAwareCompare(it->name, name) != 0)
        return nullptr;

    return &(*it);
}

Book *CustomTree::findBook(const QString &countryName, const QString &authorName, const QString &name)
{
    if (countryName.isEmpty() || authorName.isEmpty() || name.isEmpty()) return nullptr;

    Author *author = findAuthor(countryName, authorName);
    if (!author) return nullptr;

    auto it = std::lower_bound(author->books.begin(), author->books.end(), name, bookComparatorName);
    if (it == author->books.end() || QString::localeAwareCompare(it->name, name) != 0)
        return nullptr;

    return &(*it);
}
