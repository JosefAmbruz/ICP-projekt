#ifndef QVARIABLEROW_H
#define QVARIABLEROW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class QVariableRow : public QWidget
{
    Q_OBJECT
public:
    explicit QVariableRow(QWidget *parent = nullptr, const QString &name = "", const QString &value = "");

    void setVariableName(const QString &name);
    void setVariableValue(const QString &value);
    QString getVariableValue() const;

private:
    QLabel *nameLabel;
    QLineEdit *valueEdit;
signals:
};

#endif // QVARIABLEROW_H

