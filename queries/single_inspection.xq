declare variable $inputDocument external;
declare function local:returnSubVar (  $i as element(), $v as element())  {

  let $iv := $i/var[@id=$v/@id]
  for $w in $v/var
		return (
		if (empty($iv/var[@id=$w/@id]) and not(count($w/value))) then ()
		else (
			let $iw := $iv/var[@id=$w/@id]
			return <tr><td style="text-align: right; width:50%;">{
						data($v/@name),
						<i18n>: </i18n>,
						if (empty(data($w/@name))) then (
							data($w/@id)
						) else ( data($w/@name) ),
						<i18n>: </i18n>
					}</td><td id="{ data($w/@id) }">{
							if (not(count($w/value))) then (
								if (data($w/@compare_nom="true")) then (
									if (xs:double($iw) > xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id]/var[@id=$w/@id])) then (
										string("↑")
									) else if (xs:double($iw) < xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id]/var[@id=$w/@id])) then (
										string("↓")
									) else ()
								) else (),
								data($iw)
							) else (
								<expression>{
								if (data($w/@compare_nom)="true") then (
									for $ev in $w/value/ec
									return (
										if (empty($ev/@f)) then (
											if (empty($i/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
												if (empty($i/../inspection[@nominal="true"]/var/var[@id=$ev/@id])) then (0)
												else string($i/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
											)
											else (
												string($i/../inspection[@nominal="true"]/var[@id=$ev/@id])
											)
										) else (
											data($ev/@f)
										)
									),
									string("?")
								) else (),
								for $ev in $w/value/ec
									return
										if (count($ev/@f)) then (
											data($ev/@f)
										) else if (count($ev/@sum)) then (
											for $sum in $i/../inspection[substring-before($i/@date, '.') = substring-before(@date, '.')]
											return ( if (empty($sum/var/var[@id=$ev/@sum])) then ()
													else string("+"), data($sum/var/var[@id=$ev/@sum]) )
										) else if (count($ev/@cc_attr)) then (
											for $att in $i/../@*
											return (
												if (name($att) = data($ev/@cc_attr)) then (
													data($att)
												) else ()
											)
										) else (
											if (empty($i/var[@id=$ev/@id])) then (
												if (empty($i/var/var[@id=$ev/@id])) then (0)
												else string($i/var/var[@id=$ev/@id])
											) else (
												string($i/var[@id=$ev/@id])
											)
										)
								}</expression>
							),
							if (empty($w/@unit)) then ()
							else data($w/@unit)
						}</td></tr>
				))
 };

declare function local:returnVar ($i as element(), $v as element())  {

	let $iv := $i/var[@id=$v/@id]
	return if (empty($iv) and not(count($v/value))) then ()
	else (
					<tr><td style="text-align: right; width:50%;">{
						data($v/@name),
						<i18n>: </i18n>
						}</td><td id="{ data($v/@id) }">{
							if (not(count($v/value))) then (
								if (data($v/@compare_nom)="true") then (
									if (xs:double($iv) > xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id])) then (
										string("↑")
									) else if (xs:double($iv) < xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id])) then (
										string("↓")
									) else ()
								) else (),
								data($iv)
							) else (
								<expression>{
								if (data($v/@compare_nom)="true") then (
									for $ev in $v/value/ec
									return (
										if (empty($ev/@f)) then (
											if (empty($i/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
												if (empty($i/../inspection[@nominal="true"]/var/var[@id=$ev/@id])) then (0)
												else string($i/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
											)
											else (
												string($i/../inspection[@nominal="true"]/var[@id=$ev/@id])
											)
										) else (
											data($ev/@f)
										)
									)
								) else (),
								string("?"),
								for $ev in $v/value/ec
									return
										if (empty($ev/@f)) then (
											if (empty($i/var[@id=$ev/@id])) then (
												if (empty($i/var/var[@id=$ev/@id])) then (0)
												else string($i/var/var[@id=$ev/@id])
											) else (
												string($i/var[@id=$ev/@id])
											)
										) else (
											data($ev/@f)
										)
								}</expression>
							),
							if (empty($v/@unit)) then ()
							else data($v/@unit)

				}</td></tr>
	)
};

declare function local:returnAnyVar ($i as element(), $v as element())  {

	if (count($v/var)) then (
			local:returnSubVar($i, $v)
		) else (
			local:returnVar($i, $v)
		)
};

 <html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Inspection</title>
<style type="text/css">
<!--
body,td,th {
	font-family: "Lucida Grande", "Lucida Sans Unicode", verdana, lucida, sans-serif;
	font-size: small;
	color: #333333;
}
a:link {
	color: #333333;
	text-decoration: none;
}
a:visited {
	color: #333333;
	text-decoration: none;
}
a:hover {
	color: #660000;
	text-decoration: underline;
}
a:active {
	color: #333333;
	text-decoration: none;
}
-->
</style>
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
				nominal = (new Number(nominal != '' ? eval(nominal) : '0').toFixed(2).toLocaleString());
			}
			catch (e) {
				nominal = "";
			}
		}
		if (expression.match(/^[0-9+\-*/(). ]*$/)) {
			try {
				value = (new Number(expression != '' ? eval(expression) : '0').toFixed(2).toLocaleString());
			}
			catch (e) {
				// Syntax error
			}
		}
		if (value != "") {
			if (Math.min(nominal, value) == nominal && nominal != value) {
				value = "↑" + value.toLocaleString();
			} else if (nominal != value) {
				value = "↓" + value.toLocaleString();
			} else {
				value = value.toLocaleString();
			}
		}
		expressions[i].innerHTML = value;
	}
}

function splitTable() {
	var table_l = document.getElementById("table_l");
	var table_r = document.getElementById("table_r");
	var rows = table_l.getElementsByTagName("tr");
	var begin = parseInt(rows.length / 2);
	if (rows.length % 2 == 1) { begin++; }
	for (var i = begin; i < rows.length;) {
		table_r.appendChild(rows[i]);
	}
}

function showWarnings() {
	var tbody = document.getElementById("main_table_body");
	var tds = tbody.getElementsByTagName("td");
	var warnings = document.getElementsByTagName("warnings");
	var poruchy_elem = document.getElementById("poruchy_element");
	var poruchy = new Array();
	for (var w = 0; w < warnings.length; w++) {
		var warning = warnings[w].getElementsByTagName("warning");
		for (var a = 0; a < warning.length; a++) {
			var warn = true; var found = false;
			var w_vars = warning[a].getElementsByTagName("var");
			for (var v = 0; v < w_vars.length; v++) {
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
			}
			if (found == false) { warn = false; }
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

%1
-->
</script>
</head>
<body onLoad="splitTable(); evaluateExpressions(); showWarnings(); translate();">

<table cellspacing="0" style="width:100%;">
{
let $d := doc($inputDocument)

let $i := $d/leaklog/customers/customer[@id="%2"]/circuit[@id="%3"]/inspection[@date="%4"]
let $vars := $d/leaklog/variables
return (
	<tr><td><table cellspacing="0" cellpadding="4" style="width:100%;">
	<tbody id="main_table_body">
	<tr style="background-color: #eee;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b><i18n>Circuit: </i18n>
	<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }">{data($i/../@id)}</a></b></td></tr>
	<tr style="background-color: #DFDFDF;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b>{
			if (data($i/@nominal)="true") then <i18n>Nominal inspection: </i18n>
			else <i18n>Inspection: </i18n>
		}<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }/inspection:{ data($i/@date) }/modify">
	{data($i/@date)}</a></b></td></tr>
	<tr><td><table id="table_l" cellspacing="0" cellpadding="4" style="width:100%;">{
	for $v in $vars/var
	return (
		local:returnAnyVar($i, $v)
	)
	}</table></td>
	<td style="width:50%;"><table id="table_r" cellspacing="0" cellpadding="4" style="width:100%;">
	</table></td></tr>
	</tbody>
	</table></td></tr>,<tr><td>
	<table cellspacing="0" cellpadding="4" style="width:100%;" id="poruchy_element">
	<tr><td colspan="2" style="font-size: larger; width:100%;">
	<b><i18n>Warnings</i18n></b></td></tr>
	{$d/leaklog/warnings}
	</table></td></tr>
)
}
</table>
</body>
</html>
