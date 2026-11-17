#ifndef STRUCTS_H
#define STRUCTS_H

#include <QString>
#include <QVector>

struct Quote
{
    int id = -1;    /// id из базы
    QString text;   /// текст цитаты
    QString author; /// автор цитаты (персонаж)
};

struct Book
{
    int id = -1;
    QString name;
    QVector<Quote> quotes;
};

struct Author
{
    int id = -1;
    QString name;
    QVector<Book> books;
};

struct Country
{
    int id = -1;
    QString name;
    QVector<Author> authors;
};

enum class AddStatus
{
    Added,
    AlreadyExists,
    EmptyName,
    NotFound
};

/// возвращаем результат и в случае успеха позицию для вставки
struct AddResult
{
    AddStatus status;
    int row = -1;
};

/// Тип диалога ввода — определяет заголовок и набор полей.
enum class DialogType {
    Country,   /// 1 поле: название страны
    Author,    /// 1 поле: имя автора
    Book,      /// 1 поле: название книги
    Quote      /// 2 поля: текст цитаты + автор цитаты
};

#endif // STRUCTS_H
