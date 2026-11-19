#include "quoteform.h"
#include "ui_quoteform.h"

QuoteForm::QuoteForm(const QString &quote, const QString &author, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QuoteForm)
    , m_text(quote)
    , m_author(author)
{
    ui->setupUi(this);

    ui->lbQuote->setWordWrap(true);

    refreshUi();
}

QuoteForm::~QuoteForm()
{
    delete ui;
}

QString QuoteForm::text() const
{
    return m_text;
}

QString QuoteForm::quoteAuthor() const
{
    return m_author;
}

void QuoteForm::setText(const QString &text)
{
    if (m_text == text) return;
    m_text = text;
    refreshUi();
}

void QuoteForm::setQuoteAuthor(const QString &author)
{
    if (m_author == author) return;
    m_author = author;
    refreshUi();
}

void QuoteForm::refreshUi()
{
    ui->lbQuote->setText(QString("«%1», —").arg(m_text));
    ui->lbAuthor->setText(m_author);
    updateGeometry();
}
