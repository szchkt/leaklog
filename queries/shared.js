String.prototype.trim = function() { return this.replace(/^\s+|\s+$/g, ''); }

function Dictionary(startValues) {
    this.values = startValues || {};
}
Dictionary.prototype.add = function(name, value) {
    this.values[name] = value;
};
Dictionary.prototype.value = function(name) {
    return this.values[name];
};
Dictionary.prototype.contains = function(name) {
    return Object.prototype.hasOwnProperty.call(this.values, name) &&
        Object.prototype.propertyIsEnumerable.call(this.values, name);
};
Dictionary.prototype.each = function(action) {
    forEachIn(this.values, action);
};

//---------------------------Gloabal variables------------------------------
var nom_ins_date;

//---------------------------Dictionaries-----------------------------------
var dict_field = new Dictionary();
dict_field.add("car", "<i18n>Car air conditioning</i18n>");
dict_field.add("lowrise", "<i18n>Low-rise residential buildings</i18n>");
dict_field.add("highrise", "<i18n>High-rise residential buildings</i18n>");
dict_field.add("commercial", "<i18n>Commercial buildings</i18n>");
dict_field.add("institutional", "<i18n>Institutional buildings</i18n>");
dict_field.add("industrial", "<i18n>Industrial spaces</i18n>");
dict_field.add("transportation", "<i18n>Transportation</i18n>");
dict_field.add("airconditioning", "<i18n>Air conditioning</i18n>");
dict_field.add("heatpumps", "<i18n>Heat pumps</i18n>");

var dictionaries = new Dictionary();
dictionaries.add("field", dict_field);

var values_hash = new Hash();

//------------------------------Table---------------------------------------
function onTableLoad() {
	removeRepeated();
	fillInEmptyElements();
	evaluateFootExpressions();
	evaluateExpressions();
	loadValuesDict();
	showTableWarnings();
}

function removeRepeated() {
	var tds = document.getElementsByTagName("td");
	var array = new Array();
	for (var i = 0; i < tds.length; i++) {
		if (tds[i].hasAttribute("remove") && array.indexOf(tds[i].getAttribute("remove")) >= 0) {
			tds[i].parentNode.removeChild(tds[i]);
		} else {
			if (tds[i].hasAttribute("remove") && tds[i].getAttribute("remove") != "") {
				array.push(tds[i].getAttribute("remove"));
			}
		}
	}
}

function evaluateFootExpressions() {
	var remove_exprs = document.getElementsByTagName("remove_expr");
	for (var r = 0; r < remove_exprs.length; r++) {
		var exprs = remove_exprs[r].getElementsByTagName("expr");
		var array = new Array();
		var expression = new Number;
		for (var i = 0; i < exprs.length; i++) {
			if (array.indexOf(exprs[i].getAttribute("date").split('.')[0]) < 0) {
				array.push(exprs[i].getAttribute("date").split('.')[0]);
				var expr = exprs[i].innerText;
				if (expr.match(/^[0-9+\-*/(). ]*$/)) {
					try {
						expression += "+" + (new Number(expr != '' ? eval(expr) : '0').toFixed(2).toLocaleString());
					}
					catch (e) {
						// Syntax error
					}
				}
				exprs[i].parentNode.removeChild(exprs[i]);
			}
		}
		if (expression.match(/^[0-9+\-*/(). ]*$/)) {
			try {
				expression = (new Number(expression != '' ? eval(expression) : '0').toFixed(2).toLocaleString());
			}
			catch (e) {
				// Syntax error
			}
		}
		remove_exprs[r].innerText = expression;
	}
}

function showTableWarnings() {
	var tbody = document.getElementById("main_table_body");
	nom_ins_date = tbody.getElementsByTagName("tr")[0].getElementsByTagName("td")[0].innerText;
	var warnings = document.getElementsByTagName("warnings");
	var warnings_element = document.getElementById("warnings_element");
	var warning;
	var warnings_array = new Array();
	var last_warnings_array = new Array();
	var repeated_wars_hash = new Hash();
	values_hash.each(function(pair) {
		if (pair.key == nom_ins_date) return;
		for (var w = 0; w < warnings.length; w++) {
			var warning = warnings[w].getElementsByTagName("warning");
			warnings_array = sharedWarnings(warning, pair.key, pair.value);
			if (warnings_array.length > 0) {
				for (var r = 0; r < last_warnings_array.length; r++) {
					for (var m = 0; m < warnings_array.length; m++) {
						if (last_warnings_array[r] == warnings_array[m] || last_warnings_array[r] == ("<span style=\"color: red;\"><b>" + warnings_array[m] + "</b></span>")) {
							warnings_array[m] = "<span style=\"color: red;\"><b>" + warnings_array[m] + "</b></span>";
						}
					}
					/*
					if (isNaN(repeated_wars_hash.get(warnings_array[r]))) {
						repeated_wars_hash.set(warnings_array[r], 1);
					} else {
						repeated_wars_hash.set(warnings_array[r], repeated_wars_hash.get(warnings_array[r])+1);
						warnings_array[r] = "<b>" + repeated_wars_hash.get(warnings_array[r]) + ". " + warnings_array[r] + "</b>";
					}*/
				}
				var warnings_html = warnings_element.innerHTML;
				warnings_html += "<tr><td>" + pair.key + "</td>";
				warnings_html += "<td colspan=\"" + (tbody.getElementsByTagName("tr")[0].getElementsByTagName("td").length) + "\">" + warnings_array.join(", ") + "</td></tr>";
				warnings_element.innerHTML = warnings_html;
			}
			last_warnings_array = warnings_array;
		}
	});
	for (var w = 0; w < warnings.length; w++) {
		warnings[w].parentNode.removeChild(warnings[w]);
	}
	if (warnings_element.getElementsByTagName("tr").length < 2) {
		warnings_element.parentNode.removeChild(warnings_element);
	} else {
		var foot = document.getElementById("table_foot");
		var move_trs = warnings_element.getElementsByTagName("tr");
		var move_length = move_trs.length;
		for (var i = 0; i < move_length; i++) {
			foot.appendChild(move_trs[0]);
		}
	}
}

function fillInEmptyElements() {
	var tds = document.getElementById('main_table_body').getElementsByTagName('td');
	for (var i = 0; i < tds.length; i++) {
		if (tds[i].innerText == "") {
			tds[i].innerHTML = '0';
		}
	}
}

//------------------------------Inspection---------------------------------------
function onInspectionLoad() {
	splitTable();
	evaluateExpressions();
	loadValuesDict();
	removeNominal();
	showInspectionWarnings();
}

function splitTable() {
	var table_l = document.getElementById("table_l");
	var table_r = document.getElementById("table_r");
	var all_rows = table_l.getElementsByTagName("tr");
	var rows = new Array();
	for (var i = 0; i < all_rows.length; i++) {
		if (all_rows[i].hasAttribute("move")) { rows.push(all_rows[i]); }
	}
	var rows_length = rows.length;
	var begin = parseInt(rows_length / 2);
	if (rows_length % 2 == 1) { begin++; }
	for (var i = begin; i < rows_length; i++) {
		table_r.appendChild(rows[i]);
	}
}

function showInspectionWarnings() {
	var tbody = document.getElementById("main_table_body");
	var warnings = document.getElementsByTagName("warnings");
	var warnings_element = document.getElementById("warnings_element");
	var warning;
	var warnings_array = new Array();
	values_hash.each(function(pair) {
		if (pair.key == nom_ins_date) return;
		for (var w = 0; w < warnings.length; w++) {
			var warning = warnings[w].getElementsByTagName("warning");
			warnings_array = sharedWarnings(warning, pair.key, pair.value);
		}
	});
	if (warnings_array.length > 0) {
		var warnings_html = warnings_element.innerHTML;
		warnings_html += "<tr><td colspan=\"2\">" + warnings_array.join(", ") + "</td></tr>";
		warnings_element.innerHTML = warnings_html;
	}
	for (var w = 0; w < warnings.length; w++) {
		warnings[w].parentNode.removeChild(warnings[w]);
	}
	if (warnings_array.length == 0) {
		warnings_element.parentNode.removeChild(warnings_element);
	}
}

function removeNominal() {
	var nom_ins = document.getElementById("nominal_inspection");
	nom_ins_date = nom_ins.getAttribute("date");
	nom_ins.parentNode.removeChild(nom_ins);
}

//------------------------------Shared------------------------------
function evaluateExpressions() {
	var expressions = document.getElementsByTagName("expression");
	var value; var expression; var nominal;
	var array = new Array();
	for (var i = 0; i < expressions.length; i++) {
		evaluateExpression(expressions[i]);
	}
}

function evaluateExpression(expression_elem) {
	expression_elem.innerHTML = evaluateString(expression_elem.innerText);
}

function evaluateString(expression) {
	var value; var nominal;
	var array = expression.split('?');
	if (array.length == 1) {
		nominal = array[0];
		expression = array[0];
	} else {
		nominal = array[0];
		expression = array[1];
	}
	if (nominal.match(/^[0-9+\-*/(). ]*$/)) {
		try {
			nominal = new Number(nominal != '' ? eval(nominal) : '0').toFixed(2);
		}
		catch (e) {
			nominal = "";
		}
	}
	if (expression.match(/^[0-9+\-*/(). ]*$/)) {
		try {
			value = new Number(expression != '' ? eval(expression) : '0').toFixed(2);
		}
		catch (e) {
			// Syntax error
		}
	}
	if (value.length >= 3 && value.substr(value.length-3, 3) == '.00') {
		value = value.substring(0, value.length-3);
	}
	if (nominal.length >= 3 && nominal.substr(nominal.length-3, 3) == '.00') {
		nominal = nominal.substring(0, nominal.length-3);
	}
	if (value != "") {
		if (Math.min(nominal, value) == nominal && nominal != value) {
			value = "<table class=\"no_border\" cellpadding=\"0\" cellspacing=\"0\"><tr><td class=\"no_border\" width=\"1%\" align=\"right\" valign=\"center\" style=\"font-size: large\">↑</td><td class=\"no_border\" valign=\"center\">" + value.toLocaleString() + "</td></tr></table>";
		} else if (nominal != value) {
			value = "<table class=\"no_border\" cellpadding=\"0\" cellspacing=\"0\"><tr><td class=\"no_border\" width=\"1%\" align=\"right\" valign=\"center\" style=\"font-size: large\">↓</td><td class=\"no_border\" valign=\"center\">" + value.toLocaleString() + "</td></tr></table>";
		} else {
			value = value.toLocaleString();
		}
	}
	//value += " ";
	return value;
}

function translateTextValues() {
	var elements = document.getElementsByTagName("textvalue");
	var element;
	for (var i = 0; i < elements.length; i++) {
		element = elements[i];
		if (dictionaries.value(element.getAttribute("type")).contains(element.innerText)) {
			element.innerHTML = dictionaries.value(element.getAttribute("type")).value(element.innerText);
		}
	}
}

function loadValuesDict() {
	var values = document.getElementsByTagName("var_value");
	var value_text;
	for (var i = 0; i < values.length; i++) {
		if (String(values_hash.get(values[i].getAttribute("date"))) != "undefined") {
			value_text = "";
			if (values[i].innerText[0] == "↑") {
				value_text += values[i].innerText.split('↑').join('');
			} else if (values[i].innerText[0] == "↓") {
				value_text += values[i].innerText.split('↓').join('');
				//window.alert(new_text);
			} else {
				value_text += values[i].innerText;
			}
			value_text = parseFloat(value_text);
			values_hash.get(values[i].getAttribute("date")).set(values[i].id, value_text);
		} else {
			value_text = "";
			if (values[i].innerText == "↑") {
				value_text += values[i].innerText.split('↑').join('');
			} else if (values[i].innerText == "↓") {
				value_text += values[i].innerText.split('↓').join('');
				//window.alert(new_text);
			} else {
				value_text += values[i].innerText;
			}
			value_text = parseInt(value_text);
			var sub_hash = new Hash();
			sub_hash.set(values[i].id, value_text);
			values_hash.set(values[i].getAttribute("date"), sub_hash);
		}
	}
}

function sharedWarnings(warning, date, sub_hash) {
	var warnings_array = new Array();
	for (var a = 0; a < warning.length; a++) {
		var warn = true;
		var cc_attrs = warning[a].getElementsByTagName("rem_dots");
		for (var cc = 0; cc < cc_attrs.length; cc++) {
			var cc_text = cc_attrs[cc].innerText;
			var count = 0;
			var cc_split = cc_text.split('');
			for (var d = 0; d < cc_split.length; d++) {
				if (cc_split[d] == '.') { count++; }
			}
			if (count > 1) {
				cc_text = cc_text.split('.').join('');
			}
			cc_attrs[cc].innerText = cc_text;
		}
		var w_conds = warning[a].getElementsByTagName("condition");
		for (var c = 0; c < w_conds.length; c++) {
			var fe = w_conds[c].getElementsByTagName("first_expression");
			var fe_text = fe[0].innerText;
			var f = w_conds[c].getElementsByTagName("f")[0].innerText;
			var se = w_conds[c].getElementsByTagName("second_expression");
			var se_text = se[0].innerText;
			var replace_ids_1 = fe[0].getElementsByTagName("replace_id");
			if (w_conds[c].getElementsByTagName("cc_attr").length == 0) {
				var continue_eval = true;
				for (var r = 0; r < replace_ids_1.length; r++) {
					if (String(sub_hash.get(replace_ids_1[r].innerText)) != "undefined") {
						fe_text = fe_text.replace(replace_ids_1[r].innerText, sub_hash.get(replace_ids_1[r].innerText));
					} else { continue_eval = false; }
				}
				var replace_nom_ids_2 = se[0].getElementsByTagName("replace_nom_id");
				for (var r = 0; r < replace_nom_ids_2.length; r++) {
					if (String(values_hash.get(nom_ins_date).get(replace_nom_ids_2[r].innerText)) != "undefined") {
						se_text = se_text.replace(replace_nom_ids_2[r].innerText, values_hash.get(nom_ins_date).get(replace_nom_ids_2[r].innerText));
					} else { continue_eval = false; }
				}
				if (continue_eval == false) {
					warn = false; break;
				}
				fe_text = evaluateString(fe_text);
				se_text = evaluateString(se_text);
			}
			var fe_text = fe_text.trim();
			var se_text = se_text.trim();
			if (f == "less" && Math.min(fe_text, se_text) == fe_text && fe_text != se_text) {
				//window.alert(f + "  " + fe_text + "  " + se_text + "  " + "less");
			} else if (f == "lessorequal" && Math.min(fe_text, se_text) == fe_text) {
				//window.alert(f + "  " + fe_text + "  " + se_text + "  " + "lessorequal");
			} else if (f == "more" && Math.min(fe_text, se_text) == se_text && fe_text != se_text) {
				//window.alert(f + "  " + fe_text + "  " + se_text + "  " + "more");
			} else if (f == "moreorequal" && Math.min(fe_text, se_text) == se_text) {
				//window.alert(f + "  " + fe_text + "  " + se_text + "  " + "moreorequal");
			} else if (f == "equal" && fe_text == se_text) {
				//window.alert(f + "  " + fe_text + "  " + se_text + "  " + "equal");
			} else {
				//window.alert(f + "  " + fe_text + "  " + se_text + "  " + "unknown");
				warn = false;
			}
			//window.alert(f + "  " + fe_text + "  " + se_text + "   " + fe_text.length + se_text.length);
		}
		if (warn == true) {
			warnings_array.push(warning[a].id);
		}
	}
	return warnings_array;
}
