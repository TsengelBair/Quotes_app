#ifndef QUOTEFORM_H
#define QUOTEFORM_H

#include <QWidget>
#include <QString>

namespace Ui {
class QuoteForm;
}

class QuoteForm : public QWidget
{
    Q_OBJECT

public:
    explicit QuoteForm(const QString &quote, const QString &author, QWidget *parent = nullptr);
    ~QuoteForm();

    QString text() const;
    QString quoteAuthor() const;

    void setText(const QString &text);
    void setQuoteAuthor(const QString &author);

private:
    void refreshUi();

    Ui::QuoteForm *ui;
    QString m_text;
    QString m_author;
};

#endif // QUOTEFORM_H
