/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************/

#include "journalentry.h"

QString JournalEntry::tableName()
{
    return "journal";
}

class JournalEntryColumns
{
public:
    JournalEntryColumns() {
        columns << Column("id", "SERIAL PRIMARY KEY");
        columns << Column("source_uuid", "UUID NOT NULL");
        columns << Column("version", "SMALLINT NOT NULL DEFAULT 0");
        columns << Column("entry_id", "INTEGER NOT NULL");
        columns << Column("operation_id", "SMALLINT NOT NULL DEFAULT 1");
        columns << Column("table_id", "SMALLINT NOT NULL");
        columns << Column("record_uuid", "UUID NOT NULL");
        columns << Column("column_id", "SMALLINT NOT NULL DEFAULT 0");
        columns << Column("date_created", "TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP");
    }

    ColumnList columns;
};

const ColumnList &JournalEntry::columns()
{
    static JournalEntryColumns columns;
    return columns.columns;
}

static class TableIDs
{
public:
    TableIDs() {
        table_names.insert(10, "service_companies");
        table_names.insert(11, "refrigerant_management");
        table_names.insert(12, "inspectors");
        table_names.insert(20, "customers");
        table_names.insert(21, "persons");
        table_names.insert(30, "repairs");
        table_names.insert(40, "circuits");
        table_names.insert(41, "compressors");
        table_names.insert(42, "circuit_unit_types");
        table_names.insert(43, "circuit_units");
        table_names.insert(50, "inspections");
        table_names.insert(51, "inspections_compressors");
        table_names.insert(52, "inspections_files");
        table_names.insert(60, "files");
        table_names.insert(70, "tables");
        table_names.insert(80, "assembly_record_types");
        table_names.insert(81, "assembly_record_type_categories");
        table_names.insert(82, "assembly_record_item_categories");
        table_names.insert(83, "assembly_record_item_types");
        table_names.insert(84, "assembly_record_items");
        table_names.insert(90, "styles");
        table_names.insert(100, "warnings");
        table_names.insert(101, "warnings_conditions");
        table_names.insert(102, "warnings_filters");
        table_names.insert(110, "variables");

        QMapIterator<int, QString> i(table_names);
        while (i.hasNext()) { i.next();
            table_ids.insert(i.value(), i.key());
        }
    }

    QMap<int, QString> table_names;
    QMap<QString, int> table_ids;
} table_ids;

int JournalEntry::tableIDForName(const QString &name)
{
    return table_ids.table_ids.value(name);
}

QString JournalEntry::tableNameForID(int id, const QString &default_value)
{
    return table_ids.table_names.value(id, default_value);
}

static class ColumnIDs
{
public:
    ColumnIDs() {
        column_names.insert(1, "acquisition_price");
        column_names.insert(2, "address");
        column_names.insert(3, "arno");
        column_names.insert(4, "ar_item_category_uuid");
        column_names.insert(5, "ar_item_type_uuid");
        column_names.insert(6, "ar_type_uuid");
        column_names.insert(7, "auto_show");
        column_names.insert(8, "avg");
        column_names.insert(9, "bbl_detect");
        column_names.insert(10, "bbl_lvl");
        column_names.insert(11, "building");
        column_names.insert(12, "certificate_country");
        column_names.insert(13, "certificate_number");
        column_names.insert(14, "circuit_attribute");
        column_names.insert(15, "circuit_uuid");
        column_names.insert(16, "commissioning");
        column_names.insert(17, "company");
        column_names.insert(18, "compressor_uuid");
        column_names.insert(19, "comp_runtime");
        column_names.insert(20, "content");
        column_names.insert(21, "corr_def");
        column_names.insert(22, "customer");
        column_names.insert(23, "customer_uuid");
        column_names.insert(24, "data");
        column_names.insert(25, "date");
        column_names.insert(26, "date_updated");
        column_names.insert(27, "decommissioning");
        column_names.insert(28, "delay");
        column_names.insert(29, "delta_t_c");
        column_names.insert(30, "delta_t_evap");
        column_names.insert(31, "description");
        column_names.insert(32, "device");
        column_names.insert(33, "discount");
        column_names.insert(34, "display_options");
        column_names.insert(35, "display_position");
        column_names.insert(36, "disused");
        column_names.insert(37, "div_tables");
        column_names.insert(38, "ean");
        column_names.insert(39, "ec_l1");
        column_names.insert(40, "ec_l2");
        column_names.insert(41, "ec_l3");
        column_names.insert(42, "el_detect");
        column_names.insert(43, "enabled");
        column_names.insert(44, "ep_comp");
        column_names.insert(45, "ev_l1");
        column_names.insert(46, "ev_l2");
        column_names.insert(47, "ev_l3");
        column_names.insert(48, "field");
        column_names.insert(49, "file_uuid");
        column_names.insert(50, "function");
        column_names.insert(51, "hermetic");
        column_names.insert(52, "hidden");
        column_names.insert(53, "highlight_nominal");
        column_names.insert(54, "id");
        column_names.insert(55, "image_file_uuid");
        column_names.insert(56, "inspection_interval");
        column_names.insert(57, "inspection_type");
        column_names.insert(58, "inspection_type_data");
        column_names.insert(59, "inspection_uuid");
        column_names.insert(60, "inspection_variable_id");
        column_names.insert(61, "inspector_uuid");
        column_names.insert(62, "leaked");
        column_names.insert(63, "leaked_reco");
        column_names.insert(64, "leak_detector");
        column_names.insert(65, "list_price");
        column_names.insert(66, "location");
        column_names.insert(67, "mail");
        column_names.insert(68, "manufacturer");
        column_names.insert(69, "name");
        column_names.insert(70, "name_format");
        column_names.insert(71, "noise_vibr");
        column_names.insert(72, "noise_vibr_comp");
        // column_names.insert(73, "nominal");
        column_names.insert(74, "notes");
        column_names.insert(75, "oil");
        column_names.insert(76, "oil_amount");
        column_names.insert(77, "oil_leak");
        column_names.insert(78, "oil_leak_am");
        column_names.insert(79, "oil_shortage");
        column_names.insert(80, "operation");
        column_names.insert(81, "operator_address");
        column_names.insert(82, "operator_company");
        column_names.insert(83, "operator_id");
        column_names.insert(84, "operator_mail");
        column_names.insert(85, "operator_phone");
        column_names.insert(86, "operator_type");
        column_names.insert(87, "output");
        column_names.insert(88, "output_t0_tc");
        column_names.insert(89, "output_unit");
        column_names.insert(90, "outside_interval");
        column_names.insert(91, "partner");
        column_names.insert(92, "partner_id");
        column_names.insert(93, "person");
        column_names.insert(94, "person_uuid");
        column_names.insert(95, "phone");
        column_names.insert(96, "position");
        column_names.insert(97, "ppsw_diff");
        column_names.insert(98, "ppsw_hip");
        column_names.insert(99, "ppsw_lop");
        // column_names.insert(100, "predefined");
        column_names.insert(101, "purchased");
        column_names.insert(102, "purchased_reco");
        column_names.insert(103, "p_0");
        column_names.insert(104, "p_c");
        column_names.insert(105, "refrigerant");
        column_names.insert(106, "refrigerant_amount");
        column_names.insert(107, "refr_add_am");
        column_names.insert(108, "refr_add_per");
        column_names.insert(109, "refr_disp");
        column_names.insert(110, "refr_reco");
        column_names.insert(111, "refr_rege");
        // column_names.insert(112, "repair");
        column_names.insert(113, "risks");
        column_names.insert(114, "rmds");
        column_names.insert(115, "runtime");
        column_names.insert(116, "scope");
        column_names.insert(117, "sftsw");
        column_names.insert(118, "sn");
        column_names.insert(119, "sold");
        column_names.insert(120, "sold_reco");
        column_names.insert(121, "source");
        column_names.insert(122, "style_uuid");
        column_names.insert(123, "sum");
        column_names.insert(124, "type");
        column_names.insert(125, "t_0");
        column_names.insert(126, "t_c");
        column_names.insert(127, "t_comp_in");
        column_names.insert(128, "t_comp_out");
        column_names.insert(129, "t_ev");
        column_names.insert(130, "t_evap_out");
        column_names.insert(131, "t_sc");
        column_names.insert(132, "t_sec_cond_in");
        column_names.insert(133, "t_sec_evap_in");
        column_names.insert(134, "t_sh_comp");
        column_names.insert(135, "t_sh_evap");
        column_names.insert(136, "unit");
        column_names.insert(137, "unit_type_uuid");
        column_names.insert(138, "updated_by");
        column_names.insert(139, "utilisation");
        column_names.insert(140, "uuid");
        column_names.insert(141, "uv_detect");
        column_names.insert(142, "value");
        column_names.insert(143, "value_data_type");
        column_names.insert(144, "value_ins");
        column_names.insert(145, "value_nom");
        column_names.insert(146, "variables");
        column_names.insert(147, "warning_uuid");
        column_names.insert(148, "website");
        column_names.insert(149, "year");
        column_names.insert(150, "batch_number");
        column_names.insert(151, "parent_uuid");
        column_names.insert(152, "compare_nom");
        column_names.insert(153, "tolerance");
        column_names.insert(154, "col_bg");
        column_names.insert(155, "starred");
        column_names.insert(156, "operator_link");
        column_names.insert(157, "inspection_data");
        column_names.insert(158, "verified_identifier");
        column_names.insert(159, "date_verified");
        column_names.insert(160, "external_uuid");
        column_names.insert(161, "decommissioning_reason");
        // Version 1
        column_names.insert(162, "service_company_uuid");
        // Version 2
        column_names.insert(163, "repair_type");
        column_names.insert(164, "website_url");
        column_names.insert(165, "maps_url");
        // Version 3
        column_names.insert(166, "purchased_rege");
        column_names.insert(167, "sold_rege");
        column_names.insert(168, "refr_add_am_recy");
        column_names.insert(169, "refr_add_am_rege");

        QMapIterator<int, QString> i(column_names);
        while (i.hasNext()) { i.next();
            column_ids.insert(i.value(), i.key());
        }
    }

    QMap<int, QString> column_names;
    QMap<QString, int> column_ids;
} column_ids;

int JournalEntry::columnIDForName(const QString &name)
{
    return column_ids.column_ids.value(name);
}

QString JournalEntry::columnNameForID(int id, const QString &default_value)
{
    return column_ids.column_names.value(id, default_value);
}

int JournalEntry::versionForColumnID(int column_id)
{
    if (column_id >= 166)
        return 3;
    if (column_id >= 163)
        return 2;
    if (column_id >= 162)
        return 1;
    return 0;
}

bool JournalEntry::shouldJournalUpdateOnInsertionForColumnID(int column_id, const QVariant &value)
{
    switch (column_id) {
        case 163:
            return value.toInt() != 0;
        case 164:
        case 165:
            return !value.toString().isEmpty();
        case 166:
        case 167:
        case 168:
        case 169:
            return value.toDouble() != 0.0;
    }
    return column_id >= 162;
}
