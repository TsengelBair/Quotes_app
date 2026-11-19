#include "quoteform.h"
#include "ui_quoteform.h"

QuoteForm::QuoteForm(const QString &quote, const QString &author, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QuoteForm)
{
    ui->setupUi(this);

    ui->lbQuote->setWordWrap(true);

    const QString formatted = QString("«%1», —").arg(quote);
    ui->lbQuote->setText(formatted);
    ui->lbAuthor->setText(author);
}

QuoteForm::~QuoteForm()
{
    delete ui;
}
