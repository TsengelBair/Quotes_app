#include "inputdialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>

InputDialog::InputDialog(DialogType type,
                         QWidget *parent,
                         const QString &initialFirst,
                         bool editMode,
                         const QString &initialSecond)
    : QDialog(parent),
      m_type(type),
      m_editMode(editMode)
{
    setModal(true);

    auto *form = new QFormLayout;

    switch (m_type)
    {
    case DialogType::Country:
        setWindowTitle(m_editMode ? tr("Edit country") : tr("Add country"));
        m_firstEdit = new QLineEdit(this);
        m_firstEdit->setObjectName("inputEdit");
        m_firstEdit->setPlaceholderText(tr("Country name"));
        m_firstEdit->setText(initialFirst);
        form->addRow(tr("Country:"), m_firstEdit);
        connect(m_firstEdit, &QLineEdit::textChanged, this, &InputDialog::updateOkEnabled);
        break;

    case DialogType::Author:
        setWindowTitle(m_editMode ? tr("Edit author") : tr("Add author"));
        m_firstEdit = new QLineEdit(this);
        m_firstEdit->setObjectName("inputEdit");
        m_firstEdit->setPlaceholderText(tr("Author name"));
        m_firstEdit->setText(initialFirst);
        form->addRow(tr("Author:"), m_firstEdit);
        connect(m_firstEdit, &QLineEdit::textChanged, this, &InputDialog::updateOkEnabled);
        break;

    case DialogType::Book:
        setWindowTitle(m_editMode ? tr("Edit book") : tr("Add book"));
        m_firstEdit = new QLineEdit(this);
        m_firstEdit->setObjectName("inputEdit");
        m_firstEdit->setPlaceholderText(tr("Book title"));
        m_firstEdit->setText(initialFirst);
        form->addRow(tr("Book:"), m_firstEdit);
        connect(m_firstEdit, &QLineEdit::textChanged, this, &InputDialog::updateOkEnabled);
        break;

    case DialogType::Quote:
        setWindowTitle(m_editMode ? tr("Edit quote") : tr("Add quote"));
        m_textEdit = new QPlainTextEdit(this);
        m_textEdit->setObjectName("inputText");
        m_textEdit->setPlaceholderText(tr("Quote text..."));
        m_textEdit->setMinimumHeight(100);
        m_textEdit->setTabChangesFocus(true);
        m_textEdit->setPlainText(initialFirst);
        form->addRow(tr("Text:"), m_textEdit);

        m_secondEdit = new QLineEdit(this);
        m_secondEdit->setObjectName("inputEdit");
        m_secondEdit->setPlaceholderText(tr("Quote author (character), optional"));
        m_secondEdit->setText(initialSecond);
        form->addRow(tr("Author:"), m_secondEdit);

        connect(m_textEdit, &QPlainTextEdit::textChanged, this, &InputDialog::updateOkEnabled);
        break;
    }

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_buttons);

    updateOkEnabled();

    if (m_firstEdit)        m_firstEdit->setFocus();
    else if (m_textEdit)    m_textEdit->setFocus();
}

QString InputDialog::firstFieldText() const
{
    if (m_firstEdit) return m_firstEdit->text().trimmed();
    if (m_textEdit)  return m_textEdit->toPlainText().trimmed();
    return QString();
}

QString InputDialog::firstValue() const
{
    return firstFieldText();
}

QString InputDialog::secondValue() const
{
    if (m_secondEdit) return m_secondEdit->text().trimmed();
    return QString();
}

void InputDialog::updateOkEnabled()
{
    QPushButton *ok = m_buttons ? m_buttons->button(QDialogButtonBox::Ok) : nullptr;
    if (!ok) return;

    // Only the first field is required (name / quote text).
    // For a quote, the author (m_secondEdit) is optional — does not affect OK.
    ok->setEnabled(!firstFieldText().isEmpty());
}
