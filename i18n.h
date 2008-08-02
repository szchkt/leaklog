#include <QObject>
#include <QMap>

class i18n : public QObject
{
    Q_OBJECT

public:
    QMap<QString, QString> dictionary;
    i18n() {
        dictionary.insert("ID: ", tr("ID: "));
        dictionary.insert("Company: ", tr("Company: "));
        dictionary.insert("Address: ", tr("Address: "));
        dictionary.insert("Phone: ", tr("Phone: "));
        dictionary.insert("Number of circuits: ", tr("Number of circuits: "));
        dictionary.insert("Total number of inspections: ", tr("Total number of inspections: "));
        dictionary.insert("E-mail: ", tr("E-mail: "));
        dictionary.insert("Circuit: ", tr("Circuit: "));
        dictionary.insert("Manufacturer: ", tr("Manufacturer: "));
        dictionary.insert("Type: ", tr("Type: "));
        dictionary.insert("Serial number: ", tr("Serial number: "));
        dictionary.insert("Year of purchase: ", tr("Year of purchase: "));
        dictionary.insert("Refrigerant: ", tr("Refrigerant: "));
        dictionary.insert("Oil: ", tr("Oil: "));
        dictionary.insert("Contact person: ", tr("Contact person: "));
        dictionary.insert("Date of commissioning: ", tr("Date of commissioning: "));
        dictionary.insert("Field of application: ", tr("Field of application: "));
        dictionary.insert("Amount of refrigerant: ", tr("Amount of refrigerant: "));
        dictionary.insert("Amount of oil: ", tr("Amount of oil: "));
        dictionary.insert("Service life: ", tr("Service life: "));
        dictionary.insert(" years", tr(" years"));
        dictionary.insert("Run-time per day: ", tr("Run-time per day: "));
        dictionary.insert(" hours", tr(" hours"));
        dictionary.insert("Rate of utilisation: ", tr("Rate of utilisation: "));
        dictionary.insert("Nominal: ", tr("Nominal: "));
        dictionary.insert("Nominal inspection: ", tr("Nominal inspection: "));
        dictionary.insert("Inspection: ", tr("Inspection: "));
        dictionary.insert(": ", tr(": "));
        dictionary.insert("Nominal inspection: ", tr("Nominal inspection: "));
        dictionary.insert("Nominal: ", tr("Nominal: "));
        dictionary.insert("Nominal: ", tr("Nominal: "));
        dictionary.insert("Warnings", tr("Warnings"));
    };
};