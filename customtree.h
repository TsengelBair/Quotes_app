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

    const QVector<Country>& getCountries() const { return m_countries; };
    void setCountries(QVector<Country> countries) { m_countries = std::move(countries); };

    AddResult addCountry(const QString &name);
    AddResult addAuthor(const QString & countryName, const QString &name);
    AddResult addBook(const QString & countryName, const QString &authorName, const QString &name);
    AddResult addQuote(const QString &countryName, const QString &authorName, const QString &bookName,
                                                      const QString &text, const QString &quoteAuthor);
    AddStatus editQuote(const QString &countryName, const QString &authorName, const QString &bookName,
                                    int quoteIndex, const QString & newText, const QString &newAuthor);

    bool removeCountry(const QString &name);
    bool removeAuthor(const QString &countryName, const QString &name);
    bool removeBook(const QString & countryName, const QString &authorName, const QString &name);
    bool removeQuote(const QString &countryName, const QString &authorName, const QString &bookName, int quoteIndex);

    /// исп-ся в mainWindow
    Book*    findBook(const QString & countryName, const QString &authorName, const QString &name);

private:
    Country* findCountry(const QString &name);
    Author*  findAuthor(const QString &countryName, const QString &name);

private:
    QVector<Country> m_countries;
};

#endif // CUSTOMTREE_H
