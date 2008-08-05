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
//-----------------------------Circuit--------------------------------------
function translateTextValues() {
	var elements = document.getElementsByTagName("textvalue");
	for (var i = 0; i < elements.length; i++) {
		elements[i].innerHTML = dictionaries.value(elements[i].getAttribute("type")).value(elements[i].innerText);
	}
}
//------------------------------Table---------------------------------------
function onTableLoad() {
	removeRepeated();
	fillInEmptyElements();
	evaluateFootExpressions();
	evaluateExpressions();
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
	var trs = tbody.getElementsByTagName("tr");
	var warnings = document.getElementsByTagName("warnings");
	var poruchy_elem = document.getElementById("poruchy_element");
	for (var i = 0; i < trs.length; i++) {
		for (var w = 0; w < warnings.length; w++) {
			var warning = warnings[w].getElementsByTagName("warning");
			var poruchy = new Array();
			for (var a = 0; a < warning.length; a++) {
				var warn = true;
				var w_vars = warning[a].getElementsByTagName("var");
				for (var v = 0; v < w_vars.length; v++) {
					var found = false;
					var tds = trs[i].getElementsByTagName("td");
					for (var t = 0; t < tds.length; t++) {
						if (tds[t].id == w_vars[v].id) {
							found = true;
							var td = tds[t].innerText;
							if (td != "") {
								if (td[0] == "↑") {
									if (w_vars[v].innerText != "increase") {
									}
								} else if (td[0] == "↓") {
									if (w_vars[v].innerText != "decrease") {
										warn = false;
									}
								} else { warn = false; }
							} else { warn = false; }
						}
					}
					if (found == false) { warn = false; break; }
				}
				if (warn == true) {
					poruchy.push(warning[a].getAttribute("name"));
				}
			}
			if (poruchy.length > 0) {
				var poruchy_html = poruchy_elem.innerHTML;
				poruchy_html += "<tr><td>" + trs[i].getElementsByTagName("td")[0].innerText + "</td>";
				poruchy_html += "<td colspan=\"" + (trs[i].getElementsByTagName("td").length) + "\">" + poruchy.join(", ") + "</td></tr>";
				poruchy_elem.innerHTML = poruchy_html;
			}
		}
	}
	for (var w = 0; w < warnings.length; w++) {
		warnings[w].parentNode.removeChild(warnings[w]);
	}
	if (poruchy_elem.getElementsByTagName("tr").length < 2) {
		poruchy_elem.parentNode.removeChild(poruchy_elem);
	} else {
		var foot = document.getElementById("table_foot");
		var move_trs = poruchy_elem.getElementsByTagName("tr");
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
	showInspectionWarnings();
}

function splitTable() {
	var table_l = document.getElementById("table_l");
	var table_r = document.getElementById("table_r");
	var rows = table_l.getElementsByTagName("tr");
	var rows_length = rows.length;
	var begin = parseInt(rows_length / 2);
	if (rows_length % 2 == 1) { begin++; }
	for (var i = begin; i < rows_length; i++) {
		table_r.appendChild(rows[begin]);
	}
}

function showInspectionWarnings() {
	var tbody = document.getElementById("main_table_body");
	var tds = tbody.getElementsByTagName("td");
	var warnings = document.getElementsByTagName("warnings");
	var poruchy_elem = document.getElementById("poruchy_element");
	var poruchy = new Array();
	for (var w = 0; w < warnings.length; w++) {
		var warning = warnings[w].getElementsByTagName("warning");
		for (var a = 0; a < warning.length; a++) {
			var warn = true;
			var w_vars = warning[a].getElementsByTagName("var");
			for (var v = 0; v < w_vars.length; v++) {
				var found = false;
				for (var t = 0; t < tds.length; t++) {
					if (tds[t].hasAttribute("id")) {
						if (tds[t].id == w_vars[v].id) {
							found = true;
							var td = tds[t].innerText;
							if (td != "") {
								if (td[0] == "↑") {
									if (w_vars[v].innerText != "increase") {
									}
								} else if (td[0] == "↓") {
									if (w_vars[v].innerText != "decrease") {
										warn = false;
									}
								} else { warn = false; }
							} else { warn = false; }
						}
					}
				}
				if (found == false) { warn = false; break; }
			}
			if (warn == true) {
				poruchy.push(warning[a].getAttribute("name"));
			}
		}
	}
	if (poruchy.length > 0) {
		var poruchy_html = poruchy_elem.innerHTML;
		poruchy_html += "<tr><td colspan=\"2\">" + poruchy.join(", ") + "</td></tr>";
		poruchy_elem.innerHTML = poruchy_html;
	}
	for (var w = 0; w < warnings.length; w++) {
		warnings[w].parentNode.removeChild(warnings[w]);
	}
	if (poruchy.length == 0) {
		poruchy_elem.parentNode.removeChild(poruchy_elem);
	}
}

//------------------------------Shared------------------------------
function evaluateExpressions() {
	var expressions = document.getElementsByTagName("expression");
	var value; var expression; var nominal;
	var array = new Array();
	for (var i = 0; i < expressions.length; i++) {
		value = "";
		expression = expressions[i].innerHTML;
		array = expression.split('?');
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
		if (value != "") {
			if (Math.min(nominal, value) == nominal && nominal != value) {
				value = "<span style=\"font-size: large\">↑ </span>" + value.toLocaleString();
			} else if (nominal != value) {
				value = "<span style=\"font-size: large\">↓ </span>" + value.toLocaleString();
			} else {
				value = value.toLocaleString();
			}
		}
		if (value.substr(value.length-3, 3) == '.00') {
			value = value.substring(0, value.length-3);
		}
		value = value + " ";
		expressions[i].innerHTML = value;
	}
}
