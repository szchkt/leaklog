declare variable $inputDocument external;
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
			if (nominal == "") { nominal = array[1]; }
			expression = array[1];
		}
		if (nominal.match(/^[0-9+\-*/(). ]*$/)) {
			try {
				nominal = nominal != '' ? eval(nominal) : '0';
			}
			catch (e) {
				nominal = "";
			}
		}
		if (expression.match(/^[0-9+\-*/(). ]*$/)) {
			try {
				value = expression != '' ? eval(expression) : '0';
			}
			catch (e) {
				// Syntax error
			}
		}
		if (value != "") {
			if (nominal < value) {
				value = "↑ " + value;
			} else if (nominal > value) {
				value = "↓ " + value;
			}
		}
		expressions[i].innerHTML = value + " ";
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

%1
-->
</script>
</head>
<body onLoad="splitTable(); evaluateExpressions(); translate();">
<table cellspacing="0" cellpadding="4" style="width:100%;">

{
let $d := doc($inputDocument)

let $i := $d/leaklog/customers/customer[@id="%2"]/circuit[@id="%3"]/inspection[@date="%4"]
let $vars := $d/leaklog/variables
return (
	<tr style="background-color: #eee;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b><i18n>Circuit: </i18n>
	<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }">{data($i/../@id)}</a></b></td></tr>,
	<tr style="background-color: #DFDFDF;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b>{
			if (data($i/@nominal)="true") then <i18n>Nominal inspection: </i18n>
			else <i18n>Inspection: </i18n>
		}<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }/inspection:{ data($i/@date) }/modify">
	{data($i/@date)}</a></b></td></tr>,
	<tr><td><table id="table_l" cellspacing="0" cellpadding="4" style="width:100%;">{
	for $v in $i/var
	return (
		if (count($v/var)) then (
				for $w in $v/var
				return <tr><td style="text-align: right; width:50%;">{
						if (empty(data($vars/var[@id=$v/@id]/@name))) then (
							data($v/@id)
						) else ( data($vars/var[@id=$v/@id]/@name) ),
						<i18n>: </i18n>,
						if (empty(data($vars/var[@id=$v/@id]/var[@id=$w/@id]/@name))) then (
							data($w/@id)
						) else ( data($vars/var[@id=$v/@id]/var[@id=$w/@id]/@name) ),
						<i18n>: </i18n>
					}</td><td>{
							if (empty($vars/var[@id=$v/@id]/var[@id=$w/@id]/value)) then (
								if (data($d/leaklog/variables/var[@id=$v/@id]/var[@id=$w/@id]/@compare_nom="true")) then (
									if (xs:double($i/var[@id=$v/@id]/var[@id=$w/@id]) > xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id]/var[@id=$w/@id])) then (
										string("↑")
									) else if (xs:double($i/var[@id=$v/@id]/var[@id=$w/@id]) < xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id]/var[@id=$w/@id])) then (
										string("↓")
									) else ()
								) else (),
								data($i/var[@id=$v/@id]/var[@id=$w/@id])
							) else (
								<expression>{
								if (data($vars/var[@id=$v/@id]/@compare_nom="true")) then (
									for $ev in $d/leaklog/variables/var[@id=$v/@id]/var[@id=$w/@id]/value/ec
									return (
										if (empty($ev/@f)) then (
											if (empty($i/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
												string($i/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
											)
											else (
												string($i/../inspection[@nominal="true"]/var[@id=$ev/@id])
											)
										)else (
											data($ev/@f)
										)
									)
								) else (),
								string("?"),
								for $ev in $vars/var[@id=$v/@id]/var[@id=$w/@id]/value/ec
									return
										if (empty($ev/@f)) then (
											if (empty($i/var[@id=$ev/@id])) then (
												string($i/var/var[@id=$ev/@id])
											) else (
												string($i/var[@id=$ev/@id])
											)
										) else (
											data($ev/@f)
										)
								}</expression>
							),
							if (empty($vars/var[@id=$v/@id]/var[@id=$w/@id]/@unit)) then ()
							else data($vars/var[@id=$v/@id]/var[@id=$w/@id]/@unit)
						}</td></tr>
			) else (
				<tr><td style="text-align: right; width:50%;">{
						if (empty(data($vars/var[@id=$v/@id]/@name))) then (
							data($v/@id)
						) else ( data($vars/var[@id=$v/@id]/@name) ),
						<i18n>: </i18n>
						}</td><td>{
							if (empty($vars/var[@id=$v/@id]/value)) then (
								if (data($d/leaklog/variables/var[@id=$v/@id]/@compare_nom="true")) then (
									if (xs:double($i/var[@id=$v/@id]) > xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id])) then (
										string("↑")
									) else if (xs:double($i/var[@id=$v/@id]) < xs:double($i/../inspection[@nominal="true"]/var[@id=$v/@id])) then (
										string("↓")
									) else ()
								) else (),
								data($i/var[@id=$v/@id])
							) else (
								<expression>{
								if (data($vars/var[@id=$v/@id]/@compare_nom="true")) then (
									for $ev in $d/leaklog/variables/var[@id=$v/@id]/value/ec
									return (
										if (empty($ev/@f)) then (
											if (empty($i/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
												string($i/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
											)
											else (
												string($i/../inspection[@nominal="true"]/var[@id=$ev/@id])
											)
										)else (
											data($ev/@f)
										)
									)
								) else (),
								string("?"),
								for $ev in $d/leaklog/variables/var[@id=$v/@id]/value/ec
									return
										if (empty($ev/@f)) then (
											if (empty($i/var[@id=$ev/@id])) then (
												string($i/var/var[@id=$ev/@id])
											) else (
												string($i/var[@id=$ev/@id])
											)
										) else (
											data($ev/@f)
										)
								}</expression>
							),
							if (empty($vars/var[@id=$v/@id]/@unit)) then ()
							else data($vars/var[@id=$v/@id]/@unit)
				
				}</td></tr>
			)
	)
	}</table></td>
	<td style="width:50%;"><table id="table_r" cellspacing="0" cellpadding="4" style="width:100%;">
	</table></td></tr>
)
}

</table>
</body>
</html>
