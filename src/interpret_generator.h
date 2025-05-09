#ifndef INTERPRET_GENERATOR_H
#define INTERPRET_GENERATOR_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <algorithm> // For std::replace, std::remove_if
#include <set>       // For ordered unique function names

#include "spec_parser/automaton-data.hpp"

class InterpretGenerator : public QObject
{
    Q_OBJECT
public:
    explicit InterpretGenerator(QObject *parent = nullptr);

    void generate(const Automaton& automaton, const QString& output_filename);

signals:
};

#endif // INTERPRET_GENERATOR_H
