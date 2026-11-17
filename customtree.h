#ifndef CUSTOMTREE_H
#define CUSTOMTREE_H

#include <QString>
#include <QVector>

#include "structs.h"

class CustomTree
{
public:
    CustomTree() = default;
    ~CustomTree() = default;

    AddResult addCountry(const QString &name);
    AddResult addAuthor(const QString & countryName, const QString &name);
    AddResult addBook(const QString & countryName, const QString &authorName, const QString &name);
    AddStatus addQuote(const QString &countryName, const QString &authorName, const QString &bookName,
                                                      const QString &text, const QString &quoteAuthor);

private:
    Country* findCountry(const QString &name);
    Author*  findAuthor(const QString &countryName, const QString &name);
    Book*    findBook(const QString & countryName, const QString &authorName, const QString &name);

private:
    QVector<Country> m_countries;
};

#endif // CUSTOMTREE_H
