#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include "structs.h"

class QLineEdit;
class QPlainTextEdit;
class QDialogButtonBox;

class InputDialog : public QDialog
{
    Q_OBJECT

public:
    /// type          — Country / Author / Book / Quote
    /// initialFirst  — начальное значение 1-го поля (имя / текст цитаты)
    /// initialSecond — начальное значение 2-го поля (автор цитаты; только для AddQuote)
    /// editMode      — true → заголовок «Редактировать ...»
    explicit InputDialog(DialogType type,
                         QWidget *parent = nullptr,
                         const QString &initialFirst  = QString(),
                         bool editMode = false,
                         const QString &initialSecond = QString());

    QString firstValue() const;
    QString secondValue() const;

    QString name()  const { return firstValue();  }   /// страна/автор/книга
    QString text()  const { return firstValue();  }   /// цитата — текст
    QString quoteAuthor() const { return secondValue(); } /// цитата — автор

private slots:
    void updateOkEnabled();

private:
    DialogType m_type;
    bool m_editMode = false;

    QLineEdit        *m_firstEdit  = nullptr;
    QPlainTextEdit   *m_textEdit   = nullptr;
    QLineEdit        *m_secondEdit = nullptr;
    QDialogButtonBox *m_buttons    = nullptr;

    QString firstFieldText() const;
};

#endif // INPUTDIALOG_H
