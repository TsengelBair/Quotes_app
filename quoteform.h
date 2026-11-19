#ifndef QUOTEFORM_H
#define QUOTEFORM_H

#include <QWidget>

namespace Ui {
class QuoteForm;
}

class QuoteForm : public QWidget
{
    Q_OBJECT

public:
    explicit QuoteForm(const QString &quote, const QString &author, QWidget *parent = nullptr);
    ~QuoteForm();

private:
    Ui::QuoteForm *ui;
};

#endif // QUOTEFORM_H
