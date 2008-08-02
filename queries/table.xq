declare variable $inputDocument external;
declare function local:returnAnyVar ($var as element(), $i as element()) {
	for $ev in $var/value/ec
		return
			if (count($ev/@f)) then (
				data($ev/@f)
			) else if (count($ev/@sum)) then (
				for $sum in $i/../inspection[substring-before($i/@date, '.') = substring-before(@date, '.')]
					return ( string("+"), data($sum/var/var[@id=$ev/@sum]) )
			) else if (count($ev/@cc_attr)) then (
				for $att in $i/../@*
					return (
					if (name($att) = data($ev/@cc_attr)) then (
							data($att)
					) else ()
				)
				(:data(functx:dynamic-path($circuit, concat('@', data($ev/@cc_attr)))):)
			) else (
				if (empty($i/var[@id=$ev/@id])) then (
					string($i/var/var[@id=$ev/@id])
				) else (
					string($i/var[@id=$ev/@id])
				)
			)
};

declare function local:returnFootExpression ($var as element(), $circuit as element())  {
	<remove_expr>{
	for $i in $circuit/inspection
		return
		<expr date="{
				data($i/@date)
			}">{
			string("+("),
			local:returnAnyVar ($var, $i),
			string(")")
		}</expr>
	}</remove_expr>
};

declare function local:setTableHead ($table as element(), $vars as element())  {

<tr class="border_top">
<th rowspan="3">Dátum</th>
{
for $i in $table/var
	let $s := count($vars/var[@id=$i/@id]/var)
	return <th colspan="{$s}" rowspan="{
			if ($s > 1) then xs:integer(1)
			else 2+xs:integer(empty($vars/var[@id=$i/@id]/@unit))
		}" class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($vars/var[@id=$i/@id]/@name)}</th>
}
</tr>,
<tr>
{
for $i in $table/var
	return
		if (count($vars/var[@id=$i/@id]/var)) then (
			for $z in $vars/var[@id=$i/@id]/var
				return <th rowspan="{
							if (xs:integer(empty($z/@unit)) = 1) then (
								2
							) else (1)
							}" class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($z/@name)}</th>
		)
		else ()
}
</tr>,
<tr class="border_bottom">
{
for $i in $table/var
	return
		if (count($vars/var[@id=$i/@id]/var)) then (
			for $z in $vars/var[@id=$i/@id]/var
				return
					if (empty($z/@unit)) then ()
					else (
						<th class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($z/@unit)}</th>
					)
		)
		else (
			if (empty($vars/var[@id=$i/@id]/@unit)) then ()
			else <th class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($vars/var[@id=$i/@id]/@unit)}</th>
		)
}
</tr>
};

declare function local:setTableBody ($table as element(), $circuit as element(), $vars as element())  {

for $x in $circuit/inspection
	return <tr class="{
			if (data($table/@highlight_nominal)="false") then ()
			else if ($x/@nominal="true") then (xs:string("nominal"))
			else ()
		}">{
		<td id="date"><a href="customer:{ data($circuit/../@id) }/circuit:{ data($circuit/@id) }/inspection:{ data($x/@date) }">{data($x/@date)}</a></td>,
		for $y in $table/var
			return
				if (count($vars/var[@id=$y/@id]/var)) then (
					for $z in $vars/var[@id=$y/@id]/var
						return (
						<td id="{
							data($z/@id)
						}" remove="{
								if (empty($vars/var[@id=$y/@id]/var[@id=$z/@id]/value/ec/@sum)) then ()
								else ( substring-before($x/@date, '.') )
						}" class="{ data($vars/var[@id=$y/@id]/@col_bg) }" rowspan="{
							if (empty($vars/var[@id=$y/@id]/var[@id=$z/@id]/value/ec/@sum)) then (
								1
							) else (
								if (substring-before($x/@date, '.') = substring-before($x/../inspection[position()=$x/position()-1]/@date, '.')) then (
									1
								) else (
									count($x/../inspection[substring-before($x/@date, '.') = substring-before(@date, '.')])
								)
							)
						}">{
							if (empty($vars/var[@id=$y/@id]/var[@id=$z/@id]/value)) then (
								if (data($table/@highlight_nominal)="false") then ()
								else if (empty($x/@nominal)) then (
									if (data($vars/var[@id=$y/@id]/var[@id=$z/@id]/@compare_nom)="true") then (
										if (xs:double($x/var[@id=$y/@id]/var[@id=$z/@id]) > xs:double($x/../inspection[@nominal="true"]/var[@id=$y/@id]/var[@id=$z/@id])) then (
											<span style="font-size: large">↑</span>
										) else if (xs:double($x/var[@id=$y/@id]/var[@id=$z/@id]) < xs:double($x/../inspection[@nominal="true"]/var[@id=$y/@id]/var[@id=$z/@id])) then (
											<span style="font-size: large">↓</span>
										) else ()
									) else ()
								) else(),
								data($x/var[@id=$y/@id]/var[@id=$z/@id])
							) else (
								<expression>{
								if (data($table/@highlight_nominal)="false") then ()
								else (
									if (data($vars/var[@id=$y/@id]/var[@id=$z/@id]/@compare_nom)="true") then (
										for $ev in $vars/var[@id=$y/@id]/var[@id=$z/@id]/value/ec
											return (
												if (count($ev/@id)) then (
													if (empty($x/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
														if (empty($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])) then (0)
														else string($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
													) else (
														string($x/../inspection[@nominal="true"]/var[@id=$ev/@id])
													)
												) else (
													data($ev/@f)
												)
											),
										string("?")
									) else ()
								),
								for $ev in $vars/var[@id=$y/@id]/var[@id=$z/@id]/value/ec
									return
										if (count($ev/@f)) then (
											data($ev/@f)
										) else if (count($ev/@sum)) then (
											for $sum in $x/../inspection[substring-before($x/@date, '.') = substring-before(@date, '.')]
											return ( if (empty($sum/var/var[@id=$ev/@sum])) then ()
													else string("+"), data($sum/var/var[@id=$ev/@sum]) )
										) else if (count($ev/@cc_attr)) then (
											for $att in $x/../@*
											return (
												if (name($att) = data($ev/@cc_attr)) then (
													data($att)
												) else ()
											)
											(:data(functx:dynamic-path($circuit, concat('@', data($ev/@cc_attr)))):)
										) else (
											if (empty($x/var[@id=$ev/@id])) then (
												if (empty($x/var/var[@id=$ev/@id])) then (0)
												else string($x/var/var[@id=$ev/@id])
											) else (
												string($x/var[@id=$ev/@id])
											)
										)
								}</expression>
							)
						}</td>
						)
				)
				else (<td id="{
							data($y/@id)
						}" class="{ data($vars/var[@id=$y/@id]/@col_bg) }">{
							if (empty($vars/var[@id=$y/@id]/value)) then (
								if (data($table/@highlight_nominal)="false") then ()
								else if (empty($x/@nominal)) then (
									if (data($vars/var[@id=$y/@id]/@compare_nom="true")) then (
										if (xs:double($x/var[@id=$y/@id]) > xs:double($x/../inspection[@nominal="true"]/var[@id=$y/@id])) then (
											<span style="font-size: large">↑</span>
										) else if (xs:double($x/var[@id=$y/@id]) < xs:double($x/../inspection[@nominal="true"]/var[@id=$y/@id])) then (
											<span style="font-size: large">↓</span>
										) else ()
									) else ()
								) else(),
								data($x/var[@id=$y/@id])
							) else (
								<expression>{
								if (data($table/@highlight_nominal)="false") then ()
								else if (empty($x/@nominal)) then (
									if (data($vars/var[@id=$y/@id]/@compare_nom="true")) then (
										for $ev in $vars/var[@id=$y/@id]/value/ec
											return (
												if (empty($ev/@f)) then (
													if (empty($x/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
														if (empty($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])) then (0)
														else string($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
													)
													else (
														string($x/../inspection[@nominal="true"]/var[@id=$ev/@id])
													)
												) else (
													data($ev/@f)
												)
											),
										string("?")
									) else ()
								) else(),
								for $ev in $vars/var[@id=$y/@id]/value/ec
									return
										if (empty($ev/@f)) then (
											if (empty($x/var[@id=$ev/@id])) then (
												if (empty($x/var/var[@id=$ev/@id])) then (0)
												else string($x/var/var[@id=$ev/@id])
											) else (
												string($x/var[@id=$ev/@id])
											)
										) else (
											data($ev/@f)
										)
								}</expression>
							)
						}</td>)
		}</tr>
};

declare function local:setTableFoot ($table as element(), $circuit as element(), $vars as element()) {
	if ($table/foot) then (
		<tr class="border_top border_bottom">{
			<td>{data($table/foot/@name)}</td>,
			for $i in $table/var
				let $var := $vars/var[@id=$i/@id]
				return
					if ($table/foot/var[@id=$i/@id]) then (
						let $f := $table/foot/var[@id=$i/@id]
						return
							if (count($var/var)) then (
								for $v in $vars/var[@id=$i/@id]/var
									return
										<td class="{ data($var/@col_bg) }">{
										if (data($f/@function)="sum") then (
											if (count($v/value/ec/@sum)) then (
												local:returnFootExpression($v, $circuit)
											) else (
												let $s := for $z in $circuit/inspection return $z/var[@id=$f/@id]/var[@id=$v/@id]
														return sum($s)
											)
										) else ()
										}</td>
							) else (
								if (data($f/@function)="sum") then (
									let $s := for $z in $circuit/inspection return $z/var[@id=$f/@id]
									return <td class="{ data($var/@col_bg) }">{sum($s)}</td>
								) else <td></td>
							)
					) else (
						if (count($var/var)) then (
							for $v in $var/var
								return <td class="{ data($v/../@col_bg) }"></td>
						) else (
							<td class="{ data($var/@col_bg) }"></td>
						)
					)
		}
		</tr>
	)
	else ()
};

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Table</title>
<link href="default.css" rel="stylesheet" type="text/css" />
<link href="colours.css" rel="stylesheet" type="text/css" />
<script type="text/javascript">
<!--
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
				value = "<span style=\"font-size: large\">↑</span>" + value.toLocaleString();
			} else if (nominal != value) {
				value = "<span style=\"font-size: large\">↓</span>" + value.toLocaleString();
			} else {
				value = value.toLocaleString();
			}
		}
		expressions[i].innerHTML = value;
	}
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
						expression +=  "+" + (new Number(expr != '' ? eval(expr) : '0').toFixed(2).toLocaleString());
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
				expression =  (new Number(expression != '' ? eval(expression) : '0').toFixed(2).toLocaleString());
			}
			catch (e) {
				// Syntax error
			}
		}
		remove_exprs[r].innerText = expression;
	}
}

function showWarnings() {
	var tbody = document.getElementById("main_table_body");
	var trs = tbody.getElementsByTagName("tr");
	var warnings = document.getElementsByTagName("warnings");
	var poruchy_elem = document.getElementById("poruchy_element");
	for (var i = 0; i < trs.length; i++) {
		for (var w = 0; w < warnings.length; w++) {
			var warning = warnings[w].getElementsByTagName("warning");
			var poruchy = new Array();
			for (var a = 0; a < warning.length; a++) {
				var warn = true; var found = false;
				var w_vars = warning[a].getElementsByTagName("var");
				for (var v = 0; v < w_vars.length; v++) {
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
				}
				if (found == false) { warn = false; }
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
		for (var i = 0; i < move_trs.length;) {
			foot.appendChild(move_trs[i]);
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

-->
</script>
</head>
<body onLoad="removeRepeated(); fillInEmptyElements(); evaluateFootExpressions(); evaluateExpressions(); showWarnings();">
{
let $d := doc($inputDocument)
let $table := $d/leaklog/tables/table[@id="%1"]
let $circuit := $d/leaklog/customers/customer[@id="%2"]/circuit[@id="%3"]

return (
<table>
<thead>
{
	local:setTableHead ($table, $d/leaklog/variables)
}
</thead>
<tbody id="main_table_body">
{
	local:setTableBody ($table, $circuit, $d/leaklog/variables)
}
</tbody>
<tfoot id="table_foot">
{
	local:setTableFoot ($table, $circuit, $d/leaklog/variables)
}
</tfoot>
<tfoot id="poruchy_element">
<tr><td style="text-align: center;"><b>Poruchy</b></td></tr>
{
	$d/leaklog/warnings
}
</tfoot>
</table>
)
}
</body>
</html>
