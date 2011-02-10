#ifndef VARIABLE_PARSER_H
#define VARIABLE_PARSER_H

#include <QVariantMap>
#include <QStringList>

namespace VariableEvaluation {

    class Variable;

    class EvaluationContext
    {
    public:
        EvaluationContext(const QString &, const QString &);
        ~EvaluationContext();

        void setNominalInspection(const QVariantMap & nominal_ins) { this->nominal_ins = nominal_ins; }
        QVariantMap & nominalInspection() { return nominal_ins; }

        Variable * variable(const QString & name) { return vars_map.value(name); }
        QString evaluate(Variable *, QVariantMap &, QString &);
        QString evaluate(const QString &, QVariantMap &, QString &);

        QStringList & usedIds() { return used_ids; }

    private:
        void init();

        QMap<QString, Variable *> vars_map;
        QStringList used_ids;
        QString customer_id;
        QString circuit_id;
        QVariantMap nominal_ins;
    };

    class Variable
    {

    public:
        void setName(const QString & name) { this->_name = name; }
        const QString & name() { return _name; }
        void setType(const QString & type) { this->_type = type; }
        const QString & type() { return _type; }
        void setUnit(const QString & unit) { this->_unit = unit; }
        const QString & unit() { return _unit; }
        void setCompareNom(int compare_nom) { this->_compare_nom = compare_nom; }
        int compareNom() { return _compare_nom; }
        void setColBg(const QString & col_bg) { this->_col_bg = col_bg; }
        const QString & colBg() { return _col_bg; }
        void setTolerance(double tolerance) { this->_tolerance = tolerance; }
        double tolerance() { return _tolerance; }
        void setValue(const QString & value) { this->_value = value; }
        const QString & value() { return _value; }
        void setId(const QString & id) { this->_id = id; }
        const QString & id() { return _id; }

        void addSubvariable(Variable * var) { subvars.append(var); }
        const QList<Variable *> & subvariables() { return subvars; }
        int countSubvariables() { return subvars.count(); }

        QString evaluate(const QString &, const QString &, QVariantMap &, QStringList &, QVariantMap &, QString &);

    private:
        QString _name;
        QString _type;
        QString _unit;
        int _compare_nom;
        QString _col_bg;
        double _tolerance;
        QString _value;
        QString _id;

        QList<Variable *> subvars;
    };
}

#endif // VARIABLE_PARSER_H
